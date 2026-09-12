#ifndef ITEM_H
#define ITEM_H

#include "gba/types.h"

/* 128-byte item records at 081B096C. Names and descriptions use the game
 * charmap. Stat meanings remain unverified, so fields retain byte offsets. */
struct ItemDefinition {
    u8 unknown00[16];
    char name[34];
    char description[38];
    s8 field58, field59, field5A;
    u8 unknown5B;
    s16 field5C, field5E, field60;
    s8 field62, field63, field64, field65;
    u8 unknown66[26];
};
extern const struct ItemDefinition gItemDefinitions[];
const struct ItemDefinition *ItemGetDefinition(s32 id);
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
