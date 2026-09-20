/* Procedural map-generation state and its deterministic local RNG. */
#include "map_generation.h"
#include "kmp.h"
#include "flags.h"
#include "game_state.h"

#include "rom_section.h"

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

#define MAP_HALFWORD_COPY_LIMIT 40
#define MAP_HALFWORD_RECORD_OFFSET 0x426A
#define CONSUMABLE_INVENTORY_OFFSET 0x31D0
#define MAP_HALFWORD_RECORD_COUNT 256
#define FIXED_16_16_ONE (1 << 16)

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
AT("00071EBC") u32 MapGenerationGetValue1C(void) { return MapGenerationGetState()->value1C; }
AT("00071EC8") u32 MapGenerationGetValue20(void) { return MapGenerationGetState()->value20; }
AT("00071ED4") void MapGenerationSetValue1C(u32 v) { MapGenerationGetState()->value1C = v; }
AT("00071EE4") void MapGenerationSetValue20(u32 v) { MapGenerationGetState()->value20 = v; }

/** @brief Read an entry's tile X. @param i Entry index. @return Signed tile X. */
AT("00071EF4") s32 MapGenerationGetTileX(u32 i) { return MapGenerationGetState()->tileX[i]; }
AT("00071EF4") const u8 MapGenerationGetTileXTail[2] = {0, 0};
/** @brief Read an entry's tile Y. @param i Entry index. @return Signed tile Y. */
AT("00071F0C") s32 MapGenerationGetTileY(u32 i) { return MapGenerationGetState()->tileY[i]; }
AT("00071F0C") const u8 MapGenerationGetTileYTail[2] = {0, 0};

/**
 * @brief Store a generation entry's tile coordinates.
 * @param i Entry index.
 * @param tileX Horizontal tile coordinate.
 * @param tileY Vertical tile coordinate.
 * @return Nothing.
 */
AT("00071F24")
void MapGenerationSetTilePosition(u32 i, s32 tileX, s32 tileY)
{
    MapGenerationGetState()->tileX[i] = tileX;
    MapGenerationGetState()->tileY[i] = tileY;
}

/** Set a generation entry's active state. See MapGenerationClearCurrentFieldEntries(). */
AT("00071F48") void MapGenerationSetEntryState(u32 i, s32 v) { MapGenerationGetState()->entryState[i] = v; }
/** @return A generation entry's active state (nonzero when active). */
AT("00071F60") s32 MapGenerationGetEntryState(u32 i) { return MapGenerationGetState()->entryState[i]; }
/** Set a generation entry's countdown. See MapGenerationAdvanceCurrentFieldEntries(). */
AT("00071F78") void MapGenerationSetCountdown(u32 i, s32 v) { MapGenerationGetState()->countdown[i] = v; }
/** @return A generation entry's countdown. */
AT("00071F90") s32 MapGenerationGetCountdown(u32 i) { return MapGenerationGetState()->countdown[i]; }
AT("00071F90") const u8 MapGenerationGetCountdownTail[2] = {0, 0};
/** Set a generation entry's update-pending flag. */
AT("00071FA8") void MapGenerationSetUpdatePending(u32 i, s32 v) { MapGenerationGetState()->updatePending[i] = v; }
AT("00071FA8") const u8 MapGenerationSetUpdatePendingTail[2] = {0, 0};
/** @return A generation entry's update-pending flag. */
AT("00071FC0") s32 MapGenerationGetUpdatePending(u32 i) { return MapGenerationGetState()->updatePending[i]; }
/** @return The map-generation state's value24 field. Meaning not yet
 * recovered. */
AT("00071FD8") s32 MapGenerationGetValue24(void) { return MapGenerationGetState()->value24; }
AT("00071FD8") const u8 MapGenerationGetValue24Tail[2] = {0, 0};
/** Set the map-generation state's value24 field (narrowed to a signed
 * byte). */
AT("00071FEC") void MapGenerationSetValue24(s32 v) { s32 n = (s8)v; MapGenerationGetState()->value24 = n; }
AT("00071FEC") const u8 MapGenerationSetValue24Tail[2] = {0, 0};

