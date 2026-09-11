#!/usr/bin/env python3
"""Pull the dialogue out of the game's SCRP bytecode.

Most of the script is not stored as loose strings in the ROM; it is embedded
in compiled script files. Each begins with "SCRP" and holds one "CODE" chunk,
and inside that chunk a string is introduced by opcode 0x10:

    10 <u16 length> <length bytes, Shift-JIS, NUL terminated>

The length counts the terminator. This is a candidate scan, not proof of
instruction boundaries or exhaustive extraction. A
run that does not end exactly where its length says is not a string and is
skipped rather than guessed at.

Writes text/scrp_<file>.txt in the same format as the other script files, as legacy reference views. The build instead uses text/nfp/*.SPC.txt.
"""
import glob
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import text_codec as tc

OP_STRING = 0x10


def strings_in(chunk, lossless=False):
    """Yield (offset within chunk, raw bytes) for every 0x10 string."""
    out = []
    i = 0
    n = len(chunk)
    while i < n - 3:
        if chunk[i] != OP_STRING:
            i += 1
            continue
        ln = struct.unpack_from("<H", chunk, i + 1)[0]
        if not (2 <= ln <= 0x400) or i + 3 + ln > n:
            i += 1
            continue
        raw = chunk[i + 3:i + 3 + ln]
        # the length must cover exactly one NUL-terminated string
        if raw[-1] != 0 or raw.count(0) != 1:
            i += 1
            continue
        body = raw[:-1]
        if body and (lossless or tc.roundtrips(body)):
            out.append((i, body))
            i += 3 + ln
            continue
        i += 1
    return out


def main():
    os.makedirs("text", exist_ok=True)
    total = files = 0
    for path in sorted(glob.glob("scripts/*.scrp")):
        data = open(path, "rb").read()
        if data[:4] != b"SCRP" or data[8:12] != b"CODE":
            continue
        clen = struct.unpack_from("<I", data, 12)[0]
        chunk = data[16:16 + clen]
        found = strings_in(chunk)
        if not found:
            continue
        name = os.path.basename(path).replace(".scrp", "")
        out = os.path.join("text", "scrp_%s.txt" % name)
        with open(out, "w", encoding="utf-8") as f:
            f.write("// Dialogue from %s\n" % path)
            f.write("// @<offset within the CODE chunk> <text>\n")
            f.write("// Anything after // is a comment and never reaches "
                    "the ROM.\n\n")
            for off, raw in found:
                f.write("@%06X %s\n" % (off, tc.decode(raw)))
        total += len(found)
        files += 1
    print("extracted %d strings from %d script files" % (total, files))


if __name__ == "__main__":
    main()
