#ifndef GAME_TABLES_H
#define GAME_TABLES_H

#include "gba/types.h"

#define BATTLE_PRESET_SIZE 16

/* Sentinel meaning "this friend ARM definition has no ownership bit". */
#define FRIEND_ARM_NO_OWNERSHIP_BIT 444

#define BATTLE_ACTION_HANDLER_COUNT 444
#define ENGINE_STARTUP_HANDLER_COUNT 8
#define TWO_DIGIT_RESOURCE_NAME_COUNT 48
#define GENERATED_EFFECT_NAME_COUNT 13
#define RESOURCE_SLOT_COUNT 4
#define CHARACTER_GROWTH_THRESHOLD_COUNT 100

extern const u16 gBattleRuntimePresetA[BATTLE_PRESET_SIZE];
extern const u16 gBattleRuntimePresetB[BATTLE_PRESET_SIZE];
extern const s16 gFriendArmOwnershipBits[8];
extern const u16 gBattlePartyDefaults[8];
extern void *const gBattleActionHandlers[BATTLE_ACTION_HANDLER_COUNT];
extern const char *const gTwoDigitResourceNames[TWO_DIGIT_RESOURCE_NAME_COUNT];
extern const char *const gGeneratedEffectNames[GENERATED_EFFECT_NAME_COUNT];
extern const u16 gResourceSlotIndices[RESOURCE_SLOT_COUNT];
extern const u16 gResourceSlotMasks[RESOURCE_SLOT_COUNT];
extern const u32 gCharacterGrowthThresholds[CHARACTER_GROWTH_THRESHOLD_COUNT];
extern void *const gEngineStartupHandlers[ENGINE_STARTUP_HANDLER_COUNT];

#endif
