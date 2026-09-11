#!/usr/bin/env python3
"""Extract/build the named KCL/TCL BGR555 palette members losslessly."""
import argparse
import json
from pathlib import Path
import struct
import gfx
import nfp

ROOT = Path(__file__).resolve().parent.parent
DIR = ROOT / 'graphics/palettes'


def extract():
    rom = (ROOT / 'baserom.gba').read_bytes()
    DIR.mkdir(parents=True, exist_ok=True)
    records = []
    for e in nfp.entries(rom):
        if not e['name'].endswith(('.KCL', '.TCL')):
            continue
        raw = rom[e['rom_offset']:e['end']]
        if len(raw) % 32 or any(v & 0x8000 for v, in struct.iter_unpack('<H', raw)):
            raise ValueError('Unexpected palette format: ' + e['name'])
        path = 'graphics/palettes/' + e['name'] + '.pal'
        colors = gfx.read_palette(raw, count=len(raw) // 2)
        if gfx.palette_to_bytes(colors) != raw:
            raise ValueError('Palette did not round-trip: ' + e['name'])
        gfx.write_jasc(str(ROOT / path), colors)
        records.append(dict(e, path=path, color_count=len(colors)))
    (DIR / 'manifest.json').write_text(json.dumps(records, indent=2) + '\n')
    print(f'Extracted {len(records)} named palettes; all round-trip exactly')


def build():
    records = json.loads((DIR / 'manifest.json').read_text())
    for e in records:
        colors = gfx.read_jasc(str(ROOT / e['path']))
        if len(colors) != e['color_count'] or any(not 0 <= c <= 255 for rgb in colors for c in rgb):
            raise ValueError('Palette size or color range changed: ' + e['path'])
        raw = gfx.palette_to_bytes(colors)
        dest = ROOT / 'build/graphics/palettes' / (e['name'] + '.bin')
        dest.parent.mkdir(parents=True, exist_ok=True)
        if not dest.exists() or dest.read_bytes() != raw:
            dest.write_bytes(raw)


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('command', choices=['extract', 'build'])
    args = p.parse_args()
    (extract if args.command == 'extract' else build)()