/** Set a generation entry's field identifier (narrowed to a signed
 * halfword). */
AT("00072004") void MapGenerationSetFieldId(u32 i, s32 v) { s32 n = (s16)v; MapGenerationGetState()->fieldId[i] = n; }
AT("00072004") const u8 MapGenerationSetFieldIdTail[2] = {0, 0};
/** @return A generation entry's field identifier. */
AT("00072020") s32 MapGenerationGetFieldId(u32 i) { return MapGenerationGetState()->fieldId[i]; }
/** @return The map-generation state's value25 field. Meaning not yet
 * recovered. */
AT("00072038") s32 MapGenerationGetValue25(void) { return MapGenerationGetState()->value25; }
AT("00072038") const u8 MapGenerationGetValue25Tail[2] = {0, 0};
/** Set the map-generation state's value25 field (narrowed to a signed
 * byte). */
AT("0007204C") void MapGenerationSetValue25(s32 v) { s32 n = (s8)v; MapGenerationGetState()->value25 = n; }
AT("0007204C") const u8 MapGenerationSetValue25Tail[2] = {0, 0};
/** @return The map-generation state's value08 field. Gates most generation
 * updates when its low byte is nonzero (see MapGenerationClearCurrentFieldEntries()). */
AT("00072064") s32 MapGenerationGetValue08(void) { return MapGenerationGetState()->value08; }
/** Set the map-generation state's value08 field (narrowed to a signed
 * byte). See MapGenerationGetValue08(). */
AT("00072074") void MapGenerationSetValue08(s32 v) { s32 n = (s8)v; MapGenerationGetState()->value08 = n; }
/** @return A generation entry's vector, component 0. */
AT("00072088") s32 MapGenerationGetVectorValue0(u32 i) { return MapGenerationGetState()->vectors[i].value0; }
/** @return A generation entry's vector, component 2. */
AT("000720A0") s32 MapGenerationGetVectorValue2(u32 i) { return MapGenerationGetState()->vectors[i].value2; }
/** @return A generation entry's vector, component 4. */
AT("000720B8") s32 MapGenerationGetVectorValue4(u32 i) { return MapGenerationGetState()->vectors[i].value4; }
/** @return A generation entry's vector, component 6. */
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

/** @return The active runtime room record matching roomIndex among the 64
 * slots based at the battle runtime buffer's +24, or NULL if none is
 * active with that index. */
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

/** Select one flat tile index whose attribute equals @p attribute.
 *
 * The generator supplies a ten-index stack buffer and expects the caller's
 * attribute class to fit that capacity. It uses its own deterministic random
 * stream to select one. Map dimensions are interpreted as signed values here,
 * matching the original loop's signed bounds checks.
 *
 * @return A matching flat tile index, or -1 when the map has no match.
 */
AT("00070F20")
s32 GeneratedMapChooseAttributeIndex(struct KmpViewport *view, u32 attribute)
{
    s32 matches[10];
    s32 count;
    s32 index;
    s32 *output;
    u16 *attributes;
    const struct KmpHeader *map;
    s32 result;

    count = 0;
    attributes = KmpAttributeAddress(view, 0, 0);
    index = 0;
    map = view->data;
    if (count < (s32)map->widthTiles * (s32)map->heightTiles) {
        output = matches;
        do {
            if (*attributes++ == attribute) {
                *output++ = index;
                count++;
            }
            index++;
        } while (index < (s32)map->widthTiles * (s32)map->heightTiles);
    }

    if (count == 0)
        goto no_match;
    result = matches[MapGenerationRandom() % count];
    goto done;

no_match:
    result = -1;
done:
    return result;
}
AT("00070F20") const u8 GeneratedMapChooseAttributeIndexTail[2] = {0, 0};

/** @return The current cell's room record's +20 field. Meaning not yet
 * recovered. */
AT("00070D60")
s32 GeneratedMapGetCurrentRoomProperty14(struct GeneratedFieldMap *map)
{
    struct GeneratedMapRoomRecord * volatile *rooms = &map->rooms;
    u32 roomOffset = map->cellRoomIndices[map->currentCell];
    roomOffset *= sizeof(struct GeneratedMapRoomRecord);
    return *(s32 *)((u8 *)*rooms + roomOffset + 20);
}

