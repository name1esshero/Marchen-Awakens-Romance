#ifndef BYTE_UTILS_H
#define BYTE_UTILS_H
#include "gba/types.h"
/* Byte counts, not decoded character counts. No implicit bounds checks. */
u32 CoreStringLength(const u8 *text);
u32 ByteStringLength(const u8 *text);
u32 BufferXor(const u8 *data, u32 size);
u32 BufferXorSeeded(const u8 *data, u32 size, u8 seed);
#endif
