/* Small native-script adapters used by map and event bytecode.
 *
 * The VM stores every argument in a 32-bit slot. Return value 1 completes the
 * native immediately; 0x7FFF hands control back to the script scheduler.
 */
#include "gba/types.h"

#include "rom_section.h"

extern void GameStateSetField12EC(s32 value);
extern void sub_080067DC(void);
extern void sub_0800680C(s32 a, s32 b);
extern s32 GameStateTestFlags12C(s32 flags);
extern void sub_08006858(s32 a, s32 b, s32 c);
extern s32 GameStateGetField4256(void);
extern s32 GameStateGetField60E(void);
extern void sub_08008740(s32 a, s32 b);
extern s32 GameStateGetEntry3894(s32 a, s32 b, s32 index);
extern void sub_080088E0(s32 a, s32 b, s32 c);
extern void sub_0806C7AC(s32 x, s32 y, s32 *result);
extern void GameStateSetEntry3894(s32 a, s32 b, s32 index, s32 value);
extern void sub_080561B8(s32 value);
extern void sub_080083E0(s32 a, s32 b);
extern s32 *sub_080083B8(s32 owner, s32 slot);
extern void sub_0800945C(s32 owner, s32 enabled);

AT("00012978") s32 ScriptNativeSetField12EC(u32 count, const s32 *args, s32 *result)
{
    GameStateSetField12EC(args[0]);
    return 1;
}
AT("00012978") const u8 ScriptNativeSetField12ECTail[2] = {0};

AT("00012988") s32 ScriptNativeCall067DC(u32 count, const s32 *args, s32 *result)
{
    sub_080067DC();
    return 1;
}

AT("00012994") s32 ScriptNativeCall0680C(u32 count, const s32 *args, s32 *result)
{
    sub_0800680C(args[0], args[1]);
    return 1;
}

AT("000129A4") s32 ScriptNativeTestGameStateFlags(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateTestFlags12C(args[0]);
    return 1;
}

AT("000129B8") s32 ScriptNativeCall06858(u32 count, const s32 *args, s32 *result)
{
    sub_08006858(args[0], args[1], args[2]);
    return 1;
}

AT("000129CC") s32 ScriptNativeGetField4256(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateGetField4256();
    return 1;
}
AT("000129CC") const u8 ScriptNativeGetField4256Tail[2] = {0};

AT("000129E0") s32 ScriptNativeGetField60E(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateGetField60E();
    return 1;
}
AT("000129E0") const u8 ScriptNativeGetField60ETail[2] = {0};

AT("00012AD4") s32 ScriptNativeCall08740(u32 count, const s32 *args, s32 *result)
{
    sub_08008740(args[0], args[1]);
    return 1;
}

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

AT("00012B64") s32 ScriptNativeQueryResourceSlot(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)GameStateGetEntry3894(0, 0, args[0]);
    return 1;
}

AT("00012C54") s32 ScriptNativeCall088E0(u32 count, const s32 *args, s32 *result)
{
    sub_080088E0(args[0], args[1], args[2]);
    return 1;
}

#ifdef NONMATCHING
AT("00012C68") s32 ScriptNativeConfigureActorSlots(u32 count,
    const s32 *args, s32 *result)
{
    s32 i;
    for (i = 0; i <= 2; i++) {
        s32 value = args[i + 1];
        if (value != -1) {
            s32 *slot = sub_080083B8(args[0], i);
            slot[0] = value;
            slot[2] = 100;
            slot[3] = 100;
        }
    }
    sub_0800945C(args[0], 1);
    return 1;
}
#endif

AT("00012CD0") s32 ScriptNativeMapCoordinateCall(u32 count, const s32 *args, s32 *result)
{
    const s16 *coordinates = (const s16 *)args;
    sub_0806C7AC(coordinates[0], coordinates[2], result);
    return 0x7FFF;
}
