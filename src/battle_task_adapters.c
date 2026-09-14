/* Fixed group/variant adapters for two battle task implementations.
 * The four dynamic arguments identify the owner, slot, resource and optional
 * result. These entry tables specialize the final two task fields. */
#include "gba/types.h"

#include "rom_section.h"

extern void *sub_0804A720(s32 owner, s32 slot, void *resource, void *result, s32 group, s32 variant);
extern void *sub_0804EF70(s32 owner, s32 slot, void *resource, void *result, s32 group, s32 variant);

/** Create the battle task for constructor group 0, variant 0. */
AT("0004A3D8")
void *BattleTaskACreateGroup0Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 0, 0);
}
AT("0004A3D8") const u8 BattleTaskACreateGroup0Variant0Tail[2] = {0, 0};

/** Create the battle task for constructor group 0, variant 1. */
AT("0004A3F0")
void *BattleTaskACreateGroup0Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 0, 1);
}

/** Create the battle task for constructor group 0, variant 2. */
AT("0004A408")
void *BattleTaskACreateGroup0Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 0, 2);
}

/** Create the battle task for constructor group 0, variant 3. */
AT("0004A420")
void *BattleTaskACreateGroup0Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 0, 3);
}

/** Create the battle task for constructor group 0, variant 4. */
AT("0004A438")
void *BattleTaskACreateGroup0Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 0, 4);
}

/** Create the battle task for constructor group 1, variant 0. */
AT("0004A450")
void *BattleTaskACreateGroup1Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 1, 0);
}

/** Create the battle task for constructor group 1, variant 1. */
AT("0004A468")
void *BattleTaskACreateGroup1Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 1, 1);
}
AT("0004A468") const u8 BattleTaskACreateGroup1Variant1Tail[2] = {0, 0};

/** Create the battle task for constructor group 1, variant 2. */
AT("0004A480")
void *BattleTaskACreateGroup1Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 1, 2);
}

/** Create the battle task for constructor group 1, variant 3. */
AT("0004A498")
void *BattleTaskACreateGroup1Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 1, 3);
}

/** Create the battle task for constructor group 1, variant 4. */
AT("0004A4B0")
void *BattleTaskACreateGroup1Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 1, 4);
}

/** Create the battle task for constructor group 2, variant 0. */
AT("0004A4C8")
void *BattleTaskACreateGroup2Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 2, 0);
}

/** Create the battle task for constructor group 2, variant 1. */
AT("0004A4E0")
void *BattleTaskACreateGroup2Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 2, 1);
}

/** Create the battle task for constructor group 2, variant 2. */
AT("0004A4F8")
void *BattleTaskACreateGroup2Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 2, 2);
}
AT("0004A4F8") const u8 BattleTaskACreateGroup2Variant2Tail[2] = {0, 0};

/** Create the battle task for constructor group 2, variant 3. */
AT("0004A510")
void *BattleTaskACreateGroup2Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 2, 3);
}

/** Create the battle task for constructor group 2, variant 4. */
AT("0004A528")
void *BattleTaskACreateGroup2Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 2, 4);
}

/** Create the battle task for constructor group 3, variant 0. */
AT("0004A540")
void *BattleTaskACreateGroup3Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 3, 0);
}

/** Create the battle task for constructor group 3, variant 1. */
AT("0004A558")
void *BattleTaskACreateGroup3Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 3, 1);
}

/** Create the battle task for constructor group 3, variant 2. */
AT("0004A570")
void *BattleTaskACreateGroup3Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 3, 2);
}

/** Create the battle task for constructor group 3, variant 3. */
AT("0004A588")
void *BattleTaskACreateGroup3Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 3, 3);
}
AT("0004A588") const u8 BattleTaskACreateGroup3Variant3Tail[2] = {0, 0};

/** Create the battle task for constructor group 3, variant 4. */
AT("0004A5A0")
void *BattleTaskACreateGroup3Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 3, 4);
}

/** Create the battle task for constructor group 4, variant 0. */
AT("0004A5B8")
void *BattleTaskACreateGroup4Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 4, 0);
}

