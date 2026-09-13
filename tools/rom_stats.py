#!/usr/bin/env python3
"""Print linked GBA ROM and static RAM usage from an ELF image."""

import argparse
import re
import subprocess
from pathlib import Path

try:
    from tools.ram_layout import ROOT, audit_layout
except ImportError:  # Direct execution places tools/ itself on sys.path.
    from ram_layout import ROOT, audit_layout


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


def rom_occupancy(path, capacity, minimum_run=32):
    """Count reusable erased/padding runs, not the padded file length.

    Isolated 00/FF values are ordinary data.  Runs of at least 32 identical
    fill bytes are reported as free, as is address space beyond the image.
    """
    data=path.read_bytes()
    if len(data)>capacity:
        raise ValueError('ROM image exceeds the GBA cartridge address space')
    pattern=rb'\x00{%d,}|\xff{%d,}' % (minimum_run,minimum_run)
    free=capacity-len(data)+sum(match.end()-match.start()
                                for match in re.finditer(pattern,data))
    return capacity-free,free


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("--rom", type=Path)
    parser.add_argument("--objdump", default="arm-none-eabi-objdump")
    args = parser.parse_args()

    sections = allocated_sections(args.elf, args.objdump)
    ram_report = audit_layout(ROOT / "ram_layout.json")
    print("GBA linked memory usage")
    for name, start, capacity in REGIONS:
        end = start + capacity
        linked = sum(size for _, address, size in sections
                     if start <= address < end)
        used = linked
        free=None
        if name == "ROM" and args.rom:
            used,free=rom_occupancy(args.rom,capacity)
        elif name in ram_report["totals"]:
            item = ram_report["totals"][name]
            free = item["kinds"].get("unassigned", 0)
            used = capacity - free
        percent = used * 100.0 / capacity
        suffix=f"; {format_size(free)} free" if free is not None else ""
        verb = "used" if name == "ROM" else "accounted"
        print(f"  {name:5} {format_size(used):>18} {verb} / "
              f"{format_size(capacity):>18} total  ({percent:6.2f}%{suffix})")
        if name in ram_report["totals"]:
            kinds = ram_report["totals"][name]["kinds"]
            details = ", ".join(f"{kind} {format_size(size)}"
                                for kind, size in kinds.items())
            print(f"        {details}; ELF-linked subset {format_size(linked)}")
    print("  Heap arenas and stacks are reserved ranges; live high-water usage "
          "requires emulator/runtime telemetry.")


if __name__ == "__main__":
    main()
