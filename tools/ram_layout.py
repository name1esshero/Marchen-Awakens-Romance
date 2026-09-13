#!/usr/bin/env python3
"""Validate and report the game's boot-time EWRAM/IWRAM partition map."""

import argparse
import json
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def address(value):
    return int(value, 16) if isinstance(value, str) else int(value)


def load_layout(path=ROOT / "ram_layout.json"):
    data = json.loads(Path(path).read_text())
    memories = {
        name: (address(item["start"]), int(item["size"]))
        for name, item in data["memory"].items()
    }
    partitions = []
    for item in data["partitions"]:
        entry = dict(item)
        entry["start"] = address(entry["start"])
        entry["size"] = int(entry["size"])
        entry["end"] = entry["start"] + entry["size"]
        partitions.append(entry)
    return data, memories, partitions


def audit_layout(path=ROOT / "ram_layout.json"):
    data, memories, partitions = load_layout(path)
    totals = {}
    for memory, (start, size) in memories.items():
        end = start + size
        rows = sorted((p for p in partitions if p["memory"] == memory),
                      key=lambda p: p["start"])
        cursor = start
        kinds = defaultdict(int)
        for row in rows:
            if row["start"] != cursor:
                raise ValueError(
                    f'{memory} layout gap/overlap before {row["name"]}: '
                    f'{cursor:08X} != {row["start"]:08X}')
            if row["end"] > end:
                raise ValueError(f'{row["name"]} exceeds {memory}')
            cursor = row["end"]
            kinds[row["kind"]] += row["size"]
        if cursor != end:
            raise ValueError(f'{memory} layout ends at {cursor:08X}, expected {end:08X}')
        totals[memory] = {"size": size, "kinds": dict(sorted(kinds.items()))}

    parents = {p["name"]: p for p in partitions}
    seen = set()
    for symbol in data["symbols"]:
        name = symbol["name"]
        if name in seen:
            raise ValueError(f'duplicate RAM symbol {name}')
        seen.add(name)
        start = address(symbol["address"])
        end = start + int(symbol["size"])
        parent = parents.get(symbol["parent"])
        if parent is None or not (parent["start"] <= start <= end <= parent["end"]):
            raise ValueError(f'{name} is outside parent {symbol["parent"]}')
    return {"totals": totals, "partitions": partitions,
            "symbol_count": len(data["symbols"])}


def resolve_address(value, path=ROOT / "ram_layout.json"):
    """Return the physical partition and named objects containing an address."""
    data, memories, partitions = load_layout(path)
    target = address(value)
    partition = next((p for p in partitions
                      if p["start"] <= target < p["end"]), None)
    symbols = []
    for item in data["symbols"]:
        start = address(item["address"])
        end = start + int(item["size"])
        if start <= target < end:
            symbols.append({**item, "start": start, "end": end,
                            "offset": target - start})
    symbols.sort(key=lambda item: (item["size"], item["start"]))
    return {"address": target, "partition": partition, "symbols": symbols}


def format_size(value):
    return f"{value:,} bytes"


def print_report(report):
    print("Boot-time RAM layout")
    for memory, item in report["totals"].items():
        accounted = item["size"] - item["kinds"].get("unassigned", 0)
        print(f"  {memory:5} {format_size(accounted):>18} accounted / "
              f"{format_size(item['size']):>18} total")
        for kind, size in item["kinds"].items():
            print(f"    {kind:13} {format_size(size):>18}")
    print(f"  {report['symbol_count']} named RAM objects nested in those partitions")
    print("  Heap arenas are reserved address space; live allocator use requires runtime telemetry.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--layout", type=Path, default=ROOT / "ram_layout.json")
    parser.add_argument("--json", type=Path)
    parser.add_argument("--address", action="append", default=[],
                        help="resolve a hex RAM address to its owning objects")
    args = parser.parse_args()
    report = audit_layout(args.layout)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(report, indent=2) + "\n")
    print_report(report)
    for value in args.address:
        resolved = resolve_address(value, args.layout)
        target = resolved["address"]
        partition = resolved["partition"]
        print(f"\n{target:08X}")
        if partition is None:
            print("  outside catalogued RAM")
            continue
        print(f"  partition {partition['name']} + "
              f"0x{target - partition['start']:X}")
        for symbol in resolved["symbols"]:
            print(f"  object    {symbol['name']} + 0x{symbol['offset']:X}")


if __name__ == "__main__":
    main()
