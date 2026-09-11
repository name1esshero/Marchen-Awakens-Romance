#!/usr/bin/env python3
"""Convert between PNG and the GBA's own graphics formats.

    gbagfx.py png2tiles <in.png> <out.bin> --bpp N
    gbagfx.py tiles2png <in.bin> <out.png> --bpp N --width TILES [--pal P.pal]
    gbagfx.py compress   <in.bin>  <out.lz>
    gbagfx.py decompress <in.lz>   <out.bin>

Named after pokeemerald's tool of the same job. Each asset's bit depth and
sheet width live in graphics/graphics.json and are passed in here, so a wrong
guess is corrected by editing that file rather than by changing code.
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gfx
import lz77

GRAY = [(i * 17, i * 17, i * 17) for i in range(16)]


def gray(bpp):
    n = 16 if bpp == 4 else 256
    step = 255.0 / (n - 1)
    return [(int(i * step),) * 3 for i in range(n)]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["png2tiles", "tiles2png",
                                     "compress", "decompress"])
    ap.add_argument("src")
    ap.add_argument("dst")
    ap.add_argument("--bpp", type=int, default=4)
    ap.add_argument("--width", type=int, default=16, help="sheet width in tiles")
    ap.add_argument("--pal", default=None)
    ap.add_argument("--size", type=int, default=0, help="trim output to N bytes")
    a = ap.parse_args()

    if a.mode == "decompress":
        data = open(a.src, "rb").read()
        out, _ = lz77.decompress(data, 0)
        open(a.dst, "wb").write(out)

    elif a.mode == "compress":
        open(a.dst, "wb").write(lz77.compress(open(a.src, "rb").read()))

    elif a.mode == "tiles2png":
        data = open(a.src, "rb").read()
        pal = gfx.read_jasc(a.pal) if a.pal and os.path.exists(a.pal) \
            else gray(a.bpp)
        px = gfx.tiles_to_pixels(data, a.bpp, a.width)
        gfx.write_png(a.dst, px, pal)

    else:  # png2tiles
        px, _ = gfx.read_png(a.src)
        data = gfx.pixels_to_tiles(px, a.bpp)
        if a.size:
            data = data[:a.size]
        open(a.dst, "wb").write(data)


if __name__ == "__main__":
    main()
