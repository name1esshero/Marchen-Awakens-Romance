/* Native-script adapters for inventory/resource and encounter helpers.
 * Arguments occupy 32-bit VM slots even when the underlying engine API uses
 * signed bytes or halfwords. Preserve those explicit narrowing operations.
 */
#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))

extern s32 sub_080570BC(s32 id);
extern s32 sub_08056984(s32 id);
extern s32 sub_08056EE0(s32 id);
extern void sub_08056A8C(s32 id, s32 mode);
extern s32 sub_08056290(void);
extern void sub_080562C8(s32 value);
extern void sub_080563AC(void);
extern s32 sub_08056304(s32 value);
extern void sub_08055F88(s32 value, s32 limit);
extern s32 sub_08056130(void);
extern void sub_080087EC(s32 a, s32 b, s32 value);
extern void sub_080083E0(s32 a, s32 b);
extern void sub_0806EFCC(s32 value);
extern u32 Random(void);
extern s32 sub_08080E4C(u32 random, u32 count);
extern void sub_0806F120(s32 x, s32 y, s32 a, s32 b);
extern s32 GameStateGetField42BA(void);
extern void sub_0806F620(s32 value);
extern void sub_080577C0(s32 value);
extern s32 sub_080577E4(void);
extern void sub_08057800(s32 value);
extern s32 sub_08057844(void);
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
#define RUNTIME_STATE ({ \
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})

AT("00012D98") s32 ScriptNativeQueryResourceId(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)sub_080570BC((s16)args[0]);
    return 1;
}
AT("00012D98") const u8 ScriptNativeQueryResourceIdTail[2] = {0};

#ifdef NONMATCHING
/* Decoded, but agbcc currently chooses different temporary registers and
 * literal-pool positions than the original for these two handlers. */
AT("00012DB4") s32 ScriptNativeQueryModeResource(u32 count, const s32 *args, s32 *result)
{
    s32 value;
    if (!RUNTIME_STATE[0x38B8])
        value = sub_08056984((s16)args[0]);
    else
        value = sub_08056EE0((s16)args[0]);
    *result = (s16)value;
    return 1;
}
AT("00012DB4") const u8 ScriptNativeQueryModeResourceTail[2] = {0};

AT("00012DF8") s32 ScriptNativeSetModeResource(u32 count, const s32 *args, s32 *result)
{
    if (!RUNTIME_STATE[0x38B8])
        sub_08056A8C((s16)args[0], 1);
    else
        sub_08056A8C((s16)args[0], 2);
    return 1;
}
#endif

AT("00012E34") s32 ScriptNativeQueryResourceState(u32 count, const s32 *args, s32 *result)
{
    *result = sub_08056290();
    return 1;
}
AT("00012E34") const u8 ScriptNativeQueryResourceStateTail[2] = {0};

AT("00012E48") s32 ScriptNativeSetResourceState(u32 count, const s32 *args, s32 *result)
{
    sub_080562C8(args[0]);
    return 1;
}
AT("00012E48") const u8 ScriptNativeSetResourceStateTail[2] = {0};

AT("00012E58") s32 ScriptNativeResetResourceState(u32 count, const s32 *args, s32 *result)
{
    sub_080563AC();
    return 1;
}

AT("00012E64") s32 ScriptNativeUseResource(u32 count, const s32 *args, s32 *result)
{
    if ((s16)sub_08056304((s16)args[0]))
    {
        sub_08055F88((s16)args[0], 999);
        sub_080087EC(0, 0, (s16)sub_08056130());
        sub_080083E0(0, 0);
    }
    return 1;
}

AT("00012F04") s32 ScriptNativeResetEncounterState(u32 count, const s32 *args, s32 *result)
{
    sub_0806EFCC(0);
    return 1;
}
AT("00012F04") const u8 ScriptNativeResetEncounterStateTail[2] = {0};

AT("00012F14") s32 ScriptNativeChooseRandomValue(u32 count, const s32 *args, s32 *result)
{
    s32 index = (s16)sub_08080E4C(Random(), count);
    *result = args[index];
    return 1;
}

AT("00012F38") s32 ScriptNativeStartMapCoordinateEvent(u32 count, const s32 *args, s32 *result)
{
    sub_0806F120((s16)args[0], (s16)args[1], 0, 0);
    return 0x7FFF;
}

AT("00012F54") s32 ScriptNativeGetField42BA(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)GameStateGetField42BA();
    return 1;
}
AT("00012F54") const u8 ScriptNativeGetField42BATail[2] = {0};

AT("00012F6C") s32 ScriptNativeStartEncounter(u32 count, const s32 *args, s32 *result)
{
    sub_0806F620(0);
    return 0x7FFF;
}

AT("00012F80") s32 ScriptNativeSetEncounterValue(u32 count, const s32 *args, s32 *result)
{
    sub_080577C0((s16)args[0]);
    return 1;
}

AT("00012F90") s32 ScriptNativeGetEncounterValue(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)sub_080577E4();
    return 1;
}
AT("00012F90") const u8 ScriptNativeGetEncounterValueTail[2] = {0};

AT("00012FA8") s32 ScriptNativeSetEncounterMode(u32 count, const s32 *args, s32 *result)
{
    sub_08057800((s8)args[0]);
    return 1;
}

AT("00012FB8") s32 ScriptNativeGetEncounterMode(u32 count, const s32 *args, s32 *result)
{
    *result = (s8)sub_08057844();
    return 1;
}
AT("00012FB8") const u8 ScriptNativeGetEncounterModeTail[2] = {0};
