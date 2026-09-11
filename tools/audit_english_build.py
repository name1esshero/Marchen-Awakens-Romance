#!/usr/bin/env python3
"""Verify the optional English ROM bridge, expansion symbols and change scope.

This is a static link audit. C lookup tests cover behavior; neither replaces
an emulator playthrough of translated scenes and other text printer paths.
"""
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def main():
    base=(ROOT/'baserom.gba').read_bytes()
    localized=(ROOT/'mar_english.gba').read_bytes()
    if len(localized)!=0x2000000:raise ValueError('English ROM must be 32 MB')
    differences=[i for i,(a,b) in enumerate(zip(base,localized)) if a!=b]
    if not differences or any(not 0x11790<=i<0x11870 for i in differences):
        raise ValueError('English build changed bytes outside the constructor bridge')
    output=subprocess.check_output(['arm-none-eabi-nm','-n','build/english/mar_english.elf'],cwd=ROOT,text=True)
    symbols={fields[2]:int(fields[0],16) for line in output.splitlines() if len(fields:=line.split())==3}
    names=('EnglishDialogueStart','DialogueStartOriginal','EnglishTranslateRows','gEnglishRows','gEnglishRowCount')
    for name in names:
        if not 0x09000000<=symbols[name]<0x0A000000:raise ValueError(name+' not in English extension')
    bridge=localized[0x11790:0x11870]
    target=symbols['EnglishDialogueStart']|1
    if struct.pack('<I',target) not in bridge:raise ValueError('Bridge lacks Thumb entry pointer')
    mappings=json.loads((ROOT/'reports/text/english-runtime.json').read_text())
    report=dict(sha1=hashlib.sha1(localized).hexdigest(),size=len(localized),
                original_area_differing_bytes=len(differences),
                change_scope='011790..011870 constructor bridge only; C and strings in ROM expansion',
                entry_points={name:f'{symbols[name]:08X}' for name in names},
                exact_row_mappings=mappings['exact_row_mappings'],
                emulator_playthrough_verified=False,
                limitation='Static link validation plus separate host C tests; does not establish complete scene or printer coverage.')
    (ROOT/'reports/build/english-link-audit.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
