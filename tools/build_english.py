#!/usr/bin/env python3
"""Compile reviewed exact-string mappings for the optional English dialogue path.

Ambiguous wording across scripts is excluded until runtime script identity is
recovered. Leading and trailing C/T controls survive. Translations with explicit
markup preserve interior emphasis; otherwise the translated row is rendered in
normal dialogue colors. Output uses the actual font and 21-glyph row limit.
"""
import argparse
import collections
import json
from pathlib import Path
import re
import english_layout
import text_codec

ROOT = Path(__file__).resolve().parents[1]
CONTROL = re.compile(rb'[CTct][0-9A-Fa-f]{4}')
PREFIX = re.compile(rb' *(?:[CTct][0-9A-Fa-f]{4} *)*')
SUFFIX = re.compile(rb'(?: *[CTct][0-9A-Fa-f]{4})+ *$')


def collect(root=ROOT):
    candidates = collections.defaultdict(set)
    locations = collections.defaultdict(list)
    paths = sorted((root/'text/nfp').glob('*.txt'))
    # Item names are inserted into common acquisition messages at runtime, so
    # they never appear as complete literals in the named NFP scripts.
    item_text = root/'text/script_1B0000.txt'
    if item_text.is_file():
        paths.append(item_text)
    for path in paths:
        for line in path.read_text().splitlines():
            if not line.startswith('@') or '  // EN:' not in line:continue
            body, english = line.split('  // EN:',1)
            english = english.removeprefix(' ')
            offset, japanese = body.split(' ',1)
            raw = text_codec.encode(japanese)
            candidates[raw].add(english)
            locations[raw].append(path.name+':'+offset)
    # MswStr always receives row strings, including empty padding literals.
    # One unmapped blank used to discard the translation of the entire message.
    candidates[b''].add('')
    mapping = english_layout.load_mapping(root)
    accepted = [];rejected = []
    for raw, translations in sorted(candidates.items()):
        try:
            if len(translations) != 1:raise ValueError('Context-dependent translation; runtime script identity required')
            if b'\0' in raw or len(raw) > 160:raise ValueError('Source exceeds dialogue row buffer')
            prefix = PREFIX.match(raw).group()
            body = raw[len(prefix):]
            suffix_match = SUFFIX.search(body)
            suffix = suffix_match.group() if suffix_match else b''
            visible = body[:-len(suffix)] if suffix else body
            translation = next(iter(translations))
            if CONTROL.search(visible) and not english_layout.CONTROL_TAG.search(translation):
                # Japanese emphasis boundaries cannot be transferred by byte
                # position after rewording. Reset translated text to the normal
                # dialogue palette rather than rejecting the entire message.
                translation = '{color:0F04}' + translation
            rows = english_layout.wrap_lines(translation,mapping)
            while len(rows)>1 and rows[-1]==b'\0':rows.pop()
            if len(rows) > 255:raise ValueError('Translation exceeds 255-row mapping limit')
            # The pagination task carries style across page boundaries.
            rows[0] = prefix + rows[0]
            if suffix: rows[-1] = rows[-1][:-1] + suffix + b'\0'
            if any(len(row)>127 for row in rows):raise ValueError('Controls exceed signed cursor capacity')
            accepted.append((raw, rows))
        except ValueError as exc:
            rejected.append(dict(locations=locations[raw],reason=str(exc)))
    return accepted, rejected


def literal(raw):
    return '"'+''.join('\\%03o'%b for b in raw)+'"'


def render(accepted):
    lines=['/* Generated from reviewed extracted-text comments; do not edit. */', '#include "english.h"']
    for i, (_, rows) in enumerate(accepted):
        lines.append('static const char *const rows_%d[] = {%s};' %
                     (i, ', '.join(literal(row[:-1]) for row in rows)))
    lines.append('const struct EnglishRowMapping gEnglishRows[] = {')
    for i, (raw, rows) in enumerate(accepted):
        lines.append('    {%s, rows_%d, %d},' % (literal(raw), i, len(rows)))
    lines += ['};', 'const u32 gEnglishRowCount = %d;' % len(accepted), '']
    return lines


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', default='build/english/mappings.c')
    args=parser.parse_args()
    accepted,rejected=collect()
    lines=render(accepted)
    out=ROOT/args.output;out.parent.mkdir(parents=True,exist_ok=True);out.write_text('\n'.join(lines))
    report=dict(exact_row_mappings=len(accepted), rejected_keys=len(rejected), rejected=rejected,
                profile='DialogueStart: 21 glyphs per row, 3 rows in mode 0 or 2 in mode 1',
                fallback='Whole message remains Japanese if any row is unmapped or its mode is unsupported.',
                limitations=['Pagination uses A between pages; emulator validation is still required.', 'Only the recovered DialogueStart path is hooked.',
                             'Static mapping coverage does not establish runtime message coverage.'])
    report_path=ROOT/'reports/text/english-runtime.json'
    report_path.parent.mkdir(parents=True,exist_ok=True)
    report_path.write_text(json.dumps(report,indent=2)+'\n')
    print(len(accepted),'exact row mappings;',len(rejected),'ambiguous or unsupported keys excluded')


if __name__=='__main__':main()
