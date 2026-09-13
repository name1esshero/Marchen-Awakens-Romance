#!/usr/bin/env python3
"""Extract and build the typed ÄRM and consumable definition tables.

The committed JSON stores numeric metadata by field rather than retaining an
opaque binary table.  Names and descriptions come from the existing readable
text sources and are encoded with the documented game charmap during builds.
"""
from pathlib import Path
import argparse
import json
import sys
import re
import unicodedata

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import definition_text
import text_codec

ROM_BASE = 0x08000000
METADATA = ROOT / "data" / "definition_tables.json"
OUT_DIR = ROOT / "build" / "generated"
CONSTANTS_HEADER = ROOT / "include" / "constants" / "arm.h"

ARM_BASE = 0x1B096C
ARM_COUNT = 445
ARM_STRIDE = 0x80
ITEM_BASE = 0x1BE82C
ITEM_COUNT = 13
ITEM_STRIDE = 0x50


def english_annotations(path):
    result = {}
    pattern = re.compile(r"^@([0-9A-Fa-f]+).*?\s+//\s+EN:\s*(.*?)\s*$")
    for line in (ROOT / path).read_text().splitlines():
        match = pattern.match(line)
        if match:
            result[int(match.group(1), 16)] = match.group(2)
    return result


def constant_name(prefix, name, index, used):
    normalized = unicodedata.normalize("NFKD", name).encode("ascii", "ignore").decode()
    slug = re.sub(r"[^A-Za-z0-9]+", "_", normalized).strip("_").upper()
    if not slug:
        slug = f"ID_{index:03d}"
    candidate = f"{prefix}_{slug}"
    if candidate in used:
        candidate = f"{candidate}_{index:03d}"
    used.add(candidate)
    return candidate


def write_constants(payload):
    arms = payload["arm_table"]["records"]
    items = payload["item_table"]["records"]
    lines = [
        "#ifndef CONSTANTS_ARM_H",
        "#define CONSTANTS_ARM_H",
        "",
        "/* Stable source names for the 445-entry ÄRM catalog. */",
        "enum ArmId {",
    ]
    lines.extend(f"    {record['constant']} = {record['index']},"
                 for record in arms)
    lines += ["    ARM_COUNT = 445", "};", "",
              "/* Consumables and engraving materials stored after the ÄRM table. */",
              "enum ItemId {"]
    lines.extend(f"    {record['constant']} = {record['index']},"
                 for record in items)
    lines += ["    ITEM_COUNT = 13", "};", "", "#endif", ""]
    CONSTANTS_HEADER.parent.mkdir(parents=True, exist_ok=True)
    CONSTANTS_HEADER.write_text("\n".join(lines))


def u16(data, offset):
    return int.from_bytes(data[offset:offset + 2], "little")


def s16(data, offset):
    value = u16(data, offset)
    return value - 0x10000 if value & 0x8000 else value


def u32(data, offset):
    return int.from_bytes(data[offset:offset + 4], "little")


def s8(value):
    return value - 0x100 if value & 0x80 else value


def extract_arm(record, index):
    password = record[6:14].rstrip(b"\0")
    if any(byte < 0x20 or byte > 0x7E for byte in password):
        raise ValueError(f"ÄRM {index}: password is not printable ASCII")
    return {
        "index": index,
        "icon_id": u16(record, 0x00),
        "id": u16(record, 0x02),
        "field_04": u16(record, 0x04),
        "password": password.decode("ascii"),
        "reserved_0e": u16(record, 0x0E),
        "field_54": u16(record, 0x54),
        "field_56": u16(record, 0x56),
        "field_58": s8(record[0x58]),
        "type": s8(record[0x59]),
        "field_5a": s8(record[0x5A]),
        "reserved_5b": record[0x5B],
        "field_5c": s16(record, 0x5C),
        "field_5e": s16(record, 0x5E),
        "field_60": s16(record, 0x60),
        "field_62": s8(record[0x62]),
        "field_63": s8(record[0x63]),
        "field_64": s8(record[0x64]),
        "element": s8(record[0x65]),
        "field_66": record[0x66],
        "field_67": record[0x67],
        "reserved_68": u32(record, 0x68),
        "field_6c": u32(record, 0x6C),
        "field_70": record[0x70],
        "field_71": record[0x71],
        "field_72": record[0x72],
        "field_73": record[0x73],
        "field_74": u32(record, 0x74),
        "field_78": u32(record, 0x78),
        "field_7c": record[0x7C],
        "reserved_7d": list(record[0x7D:0x80]),
    }


def extract_item(record, index):
    return {
        "index": index,
        "reserved_00": u32(record, 0x00),
        "field_04": u16(record, 0x04),
        "field_06": u16(record, 0x06),
        "field_08": u32(record, 0x08),
        "field_0c": u32(record, 0x0C),
    }


