#!/usr/bin/env python3
"""Find and extract the ROM's uncompressed character data.

Only 111 of the compressed streams are artwork, yet most of the cartridge is
graphics: character sprites, item icons and UI are stored as plain 4bpp tiles
with no header and no compression.

This walks everything not already accounted for (code, compressed assets,
padding), scores each 32-byte tile on how much it looks like artwork, and
groups long runs into assets.

Because these are uncompressed, the PNG is a lossless representation of the
exact ROM bytes: the build regenerates them from the PNG and still reproduces
the image byte for byte. Edit the PNG and the change goes straight into the
ROM.
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gfx

TILE = 32                     # bytes per 4bpp tile
MIN_TILES = 64                # ignore runs shorter than this
GRAY = [(i * 17, i * 17, i * 17) for i in range(16)]


def tile_score(rom, off):
    """Horizontal agreement between neighbouring pixels in one 4bpp tile."""
    same = 0
    for y in range(8):
        base = off + y * 4
        prev = rom[base] & 0xF
        for k in range(4):
            b = rom[base + k]
            lo, hi = b & 0xF, b >> 4
            if lo == prev:
                same += 1
            if hi == lo:
                same += 1
            prev = hi
    return same / 32.0


def is_blank(rom, off):
    return rom[off:off + TILE].count(0) == TILE


def find_runs(rom, free, min_tiles=MIN_TILES):
    """Group consecutive art-like tiles into candidate sheets."""
    runs = []
    for start, end in free:
        s = (start + TILE - 1) // TILE * TILE
        e = end // TILE * TILE
        run_start = None
        art = blank = 0
        pos = s
        while pos < e:
            blankt = is_blank(rom, pos)
            good = blankt or tile_score(rom, pos) > 0.5
            if good:
                if run_start is None:
                    run_start = pos
                    art = blank = 0
                if blankt:
                    blank += 1
                else:
                    art += 1
            else:
                if run_start is not None:
                    n = (pos - run_start) // TILE
                    # a real sheet is mostly drawn, not mostly empty
                    if n >= min_tiles and art > n * 0.35:
                        runs.append((run_start, pos))
                    run_start = None
            pos += TILE
        if run_start is not None:
            n = (e - run_start) // TILE
            if n >= min_tiles and art > n * 0.35:
                runs.append((run_start, e))
    return runs


def main():
    rom = open(sys.argv[1], "rb").read()
    free = json.load(open(sys.argv[2])) if len(sys.argv) > 2 \
        else json.load(open("/tmp/free.json"))

    runs = find_runs(rom, free)
    total = sum(b - a for a, b in runs)
    print("found %d uncompressed graphics runs, %d bytes (%.2f MB)"
          % (len(runs), total, total / 1048576.0))

    out = []
    for start, end in runs:
        n_tiles = (end - start) // TILE
        width = 32 if n_tiles >= 256 else 16
        # pad the sheet out to whole rows; the manifest keeps the true length
        name = "raw_%06X" % start
        sub = os.path.join("graphics", "raw")
        os.makedirs(sub, exist_ok=True)
        base = os.path.join(sub, name)
        data = rom[start:end]
        padded = data + bytes(-len(data) % (TILE * width))
        px = gfx.tiles_to_pixels(padded, 4, width)
        gfx.write_png(base + ".png", px, GRAY)
        gfx.write_jasc(base + ".pal", GRAY)
        out.append({
            "rom_offset": start, "raw_size": end - start,
            "kind": "raw", "bpp": 4, "width_tiles": width,
            "compressed": False,
            "path": base.replace(os.sep, "/"),
        })
    with open("assets_raw.json", "w") as f:
        json.dump(out, f, indent=1)
    print("wrote %d sheets to graphics/raw/" % len(out))


if __name__ == "__main__":
    main()
