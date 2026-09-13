#ifndef MAP_GENERATION_H
#define MAP_GENERATION_H

#include "gba/types.h"

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
    s16 values26[4];
    s16 values2E[4];
    u8 unknown36[2];
    void *table38[4];
    void *table48[4];
    void *table58[4];
    s8 values68[4];
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

#define gMapGenerationSeed (*(u32 *)0x03004044)

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
void *MapGenerationGetTable38(u32 index);
void *MapGenerationGetTable48(u32 index);
void MapGenerationSetTables(u32 index, void *table38, void *table48);
void MapGenerationSetValue26(u32 index, s32 value);
s32 MapGenerationGetValue26(u32 index);
void MapGenerationSetTable58(u32 index, void *value);
void *MapGenerationGetTable58(u32 index);
void MapGenerationSetValue68(u32 index, s32 value);
s32 MapGenerationGetValue68(u32 index);
s32 MapGenerationGetValue24(void);
void MapGenerationSetValue24(s32 value);
void MapGenerationSetValue2E(u32 index, s32 value);
s32 MapGenerationGetValue2E(u32 index);
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
s32 GeneratedMapGetCurrentRoomProperty14(struct GeneratedFieldMap *map);
s32 GeneratedMapGetCurrentRoomProperty18(struct GeneratedFieldMap *map);
void GeneratedMapSetParameter10(s32 value);
s32 GeneratedMapGetParameter10(void);
void GeneratedMapSetCurrentRoomFlag(s32 value);
s32 GeneratedMapGetCurrentRoomFlag(void);

#endif
