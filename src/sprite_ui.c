/* Initialization for the compact UI sprite state used by the menu setup path. */
#include "item.h"
#include "runtime_objects.h"
#include "sprite_engine.h"
#include "sprite_ui.h"

#include "rom_section.h"

#define SPRITE_UI_ACTIVE 1
#define SPRITE_UI_DEFAULT_RESOURCE 1
#define SPRITE_UI_BASELINE_Y -48
#define SPRITE_UI_RECORD_OFFSET 100
#define SPRITE_UI_DEFAULT_ARM 85
#define SPRITE_UI_GLYPH_SCALE 60

/**
 * @brief Initialize a UI sprite state and resolve its named resource group.
 * @param state Target 12-byte sprite state.
 * @param unused Caller-reserved argument not read by this initializer.
 * @param resourceName Name used to resolve the sprite resource group.
 * @param kind Initial kind byte.
 * @param variant Initial variant byte.
 */
AT("00019C08")
void SpriteUiInitialize(struct SpriteUiInit *state, u32 unused,
                        const char *resourceName, u32 kind, u32 variant)
{
    state->active = SPRITE_UI_ACTIVE;
    state->unknown01 = 0;
    state->kind = kind;
    state->variant = variant;
    state->activeCount = SPRITE_UI_ACTIVE;
    state->verticalOffset = SPRITE_UI_BASELINE_Y;
    state->resourceGroup = SpriteResourceFindGroup(SPRITE_UI_DEFAULT_RESOURCE,
                                                   resourceName);
}

/**
 * @brief Configure the default UI-sprite state for an actor part.
 * @param actor Runtime actor index.
 * @param part Actor-part index.
 */
AT("00019C78")
void SpriteUiSetupDefault(u32 actor, u32 part)
{
    struct SpriteUiInit *state = (struct SpriteUiInit *)
        ((u8 *)RuntimeGetActorRecord(actor, part) + SPRITE_UI_RECORD_OFFSET);

    state->kind = SPRITE_UI_ACTIVE;
    state->verticalOffset = ItemGetDefinition(SPRITE_UI_DEFAULT_ARM)->field62
                            * SPRITE_UI_GLYPH_SCALE;
}
