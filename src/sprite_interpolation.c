/* Path/interpolation work-buffer setup and paired coordinate evaluation. */
#include "sprite_engine.h"

#define AT(x) __attribute__((section(".rom." x)))
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#else
#define TARGET_REGISTER(name) asm(name)
#endif

extern s32 sub_0807DF18(s32 position, s32 step, s32 start, s32 end, s32 base);
extern s32 sub_0807E264(s32 position, s32 step, s32 start, s32 end, s32 base);

/* Divide caller storage into eight count-sized arrays, leaving one word
 * between arrays, then widen the input coordinate pairs from s16 to s32. */
AT("0007DB54")
void SpriteInterpolationInit(struct SpriteInterpolation *state, s32 *storage,
                             const s16 *x, const s16 *y, volatile s32 count)
{
    register s32 *output TARGET_REGISTER("r6");
    register const s16 *inputX TARGET_REGISTER("r5");
    register const s16 *inputY TARGET_REGISTER("r4");
    s32 remainingCount;
    register s32 *outputX TARGET_REGISTER("r1");
    register s32 remaining TARGET_REGISTER("r2");
    register s32 *outputY TARGET_REGISTER("r3") = output;
    register u8 *afterX TARGET_REGISTER("r1");
    s32 offset;

    output = storage;
    inputX = x;
    inputY = y;
    asm("" : "+r"(output), "+r"(inputX), "+r"(inputY) : : "memory");
    remainingCount = count;
    state->count = remainingCount;
    state->x = output;
    offset = remainingCount * 4;
    afterX = (u8 *)output + offset;
    outputY = (s32 *)(afterX + 4);
    state->y = outputY;
    state->segmentLength = (s32 *)((u8 *)outputY + offset + 4);
    state->segmentScale = (s32 *)((u8 *)state->segmentLength + offset + 4);
    state->work5 = (s32 *)((u8 *)state->segmentScale + offset + 4);
    state->work6 = (s32 *)((u8 *)state->work5 + offset + 4);
    state->work7 = (s32 *)((u8 *)state->work6 + offset + 4);
    state->work8 = (s32 *)((u8 *)state->work7 + offset + 4);

    if (remainingCount > 0) {
        outputX = output;
        remaining = remainingCount;
        do {
            *outputX++ = *inputX++;
            *outputY++ = *inputY++;
        } while (--remaining != 0);
    }
}

/* The cartridge stores zero alignment bytes after this helper. */
AT("0007DB54")
const u8 SpriteInterpolationInitTail[2] = {0, 0};

/* Evaluate the same interpolation position for the X and Y parameter sets. */
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

/* Evaluate a position without the looping evaluator's range wrap. */
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
