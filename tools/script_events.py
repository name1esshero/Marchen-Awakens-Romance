"""Decode FUNC relocations and literal native-call arguments; never guess opcodes.

A FUNC entry is a NUL-terminated ASCII name followed by u32 CODE offsets,
terminated by zero. Each reference targets the argument-count push (29/u32),
followed by native call 80/u32. Only statement-local contiguous literal pushes are editable.
The sequence must follow the observed 31 / 0F-u32 statement prefix; other
expressions remain read-only even when their final bytes resemble pushes.
"""
import hashlib
import json
from pathlib import Path
import struct
import lz77
from extract_scrp_text import strings_in


FIXED_WIDTHS = {
    0x11:5,0x12:6,0x13:5,0x14:1,0x20:3,0x21:6,0x22:6,
    0x28:2,0x29:5,0x2A:2,0x30:3,0x31:6,0x32:3,0x33:6,
    0x34:3,0x35:6,0x36:3,0x37:6,0x38:3,0x39:6,0x3A:2,
    0x3B:2,0x3C:2,0x40:3,0x41:6,0x42:3,0x43:6,0x44:3,
    0x45:6,0x46:2,0x50:3,0x51:6,0x52:3,0x53:6,0x54:2,
    0x58:2,0x59:2,0x5A:2,0x5B:2,0x5C:2,0x5D:2,0x60:6,
    0x61:2,0x62:1,0x63:1,0x64:1,0x70:4,0x71:4,0x72:3,
    0x73:3,0x78:3,0x79:2,0x80:5,0x8F:1,
}


def _decode_statement(code, start, ref, strings):
    """Symbolically execute one compiler statement through its outer argc."""
    cursor=start; registers={}; stack=[]; pending_call=False
    while cursor < ref:
        opcode=code[cursor]
        if opcode==0x10:
            if cursor+3>ref:return None
            width=3+struct.unpack_from('<H',code,cursor+1)[0]
        elif opcode==0x15:
            if cursor+3>ref:return None
            width=3+code[cursor+2]*8
        else:
            width=FIXED_WIDTHS.get(opcode)
        if width is None or cursor+width>ref:return None
        if opcode==0x21:
            registers[code[cursor+1]]=dict(kind='integer',offset=cursor+2,
                value=struct.unpack_from('<i',code,cursor+2)[0])
        elif opcode==0x22:
            register=code[cursor+1];target=struct.unpack_from('<I',code,cursor+2)[0]
            raw=strings.get(target+6)
            if raw is None:registers[register]=dict(kind='address',register=register,address=target)
            else:
                try:value=raw.decode('ascii')
                except UnicodeDecodeError:value=raw.hex()
                registers[register]=dict(kind='string',register=register,value=value,address=target)
        elif opcode==0x20:
            registers[code[cursor+1]]=registers.get(code[cursor+2],dict(kind='operand',register=code[cursor+2]))
        elif opcode in (0x70,0x71,0x72,0x73):
            destination=code[cursor+3] if opcode in (0x70,0x71) else code[cursor+2]
            registers[destination]=dict(kind='operand',register=destination)
        elif opcode in (0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
                        0x3A,0x3B,0x3C,0x40,0x41,0x42,0x43,0x44,0x45,0x46,
                        0x50,0x51,0x52,0x53,0x54,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x78):
            registers[code[cursor+1]]=dict(kind='operand',register=code[cursor+1])
        elif opcode==0x28:
            operand=code[cursor+1];register=operand&0x7f
            value=registers.get(register)
            if value is None and not operand&0x80:
                # The compiler commonly emits skipped string data and its
                # address load before the outer stack-allocation instruction.
                pos=code.rfind(bytes((0x22,register)),0,cursor)
                if pos>=0 and pos+6<=cursor:
                    target=struct.unpack_from('<I',code,pos+2)[0];raw=strings.get(target+6)
                    if raw is not None:
                        try:text=raw.decode('ascii')
                        except UnicodeDecodeError:text=raw.hex()
                        value=dict(kind='string',register=register,value=text,address=target)
            if value is None:value=dict(kind='operand',register=operand)
            # An indirect operand is a runtime value even if its pointer register
            # was initialized from a literal earlier in the statement.
            stack.append(dict(kind='operand',register=operand) if operand&0x80 else dict(value))
        elif opcode==0x29:
            stack.append(dict(kind='integer',offset=cursor+1,
                              value=struct.unpack_from('<i',code,cursor+1)[0]))
        elif opcode==0x80:
            if not stack or stack[-1]['kind']!='integer':return None
            argc=stack.pop()['value']
            if not 0<=argc<=len(stack):return None
            del stack[len(stack)-argc:]
            pending_call=True
        elif opcode==0x2A:
            registers[code[cursor+1]]=dict(kind='operand',register=code[cursor+1])
            pending_call=False
        cursor+=width
    return stack if cursor==ref and not pending_call else None


