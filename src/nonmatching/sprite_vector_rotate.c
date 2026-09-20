/* Readable reconstructions of the renderer's fixed-point axis rotations.
 * Clean agbcc output keeps the second component in r6 and table values in r7,
 * producing 108/112/108-byte sections. The ROM keeps the second component in
 * sl and reloads table values through r3, producing 124/120/124-byte sections.
 * Compiler versions, typed indexing, a fourth-parameter hypothesis, explicit
 * value flow, and 2,000 declaration orders did not recover that allocation. */
#include "sprite_engine.h"

#include "math_tables.h"

#define SPRITE_ANGLE_MASK 0x0FFF
#define SPRITE_QUARTER_TURN 0x0400
#define SPRITE_VECTOR_FRACTION_BITS 14

static s32 SpriteSine(s32 angle)
{
    return gSineTable14[angle & SPRITE_ANGLE_MASK];
}

static s32 SpriteCosine(s32 angle)
{
    return SpriteSine(angle + SPRITE_QUARTER_TURN);
}

/** Rotate an 18.14 fixed-point vector around the X axis. */
void SpriteVectorRotateX(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle)
{
    s32 y = in->y;
    s32 z = in->z;
    s32 sine = SpriteSine((s16)angle);
    s32 cosine = SpriteCosine((s16)angle);

    out->y = (y * cosine - z * sine) >> SPRITE_VECTOR_FRACTION_BITS;
    out->z = (y * sine + z * cosine) >> SPRITE_VECTOR_FRACTION_BITS;
    out->x = in->x;
}

/** Rotate an 18.14 fixed-point vector around the Y axis. */
void SpriteVectorRotateY(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle)
{
    s32 x = in->x;
    s32 z = in->z;
    s32 sine = SpriteSine((s16)angle);
    s32 cosine = SpriteCosine((s16)angle);

    out->x = (x * cosine + z * sine) >> SPRITE_VECTOR_FRACTION_BITS;
    out->z = (z * cosine - x * sine) >> SPRITE_VECTOR_FRACTION_BITS;
    out->y = in->y;
}

/** Rotate an 18.14 fixed-point vector around the Z axis. */
void SpriteVectorRotateZ(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle)
{
    s32 x = in->x;
    s32 y = in->y;
    s32 sine = SpriteSine((s16)angle);
    s32 cosine = SpriteCosine((s16)angle);

    out->x = (x * cosine - y * sine) >> SPRITE_VECTOR_FRACTION_BITS;
    out->y = (x * sine + y * cosine) >> SPRITE_VECTOR_FRACTION_BITS;
    out->z = in->z;
}
