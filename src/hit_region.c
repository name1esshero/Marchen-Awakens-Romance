#include "hit_region.h"
#include "game_state.h"
#include "rom_section.h"

extern u8 gIwramBase[];

/** The 16-slot HitRegion table at IWRAM-root +0x1090. */
AT("00011464") struct HitRegion *GameStateGetHitRegion(s32 id)
{
    struct IwramGameStateRootLayout *iwram =
        (struct IwramGameStateRootLayout *)gIwramBase;

    return (struct HitRegion *)(iwram->gameState + 0x1090 + id * 16);
}

/** Translate corner bounds by an object's signed world position. */
AT("00006C2C")
void HitBoundsTranslate(struct HitBounds *destination, s32 x, s32 y,
                        const struct HitBounds *source)
{
    s32 translatedX = (s16)x;
    s32 translatedY = (s16)y;
    s32 field;
    s32 top;

    field = (u16)source->left;
    destination->left = translatedX + field;
    field = (u16)source->right;
    destination->right = translatedX + field;
    top = (u16)source->top;
    /* Keep the signed addition rooted at translatedY. This is equivalent to
     * translatedY + top and preserves the original agbcc operand order. */
    top = translatedY - -top;
    destination->top = top;
    translatedY += (u16)source->bottom;
    destination->bottom = translatedY;
}

/** Return the horizontal fixed-point correction that places a bound on an
 * eight-pixel tile edge. Eastward movement aligns the far edge; every other
 * direction aligns the near edge. */
AT("0000D4E8")
s32 HitBoundsGetHorizontalTileCorrection(enum MapProbeDirection direction,
                                         s32 fixedPosition,
                                         const struct HitBounds *bounds)
{
    s32 near = bounds->left;
    s32 offset = fixedPosition >> 16;
    s32 nearTile = (near + offset) >> 3;
    s32 far = bounds->right;
    s32 farTile = (far + offset) >> 3;
    s32 correction;

    if (direction == MAP_DIR_EAST)
        correction = ((farTile + 1) << 3) - far - 1;
    else
        correction = (nearTile << 3) - near;

    fixedPosition = correction << 16;
    return fixedPosition;
}

/** Return the vertical fixed-point correction that places a bound on an
 * eight-pixel tile edge. Southward movement aligns the far edge; every other
 * direction aligns the near edge. */
AT("0000D518")
s32 HitBoundsGetVerticalTileCorrection(enum MapProbeDirection direction,
                                       s32 fixedPosition,
                                       const struct HitBounds *bounds)
{
    s32 near = bounds->top;
    s32 offset = fixedPosition >> 16;
    s32 nearTile = (near + offset) >> 3;
    s32 far = bounds->bottom;
    s32 farTile = (far + offset) >> 3;
    s32 correction;

    if (direction == MAP_DIR_SOUTH)
        correction = ((farTile + 1) << 3) - far - 1;
    else
        correction = (nearTile << 3) - near;

    fixedPosition = correction << 16;
    return fixedPosition;
}

/** @brief Disable one indexed hit region. */
AT("00011504") void HitRegionDisable(s32 id)
{
    GameStateGetHitRegion(id)->active = 0;
}
AT("00011504") const u8 HitRegionDisableTail[2] = {0, 0};

/** @brief Disable every entry in the sixteen-region hit table. */
AT("00011514") void HitRegionDisableAll(void)
{
    struct HitRegion *region = GameStateGetHitRegion(0);
    s32 zero = 0;
    s32 i = 15;
    do {
        region->active = zero;
        i--;
        region++;
    } while (i >= 0);
}
AT("00011514") const u8 HitRegionDisableAllTail[2] = {0, 0};

/** Native HitInit (080121C4) creates an overlap region. Signed halfwords
 * deliberately retain the original truncation of script integer arguments. */
AT("00011530") void HitRegionInit(s32 id, s32 x, s32 y, s32 width, s32 height)
{
    struct HitRegion *region = GameStateGetHitRegion(id);
    region->active = 1;
    region->rect.x = x;
    region->rect.y = y;
    region->rect.width = width;
    region->rect.height = height;
    region->mode = 0;
}

/** Native HitHitRect (08012244) reactivates and changes the rectangle while
 * preserving its mode. Despite its name, this function performs no hit test. */
AT("00011654") void HitRegionSetRect(s32 id, s32 x, s32 y, s32 width, s32 height)
{
    struct HitRegion *region = GameStateGetHitRegion(id);
    region->active = 1;
    region->rect.x = x;
    region->rect.y = y;
    region->rect.width = width;
    region->rect.height = height;
}

/** Test the actor's translated corner bounds against active map regions.
 * All comparisons are strict: touching an edge never counts as a hit.
 * Signed coordinate promotion and unrestricted signed widths intentionally
 * preserve the original behavior, including malformed/inverted rectangles.
 */
AT("00018C4C") s32 HitRegionTest(s16 x, s16 y, const struct HitBounds *bounds)
{
    s32 left, right, top, bottom;
    s32 px = x, py = y;
    struct HitRegion *region;
    s32 i;
    struct HitRect *rect;

    left = bounds->left + px;
    right = bounds->right + px;
    top = bounds->top + py;
    bottom = bounds->bottom + py;
    region = GameStateGetHitRegion(0);
    for (i = 0; i < 16; i++, region++) {
        if (region->active) {
            rect = &region->rect;
            if (region->mode == 0) {
                if (left < rect->width + rect->x && rect->x < right
                    && top < rect->height + rect->y && rect->y < bottom)
                    return i + 1;
            } else if (region->mode == 1) {
                if (left > rect->x && right < rect->width + rect->x
                    && top > rect->y && bottom < rect->height + rect->y)
                    return i + 1;
            }
        }
    }
    return 0;
}
AT("00018C4C") const u8 HitRegionTestTail[2] = {0, 0};
