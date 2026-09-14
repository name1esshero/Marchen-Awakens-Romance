#include "gba/types.h"
#include "math_tables.h"

#include "rom_section.h"
#define SIN_TABLE gSineTable14

#ifdef AGBCC
#define TARGET_REGISTER(name) asm(name)
#define MATCH_RW(value) asm volatile("" : "+r"(value))
#define MATCH_OUT(value) asm volatile("" : "=r"(value))
#define MATCH_RW3(a, b, c) asm volatile("" : "+r"(a), "+r"(b), "+r"(c))
#define MATCH_IN3(a, b, c) asm volatile("" : : "r"(a), "r"(b), "r"(c))
#else
#define TARGET_REGISTER(name)
#define MATCH_RW(value) ((void)0)
#define MATCH_OUT(value) ((void)0)
#define MATCH_RW3(a, b, c) ((void)0)
#define MATCH_IN3(a, b, c) ((void)0)
#endif

extern void *SpriteEngineGetBuffer4Entry(s32 index);
extern s32 SpriteMathDivide65536ByS16(s32 value);

/* OAM affine coefficients occupy the fourth halfword of four consecutive
 * object entries. */
struct AffineOamColumns {
    u8 padding0[6]; s16 pa;
    u8 padding8[6]; s16 pb;
    u8 padding10[6]; s16 pc;
    u8 padding18[6]; s16 pd;
};

AT("0007CE50")
void SpriteAffineWriteNormal(s32 index, s32 angle0, s32 scaleX0, s32 scaleY0)
{
    register s32 angle TARGET_REGISTER("r4") = angle0;
    register s32 scaleX TARGET_REGISTER("r5") = scaleX0;
    register s32 scaleY TARGET_REGISTER("r6") = scaleY0;
    register const s16 *table TARGET_REGISTER("r2");
    register u32 mask TARGET_REGISTER("r1");
    register struct AffineOamColumns *entry TARGET_REGISTER("r8");
    register s32 sine TARGET_REGISTER("r10");
    register s32 sineTemp TARGET_REGISTER("r3");
    register s32 tableHold TARGET_REGISTER("r7");
    register s32 valueHold TARGET_REGISTER("r0");
    register s32 cosine TARGET_REGISTER("r9");
    register s32 invX TARGET_REGISTER("r4");
    register s32 invY TARGET_REGISTER("r0");
    MATCH_RW3(angle, scaleX, scaleY);
    angle = (s16)angle; scaleX = (s16)scaleX; scaleY = (s16)scaleY;
    MATCH_RW3(angle, scaleX, scaleY);
    entry = SpriteEngineGetBuffer4Entry(index);
    table = SIN_TABLE; mask = 0xFFF;
    sineTemp = table[angle & mask];
    sine = sineTemp;
    MATCH_OUT(tableHold);
    angle += 0x400;
    angle &= mask;
    angle <<= 1;
    angle += (s32)table;
    MATCH_RW(angle);
    MATCH_OUT(valueHold);
    cosine = *(const s16 *)angle;
    MATCH_IN3(sineTemp, tableHold, valueHold);
    invX = SpriteMathDivide65536ByS16(scaleX);
    MATCH_RW(invX);
    invX = (s16)invX;
    invY = SpriteMathDivide65536ByS16(scaleY);
    invY = (s16)invY;
    {
        register s32 value TARGET_REGISTER("r1");
        value = (cosine * invX) >> 14;
        MATCH_RW(value);
        {
            register struct AffineOamColumns *base TARGET_REGISTER("r3") = entry;
            register s32 finalValue TARGET_REGISTER("r7");
            register s32 finalShifted TARGET_REGISTER("r0");
            base->pa = value;
            base->pb = (sine * invX) >> 14;
            base->pc = -((sine * invY) >> 14);
            finalValue = cosine * invY;
            MATCH_RW(finalValue);
            finalShifted = finalValue;
            MATCH_RW(finalShifted);
            base->pd = finalShifted >> 14;
        }
    }
}

