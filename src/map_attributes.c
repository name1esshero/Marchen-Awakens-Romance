/* KMP attribute helpers used by procedural-map and movement code. */
#include "kmp.h"
#include "map_generation.h"
#include "hit_region.h"

#include "rom_section.h"

/* Sample the leading edge/corner of an actor's collision bounds. Directions
 * run clockwise: north, northeast, east, southeast, south, southwest, west,
 * northwest. Invalid directions return zero without reading the map. */
AT("00018D10")
s32 MapAttributeProbeDirection(s32 direction, s32 x, s32 y,
                               const struct HitBounds *bounds)
{
    s32 px = (s16)x;
    s32 py = (s16)y;
    s32 probeX;
    s32 probeY;

    switch (direction) {
    case MAP_DIR_NORTH:
        probeX = px;
        probeY = py + bounds->top;
        break;
    case MAP_DIR_NORTHEAST:
        probeX = px + bounds->right;
        probeY = py + bounds->top;
        break;
    case MAP_DIR_EAST:
        probeX = px + bounds->right;
        probeY = py;
        break;
    case MAP_DIR_SOUTHEAST:
        probeX = px + bounds->right;
        probeY = py + bounds->bottom;
        break;
    case MAP_DIR_SOUTH:
        probeX = px;
        probeY = py + bounds->bottom;
        break;
    case MAP_DIR_SOUTHWEST:
        probeX = px + bounds->left;
        probeY = py + bounds->bottom;
        break;
    case MAP_DIR_WEST:
        probeX = px + bounds->left;
        probeY = py;
        break;
    case MAP_DIR_NORTHWEST:
        probeX = px + bounds->left;
        probeY = py + bounds->top;
        break;
    default:
        return 0;
    }
    return KmpReadAttribute(gKmpViewports, probeX, probeY);
}

/* Store the same directional collision probe coordinate for the procedural
 * connection machinery. Invalid directions leave the saved coordinate alone. */
AT("00018DA4")
void MapGenerationSetProbeDirection(s32 direction, s32 x, s32 y,
                                    const struct HitBounds *bounds)
{
    s32 px = (s16)x;
    s32 py = (s16)y;
    s32 probeX;
    s32 probeY;

    switch (direction) {
    case MAP_DIR_NORTH:
        probeX = px;
        probeY = py + bounds->top;
        break;
    case MAP_DIR_NORTHEAST:
        probeX = px + bounds->right;
        probeY = py + bounds->top;
        break;
    case MAP_DIR_EAST:
        probeX = px + bounds->right;
        probeY = py;
        break;
    case MAP_DIR_SOUTHEAST:
        probeX = px + bounds->right;
        probeY = py + bounds->bottom;
        break;
    case MAP_DIR_SOUTH:
        probeX = px;
        probeY = py + bounds->bottom;
        break;
    case MAP_DIR_SOUTHWEST:
        probeX = px + bounds->left;
        probeY = py + bounds->bottom;
        break;
    case MAP_DIR_WEST:
        probeX = px + bounds->left;
        probeY = py;
        break;
    case MAP_DIR_NORTHWEST:
        probeX = px + bounds->left;
        probeY = py + bounds->top;
        break;
    default:
        return;
    }
    MapGenerationSetValues00And04(probeX, probeY);
}

/* Store a generated connection's four signed pixel offsets. The unusual
 * argument order reflects the native caller: horizontal endpoints arrive
 * before the two vertical endpoints. */
AT("000720E8")
void MapGenerationSetVector(u32 index, s32 value0, s32 value4,
                            s32 value2, s32 value6)
{
    MapGenerationGetState()->vectors[index].value0 = value0;
    MapGenerationGetState()->vectors[index].value4 = value4;
    MapGenerationGetState()->vectors[index].value2 = value2;
    MapGenerationGetState()->vectors[index].value6 = value6;
}
AT("000720E8") const u8 MapGenerationSetVectorTail[2] = {0, 0};

/* Neighbor connection bits returned by MapAttributeGetConnectionMask. */
#define MAP_CONNECTION_NORTH 1
#define MAP_CONNECTION_EAST  2
#define MAP_CONNECTION_SOUTH 4
#define MAP_CONNECTION_WEST  8

/* Attribute classes treated as a procedural connection tile. */
#define MAP_ATTR_CONNECTION_CLASS_LO       400
#define MAP_ATTR_CONNECTION_CLASS_HI      5400
#define MAP_ATTR_CONNECTION_CLASS_COUNT     99

/* Return a NESW bit mask for neighboring procedural connection tiles.
 * Classes 400..499 and 5400..5499 are treated alike by the original code.
 * This establishes connectivity semantics, but does not yet establish the
 * complete collision meaning of either class on ordinary field maps. */
AT("00072130")
u32 MapAttributeGetConnectionMask(s32 tileX, s32 tileY)
{
    u32 mask = 0;
    s32 attribute;

    attribute = KmpReadAttribute(gKmpViewports, tileX << 3, (tileY - 1) << 3);
    if ((u32)(attribute - MAP_ATTR_CONNECTION_CLASS_LO) <= MAP_ATTR_CONNECTION_CLASS_COUNT
     || (u32)(attribute - MAP_ATTR_CONNECTION_CLASS_HI) <= MAP_ATTR_CONNECTION_CLASS_COUNT)
        mask = MAP_CONNECTION_NORTH;

    attribute = KmpReadAttribute(gKmpViewports, (tileX + 1) << 3, tileY << 3);
    if ((u32)(attribute - MAP_ATTR_CONNECTION_CLASS_LO) <= MAP_ATTR_CONNECTION_CLASS_COUNT
     || (u32)(attribute - MAP_ATTR_CONNECTION_CLASS_HI) <= MAP_ATTR_CONNECTION_CLASS_COUNT)
        mask |= MAP_CONNECTION_EAST;

    attribute = KmpReadAttribute(gKmpViewports, tileX << 3, (tileY + 1) << 3);
    if ((u32)(attribute - MAP_ATTR_CONNECTION_CLASS_LO) <= MAP_ATTR_CONNECTION_CLASS_COUNT
     || (u32)(attribute - MAP_ATTR_CONNECTION_CLASS_HI) <= MAP_ATTR_CONNECTION_CLASS_COUNT)
        mask |= MAP_CONNECTION_SOUTH;

    attribute = KmpReadAttribute(gKmpViewports, (tileX - 1) << 3, tileY << 3);
    if ((u32)(attribute - MAP_ATTR_CONNECTION_CLASS_LO) <= MAP_ATTR_CONNECTION_CLASS_COUNT
     || (u32)(attribute - MAP_ATTR_CONNECTION_CLASS_HI) <= MAP_ATTR_CONNECTION_CLASS_COUNT)
        mask |= MAP_CONNECTION_WEST;

    return mask;
}
