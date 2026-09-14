/* Procedural map-generation state and its deterministic local RNG. */
#include "map_generation.h"
#include "kmp.h"

#include "rom_section.h"

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

/** Map generation seed random using the recovered runtime layout. */
AT("00071DD4")
void MapGenerationSeedRandom(u32 seed) { gMapGenerationSeed = seed; }

/** Map generation random using the recovered runtime layout. */
AT("00071DE0")
u32 MapGenerationRandom(void)
{
    gMapGenerationSeed = gMapGenerationSeed * 0x41C64E6D + 0x3039;
    return (gMapGenerationSeed << 1) >> 17;
}

/** Get the map-generation state. */
AT("00071E00")
struct MapGenerationState *MapGenerationGetState(void)
{
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset);
    return (struct MapGenerationState *)((u8 *)*root + 0x1304);
}

/** Set the map-generation values00 and04. */
AT("00071E1C")
void MapGenerationSetValues00And04(u32 value00, u32 value04)
{
    MapGenerationGetState()->value00 = value00;
    MapGenerationGetState()->value04 = value04;
}

/** Get the map-generation value00. */
AT("00071E34") u32 MapGenerationGetValue00(void) { return MapGenerationGetState()->value00; }
/** Get the map-generation value04. */
AT("00071E40") u32 MapGenerationGetValue04(void) { return MapGenerationGetState()->value04; }
/** Set the map-generation pointer0 c. */
AT("00071E4C") void MapGenerationSetPointer0C(void *v) { MapGenerationGetState()->pointer0C = v; }
/** Set the map-generation pointer14. */
AT("00071E5C") void MapGenerationSetPointer14(void *v) { MapGenerationGetState()->pointer14 = v; }
/** Set the map-generation pointer10. */
AT("00071E6C") void MapGenerationSetPointer10(void *v) { MapGenerationGetState()->pointer10 = v; }
/** Set the map-generation pointer18. */
AT("00071E7C") void MapGenerationSetPointer18(void *v) { MapGenerationGetState()->pointer18 = v; }
/** Get the map-generation pointer0 c. */
AT("00071E8C") void *MapGenerationGetPointer0C(void) { return MapGenerationGetState()->pointer0C; }
AT("00071E98") void *MapGenerationGetPointer14(void) { return MapGenerationGetState()->pointer14; }
AT("00071EA4") void *MapGenerationGetPointer10(void) { return MapGenerationGetState()->pointer10; }
AT("00071EB0") void *MapGenerationGetPointer18(void) { return MapGenerationGetState()->pointer18; }
AT("00071EBC") void *MapGenerationGetPointer1C(void) { return MapGenerationGetState()->pointer1C; }
AT("00071EC8") void *MapGenerationGetPointer20(void) { return MapGenerationGetState()->pointer20; }
AT("00071ED4") void MapGenerationSetPointer1C(void *v) { MapGenerationGetState()->pointer1C = v; }
AT("00071EE4") void MapGenerationSetPointer20(void *v) { MapGenerationGetState()->pointer20 = v; }

AT("00071EF4") void *MapGenerationGetTable38(u32 i) { return MapGenerationGetState()->table38[i]; }
AT("00071EF4") const u8 MapGenerationGetTable38Tail[2] = {0, 0};
AT("00071F0C") void *MapGenerationGetTable48(u32 i) { return MapGenerationGetState()->table48[i]; }
AT("00071F0C") const u8 MapGenerationGetTable48Tail[2] = {0, 0};

AT("00071F24")
void MapGenerationSetTables(u32 i, void *table38, void *table48)
{
    MapGenerationGetState()->table38[i] = table38;
    MapGenerationGetState()->table48[i] = table48;
}

