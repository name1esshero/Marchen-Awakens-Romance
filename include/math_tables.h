#ifndef MATH_TABLES_H
#define MATH_TABLES_H

#include "gba/types.h"

/* Dimensions are stored as interleaved X/Y pairs indexed by OBJ shape/size. */
extern const u8 gOamAttributeMasks[16];
extern const u8 gOamHalfDimensions[32];
extern const u8 gOamDimensions[32];
extern const u32 gTileRemainderMasks[9];

/* One turn is 256 entries for 8.8 values and 4096 entries for 2.14 values. */
extern const s16 gSineTable8_8[512];
extern const s16 gSineTable14[4096];

#endif
