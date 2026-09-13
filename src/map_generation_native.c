/* Native-script adapters for procedural map state.
 *
 * The VM passes 32-bit argument slots in r1 and an optional result slot in
 * r2. These handlers intentionally ignore the count in r0, as the original
 * bytecode dispatcher has already checked each native call's arity.
 */
#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))

extern void MapGenerationSetValue08(s32 value);
extern void MapGenerationSetValue25(s32 value);
extern void sub_08075984(s32 value);
extern void sub_08009F44(s32 value);
extern void RuntimeActorSetField340(s32 actor, s32 value);
extern void sub_0801C820(s32 value);
extern s32 MapGenerationGetPointer1C(void);
extern s32 MapGenerationGetPointer20(void);
extern void *GameStateGetBuffer38C0(void);
extern void sub_080700A8(void *state, u32 a, u32 b, u32 c);
extern void sub_08070140(void *state);
extern void sub_0807017C(void *state, u32 a, u32 b);
extern u32 Random(void);
extern void sub_08070238(void *state, u32 value, u32 random);
extern s32 sub_08070D60(void *state);
extern void sub_08070DA8(void *state, s32 a, s32 b);
extern s32 sub_08070D84(void *state);
extern void sub_08070F80(void *state, s32 value);
extern void sub_08071088(s32 value);
extern s32 sub_080710A4(void);
extern void sub_08070214(void *state, u32 value);
extern s32 GameStateGetField4258(void);
extern void GameStateSetField4258(s32 value);

AT("000124B0") s32 ScriptNativeMapSetValue08(u32 count, const s32 *args, s32 *result)
{
    MapGenerationSetValue08((s8)args[0]);
    return 1;
}

AT("000124C0") s32 ScriptNativeMapSetValue25(u32 count, const s32 *args, s32 *result)
{
    MapGenerationSetValue25((s8)args[0]);
    return 1;
}

AT("000124D0") s32 ScriptNativeMapResetGenerator(u32 count, const s32 *args, s32 *result)
{
    sub_08075984(0);
    return 1;
}
AT("000124D0") const u8 ScriptNativeMapResetGeneratorTail[2] = {0};

AT("000124E0") s32 ScriptNativeMapResetActor(u32 count, const s32 *args, s32 *result)
{
    sub_08009F44(0);
    RuntimeActorSetField340(0, 0);
    sub_0801C820(0);
    return 1;
}

AT("00012500") s32 ScriptNativeMapGetPointer1COffset(u32 count, const s32 *args, s32 *result)
{
    *result = MapGenerationGetPointer1C() << 3;
    return 1;
}

AT("00012514") s32 ScriptNativeMapGetPointer20Offset(u32 count, const s32 *args, s32 *result)
{
    *result = MapGenerationGetPointer20() << 3;
    return 1;
}

AT("00012528") s32 ScriptNativeMapConfigure3(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    sub_080700A8(state, (u16)args[0], (u16)args[1], (u16)args[2]);
    return 1;
}
AT("00012528") const u8 ScriptNativeMapConfigure3Tail[2] = {0};

AT("00012544") s32 ScriptNativeMapRefresh(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    sub_08070140(state);
    return 0x7FFF;
}

AT("00012558") s32 ScriptNativeMapConfigure2(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    sub_0807017C(state, (u16)args[0], (u16)args[1]);
    return 1;
}

AT("00012570") s32 ScriptNativeMapRandomize(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    u32 value = (u16)args[0];
    u32 random = Random();
    sub_08070238(state, value, random);
    return 1;
}
AT("00012570") const u8 ScriptNativeMapRandomizeTail[2] = {0};

AT("00012594") s32 ScriptNativeMapQueryD60(u32 count, const s32 *args, s32 *result)
{
    *result = sub_08070D60(GameStateGetBuffer38C0());
    return 1;
}
AT("00012594") const u8 ScriptNativeMapQueryD60Tail[2] = {0};

AT("000125AC") s32 ScriptNativeMapClearDState(u32 count, const s32 *args, s32 *result)
{
    sub_08070DA8(GameStateGetBuffer38C0(), 0, 0);
    return 1;
}

