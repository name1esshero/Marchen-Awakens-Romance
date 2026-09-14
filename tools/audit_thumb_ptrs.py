#!/usr/bin/env python3
"""Find raw 0x08XXXXXX table entries in src/*.c that the THUMB-bit rule could
rename: an odd stored address whose real (address - 1) is already a named,
decompiled function in src/decompiled.json.

Read-only: reports candidates, does not edit any source file. See
docs/PRET_STANDARDS.md section 5 ("The THUMB-bit possibility") for the rule
itself and its verification steps, which this tool does not substitute for --
a reported candidate should still be checked against a real code region
before being treated as a function pointer.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    decompiled = json.loads((ROOT / 'src/decompiled.json').read_text())
    by_addr = {e['addr']: e for e in decompiled}

    total_candidates = 0
    for path in sorted((ROOT / 'src').glob('*.c')):
        text = path.read_text()
        addrs = sorted(set(re.findall(r'0x(08[0-9A-Fa-f]{6})', text)))
        odd = [a for a in addrs if int(a, 16) & 1]
        found = []
        for a in odd:
            real = f'{int(a, 16) - 1:08X}'
            e = by_addr.get(real)
            if e:
                found.append((a, real, e['name']))
        if found:
            print(f'{path.relative_to(ROOT)}: {len(found)} candidate(s)')
            for a, real, name in found:
                print(f'    0x{a} -> 0x{real} = {name}')
            total_candidates += len(found)

    print()
    print(f'total candidates: {total_candidates}')
    print('Each is a raw odd hex literal whose (address - 1) already has a name;')
    print('verify against its section before renaming (see PRET_STANDARDS.md).')


if __name__ == '__main__':
    main()
