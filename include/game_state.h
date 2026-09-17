#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "gba/types.h"

/* Main runtime root cached in IWRAM. The historical name is retained from
 * the map-generation code that first recovered this slot. */
extern u8 *gMapGenerationRoot;

/* Accessors for the main runtime object. Offset-based names are intentional:
 * callers establish storage type and signedness, but not every gameplay name
 * has been proved yet. */
u32 GameStateGetField4240(void);
void GameStateSetField4240(u32 value);
u32 GameStateGetField4256(void);
void GameStateSetField4256(u32 value);
s32 GameStateGetField4246(void);
void GameStateSetField4246(s32 value);
s32 GameStateGetField4248(void);
void GameStateSetField4248(s32 value);
void GameStateSetField4254(s32 value);
s32 GameStateGetField4254(void);
s32 GameStateGetField4244(void);
void GameStateSetField4244(s32 value);
s32 GameStateGetField4245(void);
void GameStateSetField4245(s32 value);
void GameStateGetField424C50(u32 *first,u32 *second);
u32 GameStateGetField424C(void);
u32 GameStateGetField4250(void);
void GameStateSetField4265(u32 index,s32 value);
s32 GameStateGetField4265(u32 index);
s32 GameStateGetField425A(void);
void GameStateSetField425B(s32 value);
s32 GameStateGetField425B(void);
void GameStateSetField4258(s32 value);
s32 GameStateGetField4258(void);
void GameStateSetField42BA(s32 value);
s32 GameStateGetField42BA(void);
void GameStateSetField42BC(s32 value);
s32 GameStateGetField42BC(void);
void GameStateSetField42C0(u32 value);
u32 GameStateGetField42C0(void);
void GameStateSetField42C4(s32 value);
s32 GameStateGetField42C4(void);
s32 GameStateGetField425C(u32 index);
void GameStateSetField425C(u32 index,s32 value);
s32 GameStateGetField4269(void);
void GameStateSetField4269(s32 value);

#endif
