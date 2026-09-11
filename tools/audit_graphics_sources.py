#!/usr/bin/env python3
"""Account for every active graphics PNG using the named source manifests.

Manifest coverage establishes a source role, not a character's identity or
the meaning of unknown metadata. Pixel/layout reconstruction is checked by
the asset builders and their ROM round-trip tests.
"""
import collections
import json
from pathlib import Path

import nfp
import sprite_sources

ROOT = Path(__file__).resolve().parents[1]


def audit():
    directory = nfp.entries((ROOT / 'baserom.gba').read_bytes())
    members = {e['rom_offset']: e for e in directory}
    expected = {}

    def add(path, role, owner):
        key = str(Path(path).relative_to(ROOT))
        previous = expected.get(key)
        value = dict(role=role, owner=owner)
        if previous is not None and previous != value:
            raise ValueError('Conflicting source roles: ' + key)
        expected[key] = value

    for entry in json.loads((ROOT / 'assets.json').read_text()):
        if not entry['path'].startswith('graphics/'):
            continue
        member = members.get(entry['rom_offset'])
        if member is None or not member['name'].endswith(('.KCG', '.TCG')):
            raise ValueError('Active compressed graphic lacks a named tile member')
        if entry['kind'] == 'mapped_image':
            if entry['path'] != 'graphics/backgrounds/' + member['name']:
                raise ValueError('Mapped background is misfiled')
            layout = json.loads((ROOT / entry['image_layout']).read_text())
            for layer in layout['layers']:
                add(ROOT / layer['image'], 'mapped_background_source', member['name'])
            if 'unused_tiles_image' in entry:
                add(ROOT / entry['unused_tiles_image'], 'unmapped_background_tiles', member['name'])
        else:
            if entry['path'] != 'graphics/tilesets/' + member['name'] or entry['kind'] != 'tilesets':
                raise ValueError('Tile source is misfiled: ' + entry['path'])
            add(ROOT / (entry['path'] + '.png'), 'named_tile_source', member['name'])
    for stem, category in sprite_sources.CATEGORIES.items():
        folder = ROOT / 'graphics' / category
        for entry in json.loads((folder / 'source/images.json').read_text()):
            add(folder / entry['path'], 'sprite_cell_source', stem + '.NCD')
        for entry in json.loads((folder / 'manifest.json').read_text())['frames']:
            add(folder / entry['path'], 'sprite_frame_editing_view', stem + '.NCD')
    for category in ('icons', 'portraits'):
        for entry in json.loads((ROOT / 'graphics' / category / 'manifest.json').read_text()):
            add(ROOT / entry['path'], category + '_editing_view', 'SYSTEM.NCD')
    for path in ('graphics/icons/ICONMINI_contact.png', 'graphics/icons/ICON_contact.png',
                 'graphics/portraits/contact.png'):
        add(ROOT / path, 'sprite_contact_preview', 'SYSTEM.NCD')
    add(ROOT / 'graphics/fonts/font.png', 'font_source', 'FONT.NFT')
    add(ROOT / 'graphics/fonts/preview.png', 'font_preview', 'FONT.NFT')

    actual = {str(p.relative_to(ROOT)): p for p in (ROOT / 'graphics').rglob('*.png')}
    missing = sorted(set(expected) - set(actual))
    untracked = sorted(set(actual) - set(expected))
    invalid = []
    for name, path in actual.items():
        with path.open('rb') as stream:
            if stream.read(8) != b'\x89PNG\r\n\x1a\n':
                invalid.append(name)
    report = dict(active_pngs=len(actual), roles=dict(collections.Counter(e['role'] for e in expected.values())),
                  missing=missing, untracked=untracked, invalid_png_signatures=invalid,
                  sources=expected,
                  limitation='Source-role coverage does not prove all game assets are discovered or all metadata decoded.')
    output = ROOT / 'reports/graphics/source-coverage.json'
    output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'sources'}, indent=2))
    if missing or untracked or invalid:
        raise ValueError('Graphics source coverage has unresolved entries')
    return report


if __name__ == '__main__':
    audit()
