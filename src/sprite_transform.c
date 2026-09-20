/* Fixed-point transforms used by the sprite renderer. */
#include "sprite_engine.h"
#include "math_tables.h"

#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#else
#define TARGET_REGISTER(name) asm(name)
#endif

#define SPRITE_SINE_TABLE gSineTable14

extern s32 __divsi3(s32 dividend, s32 divisor);

/** Rotate a 18.14 fixed-point vector around the X axis. Angles use the
 * engine's 4096-step turn, and the sine table's quarter turn supplies cosine.
 *
 * The register hints and asm("" : "+r"(...)) fences below are load-bearing,
 * not decoration: removing them (tested) changes agbcc's instruction
 * selection and the function no longer matches the ROM byte-for-byte. */
AT("0007D044")
void SpriteVectorRotateX(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle)
{
    s32 first;
    s32 second;
    const s16 *tableFirst;
    const s16 *table;
    s32 cosineIndex;
    s32 mask;
    s32 accum;
    register s32 value TARGET_REGISTER("r3");
    s32 temp;

    angle = (angle << 16) >> 16;
    first = in->y;
    temp = in->z;
    second = temp;
    tableFirst = SPRITE_SINE_TABLE;
    table = tableFirst;
    mask = 0x400;
    cosineIndex = angle + mask;
    mask = 0xFFF;
    cosineIndex &= mask;
    cosineIndex <<= 1;
    cosineIndex = (s32)((const u8 *)table + cosineIndex);
    value = *(s16 *)cosineIndex;
    accum = first * value;
    angle &= mask;
    angle <<= 1;
    angle = (s32)((const u8 *)table + angle);
    value = *(const s16 *)angle;
    {
        s32 product = second;
        product *= value;
        value = product;
    }
    accum -= value;
    accum >>= 14;
    out->y = accum;
    {
        s32 sine = *(const s16 *)angle;
        s32 product = first;
        product *= sine;
        sine = product;
        value = *(s16 *)cosineIndex;
        temp = second;
        temp *= value;
        value = temp;
        sine += value;
        sine >>= 14;
        out->z = sine;
    }
    out->x = in->x;
}

/** Rotate a vector around the Y axis.
 *
 * The register hints and asm("" : "+r"(...)) fences below are
 * load-bearing, not decoration: removing them (tested) changes
 * agbcc's instruction selection and the function no longer
 * matches the ROM byte-for-byte. */
AT("0007D0C0")
void SpriteVectorRotateY(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle)
{
    s32 first;
    s32 second;
    const s16 *tableFirst;
    const s16 *table;
    s32 cosineIndex;
    s32 mask;
    s32 accum;
    register s32 value TARGET_REGISTER("r3");
    s32 temp;

    angle = (angle << 16) >> 16;
    first = in->x;
    temp = in->z;
    second = temp;
    tableFirst = SPRITE_SINE_TABLE;
    table = tableFirst;
    mask = 0x400;
    cosineIndex = angle + mask;
    mask = 0xFFF;
    cosineIndex &= mask;
    cosineIndex <<= 1;
    cosineIndex = (s32)((const u8 *)table + cosineIndex);
    value = *(s16 *)cosineIndex;
    accum = first * value;
    angle &= mask;
    angle <<= 1;
    angle = (s32)((const u8 *)table + angle);
    value = *(const s16 *)angle;
    {
        s32 product = second;
        product *= value;
        value = product;
    }
    accum += value;
    accum >>= 14;
    out->x = accum;
    {
        s32 sine = *(const s16 *)angle;
        value = first;
        value *= sine;
        sine = *(s16 *)cosineIndex;
        {
            s32 product = second;
            product *= sine;
            sine = product;
        }
        sine -= value;
        sine >>= 14;
        out->z = sine;
    }
    out->y = in->y;
}

/** Rotate a vector around the Z axis.
 *
 * The register hints and asm("" : "+r"(...)) fences below are
 * load-bearing, not decoration: removing them (tested) changes
 * agbcc's instruction selection and the function no longer
 * matches the ROM byte-for-byte. */
AT("0007D138")
void SpriteVectorRotateZ(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle)
{
    s32 first;
    s32 second;
    const s16 *tableFirst;
    const s16 *table;
    s32 cosineIndex;
    s32 mask;
    s32 accum;
    register s32 value TARGET_REGISTER("r3");
    s32 temp;

    angle = (angle << 16) >> 16;
    first = in->x;
    temp = in->y;
    second = temp;
    tableFirst = SPRITE_SINE_TABLE;
    table = tableFirst;
    mask = 0x400;
    cosineIndex = angle + mask;
    mask = 0xFFF;
    cosineIndex &= mask;
    cosineIndex <<= 1;
    cosineIndex = (s32)((const u8 *)table + cosineIndex);
    value = *(s16 *)cosineIndex;
    accum = first * value;
    angle &= mask;
    angle <<= 1;
    angle = (s32)((const u8 *)table + angle);
    value = *(const s16 *)angle;
    {
        s32 product = second;
        product *= value;
        value = product;
    }
    accum -= value;
    accum >>= 14;
    out->x = accum;
    {
        s32 sine = *(const s16 *)angle;
        s32 product = first;
        product *= sine;
        sine = product;
        value = *(s16 *)cosineIndex;
        temp = second;
        temp *= value;
        value = temp;
        sine += value;
        sine >>= 14;
        out->y = sine;
    }
    out->z = in->z;
}

