/* SpriteFixed8Multiply, 0x0807D8F8. Real matching instructions live at
 * asm/code/code_0780C0.s; this is the readable form kept for reference.
 *
 * Signed 8.8 fixed-point multiply. Rounds a negative product toward zero
 * before dropping the fractional byte. The ROM keeps the product in r0 and
 * a *copy* of it in r1 across the sign check:
 *
 *     muls r0, r1
 *     adds r1, r0, #0     @ the copy
 *     cmp  r0, #0
 *     bge  .L
 *     adds r1, #255
 *     .L:
 *     lsls r0, r1, #8
 *
 * `product` is not read again after the compare, so agbcc's plain-C
 * allocator always coalesces `product` and `rounded` into the same
 * register once `product` is dead there. Every shape tried reproduces the
 * coalesced form instead of the ROM's genuine copy: separate locals in
 * either declaration order, if/else assignment, a ternary, reusing the
 * `left` parameter, and splitting the final shift into two statements. See
 * docs/COMPILER_HINT_CLEANUP.md for the full record of what was tried. */
#include "gba/types.h"

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
    return (rounded << 8) >> 16;
}
