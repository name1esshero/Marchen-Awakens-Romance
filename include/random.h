#ifndef RANDOM_H
#define RANDOM_H
#include "gba/types.h"
#ifndef gRandomSeed
#define gRandomSeed (*(u32 *)0x0300610C)
#endif
void RandomInit(u32 seed);
void RandomSeed(u32 seed);
u32 RandomGetSeed(void);
u32 Random(void);
u32 RuntimeRandom(void);
#endif
