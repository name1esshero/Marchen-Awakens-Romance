#ifndef RUNTIME_MISC_H
#define RUNTIME_MISC_H

#include "gba/types.h"
#include "battle_character.h"

struct InputRepeatState {
    u16 previous;
    u16 mask;
    u8 counter;
    u8 unused05;
    u8 active;
    u8 initialDelay;
    u8 repeatDelay;
};

struct MapObjectMotion {
    u8 unused00[4];
    u16 field04;
    u8 unused06[12];
    u16 field12;
    u16 field14;
    u16 field16;
};

void *GameStateGetBuffer38C0(void);
void VmAddToField220(u32 value);
u32 VmGetField10(void);
u32 VmGetField12(void);
void *RuntimeGetPointer6120Field800(void);
const void *RuntimeGetTable1AC8C0(u32 unused);
void InputRepeatInit(struct InputRepeatState *state,u32 mask);
void InputRepeatRearm(struct InputRepeatState *state);
void MapObjectResetMotion(struct MapObjectMotion *motion);
u8 *GameStateGetRecord1190(u32 index);
u32 RuntimeReturnZero(void);
const struct BattleCharacterDefinition *RuntimeGetBattleCharacterDefinition(u32 index);
u8 *RuntimeGetBlock6120(u32 slot,u32 group);
void GameStateCopyString12F4(char *destination);
void *GameStateGetRecord0B90(u32 index);
s32 GameStateTestFlagsAC(u32 bit);
s32 GameStateTestFlags12C(u32 bit);
void GameStateSetString12F4(const char *source);
void GameStateClearBlock413C(void);
void GameStateCopyMapBuffer(void);
void GameStateClearRecord426A(void);
void GameStateSaveActorRecord0(void);
void GameStateLoadActorRecord0(void);
void GameStateClearRecord1190IfZero(u32 value);
void GameStateClearRecord1090IfZero(u32 value);
void SpriteRuntimeSetAllFlags800(u32 enabled);

#endif
