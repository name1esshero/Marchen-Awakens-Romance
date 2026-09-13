#!/usr/bin/env python3
"""Pull the ROM's Shift-JIS script into editable files under text/.

Only strings that provably re-encode to their original bytes are exported as
text; anything else stays inside the binary data sections, so the default
build remains byte-exact.
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import text_codec as tc
import definition_text

MIN_CHARS = 4          # ignore very short runs; they are usually not script
TEXT_DIR = "text"


def load_excludes():
    """Ranges that are not text: compressed assets, uncompressed art, sound.

    Scanning these produces false positives, and a "string" that runs off the
    end of a data region into artwork will not fit when rebuilt.
    """
    r = []
    try:
        with open("assets.json") as source:
            assets = json.load(source)
        for m in assets:
            r.append((m["rom_offset"],
                      m["rom_offset"] + m["compressed_size"]))
    except OSError:
        pass
    try:
        with open("assets_raw.json") as source:
            assets = json.load(source)
        for m in assets:
            r.append((m["rom_offset"], m["rom_offset"] + m["raw_size"]))
    except OSError:
        pass
    # Fixed-layout definition strings have explicit field boundaries and their
    # own editable sources. A heuristic scan must never absorb or drop them.
    r.extend(definition_text.ranges())
    r.sort()
    return r


def excluded(ranges, off):
    import bisect
    starts = [a for a, _ in ranges]
    i = bisect.bisect_right(starts, off) - 1
    return i >= 0 and off < ranges[i][1]


def scan(rom, start, end, ranges=()):
    """Find NUL-terminated Shift-JIS strings in [start, end)."""
    import bisect
    starts = [a for a, _ in ranges]

    def skip(off):
        i = bisect.bisect_right(starts, off) - 1
        return ranges[i][1] if (i >= 0 and off < ranges[i][1]) else None

    def limit(off):
        """Where a string starting at off must stop: the next excluded range."""
        i = bisect.bisect_right(starts, off)
        return ranges[i][0] if i < len(ranges) else end

    found = []
    i = start
    while i < end:
        nxt = skip(i)
        if nxt is not None:
            i = nxt
            continue
        if rom[i] == 0 or rom[i] < tc.CTRL_MAX:
            i += 1
            continue
        stop = min(end, limit(i))
        j = i
        while j < stop and rom[j] != 0:
            j += 1
        if j >= stop:
            # ran into a non-text region without finding a terminator
            i = stop
            continue
        raw = rom[i:j]
        if len(raw) >= MIN_CHARS and tc.roundtrips(raw):
            txt = tc.decode(raw)
            # require some real Japanese or a decent ASCII run
            jp = sum(1 for c in txt if ord(c) > 0x2000)
            kana = sum(1 for c in txt if 0x3040 <= ord(c) <= 0x30FF)
            ascii_only = jp == 0 and all(0x20 <= ord(c) < 0x7F for c in txt)
            # real script is mostly Japanese and usually contains kana
            if (jp >= 3 and kana >= 1 and jp * 2 >= len(txt)) \
                    or (ascii_only and len(txt) >= 8):
                found.append((i, raw, txt))
                i = j + 1
                continue
        i = j + 1
    return found


def main():
    rom = open(sys.argv[1], "rb").read()
    start = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0x1B0000
    end = int(sys.argv[3], 0) if len(sys.argv) > 3 else len(rom)

    os.makedirs(TEXT_DIR, exist_ok=True)
    strings = scan(rom, start, end, load_excludes())
    print("recovered %d strings" % len(strings))

    # Split by content: Japanese dialogue is the script, while the ASCII runs
    # are mostly internal identifiers (sound sequence names and the like).
    # Both are editable, but conflating them would be misleading.
    banks = {}
    for off, raw, txt in strings:
        jp = any(ord(c) > 0x2000 for c in txt)
        kind = "script" if jp else "strings"
        banks.setdefault((kind, off >> 16), []).append((off, raw, txt))

    index = []
    for table in definition_text.TABLES:
        records = definition_text.source_records(table)
        index.append({"kind": table["kind"], "bank": "%06X" % table["base"],
                      "path": table["path"], "count": len(records),
                      "bytes": sum(len(tc.encode(text)) for text, _ in records.values()),
                      "record_size": table["stride"], "records": table["records"]})
    for (kind, bank), items in sorted(banks.items()):
        path = os.path.join(TEXT_DIR, "%s_%06X.txt" % (kind, bank << 16))
        with open(path, "w", encoding="utf-8") as f:
            f.write("// MAR %s block %06X\n" % (kind, bank << 16))
            f.write("// One record per line: @<rom offset> <text>\n")
            f.write("// <XX> is a raw control byte. Text is re-encoded to "
                    "Shift-JIS at build time;\n// a line may not grow past "
                    "its original byte length.\n")
            f.write("// Anything after // is a comment and never reaches "
                    "the ROM.\n\n")
            for off, raw, txt in items:
                f.write("@%06X %s\n" % (off, txt))
        index.append({"kind": kind, "bank": "%06X" % (bank << 16),
                      "path": path.replace(os.sep, "/"),
                      "count": len(items),
                      "bytes": sum(len(r) for _, r, _ in items)})
    with open(os.path.join(TEXT_DIR, "text.json"), "w") as f:
        json.dump(index, f, indent=1)
    from collections import Counter
    counts = Counter(k for (k, _) in banks)
    recs = Counter()
    for (k, _), items in banks.items():
        recs[k] += len(items)
    for k in sorted(counts):
        print("  text/%s_*.txt  %d files, %d records"
              % (k, counts[k], recs[k]))


if __name__ == "__main__":
    main()