/** @return The current cell's room record's +24 field. Meaning not yet
 * recovered. */
AT("00070D84")
s32 GeneratedMapGetCurrentRoomProperty18(struct GeneratedFieldMap *map)
{
    struct GeneratedMapRoomRecord * volatile *rooms = &map->rooms;
    u32 roomOffset = map->cellRoomIndices[map->currentCell];
    roomOffset *= sizeof(struct GeneratedMapRoomRecord);
    return *(s32 *)((u8 *)*rooms + roomOffset + 24);
}

/** Set the current generated field map's +parameter10 field. Meaning not
 * yet recovered. */
AT("0007106C")
void GeneratedMapSetParameter10(s32 value)
{
    ((struct GeneratedFieldMap *)GameStateGetBuffer38C0())->parameter10 = value;
}

/** @return The current generated field map's parameter10 field. */
AT("0007107C")
s32 GeneratedMapGetParameter10(void)
{
    return ((struct GeneratedFieldMap *)GameStateGetBuffer38C0())->parameter10;
}

/** Set the current cell's active runtime room's script flag, if that room
 * is found. See GeneratedMapFindRuntimeRoom(). */
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

/** @return The current cell's active runtime room's script flag. Unlike
 * GeneratedMapSetCurrentRoomFlag(), this does not guard against the room
 * lookup failing. */
AT("000710A4")
s32 GeneratedMapGetCurrentRoomFlag(void)
{
    struct GeneratedFieldMap *map = GameStateGetBuffer38C0();
    struct GeneratedMapRuntimeRoom *room =
        GeneratedMapFindRuntimeRoom(map->currentCell);
    return room->scriptFlag;
}
AT("000710A4") const u8 GeneratedMapGetCurrentRoomFlagTail[2] = {0, 0};

/** Native script command: forward to MapGenerationSetValue08().
 * @return Always 1. */
AT("000124B0") s32 ScriptNativeMapSetValue08(u32 count, const s32 *args, s32 *result)
{
    MapGenerationSetValue08((s8)args[0]);
    return 1;
}

/** Native script command: forward to MapGenerationSetValue25().
 * @return Always 1. */
AT("000124C0") s32 ScriptNativeMapSetValue25(u32 count, const s32 *args, s32 *result)
{
    MapGenerationSetValue25((s8)args[0]);
    return 1;
}

/** Native script command: forward to sub_08075984(0). @return Always 1. */
AT("000124D0") s32 ScriptNativeMapResetGenerator(u32 count, const s32 *args, s32 *result)
{
    sub_08075984(0);
    return 1;
}
AT("000124D0") const u8 ScriptNativeMapResetGeneratorTail[2] = {0};

/** Native script command: reset actor 0's field340 and related state
 * through three engine calls. @return Always 1. */
AT("000124E0") s32 ScriptNativeMapResetActor(u32 count, const s32 *args, s32 *result)
{
    sub_08009F44(0);
    RuntimeActorSetField340(0, 0);
    sub_0801C820(0);
    return 1;
}

/** Native script command: read MapGenerationGetValue1C() as a pixel
 * offset (its raw tile value shifted left 3). @return Always 1. */
AT("00012500") s32 ScriptNativeMapGetPointer1COffset(u32 count, const s32 *args, s32 *result)
{
    *result = MapGenerationGetValue1C() << 3;
    return 1;
}

/** Native script command: read MapGenerationGetValue20() as a pixel
 * offset (its raw tile value shifted left 3). @return Always 1. */
AT("00012514") s32 ScriptNativeMapGetPointer20Offset(u32 count, const s32 *args, s32 *result)
{
    *result = MapGenerationGetValue20() << 3;
    return 1;
}

/** Native script command: forward three u16-narrowed arguments to
 * sub_080700A8() on the current generated field map. @return Always 1. */
AT("00012528") s32 ScriptNativeMapConfigure3(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    sub_080700A8(state, (u16)args[0], (u16)args[1], (u16)args[2]);
    return 1;
}
AT("00012528") const u8 ScriptNativeMapConfigure3Tail[2] = {0};