def extract(rom_path):
    rom = Path(rom_path).read_bytes()
    old = json.loads(METADATA.read_text()) if METADATA.exists() else {}
    old_arms = {entry["index"]: entry.get("constant") for entry in
                old.get("arm_table", {}).get("records", [])}
    old_items = {entry["index"]: entry.get("constant") for entry in
                 old.get("item_table", {}).get("records", [])}
    arm_english = english_annotations("text/arm_definitions.txt")
    item_english = english_annotations("text/item_definitions.txt")
    arms = [extract_arm(rom[ARM_BASE + i * ARM_STRIDE:
                                ARM_BASE + (i + 1) * ARM_STRIDE], i)
            for i in range(ARM_COUNT)]
    items = [extract_item(rom[ITEM_BASE + i * ITEM_STRIDE:
                                  ITEM_BASE + (i + 1) * ITEM_STRIDE], i)
             for i in range(ITEM_COUNT)]
    used = set()
    for record in arms:
        index = record["index"]
        name = arm_english.get(ARM_BASE + index * ARM_STRIDE + 0x10,
                               f"ID {index:03d}")
        record["constant"] = old_arms.get(index) or constant_name(
            "ARM", name, index, used)
        used.add(record["constant"])
    used = set()
    for record in items:
        index = record["index"]
        name = item_english.get(ITEM_BASE + index * ITEM_STRIDE + 0x10,
                                f"ID {index:02d}")
        record["constant"] = old_items.get(index) or constant_name(
            "ITEM", name, index, used)
        used.add(record["constant"])
    payload = {
        "format": "MAR fixed-layout definition metadata",
        "arm_table": {"rom_offset": ARM_BASE, "stride": ARM_STRIDE,
                      "records": arms},
        "item_table": {"rom_offset": ITEM_BASE, "stride": ITEM_STRIDE,
                       "records": items},
    }
    METADATA.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n")
    write_constants(payload)
    print(f"extracted {len(arms)} ÄRM and {len(items)} item metadata records")


def text_fields(table):
    records = definition_text.source_records(table)
    result = {}
    for index in range(table["records"]):
        base = table["base"] + index * table["stride"]
        values = []
        for offset, size in ((table["name_offset"], table["name_size"]),
                             (table["description_offset"],
                              table["description_size"])):
            text, _line = records[base + offset]
            encoded = text_codec.encode(text)
            if len(encoded) >= size:
                raise ValueError(
                    f"record {index} text uses {len(encoded)} of {size} bytes; "
                    "a terminating NUL is required")
            values.append((text, encoded + bytes(size - len(encoded))))
        result[index] = values
    return result


def c_bytes(data):
    return "{ " + ", ".join(f"0x{value:02X}" for value in data) + " }"


def c_string(text):
    return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'


def validate_indices(records, expected, label):
    if len(records) != expected:
        raise ValueError(f"{label}: expected {expected} records, got {len(records)}")
    for index, record in enumerate(records):
        if record.get("index") != index:
            raise ValueError(f"{label}: record {index} has index {record.get('index')}")


def emit_arm(records, texts):
    lines = []
    for record in records:
        index = record["index"]
        (jp_name, name), (jp_description, description) = texts[index]
        lines.append(f"    /* {index:03d}: {jp_name} — {jp_description} */")
        password = record["password"].encode("ascii")
        if len(password) > 8:
            raise ValueError(f"ÄRM {index}: password exceeds 8 bytes")
        password += bytes(8 - len(password))
        values = [
            str(record["icon_id"]), str(record["id"]), str(record["field_04"]),
            c_bytes(password), str(record["reserved_0e"]), c_bytes(name),
            c_bytes(description), str(record["field_54"]),
            str(record["field_56"]),
            str(record["field_58"]),
            str(record["type"]), str(record["field_5a"]),
            str(record["reserved_5b"]), str(record["field_5c"]),
            str(record["field_5e"]), str(record["field_60"]),
            str(record["field_62"]), str(record["field_63"]),
            str(record["field_64"]), str(record["element"]),
            str(record["field_66"]), str(record["field_67"]),
            f"0x{record['reserved_68']:08X}", f"0x{record['field_6c']:08X}",
            str(record["field_70"]), str(record["field_71"]),
            str(record["field_72"]), str(record["field_73"]),
            f"0x{record['field_74']:08X}", f"0x{record['field_78']:08X}",
            str(record["field_7c"]), c_bytes(record["reserved_7d"]),
        ]
        lines.append("    { " + ", ".join(values) + " },")
    return "\n".join(lines) + "\n"


def emit_item(records, texts):
    lines = []
    for record in records:
        index = record["index"]
        (jp_name, name), (jp_description, description) = texts[index]
        lines.append(f"    /* {index:02d}: {jp_name} — {jp_description} */")
        values = [
            f"0x{record['reserved_00']:08X}", str(record["field_04"]),
            str(record["field_06"]), str(record["field_08"]),
            str(record["field_0c"]), c_bytes(name), c_bytes(description),
        ]
        lines.append("    { " + ", ".join(values) + " },")
    return "\n".join(lines) + "\n"


def build():
    payload = json.loads(METADATA.read_text())
    arms = payload["arm_table"]["records"]
    items = payload["item_table"]["records"]
    validate_indices(arms, ARM_COUNT, "ÄRM table")
    validate_indices(items, ITEM_COUNT, "item table")
    arm_text = text_fields(definition_text.TABLES[0])
    item_text = text_fields(definition_text.TABLES[1])
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "arm_definitions.inc").write_text(emit_arm(arms, arm_text))
    (OUT_DIR / "item_definitions.inc").write_text(emit_item(items, item_text))
    print(f"built {len(arms)} ÄRM and {len(items)} item C initializers")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("extract", "build"))
    parser.add_argument("rom", nargs="?", default="baserom.gba")
    args = parser.parse_args()
    if args.command == "extract":
        extract(args.rom)
    else:
        build()


if __name__ == "__main__":
    main()
