"""Minimal THUMB/ARM decoders.

Only enough semantics to drive control-flow recovery: instruction length,
branch behaviour, and PC-relative literal references.
"""
import struct

# flow classes
FLOW_NEXT = 0      # falls through
FLOW_BRANCH = 1    # unconditional branch, does not fall through
FLOW_COND = 2      # conditional branch, also falls through
FLOW_CALL = 3      # bl / blx, falls through
FLOW_END = 4       # returns / indirect branch, does not fall through


class Insn:
    __slots__ = ("addr", "size", "flow", "target", "pool", "pool_size")

    def __init__(self, addr, size, flow=FLOW_NEXT, target=None,
                 pool=None, pool_size=4):
        self.addr = addr
        self.size = size
        self.flow = flow
        self.target = target        # branch/call destination
        self.pool = pool            # PC-relative literal address
        self.pool_size = pool_size


def _sx(v, bits):
    m = 1 << (bits - 1)
    return (v ^ m) - m


def decode_thumb(data, off, addr):
    """Decode one THUMB instruction at data[off] mapped to `addr`."""
    if off + 1 >= len(data):
        return None
    h = data[off] | (data[off + 1] << 8)
    top = h >> 12

    # BL / BLX prefix+suffix pair
    if top == 0xF:
        if off + 3 >= len(data):
            return None
        h2 = data[off + 2] | (data[off + 3] << 8)
        if (h2 >> 13) == 0x7 and ((h2 >> 11) & 3) in (1, 3):
            hi = _sx(h & 0x7FF, 11) << 12
            lo = (h2 & 0x7FF) << 1
            tgt = (addr + 4 + hi + lo) & 0xFFFFFFFF
            if ((h2 >> 11) & 3) == 1:      # BLX -> ARM target
                tgt &= ~3
                return Insn(addr, 4, FLOW_CALL, tgt)
            return Insn(addr, 4, FLOW_CALL, tgt)
        return Insn(addr, 2)

    # unconditional branch
    if top == 0xE and (h & 0x0800) == 0:
        tgt = (addr + 4 + (_sx(h & 0x7FF, 11) << 1)) & 0xFFFFFFFF
        return Insn(addr, 2, FLOW_BRANCH, tgt)

    # conditional branch / svc
    if top == 0xD:
        cond = (h >> 8) & 0xF
        if cond == 0xF:                     # svc
            return Insn(addr, 2)
        if cond == 0xE:                     # undefined
            return Insn(addr, 2, FLOW_END)
        tgt = (addr + 4 + (_sx(h & 0xFF, 8) << 1)) & 0xFFFFFFFF
        return Insn(addr, 2, FLOW_COND, tgt)

    # ldr rd, [pc, #imm]
    if top == 0x4 and (h & 0xF800) == 0x4800:
        pool = ((addr + 4) & ~3) + ((h & 0xFF) << 2)
        return Insn(addr, 2, FLOW_NEXT, None, pool)

    # add rd, pc, #imm  (ADR)
    if (h & 0xF800) == 0xA000:
        pool = ((addr + 4) & ~3) + ((h & 0xFF) << 2)
        return Insn(addr, 2, FLOW_NEXT, None, pool)

    # bx / blx register
    if (h & 0xFF87) == 0x4700:
        return Insn(addr, 2, FLOW_END)
    if (h & 0xFF87) == 0x4780:
        return Insn(addr, 2, FLOW_CALL)

    # pop { ..., pc }
    if (h & 0xFF00) == 0xBD00:
        return Insn(addr, 2, FLOW_END)

    # mov pc, rN   (hi-register move with Rd == 15)
    if (h & 0xFF00) == 0x4600 and ((h & 0x80) >> 4 | (h & 0x7)) == 15:
        return Insn(addr, 2, FLOW_END)

    return Insn(addr, 2)


def decode_arm(data, off, addr):
    if off + 3 >= len(data):
        return None
    w = struct.unpack_from("<I", data, off)[0]
    cond = w >> 28
    op = (w >> 25) & 0x7

    # branch / branch-with-link
    if op == 0x5:
        tgt = (addr + 8 + (_sx(w & 0xFFFFFF, 24) << 2)) & 0xFFFFFFFF
        if w & (1 << 24):
            return Insn(addr, 4, FLOW_CALL, tgt)
        return Insn(addr, 4,
                    FLOW_BRANCH if cond == 0xE else FLOW_COND, tgt)

    # bx / blx reg
    if (w & 0x0FFFFFF0) == 0x012FFF10:
        return Insn(addr, 4, FLOW_END if cond == 0xE else FLOW_NEXT)
    if (w & 0x0FFFFFF0) == 0x012FFF30:
        return Insn(addr, 4, FLOW_CALL)

    # ldr rd, [pc, #imm]
    if (w & 0x0F7F0000) == 0x051F0000:
        imm = w & 0xFFF
        base = addr + 8
        pool = base + imm if (w & (1 << 23)) else base - imm
        return Insn(addr, 4, FLOW_NEXT, None, pool)

    # writes to PC -> flow ends (mov pc, lr / ldm ..., {pc})
    rd = (w >> 12) & 0xF
    if op in (0, 1) and rd == 15 and cond == 0xE:
        return Insn(addr, 4, FLOW_END)
    if op == 4 and (w & (1 << 15)) and (w & (1 << 20)) and cond == 0xE:
        return Insn(addr, 4, FLOW_END)   # ldm with pc in list

    return Insn(addr, 4)
