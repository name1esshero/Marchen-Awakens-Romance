#!/usr/bin/env python3
"""Print linked GBA ROM and static RAM usage from an ELF image."""

import argparse
import re
import subprocess
from pathlib import Path


REGIONS = (
    ("ROM", 0x08000000, 32 * 1024 * 1024),
    ("EWRAM", 0x02000000, 256 * 1024),
    ("IWRAM", 0x03000000, 32 * 1024),
)


def allocated_sections(elf, objdump):
    output = subprocess.check_output([objdump, "-h", str(elf)], text=True)
    rows = []
    lines = output.splitlines()
    for index, line in enumerate(lines):
        match = re.match(
            r"\s*\d+\s+(\S+)\s+([0-9A-Fa-f]+)\s+"
            r"([0-9A-Fa-f]+)\s+([0-9A-Fa-f]+)", line)
        if not match:
            continue
        flags = lines[index + 1] if index + 1 < len(lines) else ""
        if "ALLOC" not in flags:
            continue
        name, size, vma, _ = match.groups()
        rows.append((name, int(vma, 16), int(size, 16)))
    return rows


def format_size(value):
    return f"{value:,} bytes"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("--rom", type=Path)
    parser.add_argument("--objdump", default="arm-none-eabi-objdump")
    args = parser.parse_args()

    sections = allocated_sections(args.elf, args.objdump)
    print("GBA linked memory usage")
    for name, start, capacity in REGIONS:
        end = start + capacity
        used = sum(size for _, address, size in sections
                   if start <= address < end)
        if name == "ROM" and args.rom:
            used = args.rom.stat().st_size
        percent = used * 100.0 / capacity
        print(f"  {name:5} {format_size(used):>18} / "
              f"{format_size(capacity):>18}  ({percent:6.2f}%)")
    print("  RAM figures count linked static sections; runtime heaps and stacks "
          "are allocated by the game.")


if __name__ == "__main__":
    main()
