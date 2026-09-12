#ifndef BITSET_H
#define BITSET_H
#include "gba/types.h"
/* Bit 0 is the low bit of byte 0; callers own storage and bounds checks. */
void BitSet(u8 *bits, u32 index, s32 enabled);
u32 BitTest(const u8 *bits, u32 index);
#endif