/** Native script command: release the current generated field map's
 * resources via MapGenerationRelease(). @return Always SCRIPT_WAIT
 * (0x7FFF). */
AT("00012544") s32 ScriptNativeMapRefresh(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    MapGenerationRelease(state);
    return 0x7FFF;
}

/** Native script command: forward two u16-narrowed arguments to
 * sub_0807017C() on the current generated field map. @return Always 1. */
AT("00012558") s32 ScriptNativeMapConfigure2(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    sub_0807017C(state, (u16)args[0], (u16)args[1]);
    return 1;
}

/** Native script command: forward a u16-narrowed argument plus a fresh
 * random value to sub_08070238() on the current generated field map.
 * @return Always 1. */
AT("00012570") s32 ScriptNativeMapRandomize(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    u32 value = (u16)args[0];
    u32 random = Random();
    sub_08070238(state, value, random);
    return 1;
}
AT("00012570") const u8 ScriptNativeMapRandomizeTail[2] = {0};

/** Native script command: forward to GeneratedMapGetCurrentRoomProperty14().
 * @return Always 1. */
AT("00012594") s32 ScriptNativeMapQueryD60(u32 count, const s32 *args, s32 *result)
{
    *result = GeneratedMapGetCurrentRoomProperty14(GameStateGetBuffer38C0());
    return 1;
}
AT("00012594") const u8 ScriptNativeMapQueryD60Tail[2] = {0};

/** Native script command: forward to sub_08070DA8(state, 0, 0) on the
 * current generated field map. @return Always 1. */
AT("000125AC") s32 ScriptNativeMapClearDState(u32 count, const s32 *args, s32 *result)
{
    sub_08070DA8(GameStateGetBuffer38C0(), 0, 0);
    return 1;
}

/** Native script command: forward to GeneratedMapGetCurrentRoomProperty18().
 * @return Always 1. */
AT("000125C0") s32 ScriptNativeMapQueryD84(u32 count, const s32 *args, s32 *result)
{
    *result = GeneratedMapGetCurrentRoomProperty18(GameStateGetBuffer38C0());
    return 1;
}
AT("000125C0") const u8 ScriptNativeMapQueryD84Tail[2] = {0};

/** Native script command: forward to sub_08070F80() on the current
 * generated field map. @return Always 1. */
AT("000125D8") s32 ScriptNativeMapSetMode(u32 count, const s32 *args, s32 *result)
{
    sub_08070F80(GameStateGetBuffer38C0(), args[0]);
    return 1;
}
AT("000125D8") const u8 ScriptNativeMapSetModeTail[2] = {0};

/** Native script command: set the current generated field map's +0x14 seed
 * field. @return Always 1. */
AT("000125F0") s32 ScriptNativeMapSetSeed(u32 count, const s32 *args, s32 *result)
{
    *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x14) = args[0];
    return 1;
}

/** Native script command: read the current generated field map's +0x14
 * seed field. @return Always 1. */
AT("00012604") s32 ScriptNativeMapGetSeed(u32 count, const s32 *args, s32 *result)
{
    *result = *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x14);
    return 1;
}

/** Native script command: forward to GeneratedMapSetCurrentRoomFlag().
 * @return Always 1. */
AT("00012618") s32 ScriptNativeMapCall1088(u32 count, const s32 *args, s32 *result)
{
    GeneratedMapSetCurrentRoomFlag(args[0]);
    return 1;
}
AT("00012618") const u8 ScriptNativeMapCall1088Tail[2] = {0};

/** Native script command: forward to GeneratedMapGetCurrentRoomFlag().
 * @return Always 1. */
AT("00012628") s32 ScriptNativeMapQuery10A4(u32 count, const s32 *args, s32 *result)
{
    *result = GeneratedMapGetCurrentRoomFlag();
    return 1;
}
AT("00012628") const u8 ScriptNativeMapQuery10A4Tail[2] = {0};

/** Native script command: read the current generated field map's +0x618
 * s32 field. @return Always 1. */
AT("0001263C") s32 ScriptNativeMapGetField618(u32 count, const s32 *args, s32 *result)
{
    *result = *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x618);
    return 1;
}
AT("0001263C") const u8 ScriptNativeMapGetField618Tail[2] = {0};

