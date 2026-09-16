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
ENGLISH_OUTPUT = ROOT / "build" / "generated" / "resource_catalog_english.inc"
ENGLISH_EXTERNS_OUTPUT = ROOT / "build" / "generated" / "resource_catalog_english_externs.inc"
EXPANSION_MANIFEST = ROOT / "build" / "english" / "scripts_expansion_manifest.json"


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


def marscript_identifier(name):
    return name.replace(".", "_").replace("-", "_")


def _validated_records(payload):
    records = payload["records"]
    if payload.get("rom_offset") != BASE or payload.get("record_size") != STRIDE:
        raise ValueError("resource catalog layout metadata changed")
    if len(records) != COUNT:
        raise ValueError(f"expected {COUNT} records, got {len(records)}")
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
    return records


def build():
    payload = json.loads(METADATA.read_text())
    records = _validated_records(payload)
    lines = []
    for index, record in enumerate(records):
        offset = record["rom_offset"]
        if not 0 <= offset < 0x1000000:
            raise ValueError(f"record {index}: ROM offset is out of range")
        lines.append(f"    {{ {c_string(record['name'])}, 0x{offset:08X} }}, /* {index:03d} */")
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text("\n".join(lines) + "\n")
    print(f"built {len(records)} resource catalog entries")


def build_english():
    """Same 830 entries as build(), except any script
    tools/marscript_rom_build.py routed to the English ROM expansion region
    (because its marscript override no longer fit the original archive
    member) gets its offset resolved as a linker symbol expression instead
    of the static, extraction-time literal -- see
    tools/split_scripts_english.py (defines the symbol) and
    src/english/resource_catalog_english.c (the object this .inc is
    compiled into) for the other halves of this mechanism.

    Never used for the base mar.gba build: only mar_english.gba has an
    expansion region for a symbol like this to point into, and only
    mar_english.gba's script pipeline (tools/marscript_rom_build.py) ever
    produces output too large for a script's original archive slot.
    """
    payload = json.loads(METADATA.read_text())
    records = _validated_records(payload)
    expanded = set(json.loads(EXPANSION_MANIFEST.read_text())) if EXPANSION_MANIFEST.exists() else set()
    lines = []
    for index, record in enumerate(records):
        name = record["name"]
        if name in expanded:
            value = f"(u32)gMarscriptScript_{marscript_identifier(name)} - 0x08000000"
        else:
            offset = record["rom_offset"]
            if not 0 <= offset < 0x1000000:
                raise ValueError(f"record {index}: ROM offset is out of range")
            value = f"0x{offset:08X}"
        lines.append(f"    {{ {c_string(name)}, {value} }}, /* {index:03d} */")
    ENGLISH_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    ENGLISH_OUTPUT.write_text("\n".join(lines) + "\n")

    # Each symbol referenced above needs a declaration in scope *before* the
    # array initializer that uses it -- can't live inside ENGLISH_OUTPUT
    # itself, since that's spliced directly into the initializer's braces.
    # src/english/resource_catalog_english.c #includes this one first.
    externs = [f"extern const u8 gMarscriptScript_{marscript_identifier(name)}[];"
              for name in sorted(expanded)]
    ENGLISH_EXTERNS_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    ENGLISH_EXTERNS_OUTPUT.write_text("\n".join(externs) + "\n" if externs else "")
    print(f"built {len(records)} English resource catalog entries ({len(expanded)} expanded)")


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    extract_parser = sub.add_parser("extract")
    extract_parser.add_argument("rom")
    sub.add_parser("build")
    sub.add_parser("build-english")
    args = parser.parse_args()
    if args.command == "extract":
        extract(args.rom)
    elif args.command == "build-english":
        build_english()
    else:
        build()


if __name__ == "__main__":
    main()
