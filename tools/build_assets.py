#!/usr/bin/env python3
"""Compile PNG/layout sources into deterministic, VRAM-safe LZ77 streams.

No original compressed stream or ROM is read. The encoder uses nearest-first
longest matches of distance >= 2, as verified against all 104 original images.
Edited streams must fit their reserved span; unused bytes are filled with zero.
"""
import json
from pathlib import Path

import gfx
import lz77
import mapped_images


def pack_pixels(entry, pixels):
    bpp = entry.get('bpp', 4)
    width = entry.get('width_tiles', 16) * 8
    tile_bytes = 8 * bpp
    height = ((entry['raw_size'] + tile_bytes * (width // 8) - 1) // (tile_bytes * (width // 8))) * 8
    if len(pixels) != height or any(len(row) != width for row in pixels):
        raise ValueError(entry['path'] + ': PNG dimensions changed')
    if any(not 0 <= v < (1 << bpp) for row in pixels for v in row):
        raise ValueError(entry['path'] + ': pixel index exceeds bit depth')
    packed = gfx.pixels_to_tiles(pixels, bpp)
    if any(packed[entry['raw_size']:]):
        raise ValueError(entry['path'] + ': nonzero edits in padding tiles')
    return packed[:entry['raw_size']]


def compress_raw(entry, raw):
    if len(raw) != entry['raw_size']:
        raise ValueError(entry['path'] + ': compiled tile size changed')
    result = lz77.compress(raw, vram_safe=True)
    _, consumed = lz77.decompress(result)
    result = result[:consumed]  # Alignment belongs to the surrounding archive.
    if len(result) > entry['compressed_size']:
        raise ValueError(entry['path'] + ': edited stream does not fit original ROM span')
    return result.ljust(entry['compressed_size'], b'\0')


def compile_entry(entry, root=Path('.')):
    if entry['kind'] == 'mapped_image':
        raw = mapped_images.compile_image(entry, root)
    else:
        png = root / (entry['path'] + '.png')
        if not png.is_file():
            raise FileNotFoundError('Missing editable graphics source: ' + str(png))
        pixels, _ = gfx.read_png(str(png))
        raw = pack_pixels(entry, pixels)
    return compress_raw(entry, raw)


def main():
    manifest = json.loads(Path('assets.json').read_text())
    for entry in manifest:
        output = compile_entry(entry)
        dest = Path('build') / (entry['path'] + '.lz')
        dest.parent.mkdir(parents=True, exist_ok=True)
        if not dest.exists() or dest.read_bytes() != output:
            dest.write_bytes(output)
    print(f'assets: {len(manifest)} images compiled and compressed from editable sources; no original LZ input')


if __name__ == '__main__':
    main()