AT("00071F48") void MapGenerationSetValue26(u32 i, s32 v) { MapGenerationGetState()->values26[i] = v; }
AT("00071F60") s32 MapGenerationGetValue26(u32 i) { return MapGenerationGetState()->values26[i]; }
AT("00071F78") void MapGenerationSetTable58(u32 i, void *v) { MapGenerationGetState()->table58[i] = v; }
AT("00071F90") void *MapGenerationGetTable58(u32 i) { return MapGenerationGetState()->table58[i]; }
AT("00071F90") const u8 MapGenerationGetTable58Tail[2] = {0, 0};
AT("00071FA8") void MapGenerationSetValue68(u32 i, s32 v) { MapGenerationGetState()->values68[i] = v; }
AT("00071FA8") const u8 MapGenerationSetValue68Tail[2] = {0, 0};
AT("00071FC0") s32 MapGenerationGetValue68(u32 i) { return MapGenerationGetState()->values68[i]; }
AT("00071FD8") s32 MapGenerationGetValue24(void) { return MapGenerationGetState()->value24; }
AT("00071FD8") const u8 MapGenerationGetValue24Tail[2] = {0, 0};
AT("00071FEC") void MapGenerationSetValue24(s32 v) { s32 n = (s8)v; MapGenerationGetState()->value24 = n; }
AT("00071FEC") const u8 MapGenerationSetValue24Tail[2] = {0, 0};

AT("00072004") void MapGenerationSetValue2E(u32 i, s32 v) { s32 n = (s16)v; MapGenerationGetState()->values2E[i] = n; }
AT("00072004") const u8 MapGenerationSetValue2ETail[2] = {0, 0};
AT("00072020") s32 MapGenerationGetValue2E(u32 i) { return MapGenerationGetState()->values2E[i]; }
AT("00072038") s32 MapGenerationGetValue25(void) { return MapGenerationGetState()->value25; }
AT("00072038") const u8 MapGenerationGetValue25Tail[2] = {0, 0};
AT("0007204C") void MapGenerationSetValue25(s32 v) { s32 n = (s8)v; MapGenerationGetState()->value25 = n; }
AT("0007204C") const u8 MapGenerationSetValue25Tail[2] = {0, 0};
AT("00072064") s32 MapGenerationGetValue08(void) { return MapGenerationGetState()->value08; }
AT("00072074") void MapGenerationSetValue08(s32 v) { s32 n = (s8)v; MapGenerationGetState()->value08 = n; }
AT("00072088") s32 MapGenerationGetVectorValue0(u32 i) { return MapGenerationGetState()->vectors[i].value0; }
AT("000720A0") s32 MapGenerationGetVectorValue2(u32 i) { return MapGenerationGetState()->vectors[i].value2; }
AT("000720B8") s32 MapGenerationGetVectorValue4(u32 i) { return MapGenerationGetState()->vectors[i].value4; }
AT("000720D0") s32 MapGenerationGetVectorValue6(u32 i) { return MapGenerationGetState()->vectors[i].value6; }

/* Script-facing procedural-map commands. */
/* Native-script adapters for procedural map state.
 *
 * The VM passes 32-bit argument slots in r1 and an optional result slot in
 * r2. These handlers intentionally ignore the count in r0, as the original
 * bytecode dispatcher has already checked each native call's arity.
 */

extern void sub_08075984(s32 value);
extern void sub_08009F44(s32 value);
extern void RuntimeActorSetField340(s32 actor, s32 value);
extern void sub_0801C820(s32 value);
extern void *GameStateGetBuffer38C0(void);
extern void sub_080700A8(void *state, u32 a, u32 b, u32 c);
extern void MapGenerationRelease(void *state);
extern void sub_0807017C(void *state, u32 a, u32 b);
extern u32 Random(void);
extern void sub_08070238(void *state, u32 value, u32 random);
extern void sub_08070DA8(void *state, s32 a, s32 b);
extern void sub_08070F80(void *state, s32 value);
extern void BattleRuntimeSetArena(void *buffer, u32 arenaIndex);
extern s32 GameStateGetField4258(void);
extern void GameStateSetField4258(s32 value);

AT("00070EEC")
struct GeneratedMapRuntimeRoom *GeneratedMapFindRuntimeRoom(s32 roomIndex)
{
    struct GeneratedMapRuntimeRoom *room;
    s32 i;

    room = (struct GeneratedMapRuntimeRoom *)
        ((u8 *)GameStateGetBuffer38C0() + 24);
    for (i = 0; i <= 63; room++, i++) {
        if (room->active != 0 && room->roomIndex == roomIndex)
            return room;
    }
    return 0;
}
AT("00070EEC") const u8 GeneratedMapFindRuntimeRoomTail[2] = {0, 0};

