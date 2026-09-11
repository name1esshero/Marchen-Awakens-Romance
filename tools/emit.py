#!/usr/bin/env python3
"""Turn the recovered layout into assemblable source under asm/.

Every byte of the code region is accounted for as exactly one of:
  * a THUMB or ARM instruction start (from recursive descent),
  * a PC-relative literal word,
  * unclassified data, emitted as literals so the rebuild stays byte-exact.

Anything the disassembler will not vouch for falls back to `.2byte`, so the
output always reassembles to the original image.
"""
import json
import os
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import disasm

ROM_BASE = 0x08000000
CHUNK = 0x8000          # bytes of ROM per generated .s file

# MMIO addresses worth calling out in comments; keeps the raw disassembly
# readable without needing the hardware manual open.
IO_NAMES = {
    0x04000000: "REG_DISPCNT", 0x04000004: "REG_DISPSTAT",
    0x04000006: "REG_VCOUNT", 0x04000008: "REG_BG0CNT",
    0x0400000A: "REG_BG1CNT", 0x0400000C: "REG_BG2CNT",
    0x0400000E: "REG_BG3CNT", 0x04000040: "REG_WIN0H",
    0x04000050: "REG_BLDCNT", 0x040000B0: "REG_DMA0SAD",
    0x040000BC: "REG_DMA1SAD", 0x040000C8: "REG_DMA2SAD",
    0x040000D4: "REG_DMA3SAD", 0x04000100: "REG_TM0CNT_L",
    0x04000130: "REG_KEYINPUT", 0x04000132: "REG_KEYCNT",
    0x04000200: "REG_IE", 0x04000202: "REG_IF",
    0x04000208: "REG_IME", 0x04000204: "REG_WAITCNT",
    0x05000000: "PLTT", 0x06000000: "VRAM", 0x07000000: "OAM",
    0x02000000: "EWRAM", 0x03000000: "IWRAM",
}


def region_comment(v):
    """Describe a literal word that points at hardware or a known memory area."""
    if v in IO_NAMES:
        return IO_NAMES[v]
    for base, name in ((0x04000000, "IO"), (0x05000000, "PLTT"),
                       (0x06000000, "VRAM"), (0x07000000, "OAM")):
        if base <= v < base + 0x01000000:
            return "%s+0x%X" % (name, v - base)
    if 0x02000000 <= v < 0x02040000:
        return "EWRAM+0x%X" % (v - 0x02000000)
    if 0x03000000 <= v < 0x03008000:
        return "IWRAM+0x%X" % (v - 0x03000000)
    if 0x08000000 <= v < 0x0A000000:
        return "ROM+0x%X" % (v - 0x08000000)
    return None


# Addresses whose purpose is known from the boot sequence; giving them real
# names makes the generated assembly much easier to follow.
KNOWN_SYMBOLS = {
    0x080000C0: "Entry",
    0x08000104: "IntrMain",
}

# Names recovered elsewhere: library routines matched against agbcc's libc,
# and anything else identified by hand. Loading them here makes every call
# site in the disassembly readable.
def _load_symbols():
    import json as _json
    try:
        with open("symbols.json") as f:
            for addr, name in _json.load(f).items():
                KNOWN_SYMBOLS.setdefault(int(addr, 16), name)
    except OSError:
        pass


_load_symbols()


class OutOfRange(Exception):
    """Raised when an instruction references an address we do not emit."""


