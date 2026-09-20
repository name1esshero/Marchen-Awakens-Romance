/*
 * Clean reference implementation for ROM 0x08057078. The inventory address
 * is formed with ordinary pointer arithmetic, matching the operation visible
 * in the ROM: load the game-state pointer, add the inventory offset, then
 * advance that pointer by one signed-halfword slot per iteration.
 *
 * This candidate is intentionally excluded from the matching build. agbcc
 * assigns the base and offset to different low registers than the ROM. A
 * pointer/u32 union can force the desired allocation, but that changes the C
 * model solely to steer code generation and is rejected under
 * PRET_STANDARDS.md. The exact implementation remains in
 * asm/code/code_0500C0.s until a natural source shape is found.
 */
#include "game_state.h"
#include "gba/types.h"

enum
{
    CONSUMABLE_INVENTORY_CAPACITY = 256,
    CONSUMABLE_INVENTORY_OFFSET = 0x31D0,
    FIXED_POINT_ONE = 1 << 16
};

s32 CountConsumableInventoryCopiesCandidate(s32 id)
{
    s32 target;
    s32 count;
    u8 *entry;
    s32 indexFixed;
    s32 countFixed;
    s32 step;

    target = (s16)id;
    count = 0;
    entry = gMapGenerationRoot + CONSUMABLE_INVENTORY_OFFSET;
    indexFixed = FIXED_POINT_ONE;
    countFixed = indexFixed;
    step = indexFixed;
    do
    {
        if (*(s16 *)entry == target)
        {
            s32 previous = countFixed;

            countFixed += step;
            count = previous >> 16;
        }
        {
            s32 previous = indexFixed;

            indexFixed += step;
            entry += sizeof(s16);
            if ((previous >> 16) >= CONSUMABLE_INVENTORY_CAPACITY)
                break;
        }
    } while (TRUE);
    return count;
}
