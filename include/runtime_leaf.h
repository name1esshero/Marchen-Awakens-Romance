#ifndef RUNTIME_LEAF_H
#define RUNTIME_LEAF_H

#include "gba/types.h"

void Lz77UnCompVramSwapped(void *destination, const void *source);
void SpriteAuxiliaryReset(void *state);
s32 MapGeneratorStep(void);
s32 RuntimeReadSignedByte(void);
void SoundSongStartU16(u32 song);
void RuntimeResetSelection(void);
void SoundStopPlayers4And5(void);
void SpriteSetViewportOrigin(u16 x,u16 y);
void HeapFreeDefault(void *allocation);
s32 Crc32Difference(const void *data,u32 size,s32 expected);

#endif
