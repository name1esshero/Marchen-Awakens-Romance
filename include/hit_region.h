#ifndef HIT_REGION_H
#define HIT_REGION_H
#include "gba/types.h"

/* Sixteen 16-byte regions at engine state +0x1090 (lookup 08011464).
 * 08018C4C compares the actor's translated bounding box against these fields.
 * Mode 0: strict overlap. Mode 1: strict containment (touching edges fails).
 * Other modes do not hit. Inactive records retain their rectangle and mode.
 * A hit returns the first matching region index + 1; no hit returns zero.
 */
struct HitRect
{
    s16 x, y, width, height;
};

struct HitRegion
{
    s32 active;
    struct HitRect rect;
    s32 mode;
};
struct HitRegion *GameStateGetHitRegion(s32 id);

/* Actor bounds use corner offsets, unlike a region's width/height fields. */
struct HitBounds
{
    s16 left, top, right, bottom;
};
void HitBoundsTranslate(struct HitBounds *destination, s32 x, s32 y,
                        const struct HitBounds *source);
s32 HitRegionTest(s16 x, s16 y, const struct HitBounds *bounds);

/* Directional collision probe, clockwise from north. Used by
 * MapAttributeProbeDirection and MapGenerationSetProbeDirection. */
enum MapProbeDirection
{
    MAP_DIR_NORTH = 1,
    MAP_DIR_NORTHEAST,
    MAP_DIR_EAST,
    MAP_DIR_SOUTHEAST,
    MAP_DIR_SOUTH,
    MAP_DIR_SOUTHWEST,
    MAP_DIR_WEST,
    MAP_DIR_NORTHWEST,
};

s32 HitBoundsGetHorizontalTileCorrection(enum MapProbeDirection direction,
                                         s32 fixedPosition,
                                         const struct HitBounds *bounds);
s32 HitBoundsGetVerticalTileCorrection(enum MapProbeDirection direction,
                                       s32 fixedPosition,
                                       const struct HitBounds *bounds);

/* The original lookup does not bounds-check id; valid table indices are 0..15. */
void HitRegionDisable(s32 id);
void HitRegionDisableAll(void);
void HitRegionInit(s32 id, s32 x, s32 y, s32 width, s32 height);
void HitRegionSetRect(s32 id, s32 x, s32 y, s32 width, s32 height);
#endif
