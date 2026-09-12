#!/usr/bin/env python3
"""Compile English startup backgrounds into fixed-size, separate ROM sections.

Only sibling *_en.png images override a layer. Shared tile, palette-bank and
size validation remains identical to the Japanese mapped-image compiler.
Missing variants explicitly fall back to their Japanese source.
"""
import json
from pathlib import Path
import lz77
import build_assets
import gfx
import copy
import mapped_images

ROOT = Path(__file__).resolve().parents[1]
NAMES = ('T_C01.KCG', 'T_TTL06.KCG', 'SM_BG12.KCG')


def compile_entry(entry, root=ROOT):
    layout = json.loads((root / entry['image_layout']).read_text())
    overrides = {}
    for layer in layout['layers']:
        path = Path(layer['image'])
        variant = path.with_name(path.stem + '_en.png')
        if (root / variant).is_file():
            overrides[str(path)] = str(variant)
    map_data = (root / layout['map_path']).read_bytes()
    if not overrides:
        raw = mapped_images.compile_image(entry, root)
    else:
        # Retile the complete image: translated letters must not inherit shared
        # Japanese glyph tiles. Each 8x8 cell still uses one hardware palette bank.
        layout = copy.deepcopy(layout)
        tiles, indices = [], {}
        for layer in layout['layers']:
            pixels, _ = gfx.read_png(str(root / overrides.get(layer['image'], layer['image'])))
            width, height = layout['width_tiles'], layout['height_tiles']
            if len(pixels) != height*8 or any(len(row) != width*8 for row in pixels):
                raise ValueError('English background dimensions changed')
            words = []
            for y in range(0, height*8, 8):
                for x in range(0, width*8, 8):
                    cell = [row[x:x+8] for row in pixels[y:y+8]]
                    # Every 4bpp bank encodes its transparent color as local
                    # index zero. Global-index previews may represent that as
                    # 0, 16, 32, ...; none of those selects a visible bank.
                    banks = {v//16 for row in cell for v in row if v % 16}
                    if len(banks) > 1 or (banks and max(banks) >= entry['palette_banks']):
                        raise ValueError(f'{entry["archive_name"]}: tile {x//8},{y//8} crosses visible palette banks {sorted(banks)}')
                    cell_index = y//8 * width + x//8
                    bank = next(iter(banks)) if banks else (layer['entries'][cell_index] >> 12) & 15
                    tile = gfx.pixels_to_tiles([[v%16 for v in row] for row in cell], 4)
                    if tile not in indices:
                        indices[tile] = len(tiles)
                        tiles.append(tile)
                    words.append(indices[tile] | bank << 12)
            layer['entries'] = words
        raw = b''.join(tiles)
        if len(raw) > 0x4000:
            raise ValueError(f"{entry['archive_name']}: {len(tiles)} English tiles exceed one 16 KiB character block")
        raw = raw.ljust(entry['raw_size'], b'\0')
        entry = dict(entry, raw_size=len(raw))
        map_data = mapped_images.build_map(map_data, layout)
    compressed = build_assets.compress_raw(entry, raw)
    if lz77.decompress(compressed)[0] != raw:
        raise ValueError('English background compression roundtrip failed')
    return compressed, map_data, overrides



def main():
    output = ROOT / 'build/english/graphics/backgrounds'
    output.mkdir(parents=True, exist_ok=True)
    entries = {e.get('archive_name'): e for e in json.loads((ROOT/'assets.json').read_text())}
    maps = {e["name"]: e for e in json.loads((ROOT/"maps/nfp/manifest.json").read_text())}
    assembly, report = [], []
    for name in NAMES:
        entry = entries[name]
        data, map_data, overrides = compile_entry(entry)
        path = output / (name + '.lz')
        path.write_bytes(data)
        assembly.extend([f'.section .rom.{entry["rom_offset"]:08X}, "a"',
                         f'.incbin "{path.relative_to(ROOT).as_posix()}"'])
        map_entry = maps[name.replace('.KCG', '.KMP')]
        map_path = output / map_entry['name']
        if len(map_data) != map_entry['size']:
            raise ValueError('English map allocation changed')
        map_path.write_bytes(map_data)
        assembly.extend([f'.section .rom.{map_entry["rom_offset"]:08X}, "a"',
                         f'.incbin "{map_path.relative_to(ROOT).as_posix()}"'])
        report.append(dict(map_offset=map_entry['rom_offset'], map_size=len(map_data), name=name, rom_offset=entry['rom_offset'], size=len(data),
                           overrides=overrides, status='english' if overrides else 'japanese_fallback'))
    (ROOT/'build/english/background_graphics.s').write_text('\n'.join(assembly)+'\n')
    (ROOT/'build/english/background_graphics.json').write_text(json.dumps(report, indent=2)+'\n')
    for row in report:
        print(row['name'] + ': ' + row['status'])


if __name__ == '__main__':
    main()
