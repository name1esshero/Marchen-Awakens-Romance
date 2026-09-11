#!/usr/bin/env python3
"""Verify a built ROM against the original, reporting where they diverge."""
import hashlib
import sys


def main():
    base = open(sys.argv[1], "rb").read()
    built = open(sys.argv[2], "rb").read()

    hb = hashlib.sha1(base).hexdigest()
    hn = hashlib.sha1(built).hexdigest()
    print("base  : %s  (%d bytes)" % (hb, len(base)))
    print("built : %s  (%d bytes)" % (hn, len(built)))

    if base == built:
        print("\nOK: the build matches the original ROM exactly.")
        return 0

    if len(base) != len(built):
        print("\nSIZE MISMATCH: %+d bytes" % (len(built) - len(base)))

    n = min(len(base), len(built))
    diffs = []
    i = 0
    while i < n:
        if base[i] != built[i]:
            start = i
            while i < n and base[i] != built[i]:
                i += 1
            diffs.append((start, i))
        else:
            i += 1

    total = sum(e - s for s, e in diffs)
    print("\n%d differing region(s), %d bytes (%.4f%% of the image)"
          % (len(diffs), total, 100.0 * total / n))
    for s, e in diffs[:20]:
        print("  %06X..%06X  (%d bytes)" % (s, e, e - s))
        print("    base  %s" % base[s:min(e, s + 16)].hex(" "))
        print("    built %s" % built[s:min(e, s + 16)].hex(" "))
    if len(diffs) > 20:
        print("  ... %d more" % (len(diffs) - 20))
    return 1


if __name__ == "__main__":
    sys.exit(main())
