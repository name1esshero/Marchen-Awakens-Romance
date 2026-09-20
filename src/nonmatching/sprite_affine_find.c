#include "sprite_engine.h"

#define SPRITE_AFFINE_SLOT_COUNT 32
#define SPRITE_AFFINE_SLOT_MASK (SPRITE_AFFINE_SLOT_COUNT - 1)

/**
 * @brief Find a live affine slot with a matching key and transform.
 * @param key Resource key stored in the slot.
 * @param high High halfword of the packed transform.
 * @param low Signed low halfword of the packed transform.
 * @return The matching slot index, or -1 when no slot matches.
 *
 * The search begins at the last successful slot and wraps across all 32
 * entries. The exact ROM rematerializes the unit bit inside the loop and keeps
 * the mutable engine-root address in r8. Both agbcc frontends hoist the unit
 * bit into a second saved register from this natural source, so the exact
 * implementation remains in assembly pending source-shape recovery.
 */
s32 SpriteAffineFind(u16 key, u32 high, s16 low)
{
    struct SpriteEngineState * volatile *global = &gSpriteEngineState;
    u32 wantedTransform = (high << 16) | (u16)low;
    u32 occupied;
    u32 slot;
    s32 checked;

    slot = (*global)->affineSearchCursor;
    occupied = (*global)->flags10 ^ (*global)->flags14;
    for (checked = 0; checked < SPRITE_AFFINE_SLOT_COUNT; checked++, slot++) {
        struct SpriteEngineState *state;
        struct SpriteAffineSlot *entry;

        slot &= SPRITE_AFFINE_SLOT_MASK;
        if ((occupied & (1u << slot)) == 0)
            continue;

        state = *global;
        entry = &state->affineSlots[slot];
        if (entry->key == key && entry->transform == wantedTransform) {
            state->affineSearchCursor = slot;
            return slot;
        }
    }
    return -1;
}
