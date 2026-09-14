/* Fixed-type adapters for the battle object task created by 0802D3C0.
 * Callers pass the four dynamic arguments; each entry selects one of seven
 * object groups and one of five variants. */
#include "gba/types.h"

#include "rom_section.h"

extern void *sub_0802D3C0(s32 owner, s32 slot, void *resource, void *result,
                          s32 group, s32 variant);
#define BattleObjectCreate sub_0802D3C0

/** Create the battle task for constructor group 0, variant 0. */
AT("0002D078")
void *BattleObjectCreateGroup0Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 0, 0);
}
AT("0002D078") const u8 BattleObjectCreateGroup0Variant0Tail[2] = {0, 0};

/** Create the battle task for constructor group 0, variant 1. */
AT("0002D090")
void *BattleObjectCreateGroup0Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 0, 1);
}

/** Create the battle task for constructor group 0, variant 2. */
AT("0002D0A8")
void *BattleObjectCreateGroup0Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 0, 2);
}

/** Create the battle task for constructor group 0, variant 3. */
AT("0002D0C0")
void *BattleObjectCreateGroup0Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 0, 3);
}

/** Create the battle task for constructor group 0, variant 4. */
AT("0002D0D8")
void *BattleObjectCreateGroup0Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 0, 4);
}

/** Create the battle task for constructor group 1, variant 0. */
AT("0002D0F0")
void *BattleObjectCreateGroup1Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 1, 0);
}

/** Create the battle task for constructor group 1, variant 1. */
AT("0002D108")
void *BattleObjectCreateGroup1Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 1, 1);
}
AT("0002D108") const u8 BattleObjectCreateGroup1Variant1Tail[2] = {0, 0};

/** Create the battle task for constructor group 1, variant 2. */
AT("0002D120")
void *BattleObjectCreateGroup1Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 1, 2);
}

/** Create the battle task for constructor group 1, variant 3. */
AT("0002D138")
void *BattleObjectCreateGroup1Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 1, 3);
}

/** Create the battle task for constructor group 1, variant 4. */
AT("0002D150")
void *BattleObjectCreateGroup1Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 1, 4);
}

/** Create the battle task for constructor group 2, variant 0. */
AT("0002D168")
void *BattleObjectCreateGroup2Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 2, 0);
}

/** Create the battle task for constructor group 2, variant 1. */
AT("0002D180")
void *BattleObjectCreateGroup2Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 2, 1);
}

/** Create the battle task for constructor group 2, variant 2. */
AT("0002D198")
void *BattleObjectCreateGroup2Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 2, 2);
}
AT("0002D198") const u8 BattleObjectCreateGroup2Variant2Tail[2] = {0, 0};

/** Create the battle task for constructor group 2, variant 3. */
AT("0002D1B0")
void *BattleObjectCreateGroup2Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 2, 3);
}

/** Create the battle task for constructor group 2, variant 4. */
AT("0002D1C8")
void *BattleObjectCreateGroup2Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 2, 4);
}

/** Create the battle task for constructor group 3, variant 0. */
AT("0002D1E0")
void *BattleObjectCreateGroup3Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 3, 0);
}

/** Create the battle task for constructor group 3, variant 1. */
AT("0002D1F8")
void *BattleObjectCreateGroup3Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 3, 1);
}

/** Create the battle task for constructor group 3, variant 2. */
AT("0002D210")
void *BattleObjectCreateGroup3Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 3, 2);
}

/** Create the battle task for constructor group 3, variant 3. */
AT("0002D228")
void *BattleObjectCreateGroup3Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 3, 3);
}
AT("0002D228") const u8 BattleObjectCreateGroup3Variant3Tail[2] = {0, 0};

/** Create the battle task for constructor group 3, variant 4. */
AT("0002D240")
void *BattleObjectCreateGroup3Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 3, 4);
}

/** Create the battle task for constructor group 4, variant 0. */
AT("0002D258")
void *BattleObjectCreateGroup4Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 4, 0);
}

/** Create the battle task for constructor group 4, variant 1. */
AT("0002D270")
void *BattleObjectCreateGroup4Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 4, 1);
}

/** Create the battle task for constructor group 4, variant 2. */
AT("0002D288")
void *BattleObjectCreateGroup4Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 4, 2);
}

/** Create the battle task for constructor group 4, variant 3. */
AT("0002D2A0")
void *BattleObjectCreateGroup4Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 4, 3);
}

/** Create the battle task for constructor group 4, variant 4. */
AT("0002D2B8")
void *BattleObjectCreateGroup4Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 4, 4);
}
AT("0002D2B8") const u8 BattleObjectCreateGroup4Variant4Tail[2] = {0, 0};

/** Create the battle task for constructor group 5, variant 0. */
AT("0002D2D0")
void *BattleObjectCreateGroup5Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 5, 0);
}

/** Create the battle task for constructor group 5, variant 1. */
AT("0002D2E8")
void *BattleObjectCreateGroup5Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 5, 1);
}

/** Create the battle task for constructor group 5, variant 2. */
AT("0002D300")
void *BattleObjectCreateGroup5Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 5, 2);
}

/** Create the battle task for constructor group 5, variant 3. */
AT("0002D318")
void *BattleObjectCreateGroup5Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 5, 3);
}

/** Create the battle task for constructor group 5, variant 4. */
AT("0002D330")
void *BattleObjectCreateGroup5Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 5, 4);
}

/** Create the battle task for constructor group 6, variant 0. */
AT("0002D348")
void *BattleObjectCreateGroup6Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 6, 0);
}

/** Create the battle task for constructor group 6, variant 1. */
AT("0002D360")
void *BattleObjectCreateGroup6Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 6, 1);
}

/** Create the battle task for constructor group 6, variant 2. */
AT("0002D378")
void *BattleObjectCreateGroup6Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 6, 2);
}

/** Create the battle task for constructor group 6, variant 3. */
AT("0002D390")
void *BattleObjectCreateGroup6Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 6, 3);
}

/** Create the battle task for constructor group 6, variant 4. */
AT("0002D3A8")
void *BattleObjectCreateGroup6Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return BattleObjectCreate(owner, slot, resource, result, 6, 4);
}
