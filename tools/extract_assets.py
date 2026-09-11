#!/usr/bin/env python3
"""Extract compressed named SPC and KCG/TCG resources.

Successful decompression alone does not establish an asset type. Streams
outside the corresponding named member starts are recorded as diagnostics,
not rendered as artwork or admitted to the build manifest. Tile sources use
their archive names; their palette and bit-depth evidence is retained.
"""
import collections
import hashlib
import json
from pathlib import Path
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gfx
import lz77
import nfp

GFX_DIR = "graphics"
SCRIPT_DIR = "scripts"


def find_streams(rom, min_raw=0x40, max_raw=0x20000):
    out = []
    for off in range(0, len(rom) - 8, 4):
        if rom[off] != 0x10:
            continue
        size = rom[off + 1] | (rom[off + 2] << 8) | (rom[off + 3] << 16)
        if not (min_raw <= size <= max_raw):
            continue
        try:
            data, clen = lz77.decompress(rom, off)
        except Exception:
            continue
        if len(data) == size and clen > 4:
            out.append((off, size, clen))
    return out


def main():
    rom = Path(sys.argv[1]).read_bytes()
    directory = nfp.entries(rom)
    by_offset = {e['rom_offset']: e for e in directory}
    by_name = {e['name']: e for e in directory}
    old = {}
    if os.path.exists('assets.json'):
        old = {e['rom_offset']: e for e in json.loads(Path('assets.json').read_text())}
    streams = find_streams(rom)
    print("found %d compressed streams" % len(streams))

    manifest = []
    rejected = []
    counts = collections.Counter()
    for off, size, clen in streams:
        data, _ = lz77.decompress(rom, off)
        blob = rom[off:off + clen]

        member = by_offset.get(off)
        if member is None or not member['name'].endswith(('.SPC', '.KCG', '.TCG')):
            rejected.append(dict(rom_offset=off, raw_size=size, compressed_size=clen,
                                 owners=[e['name'] for e in nfp.owners(directory, off, off+clen)],
                                 sha256=hashlib.sha256(blob).hexdigest(),
                                 status='unverified_stream_not_a_source'))
            continue
        if data[:4] == b"SCRP" and member['name'].endswith('.SPC'):
            name = "script_%06X" % off
            os.makedirs(SCRIPT_DIR, exist_ok=True)
            base = os.path.join(SCRIPT_DIR, name)
            # Named SPC sources own the compressed member; this decoded copy
            # is only for the legacy reference-text extractor.
            Path(base + ".scrp").write_bytes(data)
            counts["scripts_owned_by_named_sources"] += 1
            continue

        if old.get(off, {}).get('kind') == 'mapped_image':
            # Keep editable assembled images and their mapping, never replace
            # them with a newly flattened tile sheet during extraction.
            entry = dict(old[off], compressed_size=clen, raw_size=size)
            manifest.append(entry)
            counts['mapped_image'] += 1
            continue

        bpp = 4
        member = by_offset.get(off)
        named_tiles = member and member['name'].endswith(('.KCG', '.TCG'))
        if named_tiles:
            bpp = old.get(off, {}).get('bpp', 8 if member['name'] in
                ('NE01.TCG', 'NE13.TCG', 'NE33.TCG', 'SK02.TCG', 'SK03.TCG') else 4)
        tile_bytes = 32 if bpp == 4 else 64
        n_tiles = len(data) // tile_bytes
        kind = "tilesets"
        # Width is not recoverable from tile data. 32 tiles gives a 256px
        # sheet, which matches how GBA character memory is usually viewed.
        # It is recorded per asset so you can correct it.
        width_tiles = 32 if n_tiles >= 256 else 16

        if not named_tiles:
            raise ValueError('Named script does not decode to SCRP')
        base = 'graphics/tilesets/' + member['name']
        width_tiles = old.get(off, {}).get('width_tiles', width_tiles)
        os.makedirs(os.path.dirname(base), exist_ok=True)


        # Named tile assets use their original KCL/TCL palette.
        steps = 16 if bpp == 4 else 256
        pal = [(i * 255 // (steps - 1),) * 3 for i in range(steps)]
        metadata = {}
        if named_tiles:
            palette = by_name[member['name'][:-1] + 'L']
            count = 16 if bpp == 4 else palette['size_with_padding'] // 2
            pal = gfx.read_palette(rom, palette['rom_offset'], count)
            metadata = dict(archive_name=member['name'], archive_member=member['name'],
                palette_path='graphics/palettes/' + palette['name'] + '.pal',
                palette_banks=palette['size_with_padding']//32)
            if bpp == 8:
                pal = [(0,0,0)] * 192 + pal
                metadata.update(palette_base=192,
                    palette_base_evidence='Nonzero indices occupy banks 12..14; placement inferred from pixel range')
        padded = data + bytes(-len(data) % (tile_bytes * width_tiles))
        px = gfx.tiles_to_pixels(padded, bpp, width_tiles)
        if px:
            gfx.write_png(base + ".png", px, pal)
            if not named_tiles:
                gfx.write_jasc(base + ".pal", pal)


        counts[kind] += 1
        manifest.append({
            "rom_offset": off, "compressed_size": clen, "raw_size": size,
            "kind": kind, "bpp": bpp, "width_tiles": width_tiles,
            "path": base.replace(os.sep, "/"),
            **metadata,
        })

    os.makedirs('reports/graphics', exist_ok=True)
    with open('reports/graphics/compression-candidates.json', 'w') as f:
        json.dump(rejected, f, indent=2)
    with open("assets.json", "w") as f:
        json.dump(manifest, f, indent=1)
    print("wrote %d assets" % len(manifest))
    for k, v in sorted(counts.items()):
        print("  %-12s %3d" % (k, v))


if __name__ == "__main__":
    main()
