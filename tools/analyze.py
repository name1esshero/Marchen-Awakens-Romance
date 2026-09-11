#!/usr/bin/env python3
"""Recover the code/data layout of the ROM by recursive descent.

Starts from the reset vector and the interrupt entry, follows every call and
branch it can prove, then sweeps pointer tables for further THUMB entries.
Emits .analysis/layout.json describing code ranges, literal pools and functions.
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import thumb as T

ROM_BASE = 0x08000000


class Analyzer:
    def __init__(self, rom):
        self.rom = rom
        self.size = len(rom)
        self.mode = {}        # addr -> 'thumb' | 'arm'  (instruction starts)
        self.pool = {}        # addr -> width of literal word
        self.funcs = {}       # addr -> 'thumb' | 'arm'
        self.calls = set()

    def valid(self, addr):
        off = addr - ROM_BASE
        return 0 <= off < self.size

    def word(self, addr):
        return struct.unpack_from("<I", self.rom, addr - ROM_BASE)[0]

    def run(self, entries, code_limit):
        self.code_limit = code_limit
        work = list(entries)
        for a, m in entries:
            self.funcs[a] = m
        seen_funcs = set(a for a, _ in entries)

        while work:
            addr, mode = work.pop()
            for naddr, nmode in self.trace(addr, mode):
                if naddr not in seen_funcs:
                    seen_funcs.add(naddr)
                    self.funcs[naddr] = nmode
                    work.append((naddr, nmode))
        return self

    def is_prologue(self, addr):
        """True if addr looks like the start of a THUMB function."""
        off = addr - ROM_BASE
        if off < 0 or off + 1 >= self.size or (addr & 1):
            return False
        h = self.rom[off] | (self.rom[off + 1] << 8)
        if (h & 0xFF00) == 0xB500:          # push {..., lr}
            return True
        if (h & 0xFF00) == 0xB000 and (h & 0x80):   # sub sp, #imm
            return True
        return False

    def sweep_pointer_tables(self, code_limit):
        """Harvest THUMB entry points stored as odd words anywhere in the ROM."""
        found = []
        for off in range(0, self.size - 3, 4):
            v = struct.unpack_from("<I", self.rom, off)[0]
            if (v & 1) == 0:
                continue
            a = v & ~1
            if not (ROM_BASE + 0xC0 <= a < ROM_BASE + code_limit):
                continue
            if a in self.funcs or not self.is_prologue(a):
                continue
            found.append(a)
        work = []
        for a in found:
            if a not in self.funcs:
                self.funcs[a] = "thumb"
                work.append((a, "thumb"))
        seen = set(self.funcs)
        while work:
            addr, mode = work.pop()
            for naddr, nmode in self.trace(addr, mode):
                if naddr not in seen:
                    seen.add(naddr)
                    self.funcs[naddr] = nmode
                    work.append((naddr, nmode))
        print("pointer sweep added %d entry points" % len(found))

    def trace(self, addr, mode):
        """Walk one function; yield newly discovered call targets."""
        new = []
        pending = [addr]
        visited = set()
        while pending:
            pc = pending.pop()
            while True:
                if pc in visited or not self.valid(pc):
                    break
                if pc - ROM_BASE >= self.code_limit:
                    break
                if self.pool.get(pc):
                    break            # ran into a literal pool: not code
                visited.add(pc)
                off = pc - ROM_BASE
                ins = (T.decode_thumb(self.rom, off, pc) if mode == "thumb"
                       else T.decode_arm(self.rom, off, pc))
                if ins is None:
                    break
                self.mode[pc] = mode

                if ins.pool is not None and self.valid(ins.pool):
                    self.pool[ins.pool] = 4
                    # A pool word holding an odd ROM address is a THUMB
                    # function pointer; harvest it.
                    v = self.word(ins.pool)
                    if (v & 1) and self.valid(v & ~1) \
                            and (v & ~1) - ROM_BASE < self.code_limit:
                        new.append(((v & ~1), "thumb"))
                    elif v % 4 == 0 and self.valid(v) \
                            and v - ROM_BASE < self.code_limit \
                            and mode == "arm":
                        pass

                if ins.flow == T.FLOW_CALL and ins.target is not None:
                    tmode = "thumb" if mode == "thumb" else "arm"
                    if self.valid(ins.target):
                        new.append((ins.target, tmode))
                        self.calls.add(ins.target)
                elif ins.flow in (T.FLOW_BRANCH, T.FLOW_COND) \
                        and ins.target is not None:
                    if self.valid(ins.target):
                        pending.append(ins.target)

                if ins.flow in (T.FLOW_BRANCH, T.FLOW_END):
                    break
                pc += ins.size
        return new


def main():
    rom_path = sys.argv[1]
    code_limit = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0x1B0000
    rom = open(rom_path, "rb").read()
    az = Analyzer(rom)

    entries = [(0x080000C0, "arm")]
    # the reset stub tail hands control to the address stored at 0x08000100
    v = struct.unpack_from("<I", rom, 0x100)[0]
    if v & 1:
        entries.append((v & ~1, "thumb"))
    else:
        entries.append((v, "arm"))
    entries.append((0x08000104, "arm"))     # interrupt dispatcher

    az.run(entries, code_limit)

    # Many handlers are only ever reached through state tables, so sweep the
    # whole image for words that look like THUMB entry points and re-trace.
    az.sweep_pointer_tables(code_limit)

    code_addrs = sorted(az.mode)
    print("functions found : %d" % len(az.funcs))
    print("instructions    : %d" % len(az.mode))
    print("literal words   : %d" % len(az.pool))
    if code_addrs:
        print("code span       : %08X..%08X" % (code_addrs[0], code_addrs[-1]))

    # coverage of the presumed code region
    covered = set()
    for a in az.mode:
        covered.add(a)
    os.makedirs(".analysis", exist_ok=True)
    with open(".analysis/layout.json", "w") as f:
        json.dump({
            "funcs": {("%08X" % a): m for a, m in az.funcs.items()},
            "mode": {("%08X" % a): m for a, m in az.mode.items()},
            "pool": ["%08X" % a for a in sorted(az.pool)],
        }, f)
    print("wrote .analysis/layout.json")


if __name__ == "__main__":
    main()
