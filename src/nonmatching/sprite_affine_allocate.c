/* Natural-C candidate for SpriteAffineAllocate (0x0807CC84).
 *
 * The previous matching source pinned registers, used an empty inline-assembly
 * barrier, and returned an integer through a pointer variable. Those are
 * compiler controls rather than a recovered source shape, so the exact routine
 * remains in assembly until this candidate can match without them.
 *
 * Two real bugs in an earlier version of this candidate are fixed below,
 * each confirmed by direct instruction comparison against the ROM
 * disassembly (not just re-reading the C):
 *
 *  - `~(state->flags10 | state->flags14)` computed the OR before the NOT.
 *    The ROM instead loads flags10, then flags14, negates flags14, then
 *    BICs (ANDs-NOT) flags10 into it -- De Morgan's `~flags14 & ~flags10`,
 *    not the OR-then-NOT this candidate previously had. Writing it as
 *    `~state->flags10 & ~state->flags14` (the two NOTs written in the
 *    opposite order from the ROM's own load order, since agbcc loads the
 *    *second*-written operand first) reproduces the ROM's load order and
 *    the mvn/bic pair exactly.
 *  - `(high << 16) | (u16)low` zero-extended `low`. The ROM narrows it with
 *    `asrs` (sign-extend), matching its declared `s16` parameter type; the
 *    `(u16)` cast was silently telling agbcc to zero-extend instead, which
 *    is a real behavioral difference for negative `low` values, not just a
 *    codegen difference. Plain `low` (relying on the parameter's own signed
 *    type) reproduces the ROM's `asrs`.
 *
 * What remains open after both fixes: the ROM preserves `key` across the
 * loop in r8 (with the extra `mov r7,r8`/`push {r7}` prologue dance Thumb
 * needs to save a high register), while this candidate's `key` lives in the
 * unsaved `ip` register instead, needing no such dance. This looks like a
 * register-pressure threshold difference rather than an instruction-order
 * one: something about the ROM's original source shape makes agbcc's
 * allocator decide it is under enough pressure to spill to r8, and nothing
 * tried here reproduces that pressure. Not yet attempted: restructuring the
 * loop to keep more values simultaneously live (e.g. not letting `mask` die
 * before the store), which might be the missing ingredient.
 */
#include "sprite_engine.h"

s32 SpriteAffineAllocateCandidate(u16 key, u32 high, s16 low)
{
    struct SpriteEngineState *state = gSpriteEngineState;
    u32 available = ~state->flags10 & ~state->flags14;
    u32 transform = (high << 16) | low;
    s32 slot;

    for (slot = 0; slot < 32; slot++)
    {
        u32 mask = 1 << slot;

        if (available & mask)
        {
            struct SpriteAffineSlot *entry = &state->affineSlots[slot];

            state->flags10 |= mask;
            entry->key = key;
            entry->transform = transform;
            state->affineSearchCursor = slot;
            return slot;
        }
    }
    return -1;
}