/** Native script command: set the current generated field map's +0x10 s32
 * field. @return Always 1. */
AT("00012658") s32 ScriptNativeMapSetField10(u32 count, const s32 *args, s32 *result)
{
    *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x10) = args[0];
    return 1;
}

/** Native script command: read the current generated field map's +0x10 s32
 * field. @return Always 1. */
AT("0001266C") s32 ScriptNativeMapGetField10(u32 count, const s32 *args, s32 *result)
{
    *result = *(s32 *)((u8 *)GameStateGetBuffer38C0() + 0x10);
    return 1;
}

/** Native script command: read the current generated field map's +0x624
 * s16 field. @return Always 1. */
AT("00012680") s32 ScriptNativeMapGetField624(u32 count, const s32 *args, s32 *result)
{
    *result = *(s16 *)((u8 *)GameStateGetBuffer38C0() + 0x624);
    return 1;
}

/** Native script command: set the current generated field map's +0x624
 * s16 field. @return Always 1. */
AT("000126A0") s32 ScriptNativeMapSetField624(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    s32 value = args[0];
    *(u16 *)((u8 *)state + 0x624) = value;
    return 1;
}

/** Native script command: read the current generated field map's +0x626
 * s16 field. @return Always 1. */
AT("000126BC") s32 ScriptNativeMapGetField626(u32 count, const s32 *args, s32 *result)
{
    *result = *(s16 *)((u8 *)GameStateGetBuffer38C0() + 0x626);
    return 1;
}

/** Native script command: set the current generated field map's +0x626
 * s16 field. @return Always 1. */
AT("000126DC") s32 ScriptNativeMapSetField626(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    s32 value = args[0];
    *(u16 *)((u8 *)state + 0x626) = value;
    return 1;
}

/** Native script command: read the current generated field map's +0x628
 * s16 field. @return Always 1. */
AT("000126F8") s32 ScriptNativeMapGetField628(u32 count, const s32 *args, s32 *result)
{
    *result = *(s16 *)((u8 *)GameStateGetBuffer38C0() + 0x628);
    return 1;
}

/** Native script command: set the current generated field map's +0x628
 * s16 field. @return Always 1. */
AT("00012714") s32 ScriptNativeMapSetField628(u32 count, const s32 *args, s32 *result)
{
    void *state = GameStateGetBuffer38C0();
    s32 value = args[0];
    *(u16 *)((u8 *)state + 0x628) = value;
    return 1;
}
AT("00012714") const u8 ScriptNativeMapSetField628Tail[2] = {0};

/** Native script command: forward to ClearBattleRuntimeBuffer().
 * @return Always 1. */
AT("00012760") s32 ScriptNativeMapFinalize(u32 count, const s32 *args, s32 *result)
{
    extern void ClearBattleRuntimeBuffer(void);
    ClearBattleRuntimeBuffer();
    return 1;
}

/** Native script command: select a battle arena (0-3), falling back to
 * arena 0 for any other value. @return Always 1. */
AT("0001276C") s32 ScriptNativeMapSelectSlot(u32 count, const s32 *args, s32 *result)
{
    if ((u32)args[0] <= 3)
        BattleRuntimeSetArena(GameStateGetBuffer38C0(), args[0]);
    else
        BattleRuntimeSetArena(GameStateGetBuffer38C0(), 0);
    return 1;
}

/** Native script command: forward to GameStateGetField4258().
 * @return Always 1. */
AT("00012794") s32 ScriptNativeGetField4258(u32 count, const s32 *args, s32 *result)
{
    *result = GameStateGetField4258();
    return 1;
}
AT("00012794") const u8 ScriptNativeGetField4258Tail[2] = {0};

