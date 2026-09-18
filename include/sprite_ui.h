#ifndef SPRITE_UI_H
#define SPRITE_UI_H

#include "gba/types.h"

/* Compact sprite state initialized by the UI setup path at 0x08019AA0.
 * The resource-name argument selects the group stored at +0x0A. */
struct SpriteUiInit
{
    u8 active;
    u8 unknown01;
    u8 kind;
    u8 variant;
    u16 unknown04;
    s16 verticalOffset;
    u16 activeCount;
    s16 resourceGroup;
};

void SpriteUiInitialize(struct SpriteUiInit *state, u32 unused,
                        const char *resourceName, u32 kind, u32 variant);
void SpriteUiSetupDefault(u32 actor, u32 part);

#endif /* SPRITE_UI_H */