class Emitter:
    def __init__(self, rom, layout, code_end, force_literal=(), data_ranges=()):
        self.data_ranges = [(ROM_BASE+a, ROM_BASE+b) for a,b in data_ranges]
        self.rom = rom
        self.code_end = code_end
        # Addresses the assembler refused: emitted as .2byte instead. These
        # are almost always data the analysis mistook for instructions.
        self.force_literal = set(force_literal)
        self.mode = {int(a, 16): m for a, m in layout["mode"].items()}
        self.pool = set(int(a, 16) for a in layout["pool"])
        self.funcs = {int(a, 16): m for a, m in layout["funcs"].items()}
        self.labels = {}
        # Only addresses inside the emitted code region can carry a label.
        self.emit_lo = ROM_BASE + 0xC0
        self.emit_hi = ROM_BASE + code_end
        self.build_labels()

    def w(self, addr):
        return struct.unpack_from("<I", self.rom, addr - ROM_BASE)[0]

    def h(self, addr):
        return struct.unpack_from("<H", self.rom, addr - ROM_BASE)[0]

    def in_range(self, addr):
        return self.emit_lo <= addr < self.emit_hi and not any(a <= addr < b for a,b in self.data_ranges)

    def build_labels(self):
        """Name every function entry and every literal word."""
        for a, m in self.funcs.items():
            if self.in_range(a):
                self.labels[a] = KNOWN_SYMBOLS.get(a, "sub_%08X" % a)
        for a in self.pool:
            if self.in_range(a):
                self.labels.setdefault(a, "_%08X" % a)

    def spans_label(self, addr, size):
        """True if a multi-byte literal starting here would bury a label."""
        return any((addr + k) in self.labels for k in range(1, size))

    def label_for(self, addr):
        """Symbol for a branch or literal target, creating one on demand.

        Targets outside the emitted region come from bytes that decoded as an
        instruction but are really data; the caller emits those as literals.
        """
        if addr in self.labels:
            return self.labels[addr]
        if self.in_range(addr):
            self.labels[addr] = "_%08X" % addr
            return self.labels[addr]
        raise OutOfRange(addr)

    def prescan(self, start, end):
        """Create every label in [start, end) before any code is emitted.

        Chunks are written in address order, so a backward branch reaching
        from one chunk into an earlier one would otherwise name a label after
        that label's address had already gone past.
        """
        addr = start
        while addr < end:
            if self.mode.get(addr) == "thumb" and addr not in self.pool:
                h = self.h(addr)
                h2 = self.h(addr + 2) if addr + 2 < end else None
                try:
                    disasm.disasm_thumb(h, addr, h2, self.label_for)
                except OutOfRange:
                    pass
            addr += 2

    def emit_range(self, start, end, out, first_line=1):
        """Write assembly covering [start, end).

        Returns {line number: address} so a failed assembly can be traced
        back to the byte it came from.
        """
        addr = start
        lines = []
        linemap = {}
        # Pre-create labels for every branch target in range so forward
        # references resolve to names rather than raw addresses.
        while addr < end:
            if addr in self.mode and self.mode[addr] == "thumb" \
                    and addr not in self.pool:
                h = self.h(addr)
                h2 = self.h(addr + 2) if addr + 2 < end else None
                try:
                    disasm.disasm_thumb(h, addr, h2, self.label_for)
                except OutOfRange:
                    pass
            addr += 2

        addr = start
        cur_mode = None
        while addr < end:
            lbl = self.labels.get(addr)
            if addr in self.funcs:
                lines.append("")
                # No .align here: the layout is already byte-exact, and any
                # padding the assembler inserted would shift every later label.
                if self.funcs[addr] == "thumb":
                    lines.append("\t.thumb_func")
                    cur_mode = "thumb"
                    lines.append("\t.thumb")
                lines.append("\t.global %s" % self.labels[addr])
                lines.append("%s:" % self.labels[addr])
            elif lbl:
                lines.append("\t.global %s" % lbl)
                lines.append("%s:" % lbl)

            if addr in self.pool:
                if self.spans_label(addr, 4):
                    # a label sits inside this word; emit halves so it can
                    # still be defined at its own address
                    lines.append("\t.2byte 0x%04X" % self.h(addr))
                    addr += 2
                    continue
                v = self.w(addr)
                c = region_comment(v)
                lines.append("\t.4byte 0x%08X%s" % (v, ("  @ %s" % c) if c else ""))
                addr += 4
                continue

            m = self.mode.get(addr)
            if addr in self.force_literal:
                # .inst.n pins the exact halfword. GAS is free to pick a
                # different encoding for some mnemonics (notably the 3-operand
                # add/sub immediates), so anything that did not round-trip is
                # written as its literal encoding with the reading alongside.
                h = self.h(addr)
                try:
                    r = disasm.disasm_thumb(h, addr, None, self.label_for)
                except OutOfRange:
                    r = None
                if self.mode.get(addr) == "thumb":
                    note = ("  @ %s" % r[0]) if r and r[1] == 2 else ""
                    lines.append("\t.inst.n 0x%04X%s" % (h, note))
                else:
                    lines.append("\t.2byte 0x%04X" % h)
                addr += 2
                continue
            if m == "thumb":
                if cur_mode != "thumb":
                    lines.append("\t.thumb")
                    cur_mode = "thumb"
                h = self.h(addr)
                h2 = self.h(addr + 2) if addr + 2 < end else None
                try:
                    r = disasm.disasm_thumb(h, addr, h2, self.label_for)
                except OutOfRange:
                    r = None
                if r and addr + r[1] <= end:
                    txt, sz = r
                    # don't let a 4-byte bl swallow a labelled address
                    if sz == 4 and self.spans_label(addr, 4):
                        r = None
                    else:
                        linemap[first_line + len(lines)] = addr
                        lines.append("\t%s" % txt)
                        addr += sz
                        continue
                lines.append("\t.2byte 0x%04X" % h)
                addr += 2
                continue

            if m == "arm":
                if cur_mode != "arm":
                    lines.append("\t.arm")
                    cur_mode = "arm"
                if self.spans_label(addr, 4):
                    lines.append("\t.2byte 0x%04X" % self.h(addr))
                    addr += 2
                    continue
                lines.append("\t.4byte 0x%08X" % self.w(addr))
                addr += 4
                continue

            # unclassified: emit as data, word-wise where alignment allows
            if addr % 4 == 0 and addr + 4 <= end \
                    and not self.spans_label(addr, 4):
                lines.append("\t.4byte 0x%08X" % self.w(addr))
                addr += 4
            else:
                lines.append("\t.byte 0x%02X" % self.rom[addr - ROM_BASE])
                addr += 1

        out.write("\n".join(lines) + "\n")
        return linemap