/** Native script command: forward to GameStateSetField4258().
 * @return Always 1. */
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
#define GAME_ROOT gMapGenerationRoot

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
extern void RuntimeSetFlagC0(u32, s32);
extern void sub_08008C14(s32);
extern void sub_08016D28(void);
extern void sub_08009F44(s32);
extern void RuntimeActorSetField340(s32, s32);
extern void *IwramGetPointer2860(s32);
extern void IwramSetFlags0810(s32 mode, s32 enabled);
extern void CpuFill(void *, u32, u32);
extern u8 *GameStateGetRecord1190(u32);
extern void RuntimeSetFieldE48(s32);
extern void RuntimeSetFieldE4A(s32);
extern void sub_0807253C(s32);
extern void sub_08072710(s32);
extern void *RuntimeGetActorPartRecord(s32, s32);
extern void RuntimeActorSetField234(u32 actor, s32 value);
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

/** Native script command: forward two arguments to RuntimeSetFieldE48() and
 * RuntimeSetFieldE4A(). @return Always 1. */
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

/** Native script command: forward all VM arguments and the result slot to
 * sub_08008968(). @return Always 1. */
AT("000123EC") s32 ScriptNativeCall08968(u32 count, const s32 *args,
                                          s32 *result)
{
    sub_08008968(count, args, result);
    return 1;
}

/** Native script command: tear down the field scene's sprite-engine flags,
 * actor state, and two 2 KiB IWRAM buffers, then mark rooms 2 and 3 for
 * their bit-4 refresh. @return Always 1. */
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
    RuntimeSetFlagC0(2, 0);
    sub_08008C14(2);
    sub_08016D28();
    sub_08009F44(0);
    RuntimeActorSetField340(0, 0);
    CpuFill(IwramGetPointer2860(2), 2048, 0);
    IwramSetFlags0810(2, 0);
    IwramSetFlags0810(3, 0);
    GameStateGetRecord1190(2)[1] |= 4;
    GameStateGetRecord1190(3)[1] |= 4;
    return 1;
}
AT("000123F8") const u8 ScriptNativeResetFieldSceneTail[2] = {0};

/** Native script command: forward to sub_080728A0() on the KMP viewport
 * table, but only for values in the procedural connection class range
 * 400-499 (see docs/map-editor-roadmap.md). @return Always 1. */
AT("00012490") s32 ScriptNativeMapSetBoundedValue(u32 count, const s32 *args,
                                                   s32 *result)
{
    s32 value = args[0];
    if ((u32)(value - 400) <= 99)
        sub_080728A0((void *)gKmpViewports, value);
    return 1;
}

/** Native script command: does nothing but succeed. @return Always 1. */
AT("000124FC") s32 ScriptNativeBattleStatus(u32 count, const s32 *args,
                                             s32 *result)
{
    return 1;
}

/** Native script command: copy the current map state's name string into
 * the current generated field map's +0x62A buffer. @return Always 1. */
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

/** Native script command: copy a 20-entry s16 value table (from args[2..21])
 * into the map record selected by args[0]/args[1] and refresh it. See
 * ScriptNativeWriteMapValuesB() for the other call site sharing this body.
 * @return Always 1. */
AT("000127B8") s32 ScriptNativeWriteMapValues(u32 count, const s32 *args,
                                               s32 *result)
{
    COPY_MAP_VALUES();
    return 1;
}

extern u8 *sub_08055F4C(s32 mode);
extern void sub_08001EB4(void *dest, const void *src, u32 size);
extern void BitSet(u8 *bits, u32 index, s32 enabled);
extern void sub_080083E0(s32 first, s32 second);

#define PMB_DECK_ENTRY_COUNT 20
#define PMB_DECK_VALUES_OFFSET 14
#define PMB_DECK_MAX_ENTRY 98

/** Select one of two adjacent game-state fields used by the deck system
 * (called with mode 1 and 2 from the deck-record initializer at 0x080083E0
 * and 0x080087EC); any other mode yields NULL. Exact field meaning
 * unresolved -- likely a left/right or player/opponent deck slot pair.
 * @param mode 1 selects +0x12F0, 2 selects +0x12F2.
 * @return A pointer into the game state, or NULL for an unrecognized mode.
 */
