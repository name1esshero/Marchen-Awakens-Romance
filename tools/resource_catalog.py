#!/usr/bin/env python3
"""Extract and build the fixed-width master resource catalog."""
from pathlib import Path
import argparse
import json

ROOT = Path(__file__).resolve().parents[1]
BASE = 0x1C0960
COUNT = 830
STRIDE = 16
NAME_SIZE = 12
METADATA = ROOT / "data" / "resource_catalog.json"
OUTPUT = ROOT / "build" / "generated" / "resource_catalog.inc"


def extract(rom_path):
    rom = Path(rom_path).read_bytes()
    records = []
    for index in range(COUNT):
        record = rom[BASE + index * STRIDE:BASE + (index + 1) * STRIDE]
        raw_name = record[:NAME_SIZE]
        name = raw_name.rstrip(b"\0").decode("ascii")
        if b"\0" in raw_name[:len(name)]:
            raise ValueError(f"resource {index}: embedded NUL in name")
        records.append({
            "index": index,
            "name": name,
            "rom_offset": int.from_bytes(record[NAME_SIZE:], "little"),
        })
    payload = {
        "format": "MAR master resource catalog",
        "rom_offset": BASE,
        "record_size": STRIDE,
        "records": records,
    }
    METADATA.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"extracted {len(records)} resource catalog entries")


def c_string(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def build():
    payload = json.loads(METADATA.read_text())
    records = payload["records"]
    if payload.get("rom_offset") != BASE or payload.get("record_size") != STRIDE:
        raise ValueError("resource catalog layout metadata changed")
    if len(records) != COUNT:
        raise ValueError(f"expected {COUNT} records, got {len(records)}")
    lines = []
    seen = set()
    for index, record in enumerate(records):
        if record.get("index") != index:
            raise ValueError(f"record {index}: index mismatch")
        name = record["name"]
        encoded = name.encode("ascii")
        if len(encoded) > NAME_SIZE or "\0" in name:
            raise ValueError(f"record {index}: invalid 12-byte name")
        if name in seen:
            raise ValueError(f"record {index}: duplicate resource name {name}")
        seen.add(name)
        offset = record["rom_offset"]
        if not 0 <= offset < 0x1000000:
            raise ValueError(f"record {index}: ROM offset is out of range")
        lines.append(f"    {{ {c_string(name)}, 0x{offset:08X} }}, /* {index:03d} */")
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text("\n".join(lines) + "\n")
    print(f"built {len(records)} resource catalog entries")


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    extract_parser = sub.add_parser("extract")
    extract_parser.add_argument("rom")
    sub.add_parser("build")
    args = parser.parse_args()
    if args.command == "extract":
        extract(args.rom)
    else:
        build()


if __name__ == "__main__":
    main()