def _contiguous_arguments(code,ref,count,strings):
    """Recognize the simpler all-literal suffix used by many old scripts."""
    cursor=ref;values=[]
    for _ in range(count):
        if cursor>=5 and code[cursor-5]==0x29:
            values.append(dict(kind='integer',offset=cursor-4,
                               value=struct.unpack_from('<i',code,cursor-4)[0]));cursor-=5
        elif cursor>=2 and code[cursor-2]==0x28:
            operand=code[cursor-1];value=dict(kind='operand',register=operand)
            if not operand&0x80:
                pos=code.rfind(bytes((0x22,operand)),0,cursor)
                if pos>=0:
                    target=struct.unpack_from('<I',code,pos+2)[0];raw=strings.get(target+6)
                    if raw is not None:
                        try:text=raw.decode('ascii')
                        except UnicodeDecodeError:text=raw.hex()
                        value=dict(kind='string',register=operand,value=text,address=target)
            values.append(value);cursor-=2
        else:return [],False
    values.reverse()
    prefix=(cursor>=6 and code[cursor-6:cursor-4]==b'\x31\x0f'
            and struct.unpack_from('<I',code,cursor-4)[0]==count*4)
    return values,prefix


def statement_arguments(code, ref, count, strings):
    """Decode literal, embedded-string and dynamic native-call arguments."""
    signature=b'\x31\x0f'+struct.pack('<I',count*4)
    candidates=[];at=code.rfind(signature,0,ref)
    while at>=0 and len(candidates)<64:
        candidates.append(at);at=code.rfind(signature,0,at)
    for at in candidates:
        values=_decode_statement(code,at+6,ref,strings)
        if values is not None and len(values)==count:return values,True
    return _contiguous_arguments(code,ref,count,strings)


def unpack(blob):
    return lz77.decompress(blob)[0] if blob[0]==16 else blob


def calls(blob):
    raw=unpack(blob)
    if raw[:4]!=b'SCRP' or raw[8:12]!=b'CODE':raise ValueError('Expected SCRP/CODE')
    size=struct.unpack_from('<I',raw,12)[0]
    code=raw[16:16+size]
    if len(code)!=size:raise ValueError('Truncated CODE')
    string_values=dict(strings_in(code,lossless=True))
    string_spans=[(offset,offset+3+len(value)+1) for offset,value in string_values.items()]
    strings_by_payload={offset+3:value for offset,value in string_values.items()}
    cursor=16+size;result=[]
    while cursor+4<=len(raw):
        tag=raw[cursor:cursor+4]
        if tag==b'TERM':break
        if cursor+8>len(raw):raise ValueError('Truncated chunk')
        length=struct.unpack_from('<I',raw,cursor+4)[0]
        # The observed NVAR record includes its eight-byte chunk header in
        # the length. FUNC and CODE lengths count payload bytes instead.
        if tag==b'NVAR':length-=8
        if length<0:raise ValueError('Invalid chunk length')
        payload=raw[cursor+8:cursor+8+length]
        if len(payload)!=length:raise ValueError('Truncated chunk payload')
        cursor+=8+length
        if tag!=b'FUNC':
            if tag!=b'NVAR':raise ValueError('Unsupported script chunk '+repr(tag))
            continue
        pos=0
        while pos<len(payload) and payload[pos]:
            end=payload.index(0,pos);name=payload[pos:end].decode('ascii');pos=end+1
            while True:
                ref=struct.unpack_from('<I',payload,pos)[0];pos+=4
                if ref==0:break
                if ref+10>len(code) or code[ref]!=0x29 or code[ref+5]!=0x80:
                    raise ValueError(f'{name}: FUNC reference does not target literal argc/native call')
                count=struct.unpack_from('<I',code,ref+1)[0]
                decoded,prefix=statement_arguments(code,ref,count,strings_by_payload)
                integer_args=[a for a in decoded if a['kind']=='integer']
                start=min((a['offset']-1 for a in integer_args),default=ref)
                editable=(prefix and 0<count<=64 and len(integer_args)==count
                          and not any(start<end and begin<ref for begin,end in string_spans))
                result.append(dict(function=name,offset=ref,count=count,editable=editable,
                                   arguments=integer_args if editable else [],
                                   decoded_arguments=decoded))
    return sorted(result,key=lambda c:c['offset'])


