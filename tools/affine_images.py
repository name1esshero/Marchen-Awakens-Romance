#!/usr/bin/env python3
"""Migrate five 8bpp TCG resources to editable affine TSC image frames.

NE01's loader at 080411A4 sets BG2CNT=188A (128x128 affine BG), copies
NE01.TSC to BG2 and 96 palette bytes to color 192. These TSC maps use ONE
byte per tile, with no flip/palette bits. Other listed resources have the
same byte-index format, with 256/1024-byte square planes (128/256 pixels).
Their dimensions are inferred from complete plane sizes and tile references.
"""
import asset_safety
import json
from pathlib import Path
import gfx
import lz77
import mapped_images as mi
import rom_data_sections

ROOT = mi.ROOT
PROFILES = ('NE01', 'NE13', 'NE33', 'SK02', 'SK03')


def build_plane(layout, layer):
    entries = layer['entries']
    if len(entries) != layout['width_tiles'] * layout['height_tiles']:
        raise ValueError('Affine plane dimensions changed')
    if any(not isinstance(v, int) or not 0 <= v < 256 for v in entries):
        raise ValueError('Affine tile indices must be bytes; flips/banks are not supported')
    return bytes(entries)


def main():
    manifest = mi.read(ROOT/'assets.json')
    maps = mi.read(ROOT/'maps/nfp/manifest.json')
    rom = (ROOT/'baserom.gba').read_bytes()  # Extraction only.
    converted = []
    for entry in manifest:
        if asset_safety.has_english_art(ROOT/(entry['path']+'.png')):
            continue  # Do not migrate paths with authored English companions.
        name = entry.get('archive_name', '')
        stem = name.partition('.')[0]
        if stem not in PROFILES or entry['kind'] == 'mapped_image':
            continue
        candidates = [m for m in maps if m['name'] == stem+'.TSC' or m['name'].startswith(stem+'_') and m['name'].endswith('.TSC')]
        sizes = {m['size'] for m in candidates}
        if len(sizes) != 1 or next(iter(sizes)) not in (256, 1024):
            raise ValueError('Unexpected affine map shape: '+name)
        width = 16 if next(iter(sizes)) == 256 else 32
        raw = lz77.decompress(rom, entry['rom_offset'])[0]
        old_pixels, _ = gfx.read_png(str(ROOT/(entry['path']+'.png')))
        packed = gfx.pixels_to_tiles(old_pixels, 8)
        if packed[:len(raw)] != raw or any(packed[len(raw):]):
            raise ValueError('Preserve existing pixel edits before migration')
        tiles = mi.tiles_from_bytes(raw, 8)
        colors = [(0,0,0)] * entry.get('palette_base',0) + gfx.read_jasc(str(ROOT/entry['palette_path']))
        old = entry['path'];base = 'graphics/backgrounds/'+name
        layers = [];used = set()
        for i, source in enumerate(sorted(candidates, key=lambda m:m['name'])):
            words = list((ROOT/source['path']).read_bytes())
            if max(words) >= len(tiles):
                raise ValueError('Affine map references missing tiles')
            image = base + ('' if not i else '.layer'+str(i)) + '.png'
            if (ROOT/image).exists():raise ValueError('Refusing to overwrite '+image)
            gfx.write_png(str(ROOT/image), mi.render(tiles,words,width,width,8),colors)
            layers.append(dict(index=i, map_path=source['path'], entries=words, image=image))
            used.update(words)
        unused = sorted(set(range(len(tiles)))-used)
        layout = dict(format='affine_tsc_u8', width_tiles=width, height_tiles=width,
                      tile_count=len(tiles), layers=layers, unused_tiles=unused,
                      baseline_tiles=[mi.digest(t) for t in tiles])
        entry.update(kind='mapped_image', path=base, image_layout=base+'.json')
        if unused:
            entry['unused_tiles_image'] = 'graphics/backgrounds/unused/'+name+'.png'
            gfx.write_png(str(ROOT/entry['unused_tiles_image']),[row for i in unused for row in tiles[i]],colors)
        (ROOT/entry['image_layout']).write_text(json.dumps(layout,indent=2)+'\n')
        if mi.compile_image(entry) != raw:raise ValueError('Affine pixels failed round-trip')
        for layer in layers:
            if build_plane(layout,layer) != (ROOT/layer['map_path']).read_bytes():raise ValueError('Affine map failed round-trip')
        asset_safety.unlink_generated(ROOT/(old+'.png'))
        converted.append(dict(name=name, old_path=old, new_path=base, frames=len(layers), width=width*8, height=width*8))
    (ROOT/'assets.json').write_text(json.dumps(manifest,indent=2)+'\n')
    rom_data_sections.replace_sources(
        ROOT/'data/rom_data_sections.json',
        [(e['old_path']+'.lz', e['new_path']+'.lz') for e in converted])
    (ROOT/'reports/graphics/affine-image-migration.json').write_text(json.dumps(converted,indent=2)+'\n')
    print('Converted',len(converted),'resources,',sum(e['frames'] for e in converted),'affine frames')


if __name__=='__main__':main()
