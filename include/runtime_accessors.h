#ifndef RUNTIME_ACCESSORS_H
#define RUNTIME_ACCESSORS_H

#include "gba/types.h"

u32 SioGetPlayerId(void);
u32 RuntimeGetByte4014(void);
void RuntimeAdvanceWord4(u32 unused,u32 *state);
void GameStateSetPointer2C(u32 index,void *value);
void *GameStateGetPointer2C(u32 index);
void GameStateSetField60E(u32 value);
u32 GameStateGetField60E(void);
void GameStateSetField12EC(s32 value);
s32 GameStateGetField12EC(void);
s32 GameStateGetField12EE(void);
void GameStateSetField12EE(s32 value);
u32 IwramGetField0810(void);
void *RuntimeGetBufferE50(void);
void *GameStateGetRecord610(u32 index);

#endif