AT("00070D60")
s32 GeneratedMapGetCurrentRoomProperty14(struct GeneratedFieldMap *map)
{
    struct GeneratedMapRoomRecord * volatile *rooms = &map->rooms;
    u32 roomOffset = map->cellRoomIndices[map->currentCell];
    roomOffset *= sizeof(struct GeneratedMapRoomRecord);
    return *(s32 *)((u8 *)*rooms + roomOffset + 20);
}

AT("00070D84")
s32 GeneratedMapGetCurrentRoomProperty18(struct GeneratedFieldMap *map)
{
    struct GeneratedMapRoomRecord * volatile *rooms = &map->rooms;
    u32 roomOffset = map->cellRoomIndices[map->currentCell];
    roomOffset *= sizeof(struct GeneratedMapRoomRecord);
    return *(s32 *)((u8 *)*rooms + roomOffset + 24);
}

AT("0007106C")
void GeneratedMapSetParameter10(s32 value)
{
    ((struct GeneratedFieldMap *)GameStateGetBuffer38C0())->parameter10 = value;
}

AT("0007107C")
s32 GeneratedMapGetParameter10(void)
{
    return ((struct GeneratedFieldMap *)GameStateGetBuffer38C0())->parameter10;
}

AT("00071088")
void GeneratedMapSetCurrentRoomFlag(s32 value)
{
    struct GeneratedFieldMap *map = GameStateGetBuffer38C0();
    struct GeneratedMapRuntimeRoom *room =
        GeneratedMapFindRuntimeRoom(map->currentCell);

    if (room != 0)
        room->scriptFlag = value;
}
AT("00071088") const u8 GeneratedMapSetCurrentRoomFlagTail[2] = {0, 0};

AT("000710A4")
s32 GeneratedMapGetCurrentRoomFlag(void)
{
    struct GeneratedFieldMap *map = GameStateGetBuffer38C0();
    struct GeneratedMapRuntimeRoom *room =
        GeneratedMapFindRuntimeRoom(map->currentCell);
    return room->scriptFlag;
}
AT("000710A4") const u8 GeneratedMapGetCurrentRoomFlagTail[2] = {0, 0};

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
    *result = (u32)MapGenerationGetPointer1C() << 3;
    return 1;
}

AT("00012514") s32 ScriptNativeMapGetPointer20Offset(u32 count, const s32 *args, s32 *result)
{
    *result = (u32)MapGenerationGetPointer20() << 3;
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
    MapGenerationRelease(state);
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
    *result = GeneratedMapGetCurrentRoomProperty14(GameStateGetBuffer38C0());
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
    *result = GeneratedMapGetCurrentRoomProperty18(GameStateGetBuffer38C0());
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
    GeneratedMapSetCurrentRoomFlag(args[0]);
    return 1;
}
AT("00012618") const u8 ScriptNativeMapCall1088Tail[2] = {0};

