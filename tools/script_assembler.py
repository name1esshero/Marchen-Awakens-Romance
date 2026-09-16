#!/usr/bin/env python3
"""Assemble readable JSON into a MAR SCRP/SPC program.

This deliberately accepts only opcodes whose handlers and operand widths are
verified.  The ``native`` convenience form emits the compiler's observed stack
setup, FUNC relocation, call, and result-pop sequence.
"""
import argparse
import json
import struct
from pathlib import Path

import lz77
import text_codec

OPS = {
    'jump': (0x11, 'target'), 'jump_if_zero': (0x12, 'operand,target'),
    'call': (0x13, 'target'), 'return': (0x14, ''),
    'copy': (0x20, 'operand,operand'), 'set_i32': (0x21, 'operand,i32'),
    'address': (0x22, 'operand,target'), 'push': (0x28, 'operand'),
    'push_i32': (0x29, 'i32'), 'pop': (0x2A, 'operand'),
    'add': (0x30, 'operand,operand'), 'add_i32': (0x31, 'operand,i32'),
    'sub': (0x32, 'operand,operand'), 'sub_i32': (0x33, 'operand,i32'),
    'mul': (0x34, 'operand,operand'), 'mul_i32': (0x35, 'operand,i32'),
    'div': (0x36, 'operand,operand'), 'div_i32': (0x37, 'operand,i32'),
    'mod': (0x38, 'operand,operand'), 'mod_i32': (0x39, 'operand,i32'),
    'increment': (0x3A, 'operand'), 'decrement': (0x3B, 'operand'),
    'negate': (0x3C, 'operand'), 'and': (0x40, 'operand,operand'),
    'and_i32': (0x41, 'operand,i32'), 'or': (0x42, 'operand,operand'),
    'or_i32': (0x43, 'operand,i32'), 'xor': (0x44, 'operand,operand'),
    'xor_i32': (0x45, 'operand,i32'), 'bit_not': (0x46, 'operand'),
    'logical_and': (0x50, 'operand,operand'),
    'logical_and_i32': (0x51, 'operand,i32'),
    'logical_or': (0x52, 'operand,operand'),
    'logical_or_i32': (0x53, 'operand,i32'),
    'logical_not': (0x54, 'operand'), 'sign_bit': (0x58, 'operand'),
    'le_zero': (0x59, 'operand'), 'gt_zero': (0x5A, 'operand'),
    'ge_zero': (0x5B, 'operand'), 'eq_zero': (0x5C, 'operand'),
    'ne_zero': (0x5D, 'operand'), 'set_callback': (0x60, 'u8,target'),
    'run_callback': (0x61, 'u8'), 'restore_frame': (0x62, ''),
    'clear_frame_flag': (0x63, ''), 'set_frame_flag': (0x64, ''),
    'read_context14': (0x70, 'u16,operand'),
    'read_context114': (0x71, 'u16,operand'),
    'read_table38': (0x72, 'u8,operand'),
    'read_table3c': (0x73, 'u8,operand'),
    'concat_strings': (0x78, 'operand,operand'),
    'free_string': (0x79, 'operand'), 'restore_result': (0x8F, ''),
    # Raw single-instruction form of the native-call dispatch opcode.
    # Normally built compositely by the `native` convenience form below
    # (which also emits the required stack-allocation/arg-push/pop
    # sequence around it) -- listed directly here only so
    # tools/marscript.py's decoder can reproduce an isolated real 0x80
    # instruction byte-for-byte when it isn't part of a recognized call
    # (see docs/marscript-language.md). Not meant for hand-authored
    # scripts: writing this opcode outside a real call's exact sequence
    # produces a well-formed but semantically broken dispatch.
    'native_call': (0x80, 'i32'),
}

def integer(value, lo, hi, label):
    if type(value) is not int or not lo <= value <= hi:
        raise ValueError(f'{label} must be an integer in {lo}..{hi}')
    return value

