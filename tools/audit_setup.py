#!/usr/bin/env python3
"""Read-only classification audit; never relabel candidates as proven assets."""
import collections
import json
from pathlib import Path

import nfp
import sound_assets

ROOT = Path(__file__).resolve().parents[1]


def main():
    rom = (ROOT / 'baserom.gba').read_bytes()
    directory = nfp.entries(rom)
    rows = []
    groups = collections.Counter()
    for path in sorted((ROOT / 'text').glob('script_*.txt')):
        for line in path.read_text().splitlines():
            if not line.startswith('@'):
                continue
            offset = int(line.split()[0][1:], 16)
            owners = nfp.owners(directory, offset, offset + 1)
            kind = '+'.join(sorted({e['name'].rsplit('.', 1)[-1] for e in owners}))
            kind = kind or 'outside_named_archive'
            groups[kind] += 1
            rows.append(dict(file=str(path.relative_to(ROOT)), offset=f'{offset:06X}',
                             owners=[e['name'] for e in owners], classification=kind,
                             kanji_placeholder='{kanji' in line))
    songs, tones, discovered = sound_assets.discover(rom)
    manifest = json.loads((ROOT / 'sound/samples/manifest.json').read_text())
    if discovered != manifest:
        raise ValueError('Audio source manifest differs from driver-reference discovery')
    for entry in manifest:
        start = entry['rom_offset']
        if sound_assets.compile_sample(entry) != rom[start:start + entry['size']]:
            raise ValueError('Edited/nonmatching sample: ' + entry['wav'])
    analysis = json.loads((ROOT / '.analysis/layout.json').read_text())
    false_functions = [address for address in analysis['funcs']
                       if any(e['rom_offset'] <= int(address, 16) - 0x08000000
                              < e['rom_offset'] + e['size'] for e in manifest)]
    report = dict(
        archive_members=len(directory),
        archive_extensions=dict(collections.Counter(e['name'].rsplit('.', 1)[-1]
                                                   for e in directory)),
        raw_text_counts=dict(groups), raw_text_records=rows,
        audio=dict(song_slots=len(songs), populated_slots=sum(bool(s['track_count']) for s in songs),
                   pcm_samples=len(manifest), pcm_bytes=sum(e['sample_count'] for e in manifest),
                   all_sample_sources_byte_matching=True),
        historical_function_scan=dict(false_sample_function_count=len(false_functions),
                                      excluded_sample_function_addresses=false_functions,
                                      status='Historical analysis retained; emitter excludes these sample spans.'),
        unresolved=[
            'SPC framing scans do not establish exhaustive string or opcode coverage.',
            'Raw KMP/NCD text hits are not verified text; raw SPC hits duplicate named sources.',
            'Song sequencing and PSG instruments are not reconstructed by PCM extraction.',
            'Optional English build implements DialogueStart lookup and pagination; this audit does not verify runtime behavior, every printer path, or complete translation coverage.',
            'Preserved binary spans, assembly literals and compressed streams remain; see linked-providers.json.',
        ])
    out = ROOT / 'reports/audit'
    out.mkdir(parents=True, exist_ok=True)
    (out / 'setup.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items()
                      if k not in ('raw_text_records', 'historical_function_scan')}, indent=2))


if __name__ == '__main__':
    main()
