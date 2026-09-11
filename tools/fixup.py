#!/usr/bin/env python3
"""Demote any instruction the assembler rejects to a raw literal.

Recursive descent occasionally walks into data that decodes as a plausible
but unassemblable instruction. Rather than guess, this assembles the
generated sources, maps every complaint back to the byte it came from, and
records that address so the next split emits it as `.2byte` instead.

Repeats until the whole code region assembles cleanly.
"""
import json
import os
import re
import subprocess
import sys

AS = os.environ.get("PREFIX", "arm-none-eabi-") + "as"
ASFLAGS = ["-mcpu=arm7tdmi", "-mthumb-interwork", "-I."]
ERR_RE = re.compile(r"^(.*?):(\d+): Error:", re.M)


def assemble(path):
    """Assemble one file; return the set of line numbers that failed."""
    p = subprocess.run([AS] + ASFLAGS + ["-o", "/dev/null", path],
                       capture_output=True, text=True)
    if p.returncode == 0:
        return set()
    return set(int(m.group(2)) for m in ERR_RE.finditer(p.stderr))


def instruction_starts():
    """Sorted addresses the layout considers instruction boundaries."""
    lay = json.load(open(".analysis/layout.json"))
    return sorted(int(a, 16) for a in lay["mode"])


def binary_pass(rom, force):
    """Build, diff against the original, and demote whatever did not match."""
    import bisect
    starts = instruction_starts()
    for round_no in range(1, 12):
        subprocess.run(["make", "-j", str(os.cpu_count() or 4)],
                       check=False, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        if not os.path.exists("mar.gba"):
            print("build produced no ROM", file=sys.stderr)
            return force, False
        base = open(rom, "rb").read()
        built = open("mar.gba", "rb").read()
        if base == built:
            print("binary round %d: exact match" % round_no)
            return force, True
        bad = set()
        n = min(len(base), len(built))
        for i in range(n):
            if base[i] != built[i]:
                a = 0x08000000 + i
                # blame the instruction that contains this byte
                k = bisect.bisect_right(starts, a) - 1
                bad.add("%08X" % (starts[k] if k >= 0 else (a & ~1)))
        before = len(force)
        force |= bad
        print("binary round %d: %d bytes differ -> %d addresses demoted "
              "(%d total)" % (round_no, sum(1 for i in range(n)
                                            if base[i] != built[i]),
                              len(force) - before, len(force)))
        if len(force) == before:
            print("no progress on binary diff", file=sys.stderr)
            return force, False
        with open(".analysis/force_literal.json", "w") as f:
            json.dump(sorted(force), f)
        subprocess.run([sys.executable, "tools/split.py", rom],
                       check=True, stdout=subprocess.DEVNULL)
    return force, False


def main():
    rom = sys.argv[1] if len(sys.argv) > 1 else "baserom.gba"
    force = set()
    if os.path.exists(".analysis/force_literal.json"):
        force = set(json.load(open(".analysis/force_literal.json")))

    for round_no in range(1, 9):
        linemap = json.load(open(".analysis/linemap.json"))
        failed_lines = 0
        new = set()
        for path, lm in sorted(linemap.items()):
            bad = assemble(path)
            if not bad:
                continue
            failed_lines += len(bad)
            for line in bad:
                addr = lm.get(str(line))
                if addr:
                    new.add(addr)
        if not failed_lines:
            print("round %d: all code assembles cleanly" % round_no)
            break
        before = len(force)
        force |= new
        print("round %d: %d assembler errors -> %d addresses demoted "
              "to literals (%d total)"
              % (round_no, failed_lines, len(force) - before, len(force)))
        if len(force) == before:
            print("no progress; stopping", file=sys.stderr)
            return 1
        with open(".analysis/force_literal.json", "w") as f:
            json.dump(sorted(force), f)
        subprocess.run([sys.executable, "tools/split.py", rom],
                       check=True, stdout=subprocess.DEVNULL)

    with open(".analysis/force_literal.json", "w") as f:
        json.dump(sorted(force), f)

    force, exact = binary_pass(rom, force)
    with open(".analysis/force_literal.json", "w") as f:
        json.dump(sorted(force), f)
    return 0 if exact else 1


if __name__ == "__main__":
    sys.exit(main())