AT("000125C0") s32 ScriptNativeMapQueryD84(u32 count, const s32 *args, s32 *result)
{
    *result = sub_08070D84(GameStateGetBuffer38C0());
    return 1;
}
AT("000125C0") const u8 ScriptNativeMapQueryD84Tail[2] = {0};

AT("000125D8") s32 ScriptNativeMapSetMode(u32 count, const s32 *args, s32 *result)
{
    sub_08070F80(GameStateGetBuffer38C0(), args[0]);
    return 1;
}
AT("000125D8") const u8 ScriptNativeMapSetModeTail[2] = {0};

AT("000125F0") s32 ScriptNativeMapSetSeed(u32 count, const s32 *args, s32 *result)
{
    *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x14) = args[0];
    return 1;
}

AT("00012604") s32 ScriptNativeMapGetSeed(u32 count, const s32 *args, s32 *result)
{
    *result = *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x14);
    return 1;
}

AT("00012618") s32 ScriptNativeMapCall1088(u32 count, const s32 *args, s32 *result)
{
    sub_08071088(args[0]);
    return 1;
}
AT("00012618") const u8 ScriptNativeMapCall1088Tail[2] = {0};

AT("00012628") s32 ScriptNativeMapQuery10A4(u32 count, const s32 *args, s32 *result)
{
    *result = sub_080710A4();
    return 1;
}
AT("00012628") const u8 ScriptNativeMapQuery10A4Tail[2] = {0};

AT("0001263C") s32 ScriptNativeMapGetField618(u32 count, const s32 *args, s32 *result)
{
    *result = *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x618);
    return 1;
}
AT("0001263C") const u8 ScriptNativeMapGetField618Tail[2] = {0};

AT("00012658") s32 ScriptNativeMapSetField10(u32 count, const s32 *args, s32 *result)
{
    *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x10) = args[0];
    return 1;
}

AT("0001266C") s32 ScriptNativeMapGetField10(u32 count, const s32 *args, s32 *result)
{
    *result = *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x10);
    return 1;
}

AT("00012680") s32 ScriptNativeMapGetField624(u32 count, const s32 *args, s32 *result)
{
    *result = *(s16 *)((u8 *)GameStateGetBuffer38C0() + 0x624);
    return 1;
}

AT("000126A0") s32 ScriptNativeMapSetField624(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    s32 value = args[0];
    *(u16 *)((u8 *)state + 0x624) = value;
    return 1;
}

AT("000126BC") s32 ScriptNativeMapGetField626(u32 count, const s32 *args, s32 *result)
{
    *result = *(s16 *)((u8 *)GameStateGetBuffer38C0() + 0x626);
    return 1;
}

AT("000126DC") s32 ScriptNativeMapSetField626(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    s32 value = args[0];
    *(u16 *)((u8 *)state + 0x626) = value;
    return 1;
}

AT("000126F8") s32 ScriptNativeMapGetField628(u32 count, const s32 *args, s32 *result)
{
    *result = *(s16 *)((u8 *)GameStateGetBuffer38C0() + 0x628);
    return 1;
}

AT("00012714") s32 ScriptNativeMapSetField628(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    s32 value = args[0];
    *(u16 *)((u8 *)state + 0x628) = value;
    return 1;
}
AT("00012714") const u8 ScriptNativeMapSetField628Tail[2] = {0};

AT("00012760") s32 ScriptNativeMapFinalize(u32 count, const s32 *args, s32 *result)
{
    extern void ClearBattleRuntimeBuffer(void);
    ClearBattleRuntimeBuffer();
    return 1;
}

AT("0001276C") s32 ScriptNativeMapSelectSlot(u32 count, const s32 *args, s32 *result)
{
    if ((u32)args[0] <= 3)
        sub_08070214(GameStateGetBuffer38C0(), args[0]);
    else
        sub_08070214(GameStateGetBuffer38C0(), 0);
    return 1;
}

AT("00012794") s32 ScriptNativeGetField4258(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateGetField4258();
    return 1;
}
AT("00012794") const u8 ScriptNativeGetField4258Tail[2] = {0};

AT("000127A8") s32 ScriptNativeSetField4258(u32 count, const s32 *args, s32 *result)
{
    GameStateSetField4258(args[0]);
    return 1;
}
AT("000127A8") const u8 ScriptNativeSetField4258Tail[2] = {0};
