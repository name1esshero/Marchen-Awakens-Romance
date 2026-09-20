#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "gba/types.h"

u16 CalculatePointDistance(s16 firstX, s16 firstY, s16 secondX, s16 secondY);
s16 CalculatePointAngle(s16 firstX, s16 firstY, s16 secondX, s16 secondY);

#endif /* GEOMETRY_H */
