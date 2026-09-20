/*
 * Clean reference implementation of the renderer's 20.12 fixed-point square
 * root.  It uses Newton iteration and returns -1.0 for negative inputs.
 *
 * This source is intentionally not linked into the matching ROM.  Tested
 * natural declaration, assignment, branch, and expression shapes—including
 * the old agbcc frontend—assign the estimate and constant to the opposite
 * registers from the original.  The exact routine remains in
 * asm/code/code_0780C0.s until a natural matching C shape is found.
 */
#include "gba/types.h"

extern s32 __divsi3(s32 dividend, s32 divisor);

s32 SpriteFixedSqrt(s32 value)
{
    s32 previous;
    s32 input = value;
    s32 estimate;

    if (input > 0) {
        s32 one = 0x1000;

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
