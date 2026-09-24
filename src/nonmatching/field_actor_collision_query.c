/* Readable candidate for the field actor overlap query at 0x08018A9C. */
#include "gba/types.h"
#include "geometry.h"
#include "hit_region.h"
#include "runtime_accessors.h"
#include "runtime_misc.h"
#include "script_sprite.h"

#define FIELD_ACTOR_CANDIDATE_COUNT 32
#define FIELD_ACTOR_STEP_PIXELS 4
#define FIELD_ACTOR_DIRECTION_UP 0
#define FIELD_ACTOR_DIRECTION_DOWN 1
#define FIELD_ACTOR_DIRECTION_RIGHT 2
#define FIELD_ACTOR_DIRECTION_LEFT 3
#define FIELD_ACTOR_RECORD_REQUIRED_FLAGS0 0x81
#define FIELD_ACTOR_RECORD_REQUIRED_FLAGS1 0x01

/** Partial 44-byte entry indexed alongside its rectangle at state +0x3F3C. */
struct FieldActorRuntimeRecord610
{
    u8 flags0;
    u8 flags1;
    u8 unknown02[8];
    s16 x;
    s16 y;
    u8 unknown14[30];
};

/**
 * @brief Find the nearest active collision target intersecting the next step.
 * @param isScriptSprite Receives zero for the +0x610 table or one for a
 *                       script-sprite candidate.
 * @param resultIndex Receives the selected entry index.
 * @param direction Step direction: 0 up, 1 down, 2 right, 3 left.
 * @param x Actor's current signed X coordinate.
 * @param y Actor's current signed Y coordinate.
 * @param bounds Actor bounds as signed corner offsets.
 * @return One if a candidate overlaps, otherwise zero. Outputs are untouched
 *         when no candidate overlaps.
 */
s32 FieldActorFindNearestOverlapCandidate(s32 *isScriptSprite,
                                          s32 *resultIndex,
                                          s32 direction,
                                          s16 x,
                                          s16 y,
                                          const struct HitBounds *bounds)
{
    s32 left;
    s32 right;
    s32 top;
    s32 bottom;
    s32 bestIndex = 0;
    s32 bestSource = 0;
    u16 bestDistance = 0;
    bool8 found = FALSE;
    s32 i;

    if (direction == FIELD_ACTOR_DIRECTION_UP)
        y -= FIELD_ACTOR_STEP_PIXELS;
    else if (direction == FIELD_ACTOR_DIRECTION_DOWN)
        y += FIELD_ACTOR_STEP_PIXELS;
    else if (direction == FIELD_ACTOR_DIRECTION_RIGHT)
        x += FIELD_ACTOR_STEP_PIXELS;
    else if (direction == FIELD_ACTOR_DIRECTION_LEFT)
        x -= FIELD_ACTOR_STEP_PIXELS;

    left = x + bounds->left;
    right = x + bounds->right;
    top = y + bounds->top;
    bottom = y + bounds->bottom;

    for (i = 0; i < FIELD_ACTOR_CANDIDATE_COUNT; i++)
    {
        struct FieldActorRuntimeRecord610 *record;
        struct HitBounds *candidateBounds;
        u16 distance;

        record = (struct FieldActorRuntimeRecord610 *)GameStateGetRecord610(i);
        candidateBounds = (struct HitBounds *)GameStateGetRecord3F3C(i);
        if ((record->flags0 & FIELD_ACTOR_RECORD_REQUIRED_FLAGS0) !=
                FIELD_ACTOR_RECORD_REQUIRED_FLAGS0 ||
            (record->flags1 & FIELD_ACTOR_RECORD_REQUIRED_FLAGS1) == 0)
            continue;
        if (left >= candidateBounds->right ||
            candidateBounds->left >= right ||
            top >= candidateBounds->bottom ||
            candidateBounds->top >= bottom)
            continue;

        distance = CalculatePointDistance(x, y, record->x, record->y);
        if (found && distance > bestDistance)
            continue;

        found = TRUE;
        bestDistance = distance;
        bestIndex = i;
        bestSource = 0;
    }

    for (i = 0; i < FIELD_ACTOR_CANDIDATE_COUNT; i++)
    {
        struct ScriptSprite *sprite;
        struct HitBounds *candidateBounds;
        u16 distance;

        sprite = (struct ScriptSprite *)GameStateGetRecord0B90(i);
        candidateBounds = (struct HitBounds *)GameStateGetRecord403C(i);
        if (!sprite->active || !sprite->hitBoundsEnabled ||
            (sprite->flags1 & FIELD_ACTOR_RECORD_REQUIRED_FLAGS1) == 0)
            continue;
        if (left >= candidateBounds->right ||
            candidateBounds->left >= right ||
            top >= candidateBounds->bottom ||
            candidateBounds->top >= bottom)
            continue;

        distance = CalculatePointDistance(x, y, sprite->x, sprite->y);
        if (found && distance > bestDistance)
            continue;

        found = TRUE;
        bestDistance = distance;
        bestIndex = i;
        bestSource = 1;
    }

    if (!found)
        return 0;

    *isScriptSprite = bestSource;
    *resultIndex = bestIndex;
    return 1;
}
