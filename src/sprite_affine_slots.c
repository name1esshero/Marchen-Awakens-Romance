/* Affine-transform slot lookup for the sprite renderer. */
#include "sprite_engine.h"

#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#else
#define TARGET_REGISTER(name) asm(name)
#endif

/* Find a live affine slot with the requested key and packed transform.  The
 * search wraps at 32 entries and begins at the last successful slot. */
AT("0007CC18")
s32 SpriteAffineFind(u16 key, u32 high, s16 low)
{
    register u32 wantedKey TARGET_REGISTER("r12") = key;
    register s32 lowValue TARGET_REGISTER("r2") = low;
    register u32 slot TARGET_REGISTER("r4");
    register u32 occupied TARGET_REGISTER("r5");
    register u32 transform TARGET_REGISTER("r6");
    s32 checked;
    struct SpriteEngineState **global =
        (struct SpriteEngineState **)0x03006118;
    register struct SpriteEngineState **savedGlobal TARGET_REGISTER("r8");
    register u32 slotMask TARGET_REGISTER("r3");

    slot = (*global)->affineSearchCursor;
    transform = high << 16;
    transform |= lowValue;
    occupied = (*global)->flags10 ^ (*global)->flags14;
    checked = 0;
    savedGlobal = global;
    slotMask = 31;

    do {
        register u32 one TARGET_REGISTER("r0");

        slot &= slotMask;
        one = 1;
        if ((occupied & (one << slot)) != 0) {
            register u32 entryOffset TARGET_REGISTER("r0") = slot << 3;
            register struct SpriteEngineState **globalReg TARGET_REGISTER("r2");
            register struct SpriteEngineState *state TARGET_REGISTER("r1");
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

/* Reserve the first affine slot that is absent from both allocation masks.
 * The transform is stored in the renderer's packed high:low representation. */
AT("0007CC84")
s32 SpriteAffineAllocate(u16 key, u32 high, s16 low)
{
    register u32 wantedKey TARGET_REGISTER("r8") = key;
    register s32 lowValue TARGET_REGISTER("r2") = low;
    register struct SpriteEngineState **global TARGET_REGISTER("r4") =
        (struct SpriteEngineState **)0x03006118;
    register u32 available TARGET_REGISTER("r5");
    register s32 slot TARGET_REGISTER("r3");
    register u32 one TARGET_REGISTER("r12");
    struct SpriteEngineState **savedGlobal;
    register u32 transform TARGET_REGISTER("r4");
    register u32 offset TARGET_REGISTER("r6");
    register struct SpriteEngineState *state TARGET_REGISTER("r0") = *global;
    register u32 used TARGET_REGISTER("r3") = state->flags10;
    register u32 reserved TARGET_REGISTER("r0") = state->flags14;

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
            register struct SpriteAffineSlot *entry TARGET_REGISTER("r1") =
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
