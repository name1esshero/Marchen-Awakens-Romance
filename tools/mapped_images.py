#!/usr/bin/env python3
"""Editable KMP-backed 4bpp images; compile pixels back into shared tile order.

08002650 selects a u16 plane at KMP+0x9C+4*layer and reads dimensions at
+0x14/+0x18. 08003178 loads tile/palette names at +0x1C/+0x5C and palette
base/count at +0xB0/+0xB4. Unknown KMP fields remain in the map source blob.
"""
import asset_safety
import argparse
import hashlib
import json
from pathlib import Path
import struct
import gfx
import lz77

ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return json.loads(path.read_text())


def digest(tile):
    return hashlib.sha256(bytes(v for row in tile for v in row)).hexdigest()


def tiles_from_bytes(raw, bpp=4):
    size = 8 * bpp
    return [gfx.tiles_to_pixels(raw[i:i+size], bpp, 1) for i in range(0, len(raw), size)]


def render(tiles, words, width, height, bpp=4):
    pixels = [[0] * (width * 8) for _ in range(height * 8)]
    for cell, word in enumerate(words):
        tile = tiles[word & 1023]
        bank = (word >> 12) * 16 if bpp == 4 else 0
        x, y = cell % width * 8, cell // width * 8
        for dy in range(8):
            for dx in range(8):
                pixels[y+dy][x+dx] = bank + tile[7-dy if word & 0x800 else dy][7-dx if word & 0x400 else dx]
    return pixels


def compile_image(entry, root=ROOT):
    """No ROM or original compressed stream is read to recover tile pixels."""
    layout = read(root / entry['image_layout'])
    bpp = entry['bpp']
    size = 8 * bpp
    count = entry['raw_size'] // size
    if bpp not in (4, 8) or entry['raw_size'] % size or layout['tile_count'] != count:
        raise ValueError('Unsupported mapped tile format')
    tiles = [None] * count
    changed = {}
    width, height = layout['width_tiles'], layout['height_tiles']
    for layer in layout['layers']:
        pixels, _ = gfx.read_png(str(root / layer['image']))
        if len(pixels) != height*8 or any(len(row) != width*8 for row in pixels):
            raise ValueError(layer['image'] + ': image dimensions differ from KMP layout')
        if len(layer['entries']) != width*height:
            raise ValueError('Map plane has wrong number of entries')
        for cell, word in enumerate(layer['entries']):
            if not isinstance(word, int) or not 0 <= word <= (255 if bpp == 8 else 0xFFFF):
                raise ValueError('Map entry must be a u16')
            index, bank = word & 1023, (word >> 12) * 16 if bpp == 4 else 0
            if index >= count or bank >= (entry.get('palette_bank_base',0) + entry['palette_banks']) * 16:
                raise ValueError('Map tile/palette reference outside its source')
            x, y = cell % width * 8, cell // width * 8
            tile = [[0]*8 for _ in range(8)]
            for dy in range(8):
                for dx in range(8):
                    value = pixels[y+dy][x+dx] - bank
                    if not 0 <= value < (1 << bpp):
                        raise ValueError(f"{layer['image']}: pixel ({x+dx},{y+dy}) outside palette bank {bank//16}")
                    tile[7-dy if word & 0x800 else dy][7-dx if word & 0x400 else dx] = value
            if bpp == 4 and bank < entry.get('palette_bank_base',0)*16 and any(v for row in tile for v in row):
                raise ValueError('Nontransparent tile uses a palette bank outside this resource')
            if digest(tile) != layout['baseline_tiles'][index]:
                if index in changed and changed[index] != tile:
                    raise ValueError(f'Conflicting edits to shared tile {index}')
                changed[index] = tile
            else:
                if tiles[index] is not None and tiles[index] != tile:
                    raise ValueError('Inconsistent unchanged tile copies')
                tiles[index] = tile
    unused = layout['unused_tiles']
    if unused:
        pixels, _ = gfx.read_png(str(root / entry['unused_tiles_image']))
        if len(pixels) != len(unused)*8 or any(len(row) != 8 for row in pixels):
            raise ValueError('Unused-tile strip dimensions changed')
        for position, index in enumerate(unused):
            if not 0 <= index < count or tiles[index] is not None:
                raise ValueError('Unused tile overlaps a mapped tile')
            tile = pixels[position*8:position*8+8]
            if any(not 0 <= v < (1 << bpp) for row in tile for v in row):
                raise ValueError('Unused tile exceeds pixel depth')
            tiles[index] = tile
    for index, tile in changed.items():
        tiles[index] = tile
    if any(tile is None for tile in tiles):
        raise ValueError('Mapped sources do not cover every tile')
    return b''.join(gfx.pixels_to_tiles(tile, bpp) for tile in tiles)


