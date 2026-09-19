#ifndef RUNTIME_ACCESSORS_H
#define RUNTIME_ACCESSORS_H

#include "gba/types.h"

u32 SioGetPlayerId(void);
u32 RuntimeGetByte4014(void);
u32 RuntimeTestFlag(u32 bit);
void RuntimeAdvanceWord4(void);
void GameStateSetPointer2C(u32 index,void *value);
void *GameStateGetPointer2C(u32 index);
void GameStateSetField60E(u32 value);
u32 GameStateGetField60E(void);
void GameStateSetField12EC(s32 value);
s32 GameStateGetField12EC(void);
s32 GameStateGetField12EE(void);
void GameStateSetField12EE(s32 value);
u32 IwramGetField0810(void);
void IwramSetFlags0810(s32 mode, s32 enabled);
void *RuntimeGetBufferE50(void);
void *GameStateGetRecord610(u32 index);
void IwramSetField3FD5(u32 value);
s32 IwramGetField3FD5(void);
s32 SoundGetIrqMode(void);
void InitBufferTable2050(u8 *base);
u32 IwramGetPointer2860(u32 index);
void IwramSetPointer2860(u32 index,u32 value);
void IwramEnableField2870(void);
void IwramClearField2870(void);
u32 IwramGetField2871(void);
u32 IwramGetField2870(void);
void IwramSetField2870(u32 value);
void *GameStateGetBuffer3F38(void);
s32 GameStateGetEntry2768(s32 index);
s32 GameStateGetEntry2AE0(s32 index);
s32 GameStateGetEntry31D0(s32 index);
s32 GameStateGetEncounterValue(void);
s32 GameStateGetEncounterMode(void);
void GameStateSetFlag2730(s32 index);
s32 GameStateTestFlag2730(s32 index);
s32 GameStateTestFlag26F8(s32 index);
s32 GameStateGetCurrentEntry3894(void);
void GameStateClearEntry31D0(s32 index);

#endif