/** Create the battle task for constructor group 4, variant 1. */
AT("0004A5D0")
void *BattleTaskACreateGroup4Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 4, 1);
}

/** Create the battle task for constructor group 4, variant 2. */
AT("0004A5E8")
void *BattleTaskACreateGroup4Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 4, 2);
}

/** Create the battle task for constructor group 4, variant 3. */
AT("0004A600")
void *BattleTaskACreateGroup4Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 4, 3);
}

/** Create the battle task for constructor group 4, variant 4. */
AT("0004A618")
void *BattleTaskACreateGroup4Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 4, 4);
}
AT("0004A618") const u8 BattleTaskACreateGroup4Variant4Tail[2] = {0, 0};

/** Create the battle task for constructor group 5, variant 0. */
AT("0004A630")
void *BattleTaskACreateGroup5Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 5, 0);
}

/** Create the battle task for constructor group 5, variant 1. */
AT("0004A648")
void *BattleTaskACreateGroup5Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 5, 1);
}

/** Create the battle task for constructor group 5, variant 2. */
AT("0004A660")
void *BattleTaskACreateGroup5Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 5, 2);
}

/** Create the battle task for constructor group 5, variant 3. */
AT("0004A678")
void *BattleTaskACreateGroup5Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 5, 3);
}

/** Create the battle task for constructor group 5, variant 4. */
AT("0004A690")
void *BattleTaskACreateGroup5Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 5, 4);
}

/** Create the battle task for constructor group 6, variant 0. */
AT("0004A6A8")
void *BattleTaskACreateGroup6Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 6, 0);
}

/** Create the battle task for constructor group 6, variant 1. */
AT("0004A6C0")
void *BattleTaskACreateGroup6Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 6, 1);
}

/** Create the battle task for constructor group 6, variant 2. */
AT("0004A6D8")
void *BattleTaskACreateGroup6Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 6, 2);
}

/** Create the battle task for constructor group 6, variant 3. */
AT("0004A6F0")
void *BattleTaskACreateGroup6Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 6, 3);
}

/** Create the battle task for constructor group 6, variant 4. */
AT("0004A708")
void *BattleTaskACreateGroup6Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804A720(owner, slot, resource, result, 6, 4);
}

/** Create the battle task for constructor group 0, variant 0. */
AT("0004EC28")
void *BattleTaskBCreateGroup0Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 0);
}
AT("0004EC28") const u8 BattleTaskBCreateGroup0Variant0Tail[2] = {0, 0};

/** Create the battle task for constructor group 0, variant 1. */
AT("0004EC40")
void *BattleTaskBCreateGroup0Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 1);
}

/** Create the battle task for constructor group 0, variant 2. */
AT("0004EC58")
void *BattleTaskBCreateGroup0Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 2);
}

/** Create the battle task for constructor group 0, variant 3. */
AT("0004EC70")
void *BattleTaskBCreateGroup0Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 3);
}

/** Create the battle task for constructor group 0, variant 4. */
AT("0004EC88")
void *BattleTaskBCreateGroup0Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 4);
}

/** Create the battle task for constructor group 0, variant 5. */
AT("0004ECA0")
void *BattleTaskBCreateGroup0Variant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 5);
}

/** Create the battle task for constructor group 0, variant 6. */
AT("0004ECB8")
void *BattleTaskBCreateGroup0Variant6(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 0, 6);
}

/** Create the battle task for constructor group 1, variant 0. */
AT("0004ECD0")
void *BattleTaskBCreateGroup1Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 0);
}

/** Create the battle task for constructor group 1, variant 1. */
AT("0004ECE8")
void *BattleTaskBCreateGroup1Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 1);
}
AT("0004ECE8") const u8 BattleTaskBCreateGroup1Variant1Tail[2] = {0, 0};

/** Create the battle task for constructor group 1, variant 2. */
AT("0004ED00")
void *BattleTaskBCreateGroup1Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 2);
}

/** Create the battle task for constructor group 1, variant 3. */
AT("0004ED18")
void *BattleTaskBCreateGroup1Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 3);
}