AT("00012628") s32 ScriptNativeMapQuery10A4(u32 count, const s32 *args, s32 *result)
{
    *result = GeneratedMapGetCurrentRoomFlag();
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
        BattleRuntimeSetArena(GameStateGetBuffer38C0(), args[0]);
    else
        BattleRuntimeSetArena(GameStateGetBuffer38C0(), 0);
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

/* Script-facing field, layer, and inventory commands. */
/* Native commands connecting event scripts to map-generation and inventory
 * state. Script values use 32-bit slots; several older subsystems consume
 * signed halfwords, so those conversions are kept explicit here.
 */
#define GAME_ROOT (*(u8 **)0x03003FDC)

extern void sub_080728A0(void *state, s32 value);
extern void *GameStateGetBuffer38C0(void);
extern char *strcpy(char *, const char *);
extern void GameStateCopyRecord(s32, s32, const s16 *);
extern void sub_08056D4C(s32, s32, s32);
extern void sub_08056E3C(s32, s32, s32);
extern void sub_08056CF8(void);
extern void sub_0800690C(s32, s32);
extern void ScriptAddPendingTasks(s32);
extern void RuntimeSetFieldEB4(s32);
extern void GameStateClearRecord426A(void);
extern s32 GameStateGetField42BA(void);
extern void sub_08008968(u32 count, const s32 *args, s32 *result);
extern s32 RuntimeGetFieldEB2(void);
extern void RuntimeSetFieldEB2(s32);
extern s32 RuntimeGetFieldEB3(void);
extern void RuntimeSetFieldEB3(s32);
extern void SpriteEngineSetFlag20C(s32, s32);
extern void GameStateSetField4254(s32);
extern void sub_08009728(s32, s32);
extern void sub_08008C14(s32);
extern void sub_08016D28(void);
extern void sub_08009F44(s32);
extern void RuntimeActorSetField340(s32, s32);
extern void *IwramGetPointer2860(s32);
extern void CpuFill(void *, u32, u32);
extern void sub_08006ADC(s32, s32);
extern u8 *GameStateGetRecord1190(u32);
extern void RuntimeSetFieldE48(s32);
extern void RuntimeSetFieldE4A(s32);
extern void sub_0807253C(s32);
extern void sub_08072710(s32);
extern void *sub_080083B8(s32, s32);
extern void sub_0800945C(s32, s32);
extern u8 gBattleFieldRectXOffset[];
extern u8 gBattleFieldRectYOffset[];
extern u8 gBattleFieldRectWidthOffset[];
extern u8 gBattleFieldRectHeightOffset[];
extern u8 gIwramBase[];
extern u8 gMapStateOffset[];
extern u8 gIwramBaseRectX[];
extern u8 gIwramBaseRectY[];
extern u8 gIwramBaseRectWidth[];
extern u8 gIwramBaseRectHeight[];

AT("0001234C") s32 ScriptNativeSetRuntimePair(u32 count, const s32 *args,
                                                s32 *result)
{
    RuntimeSetFieldE48(args[0]);
    RuntimeSetFieldE4A(args[1]);
    return 1;
}

/** Temporarily lock/unlock the generated battlefield while an event script
 * changes its actors or collision state. */
AT("00012364") s32 ScriptNativeBattleFieldLock(u32 count, const s32 *args,
                                                s32 *result)
{
    sub_0807253C(0);
    return 1;
}
AT("00012364") const u8 ScriptNativeBattleFieldLockTail[2] = {0};

AT("00012374") s32 ScriptNativeBattleFieldUnlock(u32 count, const s32 *args,
                                                  s32 *result)
{
    sub_08072710(0);
    return 1;
}
AT("00012374") const u8 ScriptNativeBattleFieldUnlockTail[2] = {0};

/** Read one component of the active battlefield rectangle. Engine coordinates
 * are stored in metatiles and exposed to scripts in pixels. */
AT("00012384") s32 ScriptNativeGetBattleFieldRect(u32 count, const s32 *args,
                                                   s32 *result)
{
    switch (args[0]) {
    case 0: {
        u8 *base = gIwramBaseRectX;
        u32 offset = (u32)gBattleFieldRectXOffset;
        *result = *(s32 *)(base + offset) << 3;
        break;
    }
    case 1: {
        u8 *base = gIwramBaseRectY;
        u32 offset = (u32)gBattleFieldRectYOffset;
        *result = *(s32 *)(base + offset) << 3;
        break;
    }
    case 2: {
        u8 *base = gIwramBaseRectWidth;
        u32 offset = (u32)gBattleFieldRectWidthOffset;
        *result = *(s32 *)(base + offset) << 3;
        break;
    }
    case 3: {
        u8 *base = gIwramBaseRectHeight;
        u32 offset = (u32)gBattleFieldRectHeightOffset;
        *result = *(s32 *)(base + offset) << 3;
        break;
    }
    }
    return 1;
}

AT("000123EC") s32 ScriptNativeCall08968(u32 count, const s32 *args,
                                          s32 *result)
{
    sub_08008968(count, args, result);
    return 1;
}

AT("000123F8") s32 ScriptNativeResetFieldScene(u32 count, const s32 *args,
                                               s32 *result)
{
    if (RuntimeGetFieldEB2())
        SpriteEngineSetFlag20C(14, 0);
    RuntimeSetFieldEB2(0);
    if (RuntimeGetFieldEB3())
        SpriteEngineSetFlag20C(15, 0);
    RuntimeSetFieldEB3(0);
    GameStateSetField4254(0);
    sub_08009728(2, 0);
    sub_08008C14(2);
    sub_08016D28();
    sub_08009F44(0);
    RuntimeActorSetField340(0, 0);
    CpuFill(IwramGetPointer2860(2), 2048, 0);
    sub_08006ADC(2, 0);
    sub_08006ADC(3, 0);
    GameStateGetRecord1190(2)[1] |= 4;
    GameStateGetRecord1190(3)[1] |= 4;
    return 1;
}
AT("000123F8") const u8 ScriptNativeResetFieldSceneTail[2] = {0};

AT("00012490") s32 ScriptNativeMapSetBoundedValue(u32 count, const s32 *args,
                                                   s32 *result)
{
    s32 value = args[0];
    if ((u32)(value - 400) <= 99)
        sub_080728A0((void *)gKmpViewports, value);
    return 1;
}

AT("000124FC") s32 ScriptNativeBattleStatus(u32 count, const s32 *args,
                                             s32 *result)
{
    return 1;
}

AT("00012730") s32 ScriptNativeCopyGeneratedName(u32 count, const s32 *args,
                                                  s32 *result)
{
    char *destination = (char *)GameStateGetBuffer38C0() + 0x62A;
    u8 *state = *(u8 **)(gMapStateOffset + (u32)gIwramBase);
    const char *source = *(const char **)(state + 0x218);
    strcpy(destination, source);
    return 1;
}

#define COPY_MAP_VALUES()                                                   \
    s16 values[20];                                                         \
    s32 i;                                                                  \
    for (i = 0; i < 20; i++)                                                \
        values[i] = args[i + 2];                                            \
    GameStateCopyRecord((s16)args[0], (s16)args[1], values);                       \
    sub_08056D4C(0, (s16)args[0], (s16)args[1])

AT("000127B8") s32 ScriptNativeWriteMapValues(u32 count, const s32 *args,
                                               s32 *result)
{
    COPY_MAP_VALUES();
    return 1;
}

extern u8 *sub_08055F4C(s32 mode);
extern void sub_08001EB4(void *dest, const void *src, u32 size);
extern void BitSet(u8 *bits, u32 index, s32 enabled);

/** DeckMake: args[0] selects a deck/shuffle mode, same case set as
 * ShuffleDeckCopy. Builds a 20-entry s16 value table from args[1..20],
 * copies it 14 bytes into whatever sub_08055F4C(mode) returns, then flags
 * one bit per raw VM argument in the save's deck bitset (offset 0x26F8). */
AT("000127F8") s32 ScriptNativeDeckMake(u32 count, const s32 *args,
                                         s32 *result)
{
    switch (args[0]) {
    case 1:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
    case 9:
    case 22:
    {
        s16 values[20];
        register s32 mode asm("r3") = args[0];
        s32 i;
        u32 j;

        for (i = 0; i < 20; i++)
            values[i] = args[i + 1];
        sub_08001EB4(sub_08055F4C((s16)mode) + 14, values, 40);
        for (j = 0; j < count; j++)
            BitSet(GAME_ROOT + 0x26F8, args[j], 1);
    }
    }
    return 1;
}

extern void sub_080087EC(s32 a, s32 b, s32 value);

/** ShuffleDeckCopy: args[2] selects a deck/shuffle mode. Most values are
 * silent no-ops; only a handful actually forward to the shared handler. */
AT("000128C8") s32 ScriptNativeShuffleDeckCopy(u32 count, const s32 *args,
                                                s32 *result)
{
    switch (args[2]) {
    case 1:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
    case 9:
    case 22:
        sub_080087EC(args[0], args[1], args[2]);
    }
    return 1;
}

#define REFRESH_MAP_VALUES(address, name)                                  \
AT(address) s32 name(u32 count, const s32 *args, s32 *result)              \
{                                                                          \
    sub_08056E3C(0, (s16)args[0], (s16)args[1]);                           \
    sub_08056CF8();                                                         \
    sub_08056D4C(0, (s16)args[0], (s16)args[1]);                           \
    return 1;                                                               \
}

REFRESH_MAP_VALUES("0001294C", ScriptNativeRefreshMapValuesA)

AT("00012B80") s32 ScriptNativeSelectLayer(u32 count, const s32 *args,
                                            s32 *result)
{
    s32 layer = args[0];
    switch (layer) {
    case 1:
    case 2:
        sub_0800690C(layer, args[1]);
    }
    return 1;
}

/** Populate the three active battle-party slots. A value of -1 leaves that
 * slot empty; initialized combatants start with both percentage fields at
 * 100. */
AT("00012C68") s32 ScriptNativeSetBattleParty(u32 count, const s32 *args,
                                               s32 *result)
{
    s32 slot;
    for (slot = 0; slot <= 2; slot++) {
        if (args[slot + 1] != -1) {
            s32 *party = sub_080083B8(args[0], slot);
            party[0] = args[slot + 1];
            party[2] = 100;
            party[3] = 100;
        }
    }
    sub_0800945C(args[0], 1);
    return 1;
}

AT("00012C14") s32 ScriptNativeWriteMapValuesB(u32 count, const s32 *args,
                                                s32 *result)
{
    COPY_MAP_VALUES();
    return 1;
}

REFRESH_MAP_VALUES("00012CA4", ScriptNativeRefreshMapValuesB)

AT("00012CE8") s32 ScriptNativeSetPendingMapValue(u32 count, const s32 *args,
                                                   s32 *result)
{
    ScriptAddPendingTasks(1);
    RuntimeSetFieldEB4(args[0]);
    return 0x7fff;
}

AT("00012D04") s32 ScriptNativeCopyMapHalfwords(u32 count, const s32 *args,
                                                 s32 *result)
{
    u32 itemCount = count;
    register union {
        const s32 *source;
        s32 step;
    } iteration;
    s32 index;
    register s32 fixedIndex asm("r2");
    const s32 *source;
    u8 * volatile *root;
    s32 destinationOffset;

    iteration.source = args;
    GameStateClearRecord426A();
    if (itemCount > 40)
        itemCount = 40;
    index = 0;
    if (index < itemCount) {
        root = (u8 * volatile *)0x03003FDC;
        destinationOffset = 0x426A;
        fixedIndex = 0x10000;
        source = iteration.source;
        iteration.step = fixedIndex;
        do {
            register u8 *destination asm("r1");
            destination = *root;
            index <<= 1;
            destination += destinationOffset;
            destination += index;
            index = *source++;
            *(u16 *)destination = index;
            index = fixedIndex;
            fixedIndex += iteration.step;
            index >>= 16;
        } while ((u32)index < itemCount);
    }
    return 1;
}

AT("00012D4C") s32 ScriptNativeGetMapStatus(u32 count, const s32 *args,
                                            s32 *result)
{
    *result = (s16)GameStateGetField42BA();
    return 1;
}
AT("00012D4C") const u8 ScriptNativeGetMapStatusTail[2] = {0};

AT("00012D64") s32 ScriptNativeClearMapHalfwords(u32 count, const s32 *args,
                                                  s32 *result)
{
    s32 i;
    s32 fixed;
    s32 step;
    s32 value;
    register u8 **root asm("r6");
    s32 offset;
    register union {
        u8 *base;
        s32 next;
    } temporary;

    i = 0;
    root = (u8 **)0x03003FDC;
    offset = 0x31D0;
    fixed = 0x10000;
    value = 0;
    step = fixed;
    do {
        temporary.base = *root;
        *(u16 *)(temporary.base + offset + (i << 1)) = value;
        temporary.next = fixed;
        fixed += step;
        i = temporary.next >> 16;
    } while (i <= 255);
    return 1;
}

extern void HeapFree(void *heap, void *allocation);
extern void HitRegionDisableAll(void);
extern void sub_08010A2C(s32 arg0, s32 arg1);

/* The two heap blocks the generated map owns, at fixed offsets in the
 * 0x38C0 generation buffer. */
#define MAP_GENERATION_BLOCK_650 0x650
#define MAP_GENERATION_BLOCK_654 0x654

/** Release everything the current generated map owns and reset the runtime
 * state that referenced it. */
AT("00070140") void MapGenerationRelease(void *state)
{
    u8 *generation = state;

    GameStateSetField4258(0);
    HeapFree(0, *(void **)(generation + MAP_GENERATION_BLOCK_650));
    HeapFree(0, *(void **)(generation + MAP_GENERATION_BLOCK_654));
    HitRegionDisableAll();
    sub_08010A2C(0, 0);
}
