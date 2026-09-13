#!/usr/bin/env python3
"""Validate the fixed-layout ÄRM and item text sources against the ROM.

These tables are data structures, not scripts and not compressed graphics.
Keeping their ranges explicit prevents the generic Shift-JIS scanner from
silently losing them when asset classifications change.
"""
from pathlib import Path
import re

import text_codec

ROOT = Path(__file__).resolve().parents[1]
ROW = re.compile(r'^@([0-9A-Fa-f]+)\s(.*?)(?:\s+//.*)?$')
TABLES = (
    dict(kind='arm_definition_table', base=0x1B096C, records=445,
         stride=0x80, name_offset=0x10, name_size=34,
         description_offset=0x32, description_size=38,
         path='text/arm_definitions.txt'),
    dict(kind='item_definition_table', base=0x1BE82C, records=13,
         stride=0x50, name_offset=0x10, name_size=34,
         description_offset=0x32, description_size=30,
         path='text/item_definitions.txt'),
)


def ranges():
    return [(t['base'], t['base'] + t['records'] * t['stride']) for t in TABLES]


def source_records(table):
    result = {}
    for lineno, line in enumerate((ROOT / table['path']).read_text().splitlines(), 1):
        match = ROW.match(line)
        if not match:
            continue
        offset = int(match.group(1), 16)
        text = match.group(2).rstrip()
        if offset in result:
            raise ValueError(f"{table['path']}:{lineno}: duplicate offset {offset:06X}")
        result[offset] = (text, lineno)
    return result


def audit(rom):
    total = 0
    for table in TABLES:
        expected = {}
        for index in range(table['records']):
            record = table['base'] + index * table['stride']
            for field, field_offset, field_size in (
                    ('name', table['name_offset'], table['name_size']),
                    ('description', table['description_offset'], table['description_size'])):
                offset = record + field_offset
                raw = rom[offset:offset + field_size].split(b'\0', 1)[0]
                if raw:
                    expected[offset] = (text_codec.decode_lossless(raw), field, index)
        actual = source_records(table)
        if set(actual) != set(expected):
            missing = sorted(set(expected) - set(actual))
            extra = sorted(set(actual) - set(expected))
            raise ValueError(f"{table['kind']} offsets differ: missing={missing}, extra={extra}")
        for offset, (original, field, index) in expected.items():
            source, lineno = actual[offset]
            if text_codec.encode(source) != rom[offset:offset + len(text_codec.encode(source))]:
                raise ValueError(
                    f"{table['path']}:{lineno}: {field} for record {index} "
                    f"does not match ROM at {offset:06X}")
            if source != original:
                raise ValueError(f"{table['path']}:{lineno}: noncanonical decoding")
        total += len(actual)
        print(f"{table['kind']}: {table['records']} records, {len(actual)} populated text fields")
    print(f"validated {total} fixed-layout definition strings")


if __name__ == '__main__':
    audit((ROOT / 'baserom.gba').read_bytes())
