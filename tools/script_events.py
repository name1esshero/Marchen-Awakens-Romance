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


def unpack(blob):
    return lz77.decompress(blob)[0] if blob[0]==16 else blob


def calls(blob):
    raw=unpack(blob)
    if raw[:4]!=b'SCRP' or raw[8:12]!=b'CODE':raise ValueError('Expected SCRP/CODE')
    size=struct.unpack_from('<I',raw,12)[0]
    code=raw[16:16+size]
    if len(code)!=size:raise ValueError('Truncated CODE')
    string_spans=[(offset,offset+3+len(value)+1) for offset,value in strings_in(code,lossless=True)]
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
                start=ref-count*5
                editable=(start>=6 and 0<count<=64 and code[start-6:start-4]==b"\x31\x0f"
                          and all(code[start+i*5]==0x29 for i in range(count))
                          and not any(start<end and begin<ref for begin,end in string_spans))
                args=[dict(offset=start+i*5+1,value=struct.unpack_from('<i',code,start+i*5+1)[0]) for i in range(count)] if editable else []
                result.append(dict(function=name,offset=ref,count=count,editable=editable,arguments=args))
    return sorted(result,key=lambda c:c['offset'])


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
