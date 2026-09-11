#!/usr/bin/env python3
"""Identify what the uncompressed parts of the ROM actually contain.

Three detectors, each calibrated against data whose identity is already known
(the decompressed graphics assets for art, the SCRP files for not-art):

  art    4bpp character data. Real tiles use few palette entries and have
         strong vertical correlation between rows. Measured on known art:
         at most 9.1 distinct nibbles per tile, correlation 0.35 and up.
         Script bytecode and noise sit at ~10 distinct and ~0.42.

  sound  8-bit signed PCM. A waveform moves in small steps, so the mean
         absolute difference between consecutive bytes stays low while the
         overall spread stays wide. Packed pixels do not behave that way.

  data   everything else, left alone.

Anything that fails a test is left as data rather than guessed at.
"""
import json
import os
import sys

TILE = 32
ART_MAX_DISTINCT = 9.0
ART_MIN_VERT = 0.45
# Real sheets almost never repeat a tile; flat fill patterns repeat constantly.
# Known art measures 0.97; the fill patterns that fooled an earlier version of
# this detector measured 0.02 and 0.43.
ART_MIN_DIVERSITY = 0.85
PCM_MAX_STEP = 18.0
PCM_MIN_SPREAD = 48.0


def tile_features(b):
    nib = []
    for v in b:
        nib.append(v & 0xF)
        nib.append(v >> 4)
    distinct = len(set(nib))
    same = 0
    for i in range(56):
        if nib[i] == nib[i + 8]:
            same += 1
    return distinct, same / 56.0


def tile_diversity(rom, off, length):
    """Fraction of non-blank tiles that are distinct."""
    n = min(length // TILE, 512)
    tiles = [bytes(rom[off + i * TILE: off + (i + 1) * TILE]) for i in range(n)]
    nb = [t for t in tiles if t.count(0) != TILE]
    if len(nb) < 8:
        return 0.0
    return len(set(nb)) / float(len(nb))


def art_score(rom, off, length):
    n = min(length // TILE, 128)
    d = v = 0.0
    cnt = 0
    for t in range(n):
        b = rom[off + t * TILE: off + (t + 1) * TILE]
        if b.count(0) == TILE:
            continue
        dd, vv = tile_features(b)
        d += dd
        v += vv
        cnt += 1
    if cnt < 4:
        return None
    return d / cnt, v / cnt


def pcm_score(rom, off, length):
    """Mean absolute step and spread, treating bytes as signed samples."""
    n = min(length, 4096)
    vals = [(b - 256 if b > 127 else b) for b in rom[off:off + n]]
    if len(vals) < 64:
        return None
    steps = sum(abs(vals[i + 1] - vals[i]) for i in range(len(vals) - 1))
    step = steps / (len(vals) - 1)
    spread = max(vals) - min(vals)
    return step, spread


def classify(rom, off, length):
    a = art_score(rom, off, length)
    if a and a[0] <= ART_MAX_DISTINCT and a[1] >= ART_MIN_VERT \
            and tile_diversity(rom, off, length) >= ART_MIN_DIVERSITY:
        return "art"
    p = pcm_score(rom, off, length)
    if p and p[0] <= PCM_MAX_STEP and p[1] >= PCM_MIN_SPREAD:
        return "sound"
    return "data"


def scan(rom, free, block=0x800):
    """Label each block, then merge neighbours that agree."""
    runs = []
    for start, end in free:
        s = (start + TILE - 1) // TILE * TILE
        pos = s
        cur = None
        cur_start = pos
        while pos + block <= end:
            k = classify(rom, pos, block)
            if k != cur:
                if cur is not None and cur != "data":
                    runs.append((cur_start, pos, cur))
                cur = k
                cur_start = pos
            pos += block
        if cur is not None and cur != "data" and pos > cur_start:
            runs.append((cur_start, pos, cur))
    return [r for r in runs if r[1] - r[0] >= 0x800]


def main():
    rom = open(sys.argv[1], "rb").read()
    free = json.load(open(sys.argv[2]))
    runs = scan(rom, free)
    tot = {}
    for a, b, k in runs:
        tot[k] = tot.get(k, 0) + (b - a)
    print("classified %d runs in the unaccounted space:" % len(runs))
    for k, v in sorted(tot.items()):
        print("  %-6s %9d bytes (%.2f MB)" % (k, v, v / 1048576.0))
    json.dump([[a, b, k] for a, b, k in runs], open(".analysis/runs.json", "w"))


if __name__ == "__main__":
    main()
