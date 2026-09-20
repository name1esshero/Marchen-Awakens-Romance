/* Affine-transform slot lookup for the sprite renderer. */
#include "sprite_engine.h"

#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#else
#define TARGET_REGISTER(name) asm(name)
#endif

/** Find a live affine slot with the requested key and packed transform.  The
 * search wraps at 32 entries and begins at the last successful slot.
 *
 * The register hints below are load-bearing, not decoration: removing
 * them (tested) changes agbcc's instruction selection and the function
 * no longer matches the ROM byte-for-byte. */
AT("0007CC18")
s32 SpriteAffineFind(u16 key, u32 high, s16 low)
{
    u32 wantedKey = key;
    s32 lowValue = low;
    u32 slot;
    u32 occupied;
    u32 transform;
    s32 checked;
    struct SpriteEngineState **global = &gSpriteEngineState;
    struct SpriteEngineState **savedGlobal;
    u32 slotMask;

    slot = (*global)->affineSearchCursor;
    transform = high << 16;
    transform |= lowValue;
    occupied = (*global)->flags10 ^ (*global)->flags14;
    checked = 0;
    savedGlobal = global;
    slotMask = 31;

    do {
        u32 one;

        slot &= slotMask;
        one = 1;
        if ((occupied & (one << slot)) != 0) {
            register u32 entryOffset TARGET_REGISTER("r0") = slot << 3;
            register struct SpriteEngineState **globalReg TARGET_REGISTER("r2");
            struct SpriteEngineState *state;
            register struct SpriteAffineSlot *entry TARGET_REGISTER("r2");

            entryOffset += 28;
            globalReg = savedGlobal;
            state = *globalReg;
            entry = (struct SpriteAffineSlot *)((u8 *)state + entryOffset);
            if (entry->key == wantedKey && entry->transform == transform) {
                state->affineSearchCursor = slot;
                return slot;
            }
        }
        checked++;
        slot++;
    } while (checked <= 31);
    return -1;
}
AT("0007CC18") const u8 SpriteAffineFindTail[2] = {0, 0};
