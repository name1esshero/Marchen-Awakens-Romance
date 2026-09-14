/* Fixed-selector adapters for battle task constructors. */
#include "gba/types.h"

#include "rom_section.h"

extern void *sub_0802DBE0(s32, s32, void *, void *, s32);
extern void *sub_0802E3D4(s32, s32, void *, void *, s32);
extern void *sub_0802ED8C(s32, s32, void *, void *, s32, s32);
extern void *sub_0802F3A4(s32, s32, void *, void *, s32);
extern void *sub_08031158(s32, s32, void *, void *, s32);

AT("0002DB7C")
void *BattleTaskDCreateVariant0(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802DBE0(owner, slot, resource, result, 0);
}

AT("0002DB90")
void *BattleTaskDCreateVariant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802DBE0(owner, slot, resource, result, 1);
}

AT("0002DBA4")
void *BattleTaskDCreateVariant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802DBE0(owner, slot, resource, result, 2);
}

AT("0002DBB8")
void *BattleTaskDCreateVariant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802DBE0(owner, slot, resource, result, 3);
}

AT("0002DBCC")
void *BattleTaskDCreateVariant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802DBE0(owner, slot, resource, result, 4);
}

AT("0002E370")
void *BattleTaskECreateVariant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802E3D4(owner, slot, resource, result, 1);
}

AT("0002E384")
void *BattleTaskECreateVariant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802E3D4(owner, slot, resource, result, 2);
}

AT("0002E398")
void *BattleTaskECreateVariant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802E3D4(owner, slot, resource, result, 3);
}

AT("0002E3AC")
void *BattleTaskECreateVariant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802E3D4(owner, slot, resource, result, 4);
}

AT("0002E3C0")
void *BattleTaskECreateVariant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802E3D4(owner, slot, resource, result, 5);
}

AT("0002F340")
void *BattleTaskFCreateVariant5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802F3A4(owner, slot, resource, result, 5);
}

AT("0002F354")
void *BattleTaskFCreateVariant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802F3A4(owner, slot, resource, result, 4);
}

AT("0002F368")
void *BattleTaskFCreateVariant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802F3A4(owner, slot, resource, result, 3);
}

AT("0002F37C")
void *BattleTaskFCreateVariant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802F3A4(owner, slot, resource, result, 2);
}

AT("0002F390")
void *BattleTaskFCreateVariant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802F3A4(owner, slot, resource, result, 1);
}

AT("00031108")
void *BattleTaskGCreateVariant1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_08031158(owner, slot, resource, result, 1);
}

AT("0003111C")
void *BattleTaskGCreateVariant2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_08031158(owner, slot, resource, result, 2);
}

AT("00031130")
void *BattleTaskGCreateVariant3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_08031158(owner, slot, resource, result, 3);
}

AT("00031144")
void *BattleTaskGCreateVariant4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_08031158(owner, slot, resource, result, 4);
}

AT("0002ECF8")
void *BattleTaskHCreateGroup1(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802ED8C(owner, slot, resource, result, 1, 0);
}

AT("0002ED10")
void *BattleTaskHCreateGroup2(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802ED8C(owner, slot, resource, result, 2, 0);
}

AT("0002ED28")
void *BattleTaskHCreateGroup3(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802ED8C(owner, slot, resource, result, 3, 0);
}

AT("0002ED40")
void *BattleTaskHCreateGroup4(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802ED8C(owner, slot, resource, result, 4, 0);
}

AT("0002ED58")
void *BattleTaskHCreateGroup5(s32 owner, s32 slot, void *resource, void *result)
{
    return sub_0802ED8C(owner, slot, resource, result, 5, 0);
}

AT("0002ED70")
void *BattleTaskHCreateSelected(s32 owner, s32 slot, void *resource, void *result, s16 group)
{
    return sub_0802ED8C(owner, slot, resource, result, group, 1);
}
