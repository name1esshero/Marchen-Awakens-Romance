#include "script_sprite.h"
#include "runtime_misc.h"
#include "rom_section.h"
extern char *strcpy(char *,const char *);
extern char *strupr(char *);
extern s32 SpriteResourceFindGroup(s32,const char *);
/**
 * @brief Replace an active script sprite's NCD selection.
 * Preserves the original unchecked 16-byte name buffer and s16 truncation.
 * Inactive sprites are untouched; a resource lookup failure is stored as -1.
 * @param id Script sprite record index.
 * @param container NCD container to select from.
 * @param name Resource group name, uppercased before lookup.
 * @param animation New animation index.
 * @param frame New frame index.
 * @return Nothing.
 */
AT("00010C0C") void ScriptSpriteSelect(s32 id,s32 container,const char *name,s32 animation,s32 frame)
{
 char resource[16];
 struct ScriptSprite *sprite=GameStateGetRecord0B90(id);
 if(sprite->active) {
  strcpy(resource,name);strupr(resource);
  sprite->container=container;
  sprite->group=SpriteResourceFindGroup(sprite->container,resource);
  sprite->animation=animation;sprite->frame=frame;
  sprite->drawOrderBits=3;
 }
}

/** Set and enable an indexed script sprite's translated collision bounds. */
AT("00011414")
void ScriptSpriteSetHitBounds(s32 id, s32 left, s32 top, s32 right,
                              s32 bottom)
{
    struct ScriptSprite *sprite = GameStateGetRecord0B90(id);

    sprite->hitBounds.left = left;
    sprite->hitBounds.top = top;
    sprite->hitBounds.right = right;
    sprite->hitBounds.bottom = bottom;
    sprite->hitBoundsEnabled = 1;
}

/** SprChg script command: forward its five VM arguments to
 * ScriptSpriteSelect(). @return Always 1. */
AT("00011EF4") s32 ScriptNativeSpriteChange(u32 count,const union SpriteArgument *args,s32 *result)
{
 ScriptSpriteSelect(args[0].integer,args[1].integer,args[2].string,args[3].integer,args[4].integer);
 return 1;
}

extern void sub_08010AEC(s32,s32,const char *,s32,s32,s32,s32);
/**
 * @brief SprInit script command: schedule sprite creation through
 * sub_08010AEC(). The fifth script value is forwarded unchanged; it is not
 * a start-frame selector. The creation task initializes the animation at
 * frame zero.
 * @return Always 1.
 */
AT("00011ECC") s32 ScriptNativeSpriteInit(u32 count,const union SpriteArgument *args,s32 *result)
{
 sub_08010AEC(args[0].integer,args[1].integer,args[2].string,args[3].integer,args[4].integer,1,0);
 return 1;
}

/* Native sprite property adapters: SprSet and SprGet script commands. */
extern s32 sub_08010E44(s32,s32,s32,s32,s32,s32,s32);
extern s32 sub_08011174(s32,s32,s32 *,s32);
/** SprSet script command: fix the task parameters to 0,0,value,1,0 and
 * delegate to sub_08010E44(). Leaves the VM result pointer untouched.
 * @return Always 1, regardless of the delegated helper's return value. */
AT("00011F40") s32 ScriptNativeSpriteSet(u32 count,const s32 *args,s32 *result)
{
 sub_08010E44(args[0],args[1],0,0,args[2],1,0);
 return 1;
}
AT("00011F40") const u8 ScriptNativeSpriteSetTail[2]={0,0};
/** SprGet script command: pass the VM result pointer through to
 * sub_08011174(). @return Always 1, regardless of the delegated helper's
 * return value. */
AT("00011F68") s32 ScriptNativeSpriteGet(u32 count,const s32 *args,s32 *result)
{
 sub_08011174(args[0],args[1],result,0);
 return 1;
}
AT("00011F68") const u8 ScriptNativeSpriteGetTail[2]={0,0};

/* SprInit worker task and its supporting declarations. */
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
extern s32 SpriteResourceFindGroup(s32,const char *);
extern void ScriptCompletePendingTasks(u32);
extern void FinishTask(void *);
/**
 * @brief SprInit worker task (08010B6C..08010C0C). State 0 schedules
 * preparation; state 16 waits on payload +44, then activates the sprite's
 * resources. Auxiliary block semantics and flag bits remain unnamed where
 * unverified. Preserves the original unchecked allocation and failure
 * behavior.
 * @param task This task's own record.
 * @return Nothing.
 */
AT("00010B6C") void ScriptSpriteInitTask(struct SpriteInitTask *task)
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
  sprite->group=SpriteResourceFindGroup(sprite->container,(char *)(payload+1));
  sprite->animation=payload[8];sprite->frame=payload[9];
  sprite->drawOrderBits=3;sprite->hitBoundsEnabled=0;
  ((struct SpriteFlagByte *)&sprite->flags1)->enabled=1;*((u8 *)sprite)|=2;
  ScriptCompletePendingTasks(1);
  if(task->result)*task->result=-1;
  FinishTask(task);
 }
 }
}
AT("00010B6C") const u8 ScriptSpriteInitTaskTail[2]={0,0};

/* Deferred script-sprite reset workers and their supporting declarations. */
#include "runtime_misc.h"
#include "runtime_leaf.h"
struct ResetFields {u8 before[20];s16 value20,value22;};
extern void HeapFree(void *,void *);
extern void CpuFill(void *,u32,u32);
extern void ScriptCompletePendingTasks(u32);
extern void FinishTask(void *);
/**
 * @brief Deferred script-sprite reset worker for a single sprite. Waits
 * while the sprite's +0x1A field is nonzero; otherwise tears down its
 * auxiliary block (if any) and clears its 40-byte record. Fields +0x14/+0x16
 * reset to 256; their exact semantics remain unverified.
 * @param task This task's own record; +32 holds the sprite index.
 * @return Nothing.
 */
AT("000109C4") void ScriptSpriteResetTask(void *task)
{
 struct ScriptSprite *sprite=GameStateGetRecord0B90(*(s32 *)((u8 *)task+32));
 if(sprite->active){
  if(*(u16 *)((u8 *)sprite+26))return;
  if(*(void **)((u8 *)sprite+36)){
   SpriteAuxiliaryReset(*(void **)((u8 *)sprite+36));
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
AT("000109C4") const u8 ScriptSpriteResetTaskTail[2]={0,0};

/**
 * @brief Deferred reset worker that visits all 32 script sprite slots,
 * resetting each inactive-after-wait sprite the same way as
 * ScriptSpriteResetTask(). Slots still waiting on +0x1A are left untouched
 * and counted; the task only finishes once none remain waiting.
 * @param task This task's own record.
 * @return Nothing.
 */
AT("00010A70") void ScriptSpriteResetAllTask(void *task)
{
 struct ScriptSprite *sprite=GameStateGetRecord0B90(0);
 s32 waiting=0,i=31;
 do {
  if(sprite->active){
   if(*(u16 *)((u8 *)sprite+26))waiting++;
   else {
    if(*(void **)((u8 *)sprite+36)){
     SpriteAuxiliaryReset(*(void **)((u8 *)sprite+36));
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
AT("00010A70") const u8 ScriptSpriteResetAllTaskTail[2]={0,0};
