/* Deferred script-sprite reset workers. Active records wait while +0x1A
 * is nonzero. Auxiliary teardown precedes clearing the 40-byte record.
 * Fields +0x14/+0x16 reset to 256; their exact semantics remain unverified.
 * The all-sprites worker visits 32 slots and leaves inactive slots untouched. */
#include "script_sprite.h"
struct ResetFields {u8 before[20];s16 value20,value22;};
extern struct ScriptSprite *sub_080106C8(s32);
extern void sub_08008BD8(void *);
extern void HeapFree(void *,void *);
extern void CpuFill(void *,u32,u32);
extern void ScriptCompletePendingTasks(u32);
extern void FinishTask(void *);
__attribute__((section(".rom.000109C4"))) void ScriptSpriteResetTask(void *task)
{
 struct ScriptSprite *sprite=sub_080106C8(*(s32 *)((u8 *)task+32));
 if(sprite->active){
  if(*(u16 *)((u8 *)sprite+26))return;
  if(*(void **)((u8 *)sprite+36)){
   sub_08008BD8(*(void **)((u8 *)sprite+36));
   HeapFree(0,*(void **)((u8 *)sprite+36));
  }
 }
 CpuFill(sprite,40,0);
 ((struct ResetFields *)sprite)->value20=256;
 ((struct ResetFields *)sprite)->value22=256;
 sprite->drawOrderBits=3;
 ScriptCompletePendingTasks(1);
 if(*(s32 **)((u8 *)task+24))**(s32 **)((u8 *)task+24)=-1;
 FinishTask(task);
}
__attribute__((section(".rom.000109C4"))) const u8 ScriptSpriteResetTaskTail[2]={0,0};

__attribute__((section(".rom.00010A70"))) void ScriptSpriteResetAllTask(void *task)
{
 struct ScriptSprite *sprite=sub_080106C8(0);
 s32 waiting=0,i=31;
 do {
  if(sprite->active){
   if(*(u16 *)((u8 *)sprite+26))waiting++;
   else {
    if(*(void **)((u8 *)sprite+36)){
     sub_08008BD8(*(void **)((u8 *)sprite+36));
     HeapFree(0,*(void **)((u8 *)sprite+36));
    }
    CpuFill(sprite,40,0);
    ((struct ResetFields *)sprite)->value20=256;
    ((struct ResetFields *)sprite)->value22=256;
    sprite->drawOrderBits=3;
   }
  }
  i--;sprite++;
 }while(i>=0);
 if(!waiting){
  ScriptCompletePendingTasks(1);
  if(*(s32 **)((u8 *)task+24))**(s32 **)((u8 *)task+24)=-1;
  FinishTask(task);
 }
}
__attribute__((section(".rom.00010A70"))) const u8 ScriptSpriteResetAllTaskTail[2]={0,0};
