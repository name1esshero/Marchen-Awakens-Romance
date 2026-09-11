"""GBA BIOS-compatible LZ77 codec (compression type 0x10).

Stream layout:
    byte 0      : 0x10  (LZ77, 8-bit units)
    bytes 1..3  : uncompressed length, little endian
    then groups : 1 flag byte, MSB first, describing the next 8 tokens
                    flag bit 0 -> one literal byte
                    flag bit 1 -> 2 bytes, big endian:
                                    bits 15..12 = length - 3   (3..18)
                                    bits 11..0  = displacement - 1
                                  copy `length` bytes from out[-(disp+1)]
"""

MIN_MATCH = 3
MAX_MATCH = 18
MAX_DISP = 0x1000


def decompress(data, offset=0):
    """Decode an LZ77 stream. Returns (bytes, compressed_length)."""
    if data[offset] != 0x10:
        raise ValueError("not an LZ77 stream (header 0x%02X)" % data[offset])
    size = data[offset + 1] | (data[offset + 2] << 8) | (data[offset + 3] << 16)
    src = offset + 4
    out = bytearray()
    while len(out) < size:
        if src >= len(data):
            raise ValueError("truncated LZ77 stream")
        flags = data[src]
        src += 1
        for bit in range(7, -1, -1):
            if len(out) >= size:
                break
            if flags & (1 << bit):
                if src + 1 >= len(data):
                    raise ValueError("truncated LZ77 match")
                b0, b1 = data[src], data[src + 1]
                src += 2
                length = (b0 >> 4) + MIN_MATCH
                disp = (((b0 & 0x0F) << 8) | b1) + 1
                if disp > len(out):
                    raise ValueError("LZ77 displacement past start of output")
                for _ in range(length):
                    out.append(out[len(out) - disp])
            else:
                out.append(data[src])
                src += 1
    return bytes(out[:size]), src - offset


def _find_match(data, pos, end, min_disp=1):
    """Longest match for data[pos:] within the 4K sliding window."""
    best_len = 0
    best_disp = 0
    start = pos - MAX_DISP
    if start < 0:
        start = 0
    limit = end - pos
    if limit > MAX_MATCH:
        limit = MAX_MATCH
    if limit < MIN_MATCH:
        return 0, 0
    # Search nearest-first so ties keep the smallest displacement, which is
    # the tie-breaking observed in this ROM's graphics streams.
    # Search three-byte prefixes in C, rather than checking all 4096 byte
    # positions in Python. Visit candidates nearest-first to retain the same
    # longest-match/tie policy, including legal overlapping matches.
    prefix = data[pos:pos + MIN_MATCH]
    search_end = pos - min_disp + MIN_MATCH
    cand = data.rfind(prefix, start, search_end)
    while cand >= start:
        n = MIN_MATCH
        while n < limit and data[cand + n] == data[pos + n]:
            n += 1
        if n > best_len:
            best_len = n
            best_disp = pos - cand
            if n == limit:
                break
        cand = data.rfind(prefix, start, cand + MIN_MATCH - 1)
    if best_len < MIN_MATCH:
        return 0, 0
    return best_len, best_disp


def compress(data, vram_safe=True):
    """Greedy LZ77 encoder.

    vram_safe excludes distance-one matches: the VRAM decoder writes
    halfwords and cannot read the preceding byte before that write completes.
    The encoded displacement field stores distance minus one.
    """
    size = len(data)
    out = bytearray()
    out.append(0x10)
    out += bytes(((size) & 0xFF, (size >> 8) & 0xFF, (size >> 16) & 0xFF))

    pos = 0
    while pos < size:
        flag_index = len(out)
        out.append(0)
        flags = 0
        for bit in range(7, -1, -1):
            if pos >= size:
                break
            search_pos = pos
            length, disp = _find_match(data, search_pos, size, 2 if vram_safe else 1)
            if length >= MIN_MATCH:
                flags |= 1 << bit
                d = disp - 1
                out.append(((length - MIN_MATCH) << 4) | ((d >> 8) & 0x0F))
                out.append(d & 0xFF)
                pos += length
            else:
                out.append(data[pos])
                pos += 1
        out[flag_index] = flags
    while len(out) % 4:
        out.append(0)
    return bytes(out)
