#!/usr/bin/env python3
"""Refresh legacy CODE-offset views from the named script annotations.

Canonical Japanese is checked against the old view; only known lost trailing
spaces may differ. A substantive difference stops synchronization, preserving
possible edits for review. The legacy files are not ROM build sources.
"""
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]


def sync(root=ROOT):
    if not (root/'scripts/nfp/manifest.json').exists():
        return {}
    by_offset={e['rom_offset']:e for e in json.loads((root/'scripts/nfp/manifest.json').read_text())}
    count=0;placeholders=0;changes=[]
    for path in sorted((root/'text').glob('scrp_script_*.txt')):
        entry=by_offset[int(path.stem.split('_')[-1],16)]
        source=root/entry['text']
        canonical={line.split(' ',1)[0]:line for line in source.read_text().splitlines() if line.startswith('@')}
        old=path.read_text(); rows=[]
        for line in old.splitlines():
            if not line.startswith('@'):continue
            key=line.split(' ',1)[0]
            replacement=canonical[key]
            if line.partition('  //')[0].rstrip()!=replacement.partition('  //')[0].rstrip():
                raise ValueError(f'{path}:{key}: legacy Japanese differs from named source')
            placeholders += '{kanji' in line
            if line!=replacement:count+=1
            rows.append(replacement)
        header=(f'// Legacy reference view of {entry["name"]}; edit {entry["text"]} instead.\n'
                '// CODE offsets and Japanese are checked against the named source.\n'
                '// Regenerate comments with tools/translate_comments.py; this copy is not built.\n\n')
        updated=header+'\n'.join(rows)+'\n'
        if updated!=old:path.write_text(updated);changes.append(path.name)
    return dict(updated_legacy_records=count,removed_legacy_kanji_placeholders=placeholders,files_changed=len(changes))


if __name__=='__main__':print(sync())