def semantic_summary(decoded_calls):
    """Project verified native-call arguments into map-editor concepts.

    These are static call sites, not a claim that every branch executes. Dynamic
    operands stay explicit so consumers cannot mistake them for literal IDs or
    coordinates.
    """
    result=dict(field_loads=[],sprite_resources=[],sprite_properties=[],sprite_moves=[])
    for call in decoded_calls:
        args=call.get('decoded_arguments',[])
        if not args:continue
        values=[a.get('value') if a['kind'] in ('integer','string') else None for a in args]
        base=dict(offset=call['offset'],arguments=args)
        if call['function']=='FldSet' and len(args)==3:
            result['field_loads'].append(dict(base,destination=values[0],x=values[1],y=values[2]))
        elif call['function'] in ('SprInit','SprChg') and len(args)==5:
            result['sprite_resources'].append(dict(base,operation=call['function'],sprite=values[0],
                container=values[1],resource=values[2],animation=values[3],extra=values[4]))
        elif call['function']=='SprSet' and len(args)==3:
            result['sprite_properties'].append(dict(base,sprite=values[0],property=values[1],value=values[2]))
        elif call['function']=='SprMove':
            result['sprite_moves'].append(dict(base,values=values))
    return result


def patch_path(name):
    if Path(name).name!=name or not name.endswith('.SPC'):raise ValueError('Invalid script name')
    return Path('maps/events')/(name+'.json')


def apply(blob, original, document):
    if not isinstance(document,dict) or not isinstance(document.get('arguments',{}),dict):
        raise ValueError('Invalid event document')
    if document.get('version')!=1 or document.get('source_sha256')!=hashlib.sha256(original).hexdigest():
        raise ValueError('Event source revision differs')
    allowed={arg['offset'] for call in calls(original) if call['editable'] for arg in call['arguments']}
    raw=bytearray(unpack(blob));before=bytes(raw)
    for key,value in document.get('arguments',{}).items():
        offset=int(key)
        if str(offset)!=key:raise ValueError('Argument offsets must be canonical decimal strings')
        if offset not in allowed or type(value)!=int or not -0x80000000<=value<=0x7FFFFFFF:
            raise ValueError('Event edit must target a verified signed integer argument')
        struct.pack_into('<i',raw,16+offset,value)
    if raw==before:return blob
    if original[0]!=16:
        if len(raw)!=len(original):raise ValueError('Script size changed')
        return bytes(raw)
    packed=lz77.compress(bytes(raw));_,used=lz77.decompress(packed);packed=packed[:used]
    if len(packed)>len(original):raise ValueError('Edited events exceed the script allocation; changes not saved')
    return packed+original[len(packed):]


def build(blob,original,name,root):
    path=Path(root)/patch_path(name)
    return apply(blob,original,json.loads(path.read_text())) if path.exists() else blob