def encoded_size(ins):
    if 'label' in ins:return 0
    if ins.get('op') == 'string':
        # A standalone, labelable string block: opcode 0x10 + u16 length +
        # NUL-terminated Shift-JIS bytes (see text_codec.py -- most real
        # script text is Japanese, not ASCII, and a character's encoded
        # byte length is not its Python string length). Same framing the
        # native-arg string handling below already builds inline, but as
        # its own instruction so a preceding label can be resolved as its
        # address (via the `address` opcode) the same way jump/call
        # targets already work.
        return 3+len(text_codec.encode(ins['value']))+1
    if ins.get('op') == 'switch':
        # SWITCH (opcode 0x15, fully decompiled as ScriptCmdSwitch in
        # src/script_bytecode.c): 1-byte operand register, 1-byte entry
        # count, then that many (u32 candidate, u32 target) entries.
        return 3+8*len(ins['entries'])
    if ins.get('op') == 'raw':
        # Opaque bytes carried through exactly, never interpreted -- see
        # tools/marscript.py's `raw` statement.
        return len(bytes.fromhex(ins['value']))
    if ins.get('op') == 'native':
        strings=sum(3+len(text_codec.encode(a['string']))+1 for a in ins.get('args',[]) if isinstance(a,dict) and 'string' in a)
        address_ops=6*sum(isinstance(a,dict) and 'string' in a for a in ins.get('args',[]))
        # A dict arg -- {'string':...} or {'register':...} -- pushes via the
        # 2-byte `push <reg>` form; a plain int pushes via 5-byte `push_i32`.
        return strings+address_ops+6+sum(2 if isinstance(a,dict) else 5 for a in ins.get('args',[]))+5+5+2
    op=ins.get('op'); spec=OPS.get(op)
    if not spec:raise ValueError('unknown opcode '+repr(op))
    return 1+sum({'operand':1,'u8':1,'u16':2,'i32':4,'target':4}[x] for x in spec[1].split(',') if x)