AT("0007CEE8")
void SpriteAffineWriteMirrored(s32 index, s32 angle0, s32 scaleX0, s32 scaleY0)
{
    register s32 angle TARGET_REGISTER("r4") = angle0;
    register s32 scaleX TARGET_REGISTER("r5") = scaleX0;
    register s32 scaleY TARGET_REGISTER("r6") = scaleY0;
    register const s16 *table TARGET_REGISTER("r2");
    register u32 mask TARGET_REGISTER("r1");
    register struct AffineOamColumns *entry TARGET_REGISTER("r8");
    register s32 sine TARGET_REGISTER("r10");
    register s32 sineTemp TARGET_REGISTER("r3");
    register s32 tableHold TARGET_REGISTER("r7");
    register s32 valueHold TARGET_REGISTER("r0");
    register s32 cosine TARGET_REGISTER("r9");
    register s32 invX TARGET_REGISTER("r4");
    register s32 invY TARGET_REGISTER("r0");
    MATCH_RW3(angle, scaleX, scaleY);
    angle = (s16)angle; scaleX = (s16)scaleX; scaleY = (s16)scaleY;
    MATCH_RW3(angle, scaleX, scaleY);
    entry = SpriteEngineGetBuffer4Entry(index);
    table = SIN_TABLE; mask = 0xFFF;
    sineTemp = table[angle & mask];
    sine = sineTemp;
    MATCH_OUT(tableHold);
    angle += 0x400;
    angle &= mask;
    angle <<= 1;
    angle += (s32)table;
    MATCH_RW(angle);
    MATCH_OUT(valueHold);
    cosine = *(const s16 *)angle;
    MATCH_IN3(sineTemp, tableHold, valueHold);
    invX = SpriteMathDivide65536ByS16(scaleX);
    MATCH_RW(invX);
    invX = (s16)invX;
    invY = SpriteMathDivide65536ByS16(scaleY);
    invY = (s16)invY;
    {
        register s32 value TARGET_REGISTER("r1");
        value = -((cosine * invX) >> 14);
        MATCH_RW(value);
        {
            register struct AffineOamColumns *base TARGET_REGISTER("r3") = entry;
            register s32 finalValue TARGET_REGISTER("r7");
            register s32 finalShifted TARGET_REGISTER("r0");
            base->pa = value;
            base->pb = -((sine * invX) >> 14);
            base->pc = -((sine * invY) >> 14);
            finalValue = cosine * invY;
            MATCH_RW(finalValue);
            finalShifted = finalValue;
            MATCH_RW(finalShifted);
            base->pd = finalShifted >> 14;
        }
    }
}

AT("0007CF84")
void SpriteAffineWriteAlternateAxis(s32 index, s32 angle0, s32 scaleX0, s32 scaleY0)
{
    register s32 angle TARGET_REGISTER("r4") = angle0;
    register s32 scaleX TARGET_REGISTER("r5") = scaleX0;
    register s32 scaleY TARGET_REGISTER("r6") = scaleY0;
    register const s16 *table TARGET_REGISTER("r2");
    register u32 mask TARGET_REGISTER("r1");
    register struct AffineOamColumns *entry TARGET_REGISTER("r8");
    register s32 sine TARGET_REGISTER("r10");
    register s32 sineTemp TARGET_REGISTER("r3");
    register s32 tableHold TARGET_REGISTER("r7");
    register s32 valueHold TARGET_REGISTER("r0");
    register s32 cosine TARGET_REGISTER("r9");
    register s32 invX TARGET_REGISTER("r4");
    register s32 invY TARGET_REGISTER("r0");
    MATCH_RW3(angle, scaleX, scaleY);
    angle = (s16)angle; scaleX = (s16)scaleX; scaleY = (s16)scaleY;
    MATCH_RW3(angle, scaleX, scaleY);
    entry = SpriteEngineGetBuffer4Entry(index);
    table = SIN_TABLE; mask = 0xFFF;
    sineTemp = table[angle & mask];
    sine = sineTemp;
    MATCH_OUT(tableHold);
    angle += 0x400;
    angle &= mask;
    angle <<= 1;
    angle += (s32)table;
    MATCH_RW(angle);
    MATCH_OUT(valueHold);
    cosine = *(const s16 *)angle;
    MATCH_IN3(sineTemp, tableHold, valueHold);
    invX = SpriteMathDivide65536ByS16(scaleX);
    MATCH_RW(invX);
    invX = (s16)invX;
    invY = SpriteMathDivide65536ByS16(scaleY);
    invY = (s16)invY;
    {
        register s32 value TARGET_REGISTER("r1");
        value = (cosine * invX) >> 14;
        MATCH_RW(value);
        {
            register struct AffineOamColumns *base TARGET_REGISTER("r3") = entry;
            register s32 finalValue TARGET_REGISTER("r7");
            register s32 finalShifted TARGET_REGISTER("r0");
            base->pa = value;
            base->pb = (sine * invX) >> 14;
            base->pc = (sine * invY) >> 14;
            finalValue = cosine * invY;
            MATCH_RW(finalValue);
            finalShifted = finalValue;
            MATCH_RW(finalShifted);
            base->pd = -(finalShifted >> 14);
        }
    }
}
