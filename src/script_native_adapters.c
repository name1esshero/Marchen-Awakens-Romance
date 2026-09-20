/* Small native-script adapters used by map and event bytecode.
 *
 * The VM stores every argument in a 32-bit slot. Return value 1 completes the
 * native immediately; 0x7FFF hands control back to the script scheduler.
 */
#include "gba/types.h"
#include "flags.h"

#include "rom_section.h"
#include "runtime_misc.h"
#include "task_constructors.h"

extern void GameStateSetField12EC(s32 value);
extern s32 GameStateGetField4256(void);
extern s32 GameStateGetField60E(void);
extern void sub_08008740(s32 a, s32 b);
extern s32 GameStateGetEntry3894(s32 a, s32 b, s32 index);
extern void sub_080088E0(s32 a, s32 b, s32 c);
extern void GameStateSetEntry3894(s32 a, s32 b, s32 index, s32 value);
extern void sub_080561B8(s32 value);
extern void sub_080083E0(s32 a, s32 b);

/** Native script command: forward to GameStateSetField12EC().
 * @return Always 1. */
AT("00012978") s32 ScriptNativeSetField12EC(u32 count, const s32 *args, s32 *result)
{
    GameStateSetField12EC(args[0]);
    return 1;
}
AT("00012978") const u8 ScriptNativeSetField12ECTail[2] = {0};

/** Initialize the background-attribute enable bank. @return Always 1. */
AT("00012988") s32 ScriptNativeInitializeAttributeFlags(u32 count,
    const s32 *args, s32 *result)
{
    GameStateInitializeAttributeFlags();
    return 1;
}

/** Set one background-attribute enable bit. @return Always 1. */
AT("00012994") s32 ScriptNativeSetAttributeFlag(u32 count, const s32 *args,
                                                 s32 *result)
{
    GameStateSetAttributeFlag(args[0], args[1]);
    return 1;
}

/** Test one background-attribute enable bit. @return Always 1. */
AT("000129A4") s32 ScriptNativeTestAttributeFlag(u32 count, const s32 *args,
                                                  s32 *result)
{
    *result = GameStateTestAttributeFlag(args[0]);
    return 1;
}

/** Set an inclusive range of background-attribute enable bits.
 * @return Always 1. */
AT("000129B8") s32 ScriptNativeSetAttributeFlagRange(u32 count,
    const s32 *args, s32 *result)
{
    GameStateSetAttributeFlagRange(args[0], args[1], args[2]);
    return 1;
}

/** Native script command: forward to GameStateGetField4256() (the active
 * field/map id). @return Always 1. */
AT("000129CC") s32 ScriptNativeGetField4256(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateGetField4256();
    return 1;
}
AT("000129CC") const u8 ScriptNativeGetField4256Tail[2] = {0};

/** Native script command: forward to GameStateGetField60E().
 * @return Always 1. */
AT("000129E0") s32 ScriptNativeGetField60E(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateGetField60E();
    return 1;
}
AT("000129E0") const u8 ScriptNativeGetField60ETail[2] = {0};

/** Native script command: forward two arguments to sub_08008740().
 * @return Always 1. */
AT("00012AD4") s32 ScriptNativeCall08740(u32 count, const s32 *args, s32 *result)
{
    sub_08008740(args[0], args[1]);
    return 1;
}

/** Native script command: optionally overwrite the game state's three
 * 3894-table resource slots (index -1 leaves a slot unchanged), then
 * re-apply each of the three current values through sub_080561B8() and
 * notify sub_080083E0().
 * @return Always 1. */
AT("00012AE4") s32 ScriptNativeConfigureResourceSlots(u32 count,
    const s32 *args, s32 *result)
{
    s32 value;
    if (args[0] != -1)
        GameStateSetEntry3894(0, 0, 0, args[0]);
    if (args[1] != -1)
        GameStateSetEntry3894(0, 0, 1, args[1]);
    if (args[2] != -1)
        GameStateSetEntry3894(0, 0, 2, args[2]);

    value = (s16)GameStateGetEntry3894(0, 0, 0);
    sub_080561B8(value);
    value = (s16)GameStateGetEntry3894(0, 0, 1);
    sub_080561B8(value);
    value = (s16)GameStateGetEntry3894(0, 0, 2);
    sub_080561B8(value);
    sub_080083E0(0, 1);
    return 1;
}
AT("00012AE4") const u8 ScriptNativeConfigureResourceSlotsTail[2] = {0};

/** Native script command: read one of the game state's 3894-table resource
 * slots. @return Always 1. */
AT("00012B64") s32 ScriptNativeQueryResourceSlot(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)GameStateGetEntry3894(0, 0, args[0]);
    return 1;
}

/** Native script command: forward three arguments to sub_080088E0().
 * @return Always 1. */
AT("00012C54") s32 ScriptNativeCall088E0(u32 count, const s32 *args, s32 *result)
{
    sub_080088E0(args[0], args[1], args[2]);
    return 1;
}

/** Native script command: forward a signed x/y coordinate pair (packed as
 * two of the VM's 32-bit argument slots) to CreateMapCoordinateTask().
 * @return Always 0x7FFF. */
AT("00012CD0") s32 ScriptNativeMapCoordinateCall(u32 count, const s32 *args, s32 *result)
{
    const s16 *coordinates = (const s16 *)args;
    CreateMapCoordinateTask(coordinates[0], coordinates[2], result);
    return 0x7FFF;
}
