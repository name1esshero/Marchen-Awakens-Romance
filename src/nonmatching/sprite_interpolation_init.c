/* Readable reconstruction of the interpolation work-buffer initializer. */
#include "sprite_engine.h"

/** Divide caller storage into eight count-sized arrays, leaving one word
 * between arrays, then widen the input coordinate pairs from s16 to s32.
 *
 * The routine's exact assembly remains in code_0780C0.s because agbcc assigns
 * three long-lived values to different registers for every natural C shape
 * tested so far.
 */
void SpriteInterpolationInit(struct SpriteInterpolation *state, s32 *storage,
                             const s16 *x, const s16 *y, s32 count)
{
    s32 *next;
    s32 *outputX;
    s32 *outputY;
    s32 remaining;

    state->count = count;
    state->x = storage;

    next = storage + count + 1;
    state->y = next;
    outputY = next;
    next += count + 1;
    state->segmentLength = next;
    next += count + 1;
    state->segmentScale = next;
    next += count + 1;
    state->work5 = next;
    next += count + 1;
    state->work6 = next;
    next += count + 1;
    state->work7 = next;
    next += count + 1;
    state->work8 = next;

    outputX = storage;
    remaining = count;
    while (remaining > 0) {
        *outputX++ = *x++;
        *outputY++ = *y++;
        remaining--;
    }
}
