/* Readable affine-OAM matrix writers. Both project compiler snapshots emit
 * 148/152/148-byte routines from this clean shape, while the ROM uses
 * 152/156/152 bytes and a different saved-register allocation. The former
 * matching source forced twelve registers per routine and read two
 * uninitialized dummy locals through inline assembly. Exact implementations
 * remain named assembly until the missing natural lifetime is recovered. */
#include "sprite_engine.h"

#include "math_tables.h"

#define SPRITE_ANGLE_MASK 0x0FFF
#define SPRITE_QUARTER_TURN 0x0400
#define SPRITE_VECTOR_FRACTION_BITS 14

static void SpriteAffineWrite(s32 index, s32 angle, s32 scaleX, s32 scaleY,
                              s32 xSign, s32 ySign)
{
    struct SpriteAffineOamMatrix *matrix;
    s32 sine;
    s32 cosine;
    s32 inverseX;
    s32 inverseY;

    angle = (s16)angle;
    scaleX = (s16)scaleX;
    scaleY = (s16)scaleY;
    matrix = SpriteEngineGetAffineOamMatrix(index);

    sine = gSineTable14[angle & SPRITE_ANGLE_MASK];
    cosine = gSineTable14[(angle + SPRITE_QUARTER_TURN)
                        & SPRITE_ANGLE_MASK];
    inverseX = (s16)SpriteMathDivide65536ByS16(scaleX);
    inverseY = (s16)SpriteMathDivide65536ByS16(scaleY);

    matrix->pa = xSign
               * ((cosine * inverseX) >> SPRITE_VECTOR_FRACTION_BITS);
    matrix->pb = xSign
               * ((sine * inverseX) >> SPRITE_VECTOR_FRACTION_BITS);
    matrix->pc = -ySign
               * ((sine * inverseY) >> SPRITE_VECTOR_FRACTION_BITS);
    matrix->pd = ySign
               * ((cosine * inverseY) >> SPRITE_VECTOR_FRACTION_BITS);
}

/** Write a normally oriented affine matrix into an OAM matrix slot. */
void SpriteAffineWriteNormal(s32 index, s32 angle, s32 scaleX, s32 scaleY)
{
    SpriteAffineWrite(index, angle, scaleX, scaleY, 1, 1);
}

/** Write an affine matrix mirrored across its X axis. */
void SpriteAffineWriteMirrored(s32 index, s32 angle, s32 scaleX, s32 scaleY)
{
    SpriteAffineWrite(index, angle, scaleX, scaleY, -1, 1);
}

/** Write an affine matrix with the opposite Y-axis handedness. */
void SpriteAffineWriteAlternateAxis(s32 index, s32 angle, s32 scaleX,
                                    s32 scaleY)
{
    SpriteAffineWrite(index, angle, scaleX, scaleY, 1, -1);
}
