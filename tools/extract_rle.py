#!/usr/bin/env python3
"""Scan possible BIOS RLE headers into a diagnostic JSON report only.

Successful decoding, alignment and compression ratio do not prove that a
candidate is an asset. Confirm boundaries and a runtime consumer before
promoting any result into the build. This tool never creates source graphics.
"""
import bisect
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rle

CODE_END = 0x1B0000
OUT_DIR = "reports/graphics"


def known_ranges():
    r = []
    for path in ("assets.json", "assets_raw.json"):
        try:
            man = json.load(open(path))
        except OSError:
            continue
        for m in man:
            size = m.get("compressed_size") or m["raw_size"]
            r.append((m["rom_offset"], m["rom_offset"] + size))
    r.sort()
    return r


def main():
    rom = open(sys.argv[1], "rb").read()
    ranges = known_ranges()
    starts = [a for a, _ in ranges]

    def overlaps(off):
        i = bisect.bisect_right(starts, off) - 1
        return i >= 0 and off < ranges[i][1]

    found = []
    for off in range(CODE_END, len(rom) - 8, 16):
        if rom[off] != 0x30 or overlaps(off):
            continue
        size = rom[off + 1] | (rom[off + 2] << 8) | (rom[off + 3] << 16)
        if not (0x400 <= size <= 0x40000):
            continue
        try:
            data, clen = rle.decompress(rom, off)
        except Exception:
            continue
        if len(data) != size or clen >= size:
            continue
        found.append((off, size, clen, data))

    found.sort()
    keep = []
    end = 0
    for off, size, clen, data in found:
        if off < end:
            continue
        keep.append((off, size, clen, data))
        end = off + clen

    os.makedirs(OUT_DIR, exist_ok=True)
    manifest = [dict(rom_offset=off, compressed_size=clen, raw_size=size,
                     status="unverified_scan_candidate")
                for off, size, clen, data in keep]
    path = os.path.join(OUT_DIR, "rle-candidates.json")
    with open(path, "w") as f:
        json.dump(manifest, f, indent=2)
        f.write("\n")
    print(f"{len(manifest)} unverified candidates written to {path}; no build assets created")


if __name__ == "__main__":
    main()
