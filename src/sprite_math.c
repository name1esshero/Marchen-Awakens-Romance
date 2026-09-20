/* Fixed-point helpers used by the sprite interpolation and affine renderer. */
#include "gba/types.h"

#include "rom_section.h"

extern s32 __divsi3(s32 dividend, s32 divisor);
s32 SpriteFixedSqrt(s32 value);

/**
 * @brief Multiplies two signed 8.8 fixed-point values.
 * @param left The left operand, in 8.8 fixed-point.
 * @param right The right operand, in 8.8 fixed-point.
 * @return The product, in 8.8 fixed-point, rounded toward zero.
 */
AT("0007D8F8")
s32 SpriteFixed8Multiply(s32 left, s32 right)
{
    s32 product;
    s32 rounded;

    left <<= 16;
    right <<= 16;
    right >>= 16;
    left >>= 16;
    product = left * right;
    rounded = product;
    if (product < 0)
        rounded += 255;

    product = rounded << 8;
    return product >> 16;
}

/**
 * @brief Divides one signed 8.8 fixed-point value by another.
 * @param dividend The numerator, in 8.8 fixed-point.
 * @param divisor The denominator, in 8.8 fixed-point.
 * @return The quotient, in 8.8 fixed-point.
 */
AT("0007D914")
s32 SpriteFixed8Divide(s32 dividend, s32 divisor)
{
    dividend <<= 16;
    divisor <<= 16;
    divisor >>= 16;
    dividend >>= 8;
    return (s16)__divsi3(dividend, divisor);
}

/* The cartridge stores zero alignment bytes after the divide helper. Shares
 * SpriteFixed8Divide's section so the two are emitted contiguously with no
 * inter-function padding, the same way they sat inside the old combined
 * Multiply+Divide+Tail group before Multiply moved to real assembly. */
AT("0007D914")
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
