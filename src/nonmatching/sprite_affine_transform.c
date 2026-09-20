/* Readable reconstructions of affine matrix and packed-position helpers.
 * New agbcc emits 140/116 bytes and old agbcc emits 144/120 bytes for these
 * routines; the ROM uses 152/124 bytes. Removing any of the former 13 machine
 * register and barrier hints changes code generation, so the exact routines
 * remain named assembly until their missing source-level lifetimes are known. */
#include "sprite_engine.h"

#include "math_tables.h"

#define SPRITE_ANGLE_MASK 0x0FFF
#define SPRITE_QUARTER_TURN 0x0400
#define SPRITE_VECTOR_FRACTION_BITS 14

#define PACKED_X_HIGH_MASK 0x0FFF0000
#define PACKED_Y_NIBBLE_MASK 0x000F
#define PACKED_Y_REMAINING_MASK 0x0FFFFFF0
#define PACKED_ATTRIBUTE_MASK 0xFF000000

/** Transform the sprite center-to-corner offset and pack its coordinates. */
void SpritePackAffinePosition(struct SpriteAffineTransform *transform)
{
    s32 packedX;
    s32 packedY;
    u32 xAndFlags;
    u32 coordinateBits;

    packedX = (transform->centerX << 8)
            - transform->pa * transform->halfWidth
            - transform->pb * transform->halfHeight;
    packedY = (transform->centerY << 8)
            - transform->pc * transform->halfWidth
            - transform->pd * transform->halfHeight;

    transform->packedXLow = packedX;
    xAndFlags = ((u32)packedX & PACKED_X_HIGH_MASK) >> 16;
    xAndFlags |= ((u32)packedY & PACKED_Y_NIBBLE_MASK) << 12;
    transform->packedXHighAndYLow = xAndFlags;

    coordinateBits = transform->packedYHighAndFlags & PACKED_ATTRIBUTE_MASK;
    coordinateBits |= ((u32)packedY & PACKED_Y_REMAINING_MASK) >> 4;
    transform->packedYHighAndFlags = coordinateBits;
}

/** Build the renderer's four 18.14 affine matrix coefficients. */
void SpriteBuildAffineMatrix(struct SpriteAffineTransform *transform)
{
    s32 angle = (s16)transform->angle;
    s32 sine = gSineTable14[angle & SPRITE_ANGLE_MASK];
    s32 cosine = gSineTable14[(angle + SPRITE_QUARTER_TURN)
                            & SPRITE_ANGLE_MASK];
    s32 inverseX = (s16)SpriteMathDivide65536ByS16(transform->scaleX);
    s32 inverseY = (s16)SpriteMathDivide65536ByS16(transform->scaleY);

    transform->pa = (cosine * inverseX) >> SPRITE_VECTOR_FRACTION_BITS;
    transform->pb = (sine * inverseX) >> SPRITE_VECTOR_FRACTION_BITS;
    transform->pc = -((sine * inverseY) >> SPRITE_VECTOR_FRACTION_BITS);
    transform->pd = (cosine * inverseY) >> SPRITE_VECTOR_FRACTION_BITS;
}
