"""Work out how an extracted blob should be drawn.

Tile data carries no header, so bit depth, sheet width and palette all have to
be inferred. These are measurements on the pixels, not labels from the game,
so every result is written into the per-asset rules where it can be corrected
by hand.
"""
import struct

import gfx


def bpp_score(data, bpp):
    """Neighbouring pixels in real artwork usually agree; noise does not."""
    px = gfx.tiles_to_pixels(data[:0x2000], bpp, 8)
    if not px:
        return 0.0
    same = n = 0
    for row in px:
        for x in range(len(row) - 1):
            n += 1
            if row[x] == row[x + 1]:
                same += 1
    return same / n if n else 0.0


def detect_bpp(data):
    """4bpp and 8bpp are the two GBA character formats."""
    s4 = bpp_score(data, 4)
    s8 = bpp_score(data, 8)
    return (4, s4, s8) if s4 >= s8 else (8, s8, s4)


def edge_match(data, bpp, width_tiles):
    """How well do horizontally adjacent tiles line up at their shared edge?

    For a sheet laid out as a picture the seam is continuous; at the wrong
    width the columns are unrelated and the score drops.
    """
    tile_bytes = 32 if bpp == 4 else 64
    n_tiles = len(data) // tile_bytes
    if n_tiles < width_tiles * 2:
        return 0.0
    px = gfx.tiles_to_pixels(data, bpp, width_tiles)
    if not px:
        return 0.0
    h = len(px)
    w = len(px[0])
    same = n = 0
    for x in range(7, w - 1, 8):          # every tile seam
        for y in range(h):
            n += 1
            if px[y][x] == px[y][x + 1]:
                same += 1
    return same / n if n else 0.0


def detect_width(data, bpp, choices=(8, 16, 32, 64)):
    """Pick the sheet width whose tile seams line up best."""
    tile_bytes = 32 if bpp == 4 else 64
    n_tiles = len(data) // tile_bytes
    best = 16
    best_score = -1.0
    scores = {}
    for wt in choices:
        if n_tiles < wt:
            continue
        s = edge_match(data, bpp, wt)
        scores[wt] = s
        if s > best_score:
            best_score = s
            best = wt
    return best, scores


def palette_quality(rom, off):
    """Score a 32-byte block on how much it looks like a real 16-colour palette."""
    vals = [struct.unpack_from("<H", rom, off + i * 2)[0] for i in range(16)]
    if any(v & 0x8000 for v in vals):
        return 0.0                      # bit 15 is unused on the GBA
    uniq = len(set(vals))
    if uniq < 8:
        return 0.0
    cols = [gfx.bgr555_to_rgb(v) for v in vals]
    # real palettes hold a spread of brightness rather than one flat cluster
    lum = sorted((r * 2 + g * 3 + b) / 6.0 for r, g, b in cols)
    spread = lum[-1] - lum[0]
    if spread < 60:
        return 0.0
    # and they tend to ramp smoothly rather than jump at random
    steps = [lum[i + 1] - lum[i] for i in range(len(lum) - 1)]
    avg = sum(steps) / len(steps)
    jitter = sum(abs(s - avg) for s in steps) / len(steps)
    smooth = max(0.0, 1.0 - jitter / (avg + 1e-6) / 3.0)
    return (uniq / 16.0) * (spread / 255.0) * (0.5 + 0.5 * smooth)