AT("0000696C")
void *GameStateSelectDeckPointer(s32 mode)
{
    switch (mode) {
    case 1: {
        u8 *iwram = gIwramBase;
        u32 offset = (u32)gMapGenerationRootOffset;
        return *(u8 **)(iwram + offset) + 0x12F0;
    }
    case 2: {
        u8 *iwram = gIwramBase;
        u32 offset = (u32)gMapGenerationRootOffset;
        return *(u8 **)(iwram + offset) + 0x12F2;
    }
    }
    return 0;
}

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
        s32 mode = args[0];
        s32 i;

        for (i = 0; i < 20; i++)
            values[i] = args[i + 1];
        sub_08001EB4(sub_08055F4C((s16)mode) + 14, values, 40);
        for (i = 0; (u32)i < count; i++)
            BitSet(GAME_ROOT + GAME_STATE_DECK_FLAGS_OFFSET, args[i], 1);
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

/** PmbDeckMake: clear the selected twenty-entry deck, pack valid input
 * entries at its front, and mark each accepted entry as owned. The accepted
 * modes are shared with ScriptNativeDeckMake().
 * @return Always 1. */
AT("000129F4") s32 ScriptNativePmbDeckMake(u32 count, const s32 *args,
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
        u8 *record = sub_08055F4C((s16)args[0]);
        s32 i;
        s32 outputIndex;
        s16 zero = 0;
        s16 *values;
        u8 **gameState;

        values = (s16 *)(record + PMB_DECK_VALUES_OFFSET);
        for (i = PMB_DECK_ENTRY_COUNT - 1; i >= 0; i--)
            values[i] = zero;

        gameState = &gMapGenerationRoot;
        outputIndex = 0;
        for (i = 0; i < PMB_DECK_ENTRY_COUNT; i++) {
            if ((s16)CountPmbDeckEntryCopies((s16)args[i + 1])
                <= PMB_DECK_MAX_ENTRY) {
                values[outputIndex] = (s16)args[i + 1];
                BitSet(*gameState + GAME_STATE_DECK_FLAGS_OFFSET,
                       values[outputIndex], 1);
                outputIndex++;
            }
        }
        sub_080083E0(0, 0);
        break;
    }
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

/** Native script command: forward to sub_0800690C() for layer 1 or 2 only;
 * other layer values are a silent no-op. @return Always 1. */
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
            s32 *party = RuntimeGetActorPartRecord(args[0], slot);
            party[0] = args[slot + 1];
            party[2] = 100;
            party[3] = 100;
        }
    }
    RuntimeActorSetField234(args[0], 1);
    return 1;
}

/** Native script command: same body as ScriptNativeWriteMapValues(), used
 * from a different bytecode call site. @return Always 1. */
AT("00012C14") s32 ScriptNativeWriteMapValuesB(u32 count, const s32 *args,
                                                s32 *result)
{
    COPY_MAP_VALUES();
    return 1;
}

REFRESH_MAP_VALUES("00012CA4", ScriptNativeRefreshMapValuesB)

/** Native script command: mark one script wait pending and forward to
 * RuntimeSetFieldEB4(). @return Always SCRIPT_WAIT (0x7fff). */
AT("00012CE8") s32 ScriptNativeSetPendingMapValue(u32 count, const s32 *args,
                                                   s32 *result)
{
    ScriptAddPendingTasks(1);
    RuntimeSetFieldEB4(args[0]);
    return 0x7fff;
}

/** Native script command: forward to GameStateGetField42BA().
 * @return Always 1. */
AT("00012D4C") s32 ScriptNativeGetMapStatus(u32 count, const s32 *args,
                                            s32 *result)
{
    *result = (s16)GameStateGetField42BA();
    return 1;
}
AT("00012D4C") const u8 ScriptNativeGetMapStatusTail[2] = {0};

/** Native script command ItemInit: clear all 256 consumable inventory slots.
 * @return Always 1. */