def assemble(document):
    if not isinstance(document,dict) or document.get('version') != 1:
        raise ValueError('script source must have version 1')
    instructions=document.get('instructions')
    if not isinstance(instructions,list):raise ValueError('instructions must be a list')
    labels={}; body_offset=0
    for ins in instructions:
        if not isinstance(ins,dict):raise ValueError('instruction must be an object')
        if 'label' in ins:
            name=ins['label']
            if not isinstance(name,str) or not name or name in labels:raise ValueError('invalid or duplicate label')
            labels[name]=body_offset
        body_offset += encoded_size(ins)
    out=bytearray(); references={}
    def target(value):
        if isinstance(value,str):
            # An optional "+N" suffix targets N bytes past the label rather
            # than the label itself -- needed for e.g. a `string` block,
            # where a real address load points 3 bytes into its payload
            # (past the 0x10 opcode + u16 length header), not at the block's
            # own start. Labels can only mark whole-instruction boundaries
            # (body_offset only advances between instructions), so an
            # in-instruction offset has to be expressed this way instead.
            name,plus,offset=value.partition('+')
            if name not in labels:raise ValueError('unknown label '+name)
            return labels[name]+(integer(int(offset),0,0xffffffff,'target offset') if plus else 0)
        return integer(value,0,0xffffffff,'target')
    def emit_fixed(ins):
        opcode,spec=OPS[ins['op']]; out.append(opcode)
        values=ins.get('args',[])
        kinds=[x for x in spec.split(',') if x]
        if len(values)!=len(kinds):raise ValueError(f'{ins["op"]} expects {len(kinds)} operands')
        for kind,value in zip(kinds,values):
            if kind in ('operand','u8'):out.append(integer(value,0,255,kind))
            elif kind=='u16':out.extend(struct.pack('<H',integer(value,0,65535,kind)))
            elif kind=='i32':out.extend(struct.pack('<i',integer(value,-0x80000000,0x7fffffff,kind)))
            else:out.extend(struct.pack('<I',target(value)))
    for ins in instructions:
        if 'label' in ins:continue
        if ins.get('op') == 'raw':
            out.extend(bytes.fromhex(ins['value']));continue
        if ins.get('op') == 'string':
            # Shift-JIS with `<XX>`-escaped control/private bytes, the same
            # codec tools/marscript.py's decoder and extract_scrp_text.py
            # both use for this exact byte layout -- not plain ASCII, since
            # most real script text is Japanese dialogue.
            value=text_codec.encode(ins['value'])+b'\0'
            if len(value)>65535:raise ValueError('string literal too long')
            out.extend(b'\x10'+struct.pack('<H',len(value))+value);continue
        if ins.get('op') == 'switch':
            entries=ins['entries']
            out.append(0x15)
            out.append(integer(ins['operand'],0,255,'switch operand'))
            out.append(integer(len(entries),0,255,'switch entry count'))
            for candidate,dest in entries:
                out.extend(struct.pack('<i',integer(candidate,-0x80000000,0x7fffffff,'switch candidate')))
                out.extend(struct.pack('<I',target(dest)))
            continue
        if ins.get('op') == 'native_call' and 'name' in ins:
            # Reproduces a real native_call site verbatim without also
            # rebuilding its argument pushes (unlike the `native`
            # convenience form below, which builds both) -- needed because
            # tools/marscript.py's decoder can't always safely prove where
            # a call's own argument setup begins (see _find_call_spans),
            # but every native_call site is named by a FUNC relocation in
            # the real file and that relocation must never be silently
            # dropped on recompile, or the call becomes unresolved at
            # runtime. Requires the immediately preceding instruction to be
            # the matching `push_i32 <argc>` (opcode 0x29) -- exactly the
            # adjacency tools/script_events.py's own FUNC reader checks for
            # (code[ref]==0x29 and code[ref+5]==0x80) -- rather than ever
            # registering a relocation against the wrong offset.
            name=ins['name']
            if not isinstance(name,str) or not name or '\0' in name or len(name.encode('ascii'))>63:
                raise ValueError('native_call name must be 1..63 ASCII bytes')
            if len(out)<5 or out[-5]!=0x29:
                raise ValueError('native_call "name" must immediately follow a push_i32 <argc> instruction')
            reference=6+len(out)-5
            out.extend(b'\x80\0\0\0\0')
            references.setdefault(name,[]).append(reference)
            continue
        if ins.get('op') != 'native':emit_fixed(ins);continue
        name=ins.get('name'); args=ins.get('args',[]); result=ins.get('result',0)
        if not isinstance(name,str) or not name or '\0' in name or len(name.encode('ascii'))>63:
            raise ValueError('native name must be 1..63 ASCII bytes')
        string_regs=[]
        for index,arg in enumerate(args):
            if isinstance(arg,dict) and 'string' in arg:
                value=text_codec.encode(arg['string'])+b'\0'
                if len(value)>65535:raise ValueError('embedded string too long')
                payload=len(out)+3
                out.extend(b'\x10'+struct.pack('<H',len(value))+value)
                reg=integer(arg.get('register',index),0,14,'string register')
                out.extend(bytes((0x22,reg))+struct.pack('<I',payload))
                string_regs.append((index,reg))
        out.extend(bytes((0x31,15))+struct.pack('<i',len(args)*4))
        regs=dict(string_regs)
        for index,arg in enumerate(args):
            if index in regs:out.extend(bytes((0x28,regs[index])))
            elif isinstance(arg,dict) and 'register' in arg:
                # A dynamic argument: pushes a register's live value rather
                # than a literal, matching real scripts (the map editor's own
                # audit found ~31% of sprite-related native calls need state
                # from elsewhere rather than a literal initial value).
                out.extend(bytes((0x28,integer(arg['register'],0,255,'native argument register'))))
            else:out.extend(b'\x29'+struct.pack('<i',integer(arg,-0x80000000,0x7fffffff,'native argument')))
        reference=6+len(out)
        out.extend(b'\x29'+struct.pack('<i',len(args)))
        out.extend(b'\x80\0\0\0\0')
        out.extend(bytes((0x2A,integer(result,0,14,'result register'))))
        references.setdefault(name,[]).append(reference)
    stack=integer(document.get('stack_size',1024),0,0xffffffff,'stack_size')
    entry=target(document.get('entry','start' if 'start' in labels else 0))
    if entry>65535:raise ValueError('entry offset does not fit CODE header')
    code=struct.pack('<IH',stack,entry)+out
    func=bytearray()
    for name,refs in references.items():
        func.extend(name.encode('ascii')+b'\0')
        for ref in refs:func.extend(struct.pack('<I',ref))
        func.extend(b'\0\0\0\0')
    if func:
        # One more zero byte past the last name's own 4-byte ref-list
        # terminator, marking "no more names" for the outer name-reading
        # loop (tools/script_events.py's calls(): `while pos<len(payload)
        # and payload[pos]:`) -- real scripts don't strictly need it
        # (reaching the end of the payload already stops that loop just as
        # well), but it is present with zero exceptions in 331 of 334 real
        # scripts' FUNC chunks (the other 3 have no native calls at all, so
        # no FUNC chunk). Confirmed by a full-corpus survey, not a guess.
        func.extend(b'\0')
    # Whatever chunk(s) a real script had between CODE and FUNC/TERM --
    # empty for almost every real script, an opaque, undocumented NVAR
    # chunk for at least one (see tools/marscript.py's extra_chunks()).
    # Preserved byte-for-byte, never interpreted, never generated for a
    # new script (only decompile_to_source() ever sets this field).
    extra_chunks=bytes.fromhex(document.get('extra_chunks_hex',''))
    chunks=b'CODE'+struct.pack('<I',len(code))+code+extra_chunks
    if func:chunks+=b'FUNC'+struct.pack('<I',len(func))+func
    chunks+=b'TERM'
    return b'SCRP'+struct.pack('<I',len(chunks))+chunks

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source',type=Path);parser.add_argument('output',type=Path)
    parser.add_argument('--compress',action='store_true')
    args=parser.parse_args(); raw=assemble(json.loads(args.source.read_text()))
    data=lz77.compress(raw) if args.compress else raw
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(data)
    print(f'wrote {args.output}: {len(raw)} raw bytes, {len(data)} stored bytes')

if __name__=='__main__':main()
