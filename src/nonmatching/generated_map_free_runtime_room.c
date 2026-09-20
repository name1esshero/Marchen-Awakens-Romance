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
 * not byte-exact. Explicit search/occupied labels recover the ROM's block
 * order, place the loop count in r4 and the byte offset in r5, and match every
 * instruction except the address addition in the active-byte check. The ROM
 * computes that address as
 *
 *     bl GameStateGetBuffer38C0
 *     adds r1, r5, #0      @ copy the byte-offset accumulator
 *     adds r0, r0, r1      @ base + offset
 *     adds r0, #44         @ + 20 (active's struct offset) + 24 (array base)
 *     ldrb r0, [r0, #0]
 *
 * i.e. it copies the offset accumulator into a scratch register before
 * adding it to the freshly-returned base, then adds the constant 44
 * separately. The closest ordinary C emits the equivalent single
 * `add r0, r0, r5`, making the candidate two bytes shorter. Other shapes
 * tried for this one line do not recover the copy:
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
 *   - Structured `for` and `while` spellings put the induction variable in
 *     r5 and the byte offset in r4. Naming the ROM's real occupied/continue
 *     edge explicitly fixes both registers and the success-block placement;
 *     it does not fix the final two- versus three-register add choice.
 *   - A typed wrapper containing the 24-byte buffer header followed by a
 *     64-element room array either recomputes `i * 24` on every pass or keeps
 *     the explicit offset but still coalesces it directly into r0. Member,
 *     integer-address, pointer-induction, and nested-room spellings were
 *     checked. The old_agbcc snapshot makes the same direct add as agbcc.
 *
 * The mode==-1 "found" return path (`return buffer + offset + 24;`) *does*
 * reproduce the ROM's copy-then-add grouping exactly once written as
 * `return (u8 *)GameStateGetBuffer38C0() + (offset + 24);` -- only the
 * per-iteration check differs.
 */

#include "map_generation.h"
#include "runtime_misc.h"

void *GeneratedMapFindFreeRuntimeRoom(s32 mode)
{
    s32 i;
    s32 offset;
    if (mode == 0)
        return (u8 *)GameStateGetBuffer38C0() + 24;

    if (mode != -1)
        goto no_room;

    offset = 0;
    i = 0;
search:
    if (*((s8 *)GameStateGetBuffer38C0() + offset + 24
          + GENERATED_MAP_RUNTIME_ROOM_ACTIVE_OFFSET) != 0)
        goto occupied;
    return (u8 *)GameStateGetBuffer38C0() + (offset + 24);

occupied:
    offset += sizeof(struct GeneratedMapRuntimeRoom);
    i++;
    if (i <= 63)
        goto search;

no_room:
    return 0;
}