def build_map(original, layout):
    """Replace decoded map planes; retain explicitly undecoded header/attributes."""
    width, height = struct.unpack_from('<2I', original, 0x14)
    if (width, height) != (layout['width_tiles'], layout['height_tiles']):
        raise ValueError('Changing KMP dimensions requires a separate layout migration')
    result = bytearray(original)
    for layer in layout['layers']:
        index = layer['index']
        if not 0 <= index < 4:
            raise ValueError('KMP plane index outside 0..3')
        offset = struct.unpack_from('<I', original, 0x9C + index*4)[0]
        if offset != layer['offset'] or offset < 0xC0 or offset+width*height*2 > len(original):
            raise ValueError('KMP plane span differs from source')
        if len(layer['entries']) != width*height:
            raise ValueError('Wrong KMP plane length')
        struct.pack_into('<'+'H'*(width*height), result, offset, *layer['entries'])
    return bytes(result)


def extract():
    manifest = read(ROOT / 'assets.json')
    rom = (ROOT / 'baserom.gba').read_bytes()  # Extraction only; never a build input.
    converted, skipped = [], []
    for entry in manifest:
        if asset_safety.has_english_art(ROOT/(entry['path']+'.png')):
            continue  # Do not migrate paths with authored English companions.
        if entry.get('kind') == 'mapped_image':
            continue
        name = entry.get('archive_name', '')
        if not name:
            continue
        try:
            if entry.get('bpp') != 4:
                raise ValueError('8bpp map/palette interpretation needs a separate profile')
            map_path = 'maps/nfp/' + name[:-3] + 'KMP.bin'
            source = ROOT / map_path
            if not source.exists():
                raise ValueError('No same-name KMP; may use TSC or another map')
            blob = source.read_bytes()
            width, height = struct.unpack_from('<2I', blob, 0x14)
            if not (width > 0 and height > 0 and width*height <= (len(blob)-0xC0)//2):
                raise ValueError('Unsupported map geometry')
            if blob[0x1C:0x5C].split(b'\0')[0].decode('ascii') != name:
                raise ValueError('KMP names another tile resource')
            if blob[0x5C:0x9C].split(b'\0')[0].decode('ascii') != name[:-1]+'L':
                raise ValueError('KMP names another palette resource')
            raw = lz77.decompress(rom, entry['rom_offset'])[0]
            tiles = tiles_from_bytes(raw)
            layers = []
            used = set()
            for index, offset in enumerate(struct.unpack_from('<4I', blob, 0x9C)):
                if not offset:
                    continue
                if offset < 0xC0 or offset + width*height*2 > len(blob):
                    raise ValueError('Unsupported plane bounds')
                words = list(struct.unpack_from('<'+'H'*(width*height), blob, offset))
                # A zero-filled auxiliary plane has no visible pixels when tile zero is blank.
                if index and not any(words) and not any(v for row in tiles[0] for v in row):
                    continue
                if max(v & 1023 for v in words) >= len(tiles):
                    raise ValueError('Map references tiles outside this resource')
                if max(v >> 12 for v in words) >= entry['palette_banks']:
                    raise ValueError('Map references palette banks outside this source')
                used.update(v & 1023 for v in words)
                layers.append(dict(index=index, offset=offset, entries=words))
            if not layers or layers[0]['index'] != 0:
                raise ValueError('No supported primary map plane')
        except (ValueError, UnicodeError, struct.error) as exc:
            skipped.append(dict(name=name, reason=str(exc)))
            continue
        old = entry['path']
        pixels, _ = gfx.read_png(str(ROOT / (old+'.png')))
        packed = gfx.pixels_to_tiles(pixels, 4)
        if packed[:len(raw)] != raw or any(packed[len(raw):]):
            raise ValueError('Preserve existing tile edits before migration: '+old)
        base = 'graphics/backgrounds/' + name
        if (ROOT / (base+'.png')).exists():
            raise ValueError('Refusing to overwrite an existing mapped image')
        colors = gfx.read_jasc(str(ROOT / entry['palette_path']))
        for layer in layers:
            layer['image'] = base + ('' if layer['index'] == 0 else '.layer'+str(layer['index'])) + '.png'
            gfx.write_png(str(ROOT / layer['image']), render(tiles, layer['entries'], width, height), colors)
        unused = sorted(set(range(len(tiles))) - used)
        entry.update(kind='mapped_image', path=base, image_layout=base+'.json')
        if unused:
            entry['unused_tiles_image'] = 'graphics/backgrounds/unused/' + name + '.png'
            dest = ROOT / entry['unused_tiles_image'];dest.parent.mkdir(parents=True, exist_ok=True)
            gfx.write_png(str(dest), [row for i in unused for row in tiles[i]], colors[:16])
        layout = dict(map_path=map_path, width_tiles=width, height_tiles=height,
                      tile_count=len(tiles), layers=layers, unused_tiles=unused,
                      baseline_tiles=[digest(t) for t in tiles])
        (ROOT / entry['image_layout']).write_text(json.dumps(layout, indent=2)+'\n')
        if compile_image(entry) != raw or build_map(blob, layout) != blob:
            raise ValueError('Mapped source migration did not round-trip: '+name)
        asset_safety.unlink_generated(ROOT / (old+'.png'))
        converted.append(dict(name=name, old_path=old, new_path=base, width=width*8,
                              height=height*8, layers=len(layers), unused_tiles=len(unused)))
        print(name, width*8, height*8, len(layers), 'layers', flush=True)
    (ROOT / 'assets.json').write_text(json.dumps(manifest, indent=2)+'\n')
    (ROOT / 'reports/graphics/mapped-image-migration.json').write_text(json.dumps(dict(converted=converted, skipped=skipped), indent=2)+'\n')
    for path in (ROOT / 'asm/data').glob('*.s'):
        text = path.read_text();out = text
        for entry in converted:
            out = out.replace(entry['old_path']+'.lz', entry['new_path']+'.lz')
        if out != text:
            path.write_text(out)
    print(len(converted), 'converted;', len(skipped), 'require other profiles')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['extract', 'raw', 'preview', 'dependencies'])
    parser.add_argument('--asset')
    parser.add_argument('--output')
    args = parser.parse_args()
    if args.command == 'extract':
        extract()
    elif args.command == 'dependencies':
        paths = []
        for e in read(ROOT/'assets.json'):
            if e['kind'] != 'mapped_image':
                continue
            paths.extend(layer['image'] for layer in read(ROOT/e['image_layout'])['layers'])
        print(' '.join(paths))
    else:
        entry = next(e for e in read(ROOT/'assets.json') if e.get('archive_name') == args.asset)
        out = Path(args.output);out.parent.mkdir(parents=True, exist_ok=True)
        raw = compile_image(entry)
        if args.command == 'preview':
            layout = read(ROOT/entry['image_layout'])
            layer = layout['layers'][0]
            pixels = render(tiles_from_bytes(raw, entry['bpp']), layer['entries'], layout['width_tiles'], layout['height_tiles'], entry['bpp'])
            gfx.write_png(str(out), pixels, [(0,0,0)]*entry.get('palette_base',0) + gfx.read_jasc(str(ROOT/entry['palette_path'])),
                          transparent_indices=range(0,256,16) if layout.get('format')=='regular_tsc_u16' else None)
        else:
            out.write_bytes(raw)


if __name__ == '__main__':
    main()
