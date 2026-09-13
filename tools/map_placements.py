#!/usr/bin/env python3
"""Extract and build battle arena map-piece placement metadata."""
from pathlib import Path
import argparse
import json

ROOT = Path(__file__).resolve().parents[1]
BASE = 0x1BF9EC
SET_COUNT = 4
PLACEMENT_COUNT = 21
SET_SIZE = 968
HEADER_SIZE = 44
RECORD_SIZE = 44
METADATA = ROOT / "data" / "map_placements.json"
OUTPUT = ROOT / "build" / "generated" / "map_placements.inc"
POSITION_OUTPUT = ROOT / "build" / "generated" / "map_screen_positions.inc"
NEIGHBOR_OUTPUT = ROOT / "build" / "generated" / "map_neighbor_indices.inc"
MASK_OUTPUT = ROOT / "build" / "generated" / "map_visibility_masks.inc"
POSITION_BASE = 0x1BF408
POSITION_COUNT = 84
NEIGHBOR_BASE = 0x1BF7F8
NEIGHBOR_ROWS = 28
MASK_BASE = 0x1BF9B8
MASK_COUNT = 13


def fixed_ascii(raw, label):
    value = raw.rstrip(b"\0")
    if any(byte < 0x20 or byte >= 0x7F for byte in value):
        raise ValueError(f"{label}: non-ASCII name")
    if raw != value + bytes(len(raw) - len(value)):
        raise ValueError(f"{label}: embedded NUL")
    return value.decode("ascii")


def extract(rom_path):
    rom = Path(rom_path).read_bytes()
    layouts = []
    for layout_index in range(SET_COUNT):
        start = BASE + layout_index * SET_SIZE
        raw = rom[start:start + SET_SIZE]
        count = int.from_bytes(raw[:2], "little")
        if count != PLACEMENT_COUNT:
            raise ValueError(f"layout {layout_index}: unexpected count {count}")
        placements = []
        for index in range(count):
            record = raw[HEADER_SIZE + index * RECORD_SIZE:
                         HEADER_SIZE + (index + 1) * RECORD_SIZE]
            placements.append({
                "index": record[0],
                "kind": record[1],
                "resource_name": fixed_ascii(record[2:20], f"layout {layout_index} record {index}"),
                "x": int.from_bytes(record[20:24], "little"),
                "y": int.from_bytes(record[24:28], "little"),
                "reserved": [int.from_bytes(record[pos:pos + 4], "little")
                             for pos in range(28, 44, 4)],
            })
        layouts.append({
            "index": layout_index,
            "origin_name": fixed_ascii(raw[2:HEADER_SIZE], f"layout {layout_index}"),
            "placements": placements,
        })
    positions = []
    for index in range(POSITION_COUNT):
        start = POSITION_BASE + index * 12
        values = [int.from_bytes(rom[start + pos:start + pos + 2], "little")
                  for pos in range(0, 12, 2)]
        positions.append(dict(zip(
            ("arena", "index", "x", "y", "field_08", "flags"), values)))
    neighbors = []
    for row in range(NEIGHBOR_ROWS):
        start = NEIGHBOR_BASE + row * 16
        values = []
        for pos in range(0, 16, 2):
            value = int.from_bytes(rom[start + pos:start + pos + 2], "little")
            values.append(-1 if value == 0xFFFF else value)
        neighbors.append(values)
    masks = [int.from_bytes(rom[MASK_BASE + index * 4:
                                MASK_BASE + index * 4 + 4], "little")
             for index in range(MASK_COUNT)]
    METADATA.write_text(json.dumps({
        "format": "MAR battle arena initial placements",
        "rom_offset": BASE,
        "screen_positions": positions,
        "neighbor_indices": neighbors,
        "visibility_masks": masks,
        "layouts": layouts,
    }, indent=2) + "\n")
    print(f"extracted {SET_COUNT} battle arena layouts")


def c_string(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def build():
    payload = json.loads(METADATA.read_text())
    layouts = payload["layouts"]
    if payload.get("rom_offset") != BASE or len(layouts) != SET_COUNT:
        raise ValueError("battle arena layout metadata mismatch")
    lines = []
    for layout_index, layout in enumerate(layouts):
        placements = layout["placements"]
        if layout.get("index") != layout_index or len(placements) != PLACEMENT_COUNT:
            raise ValueError(f"layout {layout_index}: invalid index/count")
        origin = layout["origin_name"]
        if len(origin.encode("ascii")) > 42:
            raise ValueError(f"layout {layout_index}: origin name too long")
        lines.append(f"    {{ {PLACEMENT_COUNT}, {c_string(origin)}, {{")
        for record_index, record in enumerate(placements):
            name = record["resource_name"]
            if len(name.encode("ascii")) > 18 or len(record["reserved"]) != 4:
                raise ValueError(f"layout {layout_index} record {record_index}: invalid record")
            reserved = ", ".join(f"0x{value:08X}" for value in record["reserved"])
            lines.append(
                f"        {{ {record['index']}, {record['kind']}, {c_string(name)}, "
                f"{record['x']}, {record['y']}, {{ {reserved} }} }},")
        lines.append("    } },")
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text("\n".join(lines) + "\n")
    positions = payload["screen_positions"]
    if len(positions) != POSITION_COUNT:
        raise ValueError("invalid screen-position count")
    POSITION_OUTPUT.write_text("\n".join(
        "    { %(arena)d, %(index)d, %(x)d, %(y)d, %(field_08)d, %(flags)d }," % record
        for record in positions) + "\n")
    neighbors = payload["neighbor_indices"]
    if len(neighbors) != NEIGHBOR_ROWS or any(len(row) != 8 for row in neighbors):
        raise ValueError("invalid neighbor table dimensions")
    NEIGHBOR_OUTPUT.write_text("\n".join(
        "    { " + ", ".join(str(value) for value in row) + " },"
        for row in neighbors) + "\n")
    masks = payload["visibility_masks"]
    if len(masks) != MASK_COUNT:
        raise ValueError("invalid visibility-mask count")
    MASK_OUTPUT.write_text("\n".join(
        f"    0x{value:08X}," for value in masks) + "\n")
    print(f"built {SET_COUNT} battle arena layouts")


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    extract_parser = sub.add_parser("extract")
    extract_parser.add_argument("rom")
    sub.add_parser("build")
    args = parser.parse_args()
    extract(args.rom) if args.command == "extract" else build()


if __name__ == "__main__":
    main()
