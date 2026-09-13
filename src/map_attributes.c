/* KMP attribute helpers used by procedural-map and movement code. */
#include "kmp.h"
#include "map_generation.h"
#include "hit_region.h"

#define AT(x) __attribute__((section(".rom." x)))

/* Sample the leading edge/corner of an actor's collision bounds. Directions
 * run clockwise: north, northeast, east, southeast, south, southwest, west,
 * northwest. Invalid directions return zero without reading the map. */
#ifdef NONMATCHING
AT("00018D10")
s32 MapAttributeProbeDirection(s32 direction, s32 x, s32 y,
                               const struct HitBounds *bounds)
{
    s32 px = (s16)x;
    s32 py = (s16)y;

    switch (direction) {
    case 1:
        py += bounds->top;
        break;
    case 2:
        px += bounds->right;
        py += bounds->top;
        break;
    case 3:
        px += bounds->right;
        break;
    case 4:
        px += bounds->right;
        py += bounds->bottom;
        break;
    case 5:
        py += bounds->bottom;
        break;
    case 6:
        px += bounds->left;
        py += bounds->bottom;
        break;
    case 7:
        px += bounds->left;
        break;
    case 8:
        px += bounds->left;
        py += bounds->top;
        break;
    default:
        return 0;
    }
    return KmpReadAttribute((struct KmpViewport *)0x03003BC4, px, py);
}

/* Store the same directional collision probe coordinate for the procedural
 * connection machinery. Invalid directions leave the saved coordinate alone. */
AT("00018DA4")
void MapGenerationSetProbeDirection(s32 direction, s32 x, s32 y,
                                    const struct HitBounds *bounds)
{
    s32 px = (s16)x;
    s32 py = (s16)y;

    switch (direction) {
    case 1:
        py += bounds->top;
        break;
    case 2:
        px += bounds->right;
        py += bounds->top;
        break;
    case 3:
        px += bounds->right;
        break;
    case 4:
        px += bounds->right;
        py += bounds->bottom;
        break;
    case 5:
        py += bounds->bottom;
        break;
    case 6:
        px += bounds->left;
        py += bounds->bottom;
        break;
    case 7:
        px += bounds->left;
        break;
    case 8:
        px += bounds->left;
        py += bounds->top;
        break;
    default:
        return;
    }
    MapGenerationSetValues00And04(px, py);
}
#endif

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

/* Return a NESW bit mask for neighboring procedural connection tiles.
 * Classes 400..499 and 5400..5499 are treated alike by the original code.
 * This establishes connectivity semantics, but does not yet establish the
 * complete collision meaning of either class on ordinary field maps. */
AT("00072130")
u32 MapAttributeGetConnectionMask(s32 tileX, s32 tileY)
{
    u32 mask = 0;
    s32 attribute;

    attribute = KmpReadAttribute((struct KmpViewport *)0x03003BC4,
                                 tileX << 3, (tileY - 1) << 3);
    if ((u32)(attribute - 400) <= 99 || (u32)(attribute - 5400) <= 99)
        mask = 1;

    attribute = KmpReadAttribute((struct KmpViewport *)0x03003BC4,
                                 (tileX + 1) << 3, tileY << 3);
    if ((u32)(attribute - 400) <= 99 || (u32)(attribute - 5400) <= 99)
        mask |= 2;

    attribute = KmpReadAttribute((struct KmpViewport *)0x03003BC4,
                                 tileX << 3, (tileY + 1) << 3);
    if ((u32)(attribute - 400) <= 99 || (u32)(attribute - 5400) <= 99)
        mask |= 4;

    attribute = KmpReadAttribute((struct KmpViewport *)0x03003BC4,
                                 (tileX - 1) << 3, tileY << 3);
    if ((u32)(attribute - 400) <= 99 || (u32)(attribute - 5400) <= 99)
        mask |= 8;

    return mask;
}
