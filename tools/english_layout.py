#!/usr/bin/env python3
"""Prepare English rows for the printer at 08011790; does not patch a ROM.

The reader at 08011AF4 skips ASCII except C/T formatting commands. Use actual
double-byte font codes. The 192-pixel backing surface and shadow permit 21
glyphs at a 9-pixel advance. Three rows are available (two if a name occupies
the first). The optional English constructor paginates longer translations.
Other printer paths and emulator validation remain unfinished.
"""
import argparse
import collections
import json
import re
from pathlib import Path

import font
import gfx

ROOT = Path(__file__).resolve().parent.parent


def load_mapping(root=ROOT):
    mapping = json.loads((root / 'text/translation/english_font.json').read_text())
    metadata = json.loads((root / 'graphics/fonts/font.json').read_text())
    pixels, _ = gfx.read_png(str(root / 'graphics/fonts/font.png'))
    result = {}
    for char, value in mapping.items():
        raw = bytes.fromhex(value)
        if len(raw) != 2 or not (0x80 <= raw[0] <= 0x9F or raw[0] >= 0xE0):
            raise ValueError(f'{char!r}: not an engine double-byte character')
        code = int.from_bytes(raw, 'big')
        code = {0xF056: 0x81FA, 0xF040: 0x81F9}.get(code, code)
        index = font.glyph_index(code)
        if index >= metadata['count']:
            raise ValueError(f'{char!r}: unsupported font slot')
        columns = len(pixels[0]) // 8
        ink = any(pixels[index // columns * 8 + y][index % columns * 8 + x]
                  for y in range(8) for x in range(8))
        if not ink and char != ' ':
            raise ValueError(f'{char!r}: blank font glyph')
        result[char] = raw
    return result


CONTROL_TAG = re.compile(r'\{(color|speed):([0-9A-Fa-f]{4})\}')


def encode_word(word, mapping):
    encoded = bytearray()
    glyphs = 0
    position = 0
    while position < len(word):
        tag = CONTROL_TAG.match(word, position)
        if tag:
            kind, argument = tag.groups()
            value = int(argument, 16)
            if kind == 'color' and (value >> 8 > 15 or value & 255 > 15):
                raise ValueError('Color ink and shadow must each be palette indices 00–0F')
            encoded.extend(('C' if kind == 'color' else 'T').encode() + argument.upper().encode())
            position = tag.end()
        else:
            char = word[position]
            if char not in mapping:
                raise ValueError(f'Unmapped English character or invalid control: {char!r}')
            encoded.extend(mapping[char]); glyphs += 1; position += 1
    return bytes(encoded), glyphs


def wrap_lines(text, mapping):
    """Wrap at word boundaries; {color:0F04}/{speed:0002} occupy no pixels."""
    lines = []
    for paragraph in text.split('\n'):
        line = b''
        width = 0
        for word in paragraph.split(' '):
            if not word: continue
            encoded, glyphs = encode_word(word, mapping)
            if glyphs > 21: raise ValueError(f'Word exceeds 21 glyphs: {word!r}')
            space = 1 if width and glyphs else 0
            if width + space + glyphs > 21:
                lines.append(line + b'\0'); line = b''; width = 0; space = 0
            if space: line += mapping[' ']
            line += encoded; width += space + glyphs
        lines.append(line + b'\0')
    return lines


def layout(text, mapping, rows=3):
    """Return one fixed-size page; reject overflow rather than truncate."""
    if rows not in (2, 3):
        raise ValueError('Only two- and three-row dialogue profiles are verified')
    lines = wrap_lines(text, mapping)
    if len(lines) > rows:
        raise ValueError(f'Message needs {len(lines)} rows; available: {rows}')
    return lines + [b'\0'] * (rows - len(lines))


def pages(text, mapping, rows=3):
    """Split a translation into pages compatible with the original printer."""
    if rows not in (2, 3):
        raise ValueError('Only two- and three-row dialogue profiles are verified')
    lines = wrap_lines(text, mapping)
    return [lines[i:i+rows] for i in range(0, len(lines), rows)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('text', nargs='?')
    parser.add_argument('--audit', action='store_true', help='check reviewed named-script comments')
    parser.add_argument('--pages', action='store_true', help='preview all button-separated pages')
    parser.add_argument('--rows', type=int, default=3, choices=(2, 3))
    args = parser.parse_args()
    if args.audit:
        mapping = load_mapping()
        counts = collections.Counter()
        failures = []
        for path in sorted((ROOT / 'text/nfp').glob('*.txt')):
            for line in path.read_text().splitlines():
                if not line.startswith('@') or '  // EN:' not in line:
                    continue
                english = line.partition('  // EN:')[2].removeprefix(' ')
                try:
                    wrapped = pages(english, mapping, args.rows)
                    counts['needs_pagination' if len(wrapped)>1 else 'fits_profile'] += 1
                except ValueError as exc:
                    counts['rejected'] += 1
                    failures.append(dict(script=path.name, offset=line.split()[0],
                                         english=english, reason=str(exc)))
        report = dict(profile=f'08011790 dialogue, 21 glyphs x {args.rows} rows',
                      limitation='Not proof that every record uses this printer.',
                      runtime_integrated=True,
                      runtime_scope='Optional DialogueStart bridge with A-button pagination; unmapped messages retain Japanese; emulator validation pending.',
                      counts=dict(counts), failures=failures)
        target = ROOT / 'reports/text/english-layout.json'
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
        print(dict(counts))
        return
    if args.text is None:
        parser.error('provide text or --audit')
    mapping = load_mapping()
    if args.pages:
        wrapped = pages(args.text, mapping, args.rows)
        print(json.dumps({'pages': [[r.hex().upper() for r in page] for page in wrapped],
                          'preview_only': True, 'glyphs_per_row': 21,
                          'rows_per_page': args.rows}, indent=2))
        return
    encoded = layout(args.text, mapping, args.rows)
    print(json.dumps({'rows': [''.join(('Ä' if r[i:i+2] == b'\xf0\x56' else '♥')
                                       if r[i:i+2] in (b'\xf0\x56', b'\xf0\x40') else r[i:i+2].decode('shift_jis')
                                       for i in range(0, len(r)-1, 2)) for r in encoded],
                      'encoded_hex': [r.hex().upper() for r in encoded],
                      'preview_only': True,
                      'runtime_note': 'Register reviewed text/nfp mappings and run make english to use translations in the ROM.'}, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