/** Create the battle task for constructor group 1, variant 4. */
AT("0004ED30")
void *BattleTaskBCreateGroup1Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 4);
}

/** Create the battle task for constructor group 1, variant 5. */
AT("0004ED48")
void *BattleTaskBCreateGroup1Variant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 5);
}

/** Create the battle task for constructor group 1, variant 6. */
AT("0004ED60")
void *BattleTaskBCreateGroup1Variant6(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 1, 6);
}

/** Create the battle task for constructor group 2, variant 0. */
AT("0004ED78")
void *BattleTaskBCreateGroup2Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 0);
}

/** Create the battle task for constructor group 2, variant 1. */
AT("0004ED90")
void *BattleTaskBCreateGroup2Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 1);
}

/** Create the battle task for constructor group 2, variant 2. */
AT("0004EDA8")
void *BattleTaskBCreateGroup2Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 2);
}
AT("0004EDA8") const u8 BattleTaskBCreateGroup2Variant2Tail[2] = {0, 0};

/** Create the battle task for constructor group 2, variant 3. */
AT("0004EDC0")
void *BattleTaskBCreateGroup2Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 3);
}

/** Create the battle task for constructor group 2, variant 4. */
AT("0004EDD8")
void *BattleTaskBCreateGroup2Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 4);
}

/** Create the battle task for constructor group 2, variant 5. */
AT("0004EDF0")
void *BattleTaskBCreateGroup2Variant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 5);
}

/** Create the battle task for constructor group 2, variant 6. */
AT("0004EE08")
void *BattleTaskBCreateGroup2Variant6(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 2, 6);
}

/** Create the battle task for constructor group 3, variant 0. */
AT("0004EE20")
void *BattleTaskBCreateGroup3Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 0);
}

/** Create the battle task for constructor group 3, variant 1. */
AT("0004EE38")
void *BattleTaskBCreateGroup3Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 1);
}

/** Create the battle task for constructor group 3, variant 2. */
AT("0004EE50")
void *BattleTaskBCreateGroup3Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 2);
}

/** Create the battle task for constructor group 3, variant 3. */
AT("0004EE68")
void *BattleTaskBCreateGroup3Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 3);
}
AT("0004EE68") const u8 BattleTaskBCreateGroup3Variant3Tail[2] = {0, 0};

/** Create the battle task for constructor group 3, variant 4. */
AT("0004EE80")
void *BattleTaskBCreateGroup3Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 4);
}

/** Create the battle task for constructor group 3, variant 5. */
AT("0004EE98")
void *BattleTaskBCreateGroup3Variant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 5);
}

/** Create the battle task for constructor group 3, variant 6. */
AT("0004EEB0")
void *BattleTaskBCreateGroup3Variant6(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 3, 6);
}

/** Create the battle task for constructor group 4, variant 0. */
AT("0004EEC8")
void *BattleTaskBCreateGroup4Variant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 0);
}

/** Create the battle task for constructor group 4, variant 1. */
AT("0004EEE0")
void *BattleTaskBCreateGroup4Variant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 1);
}

/** Create the battle task for constructor group 4, variant 2. */
AT("0004EEF8")
void *BattleTaskBCreateGroup4Variant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 2);
}

/** Create the battle task for constructor group 4, variant 3. */
AT("0004EF10")
void *BattleTaskBCreateGroup4Variant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 3);
}

/** Create the battle task for constructor group 4, variant 4. */
AT("0004EF28")
void *BattleTaskBCreateGroup4Variant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 4);
}
AT("0004EF28") const u8 BattleTaskBCreateGroup4Variant4Tail[2] = {0, 0};

/** Create the battle task for constructor group 4, variant 5. */
AT("0004EF40")
void *BattleTaskBCreateGroup4Variant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 5);
}

/** Create the battle task for constructor group 4, variant 6. */
AT("0004EF58")
void *BattleTaskBCreateGroup4Variant6(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0804EF70(owner, slot, resource, result, 4, 6);
}