AT("00012D64") s32 ScriptNativeClearConsumableInventory(u32 count, const s32 *args,
                                                  s32 *result)
{
    s32 i;
    s32 fixed;
    s32 step;
    s32 value;
    u8 * volatile *root;
    s32 offset;
    register union {
        u8 *base;
        s32 next;
    } temporary;

    i = 0;
    root = &gMapGenerationRoot;
    offset = CONSUMABLE_INVENTORY_OFFSET;
    fixed = FIXED_16_16_ONE;
    value = 0;
    step = fixed;
    do {
        temporary.base = *root;
        *(u16 *)(temporary.base + offset + (i << 1)) = value;
        temporary.next = fixed;
        fixed += step;
        i = temporary.next >> 16;
    } while (i < MAP_HALFWORD_RECORD_COUNT);
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

/**
 * @brief Clear nonzero generation entries associated with the current field.
 * The low byte of value08 gates this update; value2E identifies the field and
 * value26 holds the cleared state. Their broader gameplay meanings are pending.
 * @return Nothing.
 */
AT("000729F4")
void MapGenerationClearCurrentFieldEntries(void)
{
    s32 field = GameStateGetField4256();
    s32 i;
    if ((u8)MapGenerationGetValue08())
    {
        for (i = 0; i < MAP_GENERATION_ENTRY_COUNT; i++)
        {
            if ((s16)MapGenerationGetFieldId(i) == field
                && (u16)MapGenerationGetEntryState(i) != 0)
                MapGenerationSetEntryState(i, 0);
        }
    }
}
AT("000729F4") const u8 MapGenerationClearCurrentFieldEntriesTail[2] = {0, 0};

/**
 * @brief Find the first active generation entry belonging to the current field.
 * @param x Receives the entry's X coordinate in pixels.
 * @param y Receives the entry's Y coordinate in pixels.
 * @return One when an entry is found, otherwise zero.
 */
AT("00072A38")
s32 MapGenerationGetCurrentFieldPosition(s32 *x, s32 *y)
{
    s32 field;
    s32 i;

    field = GameStateGetField4256();
    if ((u8)MapGenerationGetValue08())
        goto search;
    goto notFound;

found:
    *x = (MapGenerationGetTileX(i) + 1) * 8;
    *y = (MapGenerationGetTileY(i) + 1) * 8;
    return 1;

search:
    for (i = 0; i < MAP_GENERATION_ENTRY_COUNT; i++)
    {
        if ((s16)MapGenerationGetFieldId(i) == field
         && (u16)MapGenerationGetEntryState(i) != 0)
            goto found;
    }

notFound:
    return 0;
}
AT("00072A38") const u8 MapGenerationGetCurrentFieldPositionTail[2] = {0, 0};

/**
 * @brief Advance active generation-entry countdowns for the current field.
 * @param field Field identifier supplied by the caller.
 * @return Nothing.
 */
AT("00072A98")
void MapGenerationAdvanceCurrentFieldEntries(s32 field)
{
    s32 activeField;
    s32 i;
    s32 countdown;

    activeField = GameStateGetField4256();
    if (field != activeField || !(u8)MapGenerationGetValue08())
        return;

    for (i = 0; i < MAP_GENERATION_ENTRY_COUNT; i++)
    {
        if ((u16)MapGenerationGetEntryState(i) != 0
         && (s16)MapGenerationGetFieldId(i) == activeField)
        {
            countdown = MapGenerationGetCountdown(i);
            countdown--;
            if (countdown == 0)
            {
                MapGenerationSetEntryState(i, 0);
                MapGenerationSetCountdown(i, 0);
            }
            else
            {
                MapGenerationSetCountdown(i, countdown);
                MapGenerationSetUpdatePending(i, 1);
            }
        }
    }
}
AT("00072A98") const u8 MapGenerationAdvanceCurrentFieldEntriesTail[2] = {0, 0};

/**
 * @brief Check whether a generated field has an inactive entry slot.
 * @param field Field identifier to search for.
 * @return One when a matching inactive entry exists, otherwise zero.
 */
AT("00072B08")
s32 MapGenerationHasFreeEntryForField(s32 field)
{
    s32 i;

    if ((u8)MapGenerationGetValue08())
        goto search;
    goto notFound;

found:
    return 1;

search:
    for (i = 0; i < MAP_GENERATION_ENTRY_COUNT; i++)
    {
        if ((s16)MapGenerationGetFieldId(i) == field
         && (u16)MapGenerationGetEntryState(i) == 0)
            goto found;
    }

notFound:
    return 0;
}
AT("00072B08") const u8 MapGenerationHasFreeEntryForFieldTail[2] = {0, 0};
