#!/usr/bin/env python3
"""Extract and rebuild every named SPC member, including uncompressed scripts.

0x10/u16-length/NUL framing is a conservative string-candidate detector, not
an exhaustive bytecode disassembler. All accepted records must round-trip.
Comments use // EN: for reviewed English and // TODO: for untranslated text.
"""
import argparse
import collections
import json
from pathlib import Path
import re
import struct
import gfx
import lz77
import nfp
import text_codec as tc
import script_events
from extract_scrp_text import strings_in

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / 'scripts/nfp/manifest.json'


def decode(blob):
    return lz77.decompress(blob)[0] if blob[0] == 0x10 else blob


def records(blob):
    raw = decode(blob)
    if raw[:4] != b'SCRP' or raw[8:12] != b'CODE':
        raise ValueError('Expected SCRP/CODE container')
    length = struct.unpack_from('<I', raw, 12)[0]
    if 16 + length > len(raw):
        raise ValueError('CODE chunk exceeds member')
    return dict(strings_in(raw[16:16 + length], lossless=True))


def extract():
    rom = (ROOT / 'baserom.gba').read_bytes()
    (ROOT / 'scripts/nfp').mkdir(exist_ok=True)
    (ROOT / 'text/nfp').mkdir(exist_ok=True)
    manifest = []
    for e in nfp.entries(rom):
        if not e['name'].endswith('.SPC'):
            continue
        path = 'scripts/nfp/' + e['name'] + '.bin'
        text = 'text/nfp/' + e['name'] + '.txt'
        blob = rom[e['rom_offset']:e['end']]
        found = records(blob)
        (ROOT / path).write_bytes(blob)
        lines = [f"// {e['name']}: ROM {e['rom_offset']:06X}",
                 '// @offset is the string opcode offset within CODE, not a ROM address.',
                 '// Edit text here; // EN: comments never reach the ROM.', '']
        lines += [f'@{off:06X} {tc.decode_lossless(raw)}  // TODO: English translation' for off, raw in found.items()]
        # Extraction must not erase comments or text edits on subsequent runs.
        if not (ROOT / text).exists():
            (ROOT / text).write_text('\n'.join(lines) + '\n', encoding='utf-8')
        else:
            existing = edits(ROOT / text)
            missing = [(off, raw) for off, raw in found.items() if off not in existing]
            if missing:
                with (ROOT / text).open('a', encoding='utf-8') as stream:
                    for off, raw in missing:
                        stream.write(f'@{off:06X} {tc.decode_lossless(raw)}  // TODO: English translation\n')
        manifest.append(dict(e, path=path, text=text, compressed=blob[0] == 16,
                             string_records=len(found)))
    MANIFEST.write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'{len(manifest)} named scripts, {sum(e["string_records"] for e in manifest)} framed string candidates')


def edits(path):
    out = {}
    for no, line in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
        if not line.strip() or line.startswith('//'):
            continue
        m = re.match(r'^@([0-9A-Fa-f]+) (.*)$', line)
        if not m:
            raise ValueError(f'{path}:{no}: malformed record')
        off = int(m[1], 16)
        if off in out:
            raise ValueError(f'{path}:{no}: duplicate record')
        # Strip exactly the two separator spaces, not meaningful trailing
        # spaces within the original string.
        body = m[2].partition('  //')[0]
        out[off] = tc.encode(body)
    return out


def rebuild(blob, changes):
    source = records(blob)
    if source.keys() != changes.keys():
        raise ValueError('String record offsets differ from original; cannot insert/delete records')
    raw = bytearray(decode(blob))
    for off, text in changes.items():
        if b'\0' in text or len(text) > len(source[off]):
            raise ValueError(f'String at CODE+{off:06X} exceeds its original space or contains NUL')
        # Keep the bytecode length unchanged; spaces shorten display text
        # without introducing embedded NULs or shifting later instructions.
        raw[16 + off + 3:16 + off + 3 + len(source[off])] = text.ljust(len(source[off]), b' ')
    if bytes(raw) == decode(blob):
        return blob
    if blob[0] != 0x10:
        return bytes(raw)
    packed = lz77.compress(bytes(raw))
    _, consumed = lz77.decompress(packed)
    packed = packed[:consumed]
    if len(packed) > len(blob):
        raise ValueError('Edited script no longer fits its named archive member')
    return packed + blob[len(packed):]


def build():
    for e in json.loads(MANIFEST.read_text()):
        dest = ROOT / 'build/scripts/nfp' / (e['name'] + '.bin')
        dest.parent.mkdir(parents=True, exist_ok=True)
        original = (ROOT / e['path']).read_bytes()
        out = script_events.build(rebuild(original, edits(ROOT / e['text'])), original, e['name'], ROOT)
        if not dest.exists() or dest.read_bytes() != out:
            dest.write_bytes(out)


def audit():
    manifest = json.loads(MANIFEST.read_text())
    totals = collections.Counter()
    rows = []
    for e in manifest:
        original = (ROOT / e['path']).read_bytes()
        values = edits(ROOT / e['text'])
        output = script_events.build(rebuild(original, values), original, e['name'], ROOT)
        notes = (ROOT / e['text']).read_text(encoding='utf-8')
        english = sum(bool(re.match(r'^@.*  // EN:(?: |$)', line)) for line in notes.splitlines())
        empty_english = sum(bool(re.match(r'^@.*  // EN:$', line)) for line in notes.splitlines())
        literals = sum(bool(re.match(r'^@.*// LITERAL: ', line)) for line in notes.splitlines())
        formatting = sum(bool(re.match(r'^@.*// FORMAT: ', line)) for line in notes.splitlines())
        for line in notes.splitlines():
            if line.startswith('@') and '  // FORMAT: ' in line:
                original_text = line.partition('  //')[0].split(' ', 1)[1]
                if re.sub(r' ?C[0-9A-F]{4} ?', '', original_text).strip():
                    raise ValueError(e['name'] + ': formatting annotation hides text')
        row = dict(name=e['name'], records=len(values), english_comments=english,
                   untranslated=len(values)-english, byte_matching=output == original)
        row.update(empty_english_comments=empty_english, ascii_literal_comments=literals, formatting_records=formatting,
                   pending_translation=len(values)-english-literals-formatting)
        rows.append(row)
        totals.update(scripts=1, records=len(values), english_comments=english,
                      untranslated=len(values)-english, byte_matching=output==original)
        totals.update(empty_english_comments=empty_english, ascii_literal_comments=literals, formatting_records=formatting,
                      pending_translation=len(values)-english-literals-formatting)
    dest = ROOT / 'reports/text'
    dest.mkdir(parents=True, exist_ok=True)
    (dest / 'audit.json').write_text(json.dumps(dict(totals=dict(totals), scripts=rows), indent=2) + '\n')
    print(dict(totals))


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('command', choices=['extract', 'build', 'audit'])
    args = p.parse_args()
    globals()[args.command]()
