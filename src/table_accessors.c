/* Typed views over fixed runtime and ROM tables. */
#include "gba/types.h"
#include "map_placements.h"
#include "item.h"

#include "runtime_state.h"
#include "rom_section.h"

AT("0000AF88")
s32 RuntimeGetSignedByteE4B(void)
{
    return *(s8 *)(gSecondaryRuntime + 0xE4B);
}

AT("0000AFCC")
s32 RuntimeGetSignedByteE4C(void)
{
    return *(s8 *)(gSecondaryRuntime + 0xE4C);
}

#ifdef NONMATCHING
AT("00056F68")
void *ArmGetField78Pointer(s32 armId)
{
    return (void *)gArmDefinitions[(s16)armId].field78;
}

AT("00056F7C")
u32 ArmGetField7C(s32 armId)
{
    return gArmDefinitions[(s16)armId].field7C;
}
#endif

AT("00072C04")
void *GetBattleDefinition(s32 index)
{
    return (void *)gBattleArenaLayoutTable[index];
}

AT("000868E8")
void *GetNewlibReentrancyState(void)
{
    return *(void **)0x08F2AFEC;
}
