#!/usr/bin/env python3
"""Export/build the actual FONT.NFT. Indexed PNG edits preserve all header bytes.

0807AE10 computes glyph addresses from +0x34; 0807ADF4 reads dimensions.
0807AE7C maps Shift-JIS values to glyph slots (including reserved holes).
"""
import argparse
import json
from pathlib import Path
import struct

import gfx
import nfp

ROOT = Path(__file__).resolve().parent.parent
FONT_DIR = ROOT / 'graphics/fonts'


def glyph_index(code):
    """Literal control-flow translation of 0807AE7C; 0xFFFF means unsupported."""
    code &= 0xFFFF
    if code <= 0xFF:
        return code + 0x1E0
    if 0x81B0 <= code <= 0x81BA or 0x8140 <= code <= 0x81FF:
        return code - 0x8140
    if 0x839F <= code <= 0x83D6:
        return code - 0x81C0
    if 0x8440 <= code <= 0x8491:
        return code - 0x8200
    if 0x8740 <= code <= 0x879C:
        return code - 0x82C0
    if 0x8240 <= code <= 0x84BF:
        if code <= 0x829A or 0x829F <= code <= 0x82F1:
            return code - 0x8180
        if 0x8340 <= code <= 0x83FF:
            return code - 0x81C0
        return 0
    if 0x8840 <= code <= 0x988F:
        return ((code >> 8) - 0x88) * 192 + (code & 255) + 0x500
    if 0x9890 <= code <= 0x9FFF:
        return ((code >> 8) - 0x98) * 192 + (code & 255) + 0x1100
    if 0xE040 <= code <= 0xEAAF:
        return ((code >> 8) - 0xE0) * 192 + (code & 255) + 0x1700
    return 0xFFFF


def text_index(char):
    raw = char.encode('shift_jis')
    code = int.from_bytes(raw, 'big')
    code = {0xF056: 0x81FA, 0xF040: 0x81F9}.get(code, code)
    index = glyph_index(code)
    return glyph_index(0x81A1) if index == 0xFFFF else index


def header(blob):
    if not blob.startswith(b'NFT Ver1.0'):
        raise ValueError('Not an NFT Ver1.0 font')
    w, h, bpp, count, palette, offset, stride = struct.unpack_from('<7I', blob, 0x20)
    if (w, h, bpp, stride) != (8, 8, 1, 8) or offset + count * stride > len(blob):
        raise ValueError('Unexpected NFT font layout')
    return dict(width=w, height=h, bpp=bpp, count=count,
                palette_offset=palette, glyph_offset=offset, stride=stride)


def decode(blob, columns=32):
    info = header(blob)
    px = [[0] * (columns * 8) for _ in range(((info['count'] + columns - 1) // columns) * 8)]
    for i in range(info['count']):
        for y in range(8):
            row = blob[info['glyph_offset'] + i * 8 + y]
            for x in range(8):
                px[i // columns * 8 + y][i % columns * 8 + x] = (row >> (7 - x)) & 1
    return px


def encode(blob, px, columns=32):
    info = header(blob)
    height = ((info['count'] + columns - 1) // columns) * 8
    if len(px) != height or any(len(row) != columns * 8 for row in px):
        raise ValueError('Font sheet dimensions changed')
    if any(v not in (0, 1) for row in px for v in row):
        raise ValueError('Font pixels must use palette indices 0 and 1')
    result = bytearray(blob)
    for i in range(info['count']):
        for y in range(8):
            row = px[i // columns * 8 + y]
            result[info['glyph_offset'] + i * 8 + y] = sum(row[i % columns * 8 + x] << (7 - x) for x in range(8))
    # The final half-row is display padding, never silently accept edits there.
    for i in range(info['count'], (height // 8) * columns):
        if any(px[i // columns * 8 + y][i % columns * 8 + x] for y in range(8) for x in range(8)):
            raise ValueError('Edited padding beyond the final glyph')
    return bytes(result)


def export(rom):
    entry = next(e for e in nfp.entries(rom) if e['name'] == 'FONT.NFT')
    blob = rom[entry['rom_offset']:entry['end']]
    info = header(blob)
    FONT_DIR.mkdir(parents=True, exist_ok=True)
    # Extraction is explicit; ordinary builds never overwrite these sources.
    (FONT_DIR / 'font.nft').write_bytes(blob)
    gfx.write_png(str(FONT_DIR / 'font.png'), decode(blob), [(255,255,255),(0,0,0)])
    (FONT_DIR / 'font.json').write_text(json.dumps(dict(entry, **info), indent=2) + '\n')
    rows = ['glyph_index\tshift_jis\tcharacter']
    for code in range(65536):
        raw = code.to_bytes(1 if code <= 255 else 2, 'big')
        try:
            char = raw.decode('shift_jis')
        except UnicodeDecodeError:
            continue
        i = glyph_index(code)
        if len(char) == 1 and char.isprintable() and i < info['count']:
            rows.append(f'{i}\t{code:04X}\t{char}')
    (FONT_DIR / 'glyphs.tsv').write_text('\n'.join(rows) + '\n', encoding='utf-8')
    mapping = ['engine_code\tglyph_index\tNFT_byte_offset']
    for code in range(65536):
        index = glyph_index(code)
        mapping.append(f'{code:04X}\t{index:04X}\t' +
                       (f'{info["glyph_offset"] + index * 8:06X}' if index != 0xFFFF else 'unsupported'))
    (FONT_DIR / 'engine_charmap.tsv').write_text('\n'.join(mapping) + '\n')
    preview(blob, 'ＭＡＲ　Ｋｎｏｃｋｉｎ　ｏｎ　Ｈｅａｖｅｎｓ　Ｄｏｏｒ\nＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ\nａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ\nバッボ　ギンタ　ドロシー\n何のこうかもない', FONT_DIR / 'preview.png')
    print(f"FONT.NFT: ROM {entry['rom_offset']:06X}, {info['count']} glyph slots, 8x8 1bpp")


def preview(blob, text, output):
    lines = text.splitlines()
    px = [[0] * (max(map(len, lines)) * 8) for _ in range(len(lines) * 12)]
    info = header(blob)
    for y, line in enumerate(lines):
        for x, char in enumerate(line):
            i = text_index(char)
            for dy in range(8):
                bits = blob[info['glyph_offset'] + i * 8 + dy]
                for dx in range(8):
                    px[y * 12 + dy][x * 8 + dx] = (bits >> (7 - dx)) & 1
    gfx.write_png(str(output), px, [(255,255,255),(0,0,0)])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['extract', 'build', 'preview'])
    parser.add_argument('--output', default='build/graphics/fonts/font.nft')
    parser.add_argument('--text', default='バッボ　ギンタ　ドロシー　ＡＢＣ　ａｂｃ　０１２３')
    args = parser.parse_args()
    if args.command == 'extract':
        export((ROOT / 'baserom.gba').read_bytes())
        return
    blob = (FONT_DIR / 'font.nft').read_bytes()
    px, _ = gfx.read_png(str(FONT_DIR / 'font.png'))
    rebuilt = encode(blob, px)
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    if args.command == 'build':
        out.write_bytes(rebuilt)
    else:
        preview(rebuilt, args.text, out)


if __name__ == '__main__':
    main()
