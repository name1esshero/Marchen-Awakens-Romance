#!/usr/bin/env python3
"""Apply the editable scripts in text/ onto the extracted data blobs.

data/*.bin holds the bytes exactly as they came out of the ROM. This copies
them to build/data/ and overwrites each string with the re-encoded contents of
the matching record in text/, so edits reach the ROM while untouched records
reproduce their original bytes.

A record may not grow past the space the original occupied; the build stops
rather than overrun whatever follows.
"""
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import text_codec as tc

SRC_DIR = "data"
OUT_DIR = "build/data"
REC = re.compile(r"^@([0-9A-Fa-f]+)\s(.*)$")
# Everything from '//' to end of line is a note for human readers and never
# reaches the ROM, matching C comment syntax. '//' does not occur in this
# ROM's text, so nothing has to be escaped to use it.
COMMENT = re.compile(r"\s*//.*$")


def load_records():
    """Every script record, keyed by ROM offset."""
    records = {}
    for path in sorted(glob.glob("text/*.txt")):
        if os.path.basename(path).startswith("scrp_"):
            # These legacy records use CODE-local offsets, not ROM addresses.
            # Named replacements live in text/nfp and use named_scripts.py.
            continue
        with open(path, encoding="utf-8") as f:
            for lineno, line in enumerate(f, 1):
                line = line.rstrip("\n")
                if not line or line.startswith("//"):
                    continue
                m = REC.match(line)
                if not m:
                    raise SystemExit("%s:%d: malformed record" % (path, lineno))
                body = COMMENT.sub("", m.group(2))
                records[int(m.group(1), 16)] = (tc.encode(body), path, lineno)
    return records


def main():
    records = load_records()
    os.makedirs(OUT_DIR, exist_ok=True)

    applied = 0
    for src in sorted(glob.glob(os.path.join(SRC_DIR, "*.bin"))):
        name = os.path.basename(src)
        m = re.match(r"data_([0-9A-F]+)\.bin$", name)
        data = bytearray(open(src, "rb").read())
        if m:
            base = int(m.group(1), 16)
            for off, (encoded, path, lineno) in records.items():
                if not (base <= off < base + len(data)):
                    continue
                pos = off - base
                end = data.find(b"\0", pos)
                if end < 0:
                    end = len(data)
                room = end - pos
                if len(encoded) > room:
                    raise SystemExit(
                        "%s:%d: text is %d bytes but only %d fit at %06X"
                        % (path, lineno, len(encoded), room, off))
                data[pos:pos + len(encoded)] = encoded
                # clear any space freed by a shorter replacement
                for i in range(pos + len(encoded), end):
                    data[i] = 0
                applied += 1
        out = os.path.join(OUT_DIR, name)
        prev = None
        if os.path.exists(out):
            prev = open(out, "rb").read()
        if prev != bytes(data):
            with open(out, "wb") as f:
                f.write(data)
    print("applied %d script records" % applied)


if __name__ == "__main__":
    main()
