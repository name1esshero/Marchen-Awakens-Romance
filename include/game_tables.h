#ifndef GAME_TABLES_H
#define GAME_TABLES_H

#include "gba/types.h"

extern const u16 gBattleRuntimePresetA[16];
extern const u16 gBattleRuntimePresetB[16];
extern const s16 gFriendArmOwnershipBits[8];
extern const u16 gBattlePartyDefaults[8];
extern void *const gBattleActionHandlers[444];
extern const char *const gTwoDigitResourceNames[48];
extern const char *const gGeneratedEffectNames[13];
extern const u16 gResourceSlotIndices[4];
extern const u16 gResourceSlotMasks[4];
extern const u32 gCharacterGrowthThresholds[100];
extern void *const gEngineStartupHandlers[8];

#endif
