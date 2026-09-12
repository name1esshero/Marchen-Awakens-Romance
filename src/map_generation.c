/* Procedural map-generation state and its deterministic local RNG. */
#include "map_generation.h"

#define AT(x) __attribute__((section(".rom." x)))

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

AT("00071DD4")
void MapGenerationSeedRandom(u32 seed) { gMapGenerationSeed = seed; }

AT("00071DE0")
u32 MapGenerationRandom(void)
{
    gMapGenerationSeed = gMapGenerationSeed * 0x41C64E6D + 0x3039;
    return (gMapGenerationSeed << 1) >> 17;
}

AT("00071E00")
struct MapGenerationState *MapGenerationGetState(void)
{
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset);
    return (struct MapGenerationState *)((u8 *)*root + 0x1304);
}

AT("00071E1C")
void MapGenerationSetValues00And04(u32 value00, u32 value04)
{
    MapGenerationGetState()->value00 = value00;
    MapGenerationGetState()->value04 = value04;
}

AT("00071E34") u32 MapGenerationGetValue00(void) { return MapGenerationGetState()->value00; }
AT("00071E40") u32 MapGenerationGetValue04(void) { return MapGenerationGetState()->value04; }
AT("00071E4C") void MapGenerationSetPointer0C(void *v) { MapGenerationGetState()->pointer0C = v; }
AT("00071E5C") void MapGenerationSetPointer14(void *v) { MapGenerationGetState()->pointer14 = v; }
AT("00071E6C") void MapGenerationSetPointer10(void *v) { MapGenerationGetState()->pointer10 = v; }
AT("00071E7C") void MapGenerationSetPointer18(void *v) { MapGenerationGetState()->pointer18 = v; }
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
