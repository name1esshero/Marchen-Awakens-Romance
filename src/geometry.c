/* Small integer geometry helpers shared by field, battle, and sprite code. */
#include "geometry.h"

#include "gba/bios.h"
#include "rom_section.h"

/** Return the integer Euclidean distance between two signed 16-bit points. */
AT("000020EC")
u16 CalculatePointDistance(s16 firstX, s16 firstY, s16 secondX, s16 secondY)
{
    s32 x1 = firstX;
    s32 y1 = firstY;
    s32 x2 = secondX;
    s32 y2 = secondY;
    s32 deltaX;
    s32 deltaY;

    deltaX = x2 - x1;
    deltaY = y2 - y1;

    return Sqrt(deltaX * deltaX + deltaY * deltaY);
}

/** Return the BIOS polar angle from the first point to the second. */
AT("00002118")
s16 CalculatePointAngle(s16 firstX, s16 firstY, s16 secondX, s16 secondY)
{
    s32 x1 = firstX;
    s32 y1 = firstY;
    s32 x2 = secondX;
    s32 y2 = secondY;
    return ArcTan2((s16)(x2 - x1), (s16)(y2 - y1));
}
AT("00002118") const u8 CalculatePointAngleTail[2] = {0, 0};
