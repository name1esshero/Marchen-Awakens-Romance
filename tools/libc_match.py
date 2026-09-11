#!/usr/bin/env python3
"""Identify library routines in the ROM by matching agbcc's libc.

The cartridge was linked against the same C library agbcc ships, which the
strcmp at 0x0808280C proves: it is byte-identical, literal pool included.
So rather than reading each routine and guessing what it does, the library
itself says. Every .text section in libc.a is searched for in the image.

Most routines contain relocations: a bl to another function, or an address
literal the linker fills in. Those bytes cannot match, so they are masked out
and only the fixed bytes are compared. The search anchors on the longest run
of fixed bytes, then verifies every other fixed byte at that position.

A match is reported only when it is unique in the image and enough bytes are
actually compared, so a short routine that is mostly relocation is rejected
rather than guessed at.
"""
import json
import os
import subprocess
import sys
import tempfile

LIB = "tools/agbcc/lib/libc.a"
MIN_BYTES = 12          # ignore stubs too short to identify confidently
MIN_FIXED = 24          # fixed bytes that must actually be compared
ANCHOR = 8              # shortest usable anchor run


def sections(obj):
    """(name, bytes) for each non-empty executable section in an object."""
    out = []
    listing = subprocess.run(["arm-none-eabi-objdump", "-h", obj],
                             capture_output=True, text=True).stdout
    for line in listing.splitlines():
        parts = line.split()
        if len(parts) < 4 or not parts[0].isdigit():
            continue
        name, size = parts[1], int(parts[2], 16)
        if size < MIN_BYTES or not name.startswith(".text"):
            continue
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
            tmp = f.name
        subprocess.run(["arm-none-eabi-objcopy", "-O", "binary",
                        "--only-section=" + name, obj, tmp], check=True)
        data = open(tmp, "rb").read()
        os.unlink(tmp)
        if data:
            out.append((name, data))
    return out


def relocations(obj, section):
    """Byte ranges the linker rewrites, which therefore cannot be compared."""
    out = []
    text = subprocess.run(["arm-none-eabi-objdump", "-r", obj],
                          capture_output=True, text=True).stdout
    current = None
    for line in text.splitlines():
        if line.startswith("RELOCATION RECORDS FOR ["):
            current = line.split("[", 1)[1].split("]", 1)[0]
            continue
        parts = line.split()
        if current != section or len(parts) < 2 or not parts[1].startswith("R_ARM"):
            continue
        off = int(parts[0], 16)
        out.append((off, 4))        # both THM_CALL and ABS32 cover four bytes
    return out


def masked_find(rom, data, holes):
    """Locate data in rom, ignoring the relocated ranges. Returns all hits."""
    mask = bytearray(b"\x01") * len(data)
    for off, size in holes:
        for i in range(off, min(off + size, len(data))):
            mask[i] = 0
    fixed = sum(mask)
    if fixed < MIN_FIXED:
        return [], fixed

    # longest run of comparable bytes, used to anchor the scan
    best_start = best_len = run_start = run = 0
    for i, m in enumerate(mask):
        if m:
            if run == 0:
                run_start = i
            run += 1
            if run > best_len:
                best_len, best_start = run, run_start
        else:
            run = 0
    if best_len < ANCHOR:
        return [], fixed

    anchor = bytes(data[best_start:best_start + best_len])
    hits = []
    pos = rom.find(anchor)
    while pos >= 0:
        start = pos - best_start
        if start >= 0 and start + len(data) <= len(rom):
            if all(rom[start + i] == data[i] for i in range(len(data)) if mask[i]):
                hits.append(start)
        pos = rom.find(anchor, pos + 1)
    return hits, fixed


def symbols(obj):
    """Global function symbols defined by this object."""
    out = []
    nm = subprocess.run(["arm-none-eabi-nm", obj],
                        capture_output=True, text=True).stdout
    for line in nm.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in ("T", "t"):
            out.append(parts[2])
    return out


def main():
    rom = open(sys.argv[1] if len(sys.argv) > 1 else "baserom.gba", "rb").read()
    work = tempfile.mkdtemp()
    names = subprocess.run(["arm-none-eabi-ar", "t", LIB],
                           capture_output=True, text=True).stdout.split()
    subprocess.run(["arm-none-eabi-ar", "x", os.path.abspath(LIB)],
                   cwd=work, check=True)

    found = []
    for member in names:
        obj = os.path.join(work, member)
        if not os.path.exists(obj):
            continue
        syms = symbols(obj)
        for name, data in sections(obj):
            holes = relocations(obj, name)
            hits, fixed = masked_find(rom, data, holes)
            if len(hits) != 1:
                continue        # absent, or not uniquely identifying
            found.append((hits[0], len(data), member, syms))

    found.sort()
    print("library routines located in the ROM: %d" % len(found))

    # These are library code, not game code. Naming them makes the
    # disassembly readable at every call site; the routines themselves are
    # not decompiled here, since the library already is the source.
    # Two library objects can compile to identical code, in which case the
    # address does not identify which one the game linked. Report the
    # ambiguity rather than letting the last match silently win.
    claims = {}
    for pos, size, member, syms in found:
        name = next((s for s in syms if s != ".gcc2_compiled."), None)
        if name:
            claims.setdefault(0x08000000 + pos, []).append((name, member, size))

    names_found = {}
    for addr in sorted(claims):
        entries = claims[addr]
        if len(entries) > 1:
            print("   %08X  ambiguous: %s  (skipped)"
                  % (addr, ", ".join(n for n, _, _ in entries)))
            continue
        name, member, size = entries[0]
        names_found["%08X" % addr] = name
        print("   %08X  %5d bytes  %-16s %s" % (addr, size, member, name))

    existing = {}
    if os.path.exists("symbols.json"):
        existing = json.load(open("symbols.json"))
    existing.update(names_found)
    with open("symbols.json", "w") as f:
        json.dump(existing, f, indent=1, sort_keys=True)
    print("wrote symbols.json with %d names" % len(existing))
    return found


if __name__ == "__main__":
    main()
