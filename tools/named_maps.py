#!/usr/bin/env python3
"""Extract and rebuild the named TSC tilemaps and KMP map members.

These were the last NFP types still reaching the ROM as anonymous data/
chunks: 90 TSC and 193 KMP files, 6,795,280 bytes between them. Everything else in the
directory already builds from a named source.

TSC is not a single screenblock format: members include 256/1024-byte
8bpp affine planes (one byte per tile), and larger regular background maps.
Only decoded profiles receive shape labels. A filename is not format proof.

KMP dimensions, tile/palette names and background planes are now partly
decoded. For mapped images, readable JSON supplies the plane words; the binary
member retains unknown fields and attributes. See mapped_images.py and kmp.h.
"""
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import nfp
import mapped_images
import affine_images
import regular_images

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / 'maps/nfp/manifest.json'
SCREENBLOCK = 2048


def tilemap_shape(blob):
    """Summarise a screenblock without inventing structure for it."""
    n = len(blob) // 2
    entries = [struct.unpack_from('<H', blob, i * 2)[0] for i in range(n)]
    return dict(entries=n,
                tiles=len(set(e & 0x3FF for e in entries)),
                max_tile=max((e & 0x3FF) for e in entries) if entries else 0,
                palettes=sorted(set(e >> 12 for e in entries)),
                flipped=sum(1 for e in entries if e & 0x0C00))


def extract():
    rom = (ROOT / 'baserom.gba').read_bytes()
    (ROOT / 'graphics/tilemaps/nfp').mkdir(parents=True, exist_ok=True)
    (ROOT / 'maps/nfp').mkdir(parents=True, exist_ok=True)

    manifest = []
    for e in nfp.entries(rom):
        name = e['name']
        if name.endswith('.TSC'):
            path = 'graphics/tilemaps/nfp/' + name + '.bin'
        elif name.endswith('.KMP'):
            path = 'maps/nfp/' + name + '.bin'
        else:
            continue
        blob = rom[e['rom_offset']:e['end']]
        (ROOT / path).write_bytes(blob)
        record = dict(name=name, path=path, rom_offset=e['rom_offset'],
                      size=len(blob))
        if name.endswith('.TSC'):
            if name.partition('.')[0].split('_')[0] in affine_images.PROFILES:
                record['format'] = 'affine_tsc_u8'
                record['shape'] = dict(entries=len(blob), max_tile=max(blob), tiles=len(set(blob)))
            else:
                record['format'] = 'regular_tsc_u16_candidate'
                record['screenblocks'] = len(blob) / SCREENBLOCK
                record['shape'] = tilemap_shape(blob)
        manifest.append(record)

    MANIFEST.write_text(json.dumps(manifest, indent=1))
    tsc = sum(1 for m in manifest if m['name'].endswith('.TSC'))
    total = sum(m['size'] for m in manifest)
    print('extracted %d members (%d TSC, %d KMP), %d bytes (%.2f MB)'
          % (len(manifest), tsc, len(manifest) - tsc, total, total / 1048576))
    return manifest


def build():
    """Stage each member into build/, refusing a size change.

    These members sit at fixed offsets inside the image, so a member that
    changed length would silently shift everything after it. The link-time
    size assertion would catch that, but failing here names the file.
    """
    manifest = json.loads(MANIFEST.read_text())
    layouts = {}
    for e in json.loads((ROOT/'assets.json').read_text()):
        if e['kind'] != 'mapped_image':continue
        layout = json.loads((ROOT/e['image_layout']).read_text())
        if layout.get('format') in ('affine_tsc_u8','regular_tsc_u16'):
            for layer in layout['layers']:
                layouts[layer['map_path']] = (layout, layer)
        else:
            layouts[layout['map_path']] = (layout, None)
    for m in manifest:
        raw = (ROOT / m['path']).read_bytes()
        if len(raw) != m['size']:
            raise ValueError('%s changed size: %d, expected %d'
                             % (m['name'], len(raw), m['size']))
        if m['path'] in layouts:
            layout, layer = layouts[m['path']]
            if layer:
                raw = (regular_images.build_plane(layout,layer) if layout['format']=='regular_tsc_u16'
                       else affine_images.build_plane(layout,layer))
            else:
                raw = mapped_images.build_map(raw, layout)
        dest = ROOT / 'build' / m['path']
        dest.parent.mkdir(parents=True, exist_ok=True)
        if not dest.exists() or dest.read_bytes() != raw:
            dest.write_bytes(raw)


def verify():
    """Every extracted member must still equal its ROM bytes."""
    rom = (ROOT / 'baserom.gba').read_bytes()
    manifest = json.loads(MANIFEST.read_text())
    bad = 0
    for m in manifest:
        blob = (ROOT / m['path']).read_bytes()
        if blob != rom[m['rom_offset']:m['rom_offset'] + m['size']]:
            print('MISMATCH', m['name'])
            bad += 1
    print('verified %d members, %d mismatches' % (len(manifest), bad))
    return bad == 0


if __name__ == '__main__':
    mode = sys.argv[1] if len(sys.argv) > 1 else 'extract'
    if mode == 'verify':
        sys.exit(0 if verify() else 1)
    elif mode == 'build':
        build()
    else:
        extract()
