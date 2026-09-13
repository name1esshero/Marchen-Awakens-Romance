#!/usr/bin/env python3
"""Extract the verified 128-entry script native-command registry."""
import json
import struct
from pathlib import Path

BASE = 0x1AFEA4
COUNT = 128

def extract(rom):
    result=[]
    for index in range(COUNT):
        name_address,handler=struct.unpack_from('<II',rom,BASE+index*8)
        name_offset=name_address-0x08000000
        if not 0<=name_offset<len(rom):raise ValueError('native name pointer outside ROM')
        end=rom.find(b'\0',name_offset,name_offset+64)
        if end<0:raise ValueError('unterminated native name')
        name=rom[name_offset:end].decode('ascii')
        result.append(dict(index=index,name=name,handler=f'{handler & ~1:08X}'))
    if len({e['name'] for e in result}) != COUNT:raise ValueError('duplicate native name')
    return result

def main():
    rom=Path('baserom.gba').read_bytes()
    entries=extract(rom)
    path=Path('scripts/native_commands.json')
    path.parent.mkdir(exist_ok=True);path.write_text(json.dumps(entries,indent=2)+'\n')
    print(f'wrote {path}: {len(entries)} name/handler pairs')

if __name__=='__main__':main()
