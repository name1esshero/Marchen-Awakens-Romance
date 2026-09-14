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
    register s32 first TARGET_REGISTER("r9");
    register s32 second TARGET_REGISTER("r10");
    register const s16 *tableFirst TARGET_REGISTER("r5");
    register const s16 *table TARGET_REGISTER("r8");
    register s32 cosineIndex TARGET_REGISTER("r4");
    register s32 mask TARGET_REGISTER("r6");
    register s32 accum TARGET_REGISTER("r5");
    register s32 value TARGET_REGISTER("r3");
    register s32 temp TARGET_REGISTER("r4");

    angle = (angle << 16) >> 16;
    asm("" : "+r"(angle));
    first = in->y;
    temp = in->z;
    asm("" : "+r"(temp));
    second = temp;
    tableFirst = SPRITE_SINE_TABLE;
    asm("" : "+r"(tableFirst));
    table = tableFirst;
    mask = 0x400;
    asm("" : "+r"(mask));
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
        register s32 product TARGET_REGISTER("r6") = second;
        product *= value;
        value = product;
    }
    accum -= value;
    accum >>= 14;
    out->y = accum;
    {
        register s32 sine TARGET_REGISTER("r2") = *(const s16 *)angle;
        register s32 product TARGET_REGISTER("r5") = first;
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
    register s32 first TARGET_REGISTER("r9");
    register s32 second TARGET_REGISTER("r10");
    register const s16 *tableFirst TARGET_REGISTER("r5");
    register const s16 *table TARGET_REGISTER("r8");
    register s32 cosineIndex TARGET_REGISTER("r4");
    register s32 mask TARGET_REGISTER("r6");
    register s32 accum TARGET_REGISTER("r5");
    register s32 value TARGET_REGISTER("r3");
    register s32 temp TARGET_REGISTER("r4");

    angle = (angle << 16) >> 16;
    asm("" : "+r"(angle));
    first = in->x;
    temp = in->z;
    asm("" : "+r"(temp));
    second = temp;
    tableFirst = SPRITE_SINE_TABLE;
    asm("" : "+r"(tableFirst));
    table = tableFirst;
    mask = 0x400;
    asm("" : "+r"(mask));
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
        register s32 product TARGET_REGISTER("r6") = second;
        product *= value;
        value = product;
    }
    accum += value;
    accum >>= 14;
    out->x = accum;
    {
        register s32 sine TARGET_REGISTER("r2") = *(const s16 *)angle;
        value = first;
        value *= sine;
        sine = *(s16 *)cosineIndex;
        {
            register s32 product TARGET_REGISTER("r6") = second;
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
    register s32 first TARGET_REGISTER("r9");
    register s32 second TARGET_REGISTER("r10");
    register const s16 *tableFirst TARGET_REGISTER("r5");
    register const s16 *table TARGET_REGISTER("r8");
    register s32 cosineIndex TARGET_REGISTER("r4");
    register s32 mask TARGET_REGISTER("r6");
    register s32 accum TARGET_REGISTER("r5");
    register s32 value TARGET_REGISTER("r3");
    register s32 temp TARGET_REGISTER("r4");

    angle = (angle << 16) >> 16;
    asm("" : "+r"(angle));
    first = in->x;
    temp = in->y;
    asm("" : "+r"(temp));
    second = temp;
    tableFirst = SPRITE_SINE_TABLE;
    asm("" : "+r"(tableFirst));
    table = tableFirst;
    mask = 0x400;
    asm("" : "+r"(mask));
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
        register s32 product TARGET_REGISTER("r6") = second;
        product *= value;
        value = product;
    }
    accum -= value;
    accum >>= 14;
    out->x = accum;
    {
        register s32 sine TARGET_REGISTER("r2") = *(const s16 *)angle;
        register s32 product TARGET_REGISTER("r5") = first;
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

/** Apply the renderer's scale and viewport origin to an X/Y point.
 *
 * The register hints below are load-bearing, not decoration: removing
 * them (tested) changes agbcc's instruction selection and the function
 * no longer matches the ROM byte-for-byte. */
AT("0007D1B4")
void SpriteProjectPoint(struct SpriteVector3 *point)
{
    register struct SpriteVector3 *out TARGET_REGISTER("r5") = point;
    s32 oldX = out->x;
    register s32 oldY TARGET_REGISTER("r9") = out->y;
    register s32 scale TARGET_REGISTER("r8") = out->z;
    u8 *state = *(u8 **)0x03006118;
    register s32 origin TARGET_REGISTER("r4") = *(s16 *)(state + 328);
    register s32 numerator TARGET_REGISTER("r0") = scale * oldX;
    register s32 *divisor TARGET_REGISTER("r6");

    numerator <<= 12;
    divisor = (s32 *)(state + 324);
    origin += __divsi3(numerator, *divisor) >> 12;
    out->x = origin;
    origin = *(s16 *)(state + 330);
    {
        register s32 secondScale TARGET_REGISTER("r2") = scale;
        numerator = oldY * secondScale;
    }
    numerator <<= 12;
    origin += __divsi3(numerator, *divisor) >> 12;
    out->y = origin;
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
    register s32 product TARGET_REGISTER("r1");
    register s32 halfWidth TARGET_REGISTER("r4");
    register s32 halfHeight TARGET_REGISTER("r3");
    register s32 packedY TARGET_REGISTER("r5");
    register u32 bits TARGET_REGISTER("r6");
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
    register struct SpriteAffineTransform *state TARGET_REGISTER("r5") = transform;
    register const s16 *table TARGET_REGISTER("r3") = SPRITE_SINE_TABLE;
    register u32 mask TARGET_REGISTER("r2");
    register u32 rawAngle TARGET_REGISTER("r1");
    register s32 sine TARGET_REGISTER("r8");
    register s32 cosine TARGET_REGISTER("r6");
    register s32 inverseX TARGET_REGISTER("r4");
    register s32 inverseY TARGET_REGISTER("r0");
    register s32 index TARGET_REGISTER("r0");

    asm("" : "+r"(table));
    rawAngle = state->angle;
    asm("" : "+r"(rawAngle));
    mask = 0xFFF;
    index = mask;
    index &= rawAngle;
    index <<= 1;
    index += (s32)table;
    sine = *(s16 *)index;
    index = *(s16 *)&state->angle;
    inverseX = 0x400;
    index += inverseX;
    index &= mask;
    index <<= 1;
    index += (s32)table;
    cosine = *(s16 *)index;
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
