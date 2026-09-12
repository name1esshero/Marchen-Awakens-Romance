/* 08010C0C: replace an active script sprite's NCD selection.
 * Preserve the original unchecked 16-byte name buffer and s16 truncation.
 * Inactive sprites are untouched; resource lookup failure is stored as -1.
 */
#include "script_sprite.h"
extern struct ScriptSprite *sub_080106C8(s32);
extern char *strcpy(char *,const char *);
extern char *strupr(char *);
extern s32 FindResourceByName(s32,const char *);
__attribute__((section(".rom.00010C0C"))) void ScriptSpriteSelect(s32 id,s32 container,const char *name,s32 animation,s32 frame)
{
 char resource[16];
 struct ScriptSprite *sprite=sub_080106C8(id);
 if(sprite->active) {
  strcpy(resource,name);strupr(resource);
  sprite->container=container;
  sprite->group=FindResourceByName(sprite->container,resource);
  sprite->animation=animation;sprite->frame=frame;
  sprite->drawOrderBits=3;
 }
}

__attribute__((section(".rom.00011EF4"))) s32 ScriptNativeSpriteChange(u32 count,const union SpriteArgument *args,s32 *result)
{
 ScriptSpriteSelect(args[0].integer,args[1].integer,args[2].string,args[3].integer,args[4].integer);
 return 1;
}
