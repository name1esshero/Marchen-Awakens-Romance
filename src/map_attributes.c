/* KMP attribute helpers used by procedural-map and movement code. */
#include "kmp.h"
#include "map_generation.h"
#include "hit_region.h"
#include "game_state.h"

#include "rom_section.h"

/** Sample the leading edge/corner of an actor's collision bounds. Directions
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

/** Store the same directional collision probe coordinate for the procedural
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

/** Store a generated connection's four signed pixel offsets. The unusual
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

/** Return a NESW bit mask for neighboring procedural connection tiles.
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

/**
 * @brief Collect tiles with an exact attribute value, scanning rows then columns.
 * @param view Viewport containing the attribute plane.
 * @param xs,ys Receive matching tile X/Y coordinates in scan order.
 * @param left,top First tile column and row to scan, inclusive.
 * @param right,bottom Last tile column and row, exclusive.
 * @param attribute Exact attribute to find, including -1 for out-of-map probes.
 * @param capacity Positive capacity of each output array, as required by callers.
 * @return One if any matches were written, otherwise zero; this is not a count.
 */
AT("00072924")
s32 MapCollectAttributePositions(struct KmpViewport *view, s32 *xs, s32 *ys,
    s32 left, s32 top, s32 right, s32 bottom, s32 attribute, s32 capacity)
{
    s32 x, y, count = 0;

    for (y = top; y < bottom; y++)
    {
        for (x = left; x < right; x++)
        {
            if (KmpReadAttribute(view, x * KMP_TILE_SIZE, y * KMP_TILE_SIZE) == attribute)
            {
                *xs++ = x;
                *ys++ = y;
                count++;
                if (count >= capacity)
                    goto found;
            }
        }
    }
    if (count == 0)
        goto notFound;
found:
    return 1;
notFound:
    return 0;
}

/**
 * @brief Find the first active current-field entry intersecting actor bounds.
 * @param unused Reserved argument; the original routine ignores it.
 * @param bounds Actor corner offsets relative to its position.
 * @param x Actor X in pixels.
 * @param y Actor Y in pixels.
 * @param direction Facing value; value 3 mirrors the horizontal bounds.
 * @return Matching field ID or zero. Touching rectangle edges count as a hit.
 * Entry vectors contain X, Y, width and height; they are not corner offsets.
 */
AT("00072B48")
s32 MapGenerationFindOverlappingEntry(s32 unused, const struct HitBounds *bounds,
    s32 x, s32 y, s32 direction)
{
    s32 left, right, top, bottom;
    s32 i, field, entryX, entryRight, entryY, entryBottom;

    if (direction == MAP_ENTRY_FACING_MIRRORED)
    {
        left = x - bounds->right;
        right = bounds->left;
        right = x - right;
    }
    else
    {
        left = x + bounds->left;
        right = bounds->right + x;
    }
    top = bounds->top + y;
    bottom = bounds->bottom + y;
    for (i = 0; i < MAP_GENERATION_ENTRY_COUNT; i++)
    {
        if ((u16)MapGenerationGetEntryState(i) == 0)
            continue;
        field = (s16)MapGenerationGetFieldId(i);
        if (field != GameStateGetField4256())
            continue;
        entryX = MapGenerationGetVectorValue0(i);
        entryRight = entryX + MapGenerationGetVectorValue4(i);
        entryY = MapGenerationGetVectorValue2(i);
        entryBottom = entryY + MapGenerationGetVectorValue6(i);
        if (left <= entryRight && top <= entryBottom && entryX <= right && entryY <= bottom)
            return (s16)MapGenerationGetFieldId(i);
    }
    return 0;
}
