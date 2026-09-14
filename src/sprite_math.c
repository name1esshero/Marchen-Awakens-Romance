/* Fixed-point helpers used by the sprite interpolation and affine renderer. */
#include "gba/types.h"

#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#else
#define TARGET_REGISTER(name) asm(name)
#endif

extern s32 __divsi3(s32 dividend, s32 divisor);
s32 SpriteFixedSqrt(s32 value);

/** Signed 8.8 helpers used by the renderer's affine calculations. Multiplication
 * rounds negative products toward zero before dropping the fractional byte. */
AT("0007D8F8")
s32 SpriteFixed8Multiply(s32 left, s32 right)
{
    register s32 product TARGET_REGISTER("r0");
    register s32 rounded TARGET_REGISTER("r1");

    left <<= 16;
    asm("" : "+r"(left));
    right <<= 16;
    right >>= 16;
    asm("" : "+r"(right));
    left >>= 16;
    product = left * right;
    rounded = product;
    if (product < 0)
        rounded += 255;
    return (rounded << 8) >> 16;
}

AT("0007D8F8")
s32 SpriteFixed8Divide(s32 dividend, s32 divisor)
{
    dividend <<= 16;
    asm("" : "+r"(dividend));
    divisor <<= 16;
    divisor >>= 16;
    asm("" : "+r"(divisor));
    dividend >>= 8;
    return (s16)__divsi3(dividend, divisor);
}

/* The cartridge stores zero alignment bytes after the divide helper. */
AT("0007D8F8")
const u8 SpriteFixed8Tail[2] = {0, 0};

/** Length of a 20.12 fixed-point vector.  Large components are reduced before
 * squaring to avoid overflow; small components are expanded to retain useful
 * precision and scaled back after taking the square root. */
AT("0007D944")
s32 SpriteVectorLengthFixed(s32 x, s32 y)
{
    s32 absX = x;
    s32 absY = y;
    s32 scale;

    if (absX < 0)
        absX = -absX;
    if (absY < 0)
        absY = -absY;

    if (absX > 0x1000 || absY > 0x1000) {
        if (absX > absY)
            scale = (absX << 4) >> 16;
        else
            scale = (absY << 4) >> 16;
        absX = __divsi3(absX, scale);
        absY = __divsi3(absY, scale);
        return SpriteFixedSqrt(((absX * absX) >> 12)
                             + ((absY * absY) >> 12)) * scale;
    }

    if (absX > absY) {
        if (absX != 0)
            scale = __divsi3(0x01000000, absX);
        else
            scale = 0;
    } else {
        if (absY != 0)
            scale = __divsi3(0x01000000, absY);
        else
            scale = 0;
    }

    absX = (absX * scale) >> 12;
    absY = (absY * scale) >> 12;
    if (scale == 0)
        return 0;
    else
        return __divsi3(
            SpriteFixedSqrt(((absX * absX) >> 12)
                          + ((absY * absY) >> 12)) << 12,
            scale);
}

/** Square root for signed 20.12 fixed-point values.  Newton iteration starts
 * at max(value, 1.0) and stops as soon as the estimate no longer decreases.
 * Negative inputs use the engine's -1.0 error sentinel. */
AT("0007D9F0")
s32 SpriteFixedSqrt(s32 value)
{
    register s32 previous TARGET_REGISTER("r4");
    register s32 input TARGET_REGISTER("r5") = value;
    register s32 estimate TARGET_REGISTER("r0");

    if (input > 0) {
        register s32 one TARGET_REGISTER("r1") = 0x1000;

        if (input >= one)
            estimate = input;
        else
            estimate = one;
        do {
            previous = estimate;
            if (previous != 0) {
                s32 rounded;

                estimate = __divsi3(input << 12, previous);
                estimate += previous;
                rounded = estimate + ((u32)estimate >> 31);
                estimate = rounded >> 1;
            } else {
                estimate = 0;
            }
        } while (estimate < previous);
        return previous;
    }
    if (input != 0)
        return -0x1000;
    return 0;
}
