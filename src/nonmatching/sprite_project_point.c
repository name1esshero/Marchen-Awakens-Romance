/* Readable reconstruction of the sprite perspective projection helper. */
#include "sprite_engine.h"

extern s32 __divsi3(s32 dividend, s32 divisor);

/** Project a fixed-point point around the renderer's viewport origin.
 *
 * The exact routine remains in code_0780C0.s because natural declarations
 * rotate six long-lived values among the saved registers under agbcc.
 */
void SpriteProjectPoint(struct SpriteVector3 *point)
{
    struct SpriteEngineState *state = gSpriteEngineState;
    s32 oldX = point->x;
    s32 oldY = point->y;
    s32 scale = point->z;
    s32 numerator;

    numerator = scale * oldX;
    point->x = state->viewportOriginX
             + (__divsi3(numerator << 12, state->projectionDivisor) >> 12);

    numerator = scale * oldY;
    point->y = state->viewportOriginY
             + (__divsi3(numerator << 12, state->projectionDivisor) >> 12);
}
