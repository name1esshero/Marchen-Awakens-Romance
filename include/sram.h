#ifndef SRAM_H
#define SRAM_H

#include "gba/types.h"

typedef void (*SramTransferFunc)(const u8 *source, u8 *destination, u32 size);
typedef u8 *(*SramVerifyFunc)(const u8 *source, const u8 *destination, u32 size);

extern SramTransferFunc ReadSramFast;
extern SramVerifyFunc VerifySramFast;

void ReadSram(const u8 *source, u8 *destination, u32 size);
void WriteSram(const u8 *source, u8 *destination, u32 size);
u8 *VerifySram(const u8 *source, const u8 *destination, u32 size);
u8 *WriteSramFast(const u8 *source, u8 *destination, u32 size);

#endif
