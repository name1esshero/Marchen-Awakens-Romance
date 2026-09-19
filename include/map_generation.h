#ifndef MAP_GENERATION_H
#define MAP_GENERATION_H

#include "gba/types.h"

#define MAP_GENERATION_ENTRY_COUNT 4

struct KmpViewport;

/** Clear generation entries whose field identifier matches the active field. */
void MapGenerationClearCurrentFieldEntries(void);

/** Read the first active current-field entry as pixel coordinates, if present. */
s32 MapGenerationGetCurrentFieldPosition(s32 *x, s32 *y);
void MapGenerationAdvanceCurrentFieldEntries(s32 field);
s32 MapGenerationHasFreeEntryForField(s32 field);

/* The actor facing value used by 08072B48, distinct from MapProbeDirection. */
#define MAP_ENTRY_FACING_MIRRORED 3
struct HitBounds;
s32 MapGenerationFindOverlappingEntry(s32 unused, const struct HitBounds *bounds,
    s32 x, s32 y, s32 direction);

struct MapGenerationVector {
    s16 value0;
    s16 value2;
    s16 value4;
    s16 value6;
};

/* Work area used by the procedural map generator. Field names retain their
 * offsets until the callers establish their game-level meaning. */
struct MapGenerationState {
    u32 value00;
    u32 value04;
    s8 value08;
    u8 unknown09[3];
    void *pointer0C;
    void *pointer10;
    void *pointer14;
    void *pointer18;
    void *pointer1C;
    void *pointer20;
    s8 value24;
    s8 value25;
    s16 entryState[4];
    s16 fieldId[4];
    u8 unknown36[2];
    s32 tileX[4];
    s32 tileY[4];
    s32 countdown[4];
    s8 updatePending[4];
    struct MapGenerationVector vectors[4];
};

/* A generated field's persistent 44-byte room definition. */
struct GeneratedMapRoomRecord {
    u8 unknown00[20];
    s32 property14;
    s32 property18;
    u8 unknown1C[16];
};

/* Per-room event state. The generator owns a fixed pool of 64 records. */
#define GENERATED_MAP_RUNTIME_ROOM_ACTIVE_OFFSET 20
struct GeneratedMapRuntimeRoom {
    s16 roomIndex;
    u8 unknown02[18];
    s8 active;
    s8 scriptFlag;
    u8 unknown16[2];
};

struct GeneratedFieldMap {
    u16 width;
    u16 height;
    u16 unknown04;
    u16 currentCell;
    u8 unknown08[8];
    s32 parameter10;
    u8 unknown14[0x640];
    u16 *cellRoomIndices;
    struct GeneratedMapRoomRecord *rooms;
};

extern u32 gMapGenerationSeed;

void MapGenerationSeedRandom(u32 seed);
u32 MapGenerationRandom(void);
struct MapGenerationState *MapGenerationGetState(void);
void MapGenerationSetValues00And04(u32 value00, u32 value04);
u32 MapGenerationGetValue00(void);
u32 MapGenerationGetValue04(void);
void MapGenerationSetPointer0C(void *value);
void MapGenerationSetPointer14(void *value);
void MapGenerationSetPointer10(void *value);
void MapGenerationSetPointer18(void *value);
void *MapGenerationGetPointer0C(void);
void *MapGenerationGetPointer14(void);
void *MapGenerationGetPointer10(void);
void *MapGenerationGetPointer18(void);
void *MapGenerationGetPointer1C(void);
void *MapGenerationGetPointer20(void);
void MapGenerationSetPointer1C(void *value);
void MapGenerationSetPointer20(void *value);
s32 MapGenerationGetTileX(u32 index);
s32 MapGenerationGetTileY(u32 index);
void MapGenerationSetTilePosition(u32 index, s32 tileX, s32 tileY);
void MapGenerationSetEntryState(u32 index, s32 value);
s32 MapGenerationGetEntryState(u32 index);
void MapGenerationSetCountdown(u32 index, s32 value);
s32 MapGenerationGetCountdown(u32 index);
void MapGenerationSetUpdatePending(u32 index, s32 value);
s32 MapGenerationGetUpdatePending(u32 index);
s32 MapGenerationGetValue24(void);
void MapGenerationSetValue24(s32 value);
void MapGenerationSetFieldId(u32 index, s32 value);
s32 MapGenerationGetFieldId(u32 index);
s32 MapGenerationGetValue25(void);
void MapGenerationSetValue25(s32 value);
s32 MapGenerationGetValue08(void);
void MapGenerationSetValue08(s32 value);
s32 MapGenerationGetVectorValue0(u32 index);
s32 MapGenerationGetVectorValue2(u32 index);
s32 MapGenerationGetVectorValue4(u32 index);
s32 MapGenerationGetVectorValue6(u32 index);
void MapGenerationSetVector(u32 index, s32 value0, s32 value4,
                            s32 value2, s32 value6);
u32 MapAttributeGetConnectionMask(s32 tileX, s32 tileY);
struct GeneratedMapRuntimeRoom *GeneratedMapFindRuntimeRoom(s32 roomIndex);
s32 GeneratedMapChooseAttributeIndex(struct KmpViewport *view, u32 attribute);
s32 GeneratedMapGetCurrentRoomProperty14(struct GeneratedFieldMap *map);
s32 GeneratedMapGetCurrentRoomProperty18(struct GeneratedFieldMap *map);
void GeneratedMapSetParameter10(s32 value);
s32 GeneratedMapGetParameter10(void);
void GeneratedMapSetCurrentRoomFlag(s32 value);
s32 GeneratedMapGetCurrentRoomFlag(void);

#endif
