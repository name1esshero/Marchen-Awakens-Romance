#!/usr/bin/env python3
"""Work out which parts of the ROM are not yet accounted for.

Subtracts the code region, every compressed asset and the trailing padding,
leaving the gaps that still need identifying.
"""
import json
import sys

CODE_END = 0x1B0000
ROM_END = 0xFFFF00


def main():
    rom = open(sys.argv[1], "rb").read()
    spans = [[0, CODE_END], [ROM_END, len(rom)]]
    for e in json.load(open("assets.json")):
        spans.append([e["rom_offset"],
                      e["rom_offset"] + e["compressed_size"]])
    spans.sort()

    merged = []
    for a, b in spans:
        if merged and a <= merged[-1][1]:
            merged[-1][1] = max(merged[-1][1], b)
        else:
            merged.append([a, b])

    free = []
    prev = 0
    for a, b in merged:
        if a > prev:
            free.append([prev, a])
        prev = max(prev, b)
    if prev < len(rom):
        free.append([prev, len(rom)])

    total = sum(b - a for a, b in free)
    print("unaccounted: %d bytes (%.2f MB) in %d gaps"
          % (total, total / 1048576.0, len(free)))
    json.dump(free, open(".analysis/free.json", "w"))


if __name__ == "__main__":
    main()
