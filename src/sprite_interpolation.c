/* Path/interpolation work-buffer setup and paired coordinate evaluation. */
#include "sprite_engine.h"

#include "rom_section.h"

extern s32 sub_0807DF18(s32 position, s32 step, s32 start, s32 end, s32 base);
extern s32 sub_0807E264(s32 position, s32 step, s32 start, s32 end, s32 base);

/** Evaluate the same interpolation position for the X and Y parameter sets. */
AT("0007DC84")
void SpriteInterpolationEvaluatePair(s32 position, s16 *outX, s16 *outY,
                                     const struct SpriteInterpolationPair *pair)
{
    *outX = sub_0807DF18(position, pair->step, pair->xStart, pair->xEnd,
                         pair->base);
    *outY = sub_0807DF18(position, pair->step, pair->yStart, pair->yEnd,
                         pair->base);
}

AT("0007DC84")
const u8 SpriteInterpolationEvaluatePairTail[2] = {0, 0};

/** Evaluate a position without the looping evaluator's range wrap. */
AT("0007E0A4")
void SpriteInterpolationEvaluatePairClamped(
    s32 position, s16 *outX, s16 *outY,
    const struct SpriteInterpolationPair *pair)
{
    *outX = sub_0807E264(position, pair->step, pair->xStart, pair->xEnd,
                         pair->base);
    *outY = sub_0807E264(position, pair->step, pair->yStart, pair->yEnd,
                         pair->base);
}

AT("0007E0A4")
const u8 SpriteInterpolationEvaluatePairClampedTail[2] = {0, 0};
