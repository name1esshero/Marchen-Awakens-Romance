/* Natural-C candidate for SpriteAffineAllocate (0x0807CC84).
 *
 * The previous matching source pinned registers, used an empty inline-assembly
 * barrier, and returned an integer through a pointer variable. Those are
 * compiler controls rather than a recovered source shape, so the exact routine
 * remains in assembly until this candidate can match without them.
 */
#include "sprite_engine.h"

s32 SpriteAffineAllocateCandidate(u16 key, u32 high, s16 low)
{
    struct SpriteEngineState *state = gSpriteEngineState;
    u32 available = ~(state->flags10 | state->flags14);
    u32 transform = (high << 16) | (u16)low;
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
