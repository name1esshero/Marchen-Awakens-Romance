#ifndef FLAGS_H
#define FLAGS_H

#include "gba/types.h"

/* Packed flag banks in the main game-state allocation.  Offsets are kept in
 * one contributor-facing header; gameplay IDs should only be named here once
 * their meaning is verified from scripts or runtime call sites. */
#define GAME_STATE_FLAGS_OFFSET 0x00AC
#define GAME_STATE_FLAG_COUNT 1024

#define GAME_STATE_ATTRIBUTE_FLAGS_OFFSET 0x012C
#define GAME_STATE_ATTRIBUTE_FLAG_COUNT 10000
#define GAME_STATE_ATTRIBUTE_FLAGS_SIZE (GAME_STATE_ATTRIBUTE_FLAG_COUNT / 8)

#define GAME_STATE_DECK_FLAGS_OFFSET 0x26F8
#define GAME_STATE_FLAGS_2730_OFFSET 0x2730

void GameStateSetFlagsAC(u32 bit, s32 enabled);
s32 GameStateTestFlagsAC(u32 bit);

void GameStateInitializeAttributeFlags(void);
void GameStateSetAttributeFlag(s32 index, s32 enabled);
s32 GameStateTestAttributeFlag(u32 bit);
void GameStateSetAttributeFlagRange(s32 first, s32 last, s32 enabled);

/* The +0x2730 bank's gameplay role is not yet proven. */
void GameStateSetFlag2730(s32 index);
s32 GameStateTestFlag2730(s32 index);
s32 GameStateTestFlag26F8(s32 index);

#endif /* FLAGS_H */
