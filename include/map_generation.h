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
    u32 value1C;
    u32 value20;
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

/* A generated field's persistent 44-byte room definition. The arena's
 * table starts with a header entry whose connections field holds the number
 * of room templates that follow it. */
struct GeneratedMapRoomRecord {
    s16 connections;               /* 00: required connection bits (0xFFF) */
    char name[18];                 /* 02: KMP resource name, without ".KMP" */
    s32 property14;
    s32 property18;
    u8 unknown1C[16];
};

/* Per-room event state. The generator owns a fixed pool of 64 records. */
#define GENERATED_MAP_RUNTIME_ROOM_ACTIVE_OFFSET 20
struct GeneratedMapRuntimeRoom {
    s16 roomIndex;
    u8 unknown02[2];
    u32 unknown04;
    s32 x;                         /* 08: spawn position in pixels */
    s32 y;                         /* 0C */
    u32 unknown10;
    s8 active;
    s8 scriptFlag;
    u8 unknown16[2];
};

#define GENERATED_MAP_RUNTIME_ROOM_COUNT 64

struct GeneratedFieldMap {
    u16 width;
    u16 height;
    u16 startCell;                 /* 004: cell the maze generator started from */
    u16 currentCell;
    u8 unknown08[4];
    u32 seed;                      /* 00C: MapGenerationRandom seed */
    s32 parameter10;
    u8 unknown14[4];
    struct GeneratedMapRuntimeRoom runtimeRooms[GENERATED_MAP_RUNTIME_ROOM_COUNT]; /* 018 */
    u8 unknown618[12];
    u16 unknown624;
    u16 unknown626;
    u16 unknown628;
    u8 unknown62A[0x1E];
    u32 arenaIndex;                /* 648: see BattleRuntimeSetArena() */
    u32 unknown64C;
    u32 *cellRecords;              /* 650: one 32-bit generation record per cell */
    u16 *cellRoomIndices;          /* 654 */
    struct GeneratedMapRoomRecord *rooms; /* 658: arena room templates */
};

/* Corridor-carving directions, in the order the generator tries them. */
enum
{
    GENERATED_MAP_DIRECTION_EAST,
    GENERATED_MAP_DIRECTION_SOUTH,
    GENERATED_MAP_DIRECTION_WEST,
    GENERATED_MAP_DIRECTION_NORTH,
};

/* A carve advances two cells and keeps a three-cell margin from the far
 * edges of the grid. */
#define GENERATED_MAP_CARVE_STEP 2
#define GENERATED_MAP_CARVE_MARGIN 3

/* Nonzero cardinal neighbors reported by GeneratedMapGetPathNeighborShape. */
#define GENERATED_MAP_NEIGHBOR_WEST  0x0001
#define GENERATED_MAP_NEIGHBOR_EAST  0x0010
#define GENERATED_MAP_NEIGHBOR_NORTH 0x0100
#define GENERATED_MAP_NEIGHBOR_SOUTH 0x1000

/* Corridor shape codes for exactly two neighbors; everything else is OTHER. */
#define GENERATED_MAP_PATH_VERTICAL   3
#define GENERATED_MAP_PATH_HORIZONTAL 4
#define GENERATED_MAP_PATH_EAST_NORTH 5
#define GENERATED_MAP_PATH_WEST_NORTH 6
#define GENERATED_MAP_PATH_WEST_SOUTH 7
#define GENERATED_MAP_PATH_EAST_SOUTH 8
#define GENERATED_MAP_PATH_OTHER      9

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
u32 MapGenerationGetValue1C(void);
u32 MapGenerationGetValue20(void);
void MapGenerationSetValue1C(u32 value);
void MapGenerationSetValue20(u32 value);
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
bool8 GeneratedMapIsValidStartCell(const struct GeneratedFieldMap *map, s32 cell);
bool8 GeneratedMapHasCarveDirection(const struct GeneratedFieldMap *map,
                                    const u8 *cells, u32 index);
bool8 GeneratedMapCanCarveTwoCellStep(const struct GeneratedFieldMap *map,
                                      const u8 *cells, u32 index, u8 direction);
u32 GeneratedMapGetPathNeighborShape(const struct GeneratedFieldMap *map,
                                     const u8 *cells, u16 index,
                                     u8 returnNeighborMask);

#endif
