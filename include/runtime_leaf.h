#ifndef RUNTIME_LEAF_H
#define RUNTIME_LEAF_H

#include "gba/types.h"

void Lz77UnCompVramSwapped(void *destination, const void *source);
void SpriteAuxiliaryReset(void *state);
u32 CalculateSaveCrc32(const void *data, u32 size);
u32 CalculateCrc32(const void *data, u32 size);
s32 RuntimeReadSignedByte(void);
u32 RuntimeTestFlagU8(u32 bit);
void SoundSongStartU16(u32 song);
void RuntimeResetSelection(void);
void SoundStopPlayers4And5(void);
void SpriteSetViewportOrigin(u16 x,u16 y);
void HeapFreeDefault(void *allocation);
s32 Crc32Difference(const void *data,u32 size,s32 expected);

#endif
