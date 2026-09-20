#ifndef BATTLE_CHARACTER_H
#define BATTLE_CHARACTER_H

#include "gba/types.h"
#include "hit_region.h"

#define BATTLE_CHARACTER_DEFINITION_COUNT 86

enum BattleCharacterIdentityField
{
    BATTLE_CHARACTER_IDENTITY_KIND,
    BATTLE_CHARACTER_IDENTITY_RUNTIME_OFFSET_FIRST,
    BATTLE_CHARACTER_IDENTITY_RUNTIME_OFFSET_SECOND,
    BATTLE_CHARACTER_IDENTITY_QUEUE_TYPE,
    BATTLE_CHARACTER_IDENTITY_QUEUE_VALUE,
    BATTLE_CHARACTER_IDENTITY_QUEUE_DURATION,
    BATTLE_CHARACTER_IDENTITY_VARIANT
};

/* Exact 120-byte ROM record used to initialize a battle character. */
struct BattleCharacterDefinition
{
    s16 baseStats[10];       /* 00: HP and combat parameters */
    u16 identity[7];         /* 14: identity and runtime-offset settings */
    s16 armIds[10];          /* 22: equipped ARM/resource identifiers */
    s16 reserved36;          /* 36 */
    struct HitBounds bounds[6]; /* 38: body/attack interaction boxes */
    s16 renderMetadata[8];   /* 68: sprite and movement configuration */
};

extern const struct BattleCharacterDefinition
    gBattleCharacterDefinitions[BATTLE_CHARACTER_DEFINITION_COUNT];

#endif