/** Transform the sprite's center-to-corner offset and pack the signed 24-bit
 * screen coordinates into the renderer's OAM-shaped work record. Existing
 * affine and attribute flag bits in the record are preserved.
 *
 * The register hints and the asm("" : "+r"(product)) fence below are
 * load-bearing, not decoration: removing them (tested) changes agbcc's
 * instruction selection and the function no longer matches the ROM
 * byte-for-byte. */
AT("0007DA38")
void SpritePackAffinePosition(struct SpriteAffineTransform *state)
{
    register s32 packedX TARGET_REGISTER("r2");
    s32 product;
    register s32 halfWidth TARGET_REGISTER("r4");
    register s32 halfHeight TARGET_REGISTER("r3");
    register s32 packedY TARGET_REGISTER("r5");
    u32 bits;
    register u32 highMask TARGET_REGISTER("r8");

    packedX = state->centerX;
    packedX <<= 8;
    product = state->pa;
    halfWidth = state->halfWidth;
    product *= halfWidth;
    packedX -= product;
    product = state->pb;
    halfHeight = state->halfHeight;
    product *= halfHeight;
    packedX -= product;
    packedY = state->centerY;
    packedY <<= 8;
    product = state->pc;
    product *= halfWidth;
    packedY -= product;
    product = state->pd;
    product *= halfHeight;
    packedY -= product;

    state->packedYLow = packedX;
    bits = 0x0FFF0000;
    highMask = bits;
    packedX &= bits;
    packedX >>= 16;
    {
        register u32 flags TARGET_REGISTER("r4") = state->packedXAndFlags;
        register u32 preserve TARGET_REGISTER("r3") = 0xFFFFF000;

        product = preserve;
        product &= flags;
        product |= packedX;
        state->packedXAndFlags = product;
        bits = 0xFFFF;
        bits &= packedY;
        product = 0xF;
        packedX = bits;
        packedX &= product;
        packedX <<= 4;
        flags = ((u8 *)state)[3];
        product &= flags;
        product |= packedX;
        ((u8 *)state)[3] = product;
        bits >>= 4;
        product = *(u16 *)((u8 *)state + 4);
        preserve &= product;
        preserve |= bits;
        *(u16 *)((u8 *)state + 4) = preserve;
    }
    product = highMask;
    asm("" : "+r"(product));
    packedY &= product;
    packedY = (u32)packedY >> 4;
    product = state->packedCoordinateBits;
    packedX = 0xFF000FFF;
    product &= packedX;
    product |= packedY;
    state->packedCoordinateBits = product;
}

/** Build the four OAM affine coefficients for an angle and independent X/Y
 * scales. The engine stores sine/cosine in 18.14 fixed point.
 *
 * The register hints and asm("" : "+r"(...)) fences below are load-bearing,
 * not decoration: removing them (tested) changes agbcc's instruction
 * selection and the function no longer matches the ROM byte-for-byte. */
AT("0007DAD0")
void SpriteBuildAffineMatrix(struct SpriteAffineTransform *transform)
{
    struct SpriteAffineTransform *state = transform;
    const s16 *table = SPRITE_SINE_TABLE;
    u32 mask;
    register u32 rawAngle TARGET_REGISTER("r1");
    s32 sine;
    s32 cosine;
    register s32 inverseX TARGET_REGISTER("r4");
    register s32 inverseY TARGET_REGISTER("r0");
    register s32 index TARGET_REGISTER("r0");

    rawAngle = state->angle;
    mask = 0xFFF;
    index = mask;
    index &= rawAngle;
    index <<= 1;
    sine = *(const s16 *)((const u8 *)table + index);
    index = *(s16 *)&state->angle;
    inverseX = 0x400;
    index += inverseX;
    index &= mask;
    index <<= 1;
    cosine = *(const s16 *)((const u8 *)table + index);
    inverseX = SpriteMathDivide65536ByS16(state->scaleX);
    asm("" : "+r"(inverseX));
    inverseX = (inverseX << 16) >> 16;
    inverseY = SpriteMathDivide65536ByS16(state->scaleY);
    inverseY = (inverseY << 16) >> 16;
    state->pa = (cosine * inverseX) >> 14;
    state->pb = (sine * inverseX) >> 14;
    state->pc = -((sine * inverseY) >> 14);
    state->pd = (cosine * inverseY) >> 14;
}
