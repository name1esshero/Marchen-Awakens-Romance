/* Native sprite properties. SprSet fixes the task parameters to 0,0,value,1,0.
 * Both adapters return 1 regardless of the delegated helper's return value.
 * SprGet passes through the VM result pointer. SprSet leaves it untouched. */
#include "script_sprite.h"
extern s32 sub_08010E44(s32,s32,s32,s32,s32,s32,s32);
extern s32 sub_08011174(s32,s32,s32 *,s32);
__attribute__((section(".rom.00011F40"))) s32 ScriptNativeSpriteSet(u32 count,const s32 *args,s32 *result)
{
 sub_08010E44(args[0],args[1],0,0,args[2],1,0);
 return 1;
}
__attribute__((section(".rom.00011F40"))) const u8 ScriptNativeSpriteSetTail[2]={0,0};
__attribute__((section(".rom.00011F68"))) s32 ScriptNativeSpriteGet(u32 count,const s32 *args,s32 *result)
{
 sub_08011174(args[0],args[1],result,0);
 return 1;
}
__attribute__((section(".rom.00011F68"))) const u8 ScriptNativeSpriteGetTail[2]={0,0};
