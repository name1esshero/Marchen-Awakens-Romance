#ifndef SCRIPT_SPRITE_H
#define SCRIPT_SPRITE_H
#include "gba/types.h"
/* 40-byte script sprite record at engine state + 0xB90 + id * 40.
 * Only fields verified through resource selection and rendering are named.
 * drawOrderBits are forwarded to NCD queue-selection bits by 08010730.
 */
struct ScriptSprite
{
    u8 active:1, other:4, drawOrderBits:2, last:1;
    u8 flags1;
    s16 container, group, animation, frame;
    s16 x, y;
    u8 rest[26];
};
void ScriptSpriteSelect(s32 id, s32 container, const char *name, s32 animation, s32 frame);
union SpriteArgument {s32 integer; const char *string;};
s32 ScriptNativeSpriteChange(u32 count, const union SpriteArgument *args, s32 *result);
s32 ScriptNativeSpriteInit(u32 count, const union SpriteArgument *args, s32 *result);
s32 ScriptNativeSpriteSet(u32 count, const s32 *args, s32 *result);
s32 ScriptNativeSpriteGet(u32 count, const s32 *args, s32 *result);
#endif
