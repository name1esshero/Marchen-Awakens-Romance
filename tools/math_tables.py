#!/usr/bin/env python3
"""Extract and build the fixed-point renderer lookup tables."""
from pathlib import Path
import argparse
import json
import struct

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "data" / "math_tables.json"
GENERATED = ROOT / "build" / "generated"

TABLES = (
    ("oam_attribute_masks", 0xF28410, 16, "u8"),
    ("oam_half_dimensions", 0xF28420, 32, "u8"),
    ("oam_dimensions", 0xF28440, 32, "u8"),
    ("sine_8_8", 0xF28460, 512, "s16"),
    ("sine_2_14", 0xF28860, 4096, "s16"),
)


def decode(rom, offset, count, kind):
    if kind == "u8":
        return list(rom[offset:offset + count])
    return list(struct.unpack_from("<" + "h" * count, rom, offset))


def extract(rom_path):
    rom = Path(rom_path).read_bytes()
    payload = {
        "format": "MAR renderer lookup tables",
        "notes": {
            "oam_dimensions": "Interleaved width/height pairs for OBJ shape and size combinations.",
            "sine_8_8": "Two identical 256-entry periods, scaled by 256.",
            "sine_2_14": "One 4096-entry turn, scaled by 16384.",
        },
        "tables": {},
    }
    for name, offset, count, kind in TABLES:
        payload["tables"][name] = {
            "rom_offset": offset,
            "type": kind,
            "values": decode(rom, offset, count, kind),
        }
    SOURCE.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"extracted {len(TABLES)} renderer lookup tables")


def emit(values, kind):
    per_line = 16 if kind == "u8" else 12
    lines = []
    for start in range(0, len(values), per_line):
        row = values[start:start + per_line]
        if kind == "u8":
            text = ", ".join(f"0x{value:02X}" for value in row)
        else:
            text = ", ".join(str(value) for value in row)
        lines.append("    " + text + ",")
    return "\n".join(lines) + "\n"


def build():
    payload = json.loads(SOURCE.read_text())
    GENERATED.mkdir(parents=True, exist_ok=True)
    for name, offset, count, kind in TABLES:
        table = payload["tables"][name]
        values = table["values"]
        if table["rom_offset"] != offset or table["type"] != kind:
            raise ValueError(f"{name}: metadata does not match its ROM layout")
        if len(values) != count:
            raise ValueError(f"{name}: expected {count} values, got {len(values)}")
        low, high = ((0, 255) if kind == "u8" else (-32768, 32767))
        if any(not isinstance(value, int) or value < low or value > high
               for value in values):
            raise ValueError(f"{name}: value outside {kind} range")
        (GENERATED / f"{name}.inc").write_text(emit(values, kind))
    print(f"built {len(TABLES)} renderer lookup table initializers")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("extract", "build"))
    parser.add_argument("rom", nargs="?", default="baserom.gba")
    args = parser.parse_args()
    extract(args.rom) if args.command == "extract" else build()


if __name__ == "__main__":
    main()
