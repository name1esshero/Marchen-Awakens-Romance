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
    struct SpriteEngineState **global =
        (struct SpriteEngineState **)0x03006118;
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

/** Reserve the first affine slot that is absent from both allocation masks.
 * The transform is stored in the renderer's packed high:low representation.
 *
 * The register hints and the asm volatile("" : "+r"(available)) fence
 * below are load-bearing, not decoration: removing them (tested) changes
 * agbcc's instruction selection and the function no longer matches the
 * ROM byte-for-byte. */
AT("0007CC84")
s32 SpriteAffineAllocate(u16 key, u32 high, s16 low)
{
    u32 wantedKey = key;
    s32 lowValue = low;
    struct SpriteEngineState **global =
        (struct SpriteEngineState **)0x03006118;
    u32 available;
    s32 slot;
    register u32 one TARGET_REGISTER("r12");
    struct SpriteEngineState **savedGlobal;
    register u32 transform TARGET_REGISTER("r4");
    u32 offset;
    register struct SpriteEngineState *state TARGET_REGISTER("r0") = *global;
    u32 used = state->flags10;
    u32 reserved = state->flags14;

    available = ~reserved;
    asm volatile("" : "+r"(available));
    available &= ~used;
    slot = 0;
    one = 1;
    savedGlobal = global;
    transform = high << 16;
    transform |= lowValue;
    offset = 28;

    {
        register u32 mask TARGET_REGISTER("r2");

    search:
        mask = one << slot;
        if ((available & mask) == 0)
            goto next;

        state = *savedGlobal;
        state->flags10 |= mask;
        {
            struct SpriteAffineSlot *entry =
                (struct SpriteAffineSlot *)((u8 *)state + offset);

            entry->key = wantedKey;
            entry->transform = transform;
        }
        state->affineSearchCursor = slot;
        state = (struct SpriteEngineState *)slot;
        goto done;

    next:
        offset += sizeof(struct SpriteAffineSlot);
        slot++;
        if (slot <= 31)
            goto search;
        state = (struct SpriteEngineState *)-1;

    done:
        return (s32)state;
    }
}
AT("0007CC84") const u8 SpriteAffineAllocateTail[2] = {0, 0};
