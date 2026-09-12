/* SprInit worker, 08010B6C..08010C0C. State 0 schedules preparation;
 * state 16 waits on payload +44. Only then are the sprite resources activated.
 * Auxiliary block semantics and flag bits remain unnamed where unverified.
 * Preserve the original unchecked allocation and failure behavior. */
#include "script_sprite.h"
#include "runtime_misc.h"
struct SpriteFlagByte {u8 enabled:1,rest:7;};
struct SpriteInitTask {
 u8 header[14];u16 state;u8 reserved[8];s32 *result;u8 reserved1[4];
 s32 id;char name[20];s32 container;u32 unknown;s32 animation,frame,wait,finished;
};
extern void sub_0801097C(s32,s32,s32 *);
extern void *HeapAlloc(void *,u32);
extern void CpuFill(void *,u32,u32);
#include "ncd.h"
extern s32 FindResourceByName(s32,const char *);
extern void ScriptCompletePendingTasks(u32);
extern void FinishTask(void *);
__attribute__((section(".rom.00010B6C"))) void ScriptSpriteInitTask(struct SpriteInitTask *task)
{
 s32 *payload=&task->id;
 struct ScriptSprite *sprite=GameStateGetRecord0B90(task->id);
 switch(task->state){case 0:sub_0801097C(task->id,payload[10],&task->finished);task->state=16;
 case 16:
 if(payload[11]){
  void **extra=(void **)((u8 *)sprite+36);
  *extra=HeapAlloc(0,72);CpuFill(*extra,72,0);NcdSpriteContainerReset(*extra);
  sprite->active=1;
  sprite->container=payload[6];
  sprite->group=FindResourceByName(sprite->container,(char *)(payload+1));
  sprite->animation=payload[8];sprite->frame=payload[9];
  sprite->drawOrderBits=3;sprite->last=0;
  ((struct SpriteFlagByte *)&sprite->flags1)->enabled=1;*((u8 *)sprite)|=2;
  ScriptCompletePendingTasks(1);
  if(task->result)*task->result=-1;
  FinishTask(task);
 }
 }
}
__attribute__((section(".rom.00010B6C"))) const u8 ScriptSpriteInitTaskTail[2]={0,0};
