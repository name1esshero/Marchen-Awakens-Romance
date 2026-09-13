#ifndef MAP_PLACEMENTS_H
#define MAP_PLACEMENTS_H

#include "gba/types.h"

#define BATTLE_ARENA_LAYOUT_COUNT 4
#define BATTLE_ARENA_PLACEMENT_COUNT 21
#define BATTLE_ARENA_SCREEN_POSITION_COUNT 84
#define BATTLE_ARENA_NEIGHBOR_ROW_COUNT 28

enum MapPlacementKind {
    MAP_PLACEMENT_ORIGIN = 0,
    MAP_PLACEMENT_ROOM = 1,
    MAP_PLACEMENT_PLAYER = 2,
};

struct MapPlacement {
    u8 index;
    u8 kind;
    char resourceName[18];
    u32 x;
    u32 y;
    u32 reserved[4];
};

struct BattleArenaLayout {
    u16 placementCount;
    char originName[42];
    struct MapPlacement placements[BATTLE_ARENA_PLACEMENT_COUNT];
};

struct BattleArenaScreenPosition {
    u16 arena;
    u16 index;
    u16 x;
    u16 y;
    u16 field08;
    u16 flags;
};

extern const struct BattleArenaScreenPosition
    gBattleArenaScreenPositions[BATTLE_ARENA_SCREEN_POSITION_COUNT];
extern const s16
    gBattleArenaNeighborIndices[BATTLE_ARENA_NEIGHBOR_ROW_COUNT][8];
extern const u32 gBattleArenaVisibilityMasks[13];

extern const struct BattleArenaLayout
    gBattleArenaLayouts[BATTLE_ARENA_LAYOUT_COUNT];
extern const struct BattleArenaLayout *const gBattleArenaLayoutTable[5];

#endif
