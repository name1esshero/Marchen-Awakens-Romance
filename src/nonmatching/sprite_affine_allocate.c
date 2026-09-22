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
 * The two callers at 0x0807C650 and 0x0807CB0E narrow the final two arguments
 * to signed halfwords before calling both SpriteAffineFind and this routine.
 * They are therefore a related pair of coordinate-like values, rather than
 * arbitrary halves inferred only from this function. Modeling them as one
 * by-value structure is not the missing source shape: agbcc then performs the
 * low-half narrowing through r0, while the ROM performs it through r2.
 *
 * A closer clean experiment introduced ordinary promoted `s32` locals for
 * both signed inputs and kept the result join explicit. That naturally
 * recovered the ROM's r8 lifetime for `key`, the unit mask in `ip`, the r6
 * entry offset, signed narrowing order, and control-flow layout. Testing all
 * 5,040 dependency-valid setup orders also recovered the packed transform in
 * r4. None recovered the remaining r3/r5 slot/availability allocation or the
 * ROM's transient r4-to-r7 global-root handoff. Those are now the precise
 * open differences. No artificial lifetime, volatile qualifier, register
 * declaration, or inline-assembly barrier is justified by the callers, so the
 * exact routine remains in assembly until another real data relationship
 * explains that final allocation.
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
