/* Initial room/player placement for the four AD battle-map families. */
#include "map_placements.h"

#include "rom_section.h"

AT("001BF408")
const struct BattleArenaScreenPosition
gBattleArenaScreenPositions[BATTLE_ARENA_SCREEN_POSITION_COUNT] = {
#include "../build/generated/map_screen_positions.inc"
};

AT("001BF7F8")
const s16 gBattleArenaNeighborIndices[BATTLE_ARENA_NEIGHBOR_ROW_COUNT][8] = {
#include "../build/generated/map_neighbor_indices.inc"
};

AT("001BF9B8")
const u32 gBattleArenaVisibilityMasks[13] = {
#include "../build/generated/map_visibility_masks.inc"
};

AT("001BF9EC")
const struct BattleArenaLayout gBattleArenaLayouts[BATTLE_ARENA_LAYOUT_COUNT] = {
#include "../build/generated/map_placements.inc"
};

/** Arena selector order used by GetBattleDefinition. */
AT("001C090C")
const struct BattleArenaLayout *const gBattleArenaLayoutTable[5] = {
    &gBattleArenaLayouts[3],
    &gBattleArenaLayouts[1],
    &gBattleArenaLayouts[0],
    &gBattleArenaLayouts[2],
    &gBattleArenaLayouts[3],
};
