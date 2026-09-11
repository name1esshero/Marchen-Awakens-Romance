"""THUMB disassembler that emits GNU as source.

Output is written so that re-assembling it reproduces the original bytes
exactly; `emit_insn` returns None for anything it will not vouch for, and the
caller falls back to a `.2byte` literal.
"""

REGS = ["r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc"]
COND = ["eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
        "hi", "ls", "ge", "lt", "gt", "le"]
ALU = ["ands", "eors", "lsls", "lsrs", "asrs", "adcs", "sbcs", "rors",
       "tst", "negs", "cmp", "cmn", "orrs", "muls", "bics", "mvns"]


def _rlist(mask, extra=None):
    names = [REGS[i] for i in range(8) if mask & (1 << i)]
    if extra:
        names.append(extra)
    return "{" + ", ".join(names) + "}"


def _sx(v, bits):
    m = 1 << (bits - 1)
    return (v ^ m) - m


def disasm_thumb(h, addr, h2=None, label_for=None):
    """Return (text, size) for one THUMB instruction, or None if unsupported.

    `label_for(addr)` supplies a symbol name for branch and literal targets.
    """
    def lbl(a):
        return label_for(a) if label_for else ("0x%08x" % a)

    op = h >> 12

    # format 1/2: shifts and add/sub (bits 15..13 == 000)
    if op in (0, 1):
        sub = (h >> 11) & 3
        rd, rs, imm = h & 7, (h >> 3) & 7, (h >> 6) & 0x1F
        if sub == 0 and imm == 0:
            # lsls rd, rs, #0 is the canonical encoding of movs rd, rs
            return ("movs %s, %s" % (REGS[rd], REGS[rs]), 2)
        if sub < 3:
            name = ["lsls", "lsrs", "asrs"][sub]
            if sub in (1, 2) and imm == 0:
                imm = 32
            return ("%s %s, %s, #%d" % (name, REGS[rd], REGS[rs], imm), 2)
        i = (h >> 10) & 1
        name = "subs" if (h >> 9) & 1 else "adds"
        rn = (h >> 6) & 7
        if i:
            return ("%s %s, %s, #%d" % (name, REGS[rd], REGS[rs], rn), 2)
        return ("%s %s, %s, %s" % (name, REGS[rd], REGS[rs], REGS[rn]), 2)

    # format 3: mov/cmp/add/sub with 8-bit immediate
    if op == 2 or op == 3:
        sub = (h >> 11) & 3
        name = ["movs", "cmp", "adds", "subs"][sub]
        return ("%s %s, #%d" % (name, REGS[(h >> 8) & 7], h & 0xFF), 2)

    if op == 4:
        if (h & 0xFC00) == 0x4000:                     # ALU
            return ("%s %s, %s" % (ALU[(h >> 6) & 0xF], REGS[h & 7],
                                   REGS[(h >> 3) & 7]), 2)
        if (h & 0xFC00) == 0x4400:                     # hi register ops / bx
            sub = (h >> 8) & 3
            rd = (h & 7) | ((h >> 4) & 8)
            rs = (h >> 3) & 0xF
            if sub == 3:
                if h & 7:
                    return None          # SBZ bits set: not a real bx/blx
                return ("%s %s" % ("blx" if h & 0x80 else "bx", REGS[rs]), 2)
            # add/cmp/mov in this format need at least one high register;
            # with two low registers the encoding is unpredictable, so leave
            # it to the caller to emit as a literal.
            if rd < 8 and rs < 8:
                return None
            return ("%s %s, %s" % (["add", "cmp", "mov"][sub],
                                   REGS[rd], REGS[rs]), 2)
        if (h & 0xF800) == 0x4800:                     # ldr rd, [pc, #imm]
            tgt = ((addr + 4) & ~3) + ((h & 0xFF) << 2)
            return ("ldr %s, %s" % (REGS[(h >> 8) & 7], lbl(tgt)), 2)
        return None

    if op == 5:
        names = ["str", "strh", "strb", "ldrsb", "ldr", "ldrh", "ldrb", "ldrsh"]
        name = names[(h >> 9) & 7]
        return ("%s %s, [%s, %s]" % (name, REGS[h & 7], REGS[(h >> 3) & 7],
                                     REGS[(h >> 6) & 7]), 2)

    if op in (6, 7):                                   # ldr/str imm5
        l = (h >> 11) & 1
        b = (h >> 12) & 1
        imm = (h >> 6) & 0x1F
        name = ("ldrb" if l else "strb") if b else ("ldr" if l else "str")
        shift = 0 if b else 2
        return ("%s %s, [%s, #%d]" % (name, REGS[h & 7], REGS[(h >> 3) & 7],
                                      imm << shift), 2)

    if op == 8:                                        # ldrh/strh imm5
        name = "ldrh" if (h >> 11) & 1 else "strh"
        return ("%s %s, [%s, #%d]" % (name, REGS[h & 7], REGS[(h >> 3) & 7],
                                      ((h >> 6) & 0x1F) << 1), 2)

    if op == 9:                                        # sp-relative
        name = "ldr" if (h >> 11) & 1 else "str"
        return ("%s %s, [sp, #%d]" % (name, REGS[(h >> 8) & 7],
                                      (h & 0xFF) << 2), 2)

    if op == 0xA:                                      # add rd, pc/sp, imm
        if (h >> 11) & 1:
            return ("add %s, sp, #%d" % (REGS[(h >> 8) & 7], (h & 0xFF) << 2), 2)
        tgt = ((addr + 4) & ~3) + ((h & 0xFF) << 2)
        return ("adr %s, %s" % (REGS[(h >> 8) & 7], lbl(tgt)), 2)

    if op == 0xB:
        if (h & 0xFF00) == 0xB000:                     # add/sub sp, #imm7
            imm = (h & 0x7F) << 2
            return ("%s sp, #%d" % ("sub" if h & 0x80 else "add", imm), 2)
        if (h & 0xF600) == 0xB400:                     # push/pop
            load = (h >> 11) & 1
            extra = ("pc" if load else "lr") if (h & 0x100) else None
            if not (h & 0xFF) and extra is None:
                return None                            # empty register list
            return ("%s %s" % ("pop" if load else "push",
                               _rlist(h & 0xFF, extra)), 2)
        return None

    if op == 0xC:                                      # ldmia/stmia
        if not (h & 0xFF):
            return None                                # empty register list
        name = "ldmia" if (h >> 11) & 1 else "stmia"
        return ("%s %s!, %s" % (name, REGS[(h >> 8) & 7], _rlist(h & 0xFF)), 2)

    if op == 0xD:
        cond = (h >> 8) & 0xF
        if cond == 0xF:
            return ("swi #%d" % (h & 0xFF), 2)
        if cond == 0xE:
            return None
        tgt = (addr + 4 + (_sx(h & 0xFF, 8) << 1)) & 0xFFFFFFFF
        return ("b%s %s" % (COND[cond], lbl(tgt)), 2)

    if op == 0xE:
        if h & 0x0800:
            return None                                # blx suffix, handled below
        tgt = (addr + 4 + (_sx(h & 0x7FF, 11) << 1)) & 0xFFFFFFFF
        return ("b %s" % lbl(tgt), 2)

    if op == 0xF:
        if h2 is None:
            return None
        if (h2 >> 11) != 0x1F:
            return None
        hi = _sx(h & 0x7FF, 11) << 12
        lo = (h2 & 0x7FF) << 1
        tgt = (addr + 4 + hi + lo) & 0xFFFFFFFF
        return ("bl %s" % lbl(tgt), 4)

    return None
