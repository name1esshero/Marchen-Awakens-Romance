"""GBA BIOS run-length codec (compression type 0x30).

    byte 0      : 0x30
    bytes 1..3  : uncompressed length, little endian
    then blocks : one flag byte
                    bit 7 set  -> run:     length = (flag & 0x7F) + 3,
                                           next byte repeated that many times
                    bit 7 clear-> literal: length = (flag & 0x7F) + 1,
                                           that many bytes follow verbatim

The game reaches this through the RLUnCompWram wrapper at 0x08079EB8, so
assets in this format exist alongside the LZ77 ones.
"""


def decompress(data, offset=0):
    if data[offset] != 0x30:
        raise ValueError("not an RLE stream (header 0x%02X)" % data[offset])
    size = data[offset + 1] | (data[offset + 2] << 8) | (data[offset + 3] << 16)
    src = offset + 4
    out = bytearray()
    while len(out) < size:
        if src >= len(data):
            raise ValueError("truncated RLE stream")
        flag = data[src]
        src += 1
        if flag & 0x80:
            n = (flag & 0x7F) + 3
            if src >= len(data):
                raise ValueError("truncated RLE run")
            out += bytes([data[src]]) * n
            src += 1
        else:
            n = (flag & 0x7F) + 1
            if src + n > len(data):
                raise ValueError("truncated RLE literal")
            out += data[src:src + n]
            src += n
    return bytes(out[:size]), src - offset


def compress(data):
    out = bytearray([0x30, len(data) & 0xFF,
                     (len(data) >> 8) & 0xFF, (len(data) >> 16) & 0xFF])
    i = 0
    n = len(data)
    while i < n:
        run = 1
        while run < 130 and i + run < n and data[i + run] == data[i]:
            run += 1
        if run >= 3:
            out.append(0x80 | (run - 3))
            out.append(data[i])
            i += run
            continue
        start = i
        while i < n and len(data) - i > 0:
            r = 1
            while r < 3 and i + r < n and data[i + r] == data[i]:
                r += 1
            if r >= 3 or i - start >= 128:
                break
            i += 1
        ln = i - start
        out.append(ln - 1)
        out += data[start:i]
    while len(out) % 4:
        out.append(0)
    return bytes(out)
