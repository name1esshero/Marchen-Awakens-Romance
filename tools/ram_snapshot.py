#!/usr/bin/env python3
"""Measure live MAR heap use from raw EWRAM and IWRAM emulator dumps."""

import argparse
import json
import struct
from pathlib import Path


MEMORIES = {
    "EWRAM": (0x02000000, 0x40000),
    "IWRAM": (0x03000000, 0x08000),
}
HEAP_HANDLES = [
    ("EWRAM heap 0", 0x03003FB4),
    ("EWRAM heap 1", 0x03003FB8),
    ("EWRAM heap 2", 0x03003FBC),
    ("EWRAM heap 3", 0x03003FC0),
    ("EWRAM heap 4", 0x03003FC4),
    ("EWRAM heap 5", 0x03003FC8),
    ("EWRAM heap 6", 0x03003FCC),
    ("IWRAM heap", 0x03003FD0),
]


class MemoryImage:
    def __init__(self, ewram, iwram):
        self.images = {
            "EWRAM": bytes(ewram),
            "IWRAM": bytes(iwram),
        }
        for name, (_, size) in MEMORIES.items():
            if len(self.images[name]) != size:
                raise ValueError(
                    f"{name} dump is {len(self.images[name]):#x} bytes; expected {size:#x}")

    def read(self, address, size):
        for name, (start, length) in MEMORIES.items():
            if start <= address and address + size <= start + length:
                offset = address - start
                return self.images[name][offset:offset + size]
        raise ValueError(f"address {address:08X} is outside dumped RAM")

    def u32(self, address):
        return struct.unpack("<I", self.read(address, 4))[0]


def inspect_heap(memory, name, handle_address):
    heap_address = memory.u32(handle_address)
    if heap_address == 0:
        return {"name": name, "handle": handle_address, "initialized": False}
    heap_size = memory.u32(heap_address)
    scan_start = memory.u32(heap_address + 4)
    if heap_size < 24 or heap_size & 3:
        raise ValueError(f"{name}: invalid heap size {heap_size:#x}")
    heap_end = heap_address + heap_size
    if scan_start < heap_address + 8 or scan_start >= heap_end:
        raise ValueError(f"{name}: scan pointer {scan_start:08X} is outside heap")

    block_address = heap_address + 8
    allocated_payload = free_payload = block_headers = block_count = 0
    largest_free = 0
    while True:
        tagged_size = memory.u32(block_address)
        block_size = tagged_size & ~3
        if block_size < 8 or block_size & 3 or block_address + block_size > heap_end:
            raise ValueError(
                f"{name}: corrupt block at {block_address:08X}, size {block_size:#x}")
        payload = block_size - 8
        occupied = bool(tagged_size & 1)
        if occupied:
            allocated_payload += payload
        else:
            free_payload += payload
            largest_free = max(largest_free, payload)
        block_headers += 8
        block_count += 1
        block_address += block_size
        if tagged_size & 2:
            break
    if block_address != heap_end:
        raise ValueError(
            f"{name}: last block ends at {block_address:08X}, heap ends at {heap_end:08X}")
    return {
        "name": name,
        "handle": handle_address,
        "initialized": True,
        "address": heap_address,
        "capacity": heap_size,
        "allocated_payload": allocated_payload,
        "free_payload": free_payload,
        "largest_free_block": largest_free,
        "allocator_overhead": 8 + block_headers,
        "block_count": block_count,
        "scan_start": scan_start,
    }


def inspect_snapshot(ewram, iwram):
    memory = MemoryImage(ewram, iwram)
    heaps = [inspect_heap(memory, *entry) for entry in HEAP_HANDLES]
    active = [heap for heap in heaps if heap["initialized"]]
    return {
        "heaps": heaps,
        "totals": {
            key: sum(heap[key] for heap in active)
            for key in ("capacity", "allocated_payload", "free_payload",
                        "allocator_overhead")
        },
    }


def print_report(report):
    print("Live heap usage")
    for heap in report["heaps"]:
        if not heap["initialized"]:
            print(f"  {heap['name']:<14} not initialized")
            continue
        print(
            f"  {heap['name']:<14} {heap['allocated_payload']:>7,} allocated, "
            f"{heap['free_payload']:>7,} free, largest {heap['largest_free_block']:>7,}, "
            f"{heap['block_count']:>3} blocks")
    totals = report["totals"]
    print(
        f"  total          {totals['allocated_payload']:>7,} allocated, "
        f"{totals['free_payload']:>7,} free, "
        f"{totals['allocator_overhead']:>7,} allocator metadata")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ewram", required=True, type=Path,
                        help="raw 0x40000-byte EWRAM dump starting at 02000000")
    parser.add_argument("--iwram", required=True, type=Path,
                        help="raw 0x8000-byte IWRAM dump starting at 03000000")
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()
    report = inspect_snapshot(args.ewram.read_bytes(), args.iwram.read_bytes())
    print_report(report)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
