/* Typed views over fixed runtime and ROM tables. */
#include "gba/types.h"
#include "map_placements.h"
#include "item.h"

#include "runtime_state.h"
#include "rom_section.h"

struct _reent;
extern struct _reent *_impure_ptr;

/** Read the signed byte at secondary-runtime offset 0xE4B. Named by offset;
 * no caller has yet established what this field tracks.
 * @return The field's current value, sign-extended. */
AT("0000AF88")
s32 RuntimeGetSignedByteE4B(void)
{
    return *(s8 *)(gSecondaryRuntime + 0xE4B);
}

/** Read the signed byte at secondary-runtime offset 0xE4C, immediately after
 * the field read by RuntimeGetSignedByteE4B(). Meaning not yet recovered.
 * @return The field's current value, sign-extended. */
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

/** Look up a battle arena's layout table entry.
 * @param index Battle arena index.
 * @return Pointer to the arena's layout data. */
AT("00072C04")
void *GetBattleDefinition(s32 index)
{
    return (void *)gBattleArenaLayoutTable[index];
}

/** @return The newlib reentrancy struct backing the C library's per-task
 * state (errno and similar), as newlib itself defines it. */
AT("000868E8")
void *GetNewlibReentrancyState(void)
{
    return _impure_ptr;
}
