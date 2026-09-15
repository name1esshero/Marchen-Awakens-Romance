/* sub_08070EA0, called as `sub_08070EA0(0)` (from ClearBattleRuntimeBuffer()
 * in src/simple_adapters.c, and twice more in asm/code/code_0700C0.s around
 * a CpuCopy/HeapFree pair that saves and restores the whole room array) and
 * as `sub_08070EA0(-1)` (twice in asm/code/code_0700C0.s, immediately before
 * a new room record's fields are written -- one of those sites first calls
 * GeneratedMapFindRuntimeRoom() and only falls through to this on a miss).
 * Any other argument returns NULL.
 *
 * The label sequence in the disassembly (sub_08070EA8, _08070EB0,
 * sub_08070ECE, sub_08070ED8, sub_08070EE6) has no real external caller --
 * confirmed with `grep -rn` against asm/ and src/ -- so it is all one
 * function ending at the `pop {r4,r5} / pop {r1} / bx r1` shared with
 * GeneratedMapFindRuntimeRoom's neighborhood. The two raw `bl` fragments
 * that the disassembler could not target (`.word 0xf8f2f7ff` and
 * `.word 0xf8e8f7ff`) both decode (sign-extended 23-bit branch-with-link
 * offset, matched against build/mar.map) to GameStateGetBuffer38C0 at
 * 0x08070090.
 *
 * mode 0 returns the same `buffer + 24` base GeneratedMapFindRuntimeRoom()
 * indexes from (see src/mapping.c). mode -1 walks the same 64-entry,
 * 24-byte-stride room array looking for the first inactive room (`active
 * == 0`, the field GeneratedMapFindRuntimeRoom checks is *nonzero*) and
 * returns a pointer to it, or NULL if all 64 are active -- i.e. this is
 * that function's "find a free slot to allocate into" counterpart.
 *
 * Logically confirmed correct (every read/write offset accounted for), but
 * not byte-exact. The mode-0 case, the loop bounds/stride, and the
 * mode-out-of-range case all match. The remaining difference is on the
 * per-iteration active-byte check: the ROM computes it as
 *
 *     bl GameStateGetBuffer38C0
 *     adds r1, r5, #0      @ copy the byte-offset accumulator
 *     adds r0, r0, r1      @ base + offset
 *     adds r0, #44         @ + 20 (active's struct offset) + 24 (array base)
 *     ldrb r0, [r0, #0]
 *
 * i.e. it copies the offset accumulator into a scratch register before
 * adding it to the freshly-returned base, then adds the constant 44
 * separately. Every C shape tried for this one line reproduces the *count*
 * of instructions in some variant but not this exact grouping:
 *   - `p = base + offset; p += 44;` (two statements) and
 *     `p = base + (offset + 44);` (grouped) both compile to a *direct*
 *     `add r0, r0, offsetReg` with no copy -- the copy never appears unless
 *     the added constant is grouped with the offset *before* the base is
 *     touched, and doing that also reorders the addition (constant added
 *     to the copy first, base added last) instead of ROM's base-first,
 *     constant-last order.
 *   - Swapping the addition's operand order (`offset + (u8 *)GameState...`)
 *     changes nothing -- see "Addition" in docs/AGBCC_CODEGEN.md.
 *   - Replacing the named `offset` accumulator with `i * 24` (letting -O2
 *     strength-reduce the multiply itself) does produce a copy, but of a
 *     *second*, separately-materialized induction variable in a third
 *     register (r6) alongside `i`'s own register -- the ROM only ever uses
 *     two registers (r4, r5) for this loop.
 *   - Every variable declaration/initialization/increment order tried
 *     (offset declared before or after `i`; incremented before or after
 *     `i` in the loop body or in the `for` header) puts the loop's
 *     induction variable (`i`, tested against 63) in r5 and the byte-offset
 *     accumulator in r4; the ROM has them the other way around (counter in
 *     r4, offset in r5). This pairs with the copy above -- both look like
 *     one more instance of GCC 2.9's allocator not being steerable from
 *     this kind of plain C for this shape, in the same family as the
 *     documented OR/ADD register-tie cases, rather than a wrong
 *     reconstruction of the logic.
 *
 * The mode==-1 "found" return path (`return buffer + offset + 24;`) *does*
 * reproduce the ROM's copy-then-add grouping exactly once written as
 * `return (u8 *)GameStateGetBuffer38C0() + (offset + 24);` -- only the
 * per-iteration check differs.
 */

#include "gba/types.h"

struct GeneratedMapRuntimeRoom {
    s16 roomIndex;
    u8 unknown02[18];
    s8 active;
    s8 scriptFlag;
    u8 unknown16[2];
};

extern void *GameStateGetBuffer38C0(void);

void *GeneratedMapFindFreeRuntimeRoom(s32 mode)
{
    s32 i;
    s32 offset;
    struct GeneratedMapRuntimeRoom *room;

    if (mode == 0)
        return (u8 *)GameStateGetBuffer38C0() + 24;

    if (mode == -1) {
        offset = 0;
        for (i = 0; i <= 63; i++) {
            room = (struct GeneratedMapRuntimeRoom *)
                ((u8 *)GameStateGetBuffer38C0() + offset + 24);
            if (room->active == 0)
                return (u8 *)GameStateGetBuffer38C0() + (offset + 24);
            offset += 24;
        }
    }
    return 0;
}
