#ifndef ITEM_H
#define ITEM_H

#include "gba/types.h"
#include "constants/arm.h"

enum ArmType {
    ARM_TYPE_NATURE,
    ARM_TYPE_GUARDIAN,
    ARM_TYPE_DARKNESS,
    ARM_TYPE_HOLY,
    ARM_TYPE_DIMENSION,
    ARM_TYPE_WEAPON,
};

enum ArmElement {
    ARM_ELEMENT_LIGHTNING,
    ARM_ELEMENT_EARTH,
    ARM_ELEMENT_WIND,
    ARM_ELEMENT_WOOD,
    ARM_ELEMENT_FIRE,
    ARM_ELEMENT_WATER,
    ARM_ELEMENT_NEUTRAL,
};

/* 128-byte ÄRM records at 081B096C.  The old disassembly called these items,
 * but they are the complete battle/deck ÄRM catalog.  Names and descriptions
 * use the game charmap.  Unverified gameplay fields retain their byte offsets
 * instead of receiving speculative names. */
struct ArmDefinition {
    u16 iconId;
    u16 id;
    u16 field04;
    char password[8];
    u16 reserved0E;
    u8 name[34];
    u8 description[34];
    u16 field54;
    u16 field56;
    s8 field58;
    s8 type;
    s8 field5A;
    u8 reserved5B;
    s16 field5C, field5E, field60;
    s8 field62, field63, field64;
    s8 element;
    u8 field66, field67;
    u32 reserved68;
    u32 field6C;
    u8 field70, field71, field72, field73;
    u32 field74;
    u32 field78;
    u8 field7C;
    u8 reserved7D[3];
};

/* 80-byte consumable/material records at 081BE82C. */
struct ItemDefinition {
    u32 reserved00;
    u16 field04;
    u16 field06;
    u32 field08;
    u32 field0C;
    u8 name[34];
    u8 description[30];
};

extern const struct ArmDefinition gArmDefinitions[ARM_COUNT];
extern const struct ItemDefinition gItemDefinitions[ITEM_COUNT];
const struct ArmDefinition *ItemGetDefinition(s32 id);
const char *ItemGetName(s32 id);
const char *ItemGetDescription(s32 id);
s32 ItemGetField63(s32 id);
s32 ItemGetField64(s32 id);
s32 ItemGetField65(s32 id);
s32 ItemGetField5C(s32 id);
s32 ItemGetField5E(s32 id);
s32 ItemGetField60(s32 id);
s32 ItemGetField59(s32 id);
s32 ItemGetField5A(s32 id);
s32 ItemGetField62(s32 id);
s32 ItemGetField58(s32 id);

#endif
