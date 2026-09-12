"""GBA graphics conversion: tiles <-> indexed PNG, BGR555 palettes.

Pure standard library (zlib only) so the build has no third-party
dependencies. GBA character data is 8x8 tiles stored in sequence; 4bpp tiles
pack two pixels per byte, low nibble first.
"""
import struct
import zlib

TILE_W = TILE_H = 8


# ---------------------------------------------------------------- palettes

def bgr555_to_rgb(h):
    """Unpack a GBA colour. 5 bits per channel, blue in the high bits."""
    r = (h & 0x1F) << 3
    g = ((h >> 5) & 0x1F) << 3
    b = ((h >> 10) & 0x1F) << 3
    # replicate the top bits into the low ones so white reaches 0xFF
    return (r | r >> 5, g | g >> 5, b | b >> 5)


def rgb_to_bgr555(r, g, b):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def read_palette(data, offset=0, count=16):
    return [bgr555_to_rgb(struct.unpack_from("<H", data, offset + i * 2)[0])
            for i in range(count)]


def write_jasc(path, colors):
    """JASC-PAL is what most tile editors expect."""
    with open(path, "w") as f:
        f.write("JASC-PAL\n0100\n%d\n" % len(colors))
        for r, g, b in colors:
            f.write("%d %d %d\n" % (r, g, b))


def read_jasc(path):
    with open(path) as stream:
        lines = stream.read().split("\n")
    n = int(lines[2])
    out = []
    for line in lines[3:3 + n]:
        r, g, b = (int(x) for x in line.split())
        out.append((r, g, b))
    return out


def palette_to_bytes(colors):
    out = bytearray()
    for r, g, b in colors:
        out += struct.pack("<H", rgb_to_bgr555(r, g, b))
    return bytes(out)


# ------------------------------------------------------------------- tiles

def tiles_to_pixels(data, bpp, width_tiles):
    """Decode packed tile data into a 2-D list of palette indices."""
    tile_bytes = 32 if bpp == 4 else 64
    n_tiles = len(data) // tile_bytes
    if n_tiles == 0:
        return []
    height_tiles = (n_tiles + width_tiles - 1) // width_tiles
    w = width_tiles * TILE_W
    h = height_tiles * TILE_H
    px = [[0] * w for _ in range(h)]
    for t in range(n_tiles):
        tx = (t % width_tiles) * TILE_W
        ty = (t // width_tiles) * TILE_H
        base = t * tile_bytes
        for y in range(TILE_H):
            if bpp == 4:
                row = base + y * 4
                for x in range(0, TILE_W, 2):
                    b = data[row + x // 2]
                    px[ty + y][tx + x] = b & 0xF
                    px[ty + y][tx + x + 1] = b >> 4
            else:
                row = base + y * 8
                for x in range(TILE_W):
                    px[ty + y][tx + x] = data[row + x]
    return px


def pixels_to_tiles(px, bpp):
    """Re-pack a pixel grid into GBA tile order."""
    h = len(px)
    w = len(px[0]) if h else 0
    out = bytearray()
    for ty in range(h // TILE_H):
        for tx in range(w // TILE_W):
            for y in range(TILE_H):
                row = px[ty * TILE_H + y]
                if bpp == 4:
                    for x in range(0, TILE_W, 2):
                        lo = row[tx * TILE_W + x] & 0xF
                        hi = row[tx * TILE_W + x + 1] & 0xF
                        out.append(lo | (hi << 4))
                else:
                    for x in range(TILE_W):
                        out.append(row[tx * TILE_W + x] & 0xFF)
    return bytes(out)


# --------------------------------------------------------------------- PNG

def _chunk(tag, payload):
    return (struct.pack(">I", len(payload)) + tag + payload +
            struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF))


def write_png(path, px, palette, transparent_index=None, transparent_indices=None):
    """Write an 8-bit indexed PNG (widely editable, keeps indices intact)."""
    h = len(px)
    w = len(px[0]) if h else 0
    raw = bytearray()
    for row in px:
        raw.append(0)                    # filter type 0 (None)
        raw += bytes(row)
    pal = bytearray()
    for r, g, b in palette:
        pal += bytes((r, g, b))
    while len(pal) < 3 * 256:
        pal += b"\0\0\0"
    out = b"\x89PNG\r\n\x1a\n"
    out += _chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 3, 0, 0, 0))
    out += _chunk(b"PLTE", bytes(pal))
    if transparent_index is not None and transparent_indices is not None:
        raise ValueError('Specify one transparency option')
    indices = list(transparent_indices) if transparent_indices is not None else ([] if transparent_index is None else [transparent_index])
    if indices:
        if any(not isinstance(i,int) or not 0 <= i < 256 for i in indices):
            raise ValueError('Transparent palette index outside 0..255')
        alpha = bytearray([255] * (max(indices)+1))
        for index in indices:alpha[index]=0
        out += _chunk(b"tRNS", bytes(alpha))
    out += _chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    out += _chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(out)


def read_png(path):
    """Read back an 8-bit indexed PNG written by write_png (or an editor)."""
    with open(path, "rb") as stream:
        data = stream.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("%s is not a PNG" % path)
    pos = 8
    w = h = depth = ctype = None
    idat = bytearray()
    palette = []
    while pos < len(data):
        ln = struct.unpack_from(">I", data, pos)[0]
        tag = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + ln]
        pos += 12 + ln
        if tag == b"IHDR":
            w, h, depth, ctype = struct.unpack(">IIBB", body[:10])
        elif tag == b"PLTE":
            palette = [tuple(body[i:i + 3]) for i in range(0, len(body), 3)]
        elif tag == b"IDAT":
            idat += body
        elif tag == b"IEND":
            break
    if ctype != 3 or depth != 8:
        raise ValueError("%s must be 8-bit indexed colour" % path)
    raw = zlib.decompress(bytes(idat))
    px = []
    stride = w
    prev = bytearray(stride)
    p = 0
    for _ in range(h):
        ft = raw[p]
        p += 1
        line = bytearray(raw[p:p + stride])
        p += stride
        if ft == 1:
            for i in range(1, stride):
                line[i] = (line[i] + line[i - 1]) & 0xFF
        elif ft == 2:
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 0xFF
        elif ft == 3:
            for i in range(stride):
                a = line[i - 1] if i else 0
                line[i] = (line[i] + ((a + prev[i]) >> 1)) & 0xFF
        elif ft == 4:
            for i in range(stride):
                a = line[i - 1] if i else 0
                c = prev[i - 1] if i else 0
                b = prev[i]
                pp = a + b - c
                pa, pb, pc = abs(pp - a), abs(pp - b), abs(pp - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 0xFF
        px.append(list(line))
        prev = line
    return px, palette
