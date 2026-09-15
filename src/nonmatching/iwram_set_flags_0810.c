/* sub_08006ADC, called from ScriptNativeResetFieldScene() in src/mapping.c
 * as `sub_08006ADC(2, 0)` and `sub_08006ADC(3, 0)`. Sets or clears one of
 * four bits (0x100/0x200/0x400/0x800, for mode 0/1/2/3 respectively) in the
 * u16 field at IwramGetField0810()'s address (IWRAM+0x810), depending on
 * whether `enable` is nonzero. An out-of-range mode is a no-op.
 *
 * The switch's case order in the disassembly (dispatch checks mode==1
 * first, then mode>1 splitting into 2/3, then falls through to mode==0) is
 * not arbitrary: confirmed with tools/agbcc_probe.py that agbcc lowers a
 * `switch` over a small dense case set into this exact decision-tree shape
 * regardless of the order the cases are written in source, so an ordinary
 * `case 0: ... case 1: ... case 2: ... case 3:` switch (written in the
 * natural order below) reproduces it -- no case reordering trick needed.
 *
 * Logically confirmed correct and works for the AND (clear) side of the
 * switch: `*(u16 *)(gIwramBase + offset) &= mask;` as a bare literal
 * assignment (no named locals for the mask) reproduces the ROM
 * instruction-for-instruction, including the shared `ands r1,r2; strh`
 * tail all four clear cases jump to.
 *
 * The OR (set) side does not fully match, and it is a new instance of the
 * "which operand becomes the destination register" tie documented for
 * addition in docs/AGBCC_CODEGEN.md, now confirmed to also apply to OR (but
 * *not* to AND, which matches cleanly with the identical structural
 * pattern -- a useful asymmetry for whoever investigates this further). The
 * ROM computes the bit constant into r3 then copies it to r1 (the OR's
 * destination) before loading the field into r2; the bare-literal C below
 * loads the field into r1 and the bit into r2 instead -- mathematically
 * identical, wrong registers. Introducing a named local for either operand
 * (the bit value or the loaded field value) does change the register
 * assignment, but also makes the four case bodies structurally identical
 * enough that agbcc merges them into one shared tail with per-case jumps,
 * which the ROM does not have on this side (unlike the AND side, which the
 * ROM *does* write with a shared tail). No source shape found gets the
 * right registers without also triggering that unwanted merge.
 */

#include "gba/types.h"

extern u8 gIwramBase[];

void IwramSetFlags0810(s32 mode, s32 enable)
{
    if (enable) {
        switch (mode) {
        case 0: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) |= 0x100;
            break;
        }
        case 1: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) |= 0x200;
            break;
        }
        case 2: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) |= 0x400;
            break;
        }
        case 3: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) |= 0x800;
            break;
        }
        }
    } else {
        switch (mode) {
        case 0: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) &= 0xFEFF;
            break;
        }
        case 1: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) &= 0xFDFF;
            break;
        }
        case 2: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) &= 0xFBFF;
            break;
        }
        case 3: {
            u32 offset = 0x81;
            offset <<= 4;
            *(u16 *)(gIwramBase + offset) &= 0xF7FF;
            break;
        }
        }
    }
}
