/* Matching VM frame operations. WorkBatch always attempts one dispatch and
 * returns accumulated work, unlike ScriptRunSlice, which checks the pending
 * counter first and returns a status. Do not merge these two entry points.
 * Pop restores the parent before freeing frame-owned allocations; the optional
 * resource belongs to heap zero, while the other blocks use the VM heap.
 * Unknown table roles remain offset-named in the recovered layout. */
#include "script_vm.h"
#include "rom_section.h"
#define VM gScriptContext
extern s32 sub_08080070(struct ScriptFrame *);
extern void HeapFree(void *,void *);
extern void CpuFill(void *,u32,u32);

/** Release allocations owned by the current frame's two temporary pools,
 * then reset the interpreter fields that refer to them. table038 entries own
 * one allocation; table03C entries own an array and each non-null element in
 * that array. ScriptPopFrame frees the two entry tables themselves afterward.
 * @return Nothing. */
AT("0007EC58") void ScriptFrameReleasePools(void)
{
 struct ScriptFrame *frame=VM->state->frame;
 struct ScriptFramePoolEntry *entry;
 s32 i;
 u8 *raw;

 if(!frame) return;

 i=0;
 entry=frame->table038;
 for(;i<frame->count034;i++,entry++)
 {
  if(entry->count>1) HeapFree(VM->state->heap,entry->data);
  entry->count=0;
  entry->data=0;
 }

 i=0;
 entry=frame->table03C;
 for(;i<frame->count036;i++,entry++)
 {
  if(entry->count>1)
  {
   u32 j=0;
   void **array=entry->data;
   for(;j<entry->count;j++,array++)
   {
    if(*array) HeapFree(VM->state->heap,*array);
   }
  }
  if(entry->data) HeapFree(VM->state->heap,entry->data);
  entry->count=0;
  entry->data=0;
 }

 frame->programCounter=0;
 CpuFill(frame->work048,SCRIPT_FRAME_WORK_RESET_SIZE,0);
 CpuFill(frame->callbackAddresses,SCRIPT_FRAME_CALLBACK_RESET_SIZE,0);
 frame->dispatchFlags=0;
 frame->flags=0;
 raw=(u8 *)frame;
 frame->field084=*(u32 *)(raw+SCRIPT_FRAME_SOURCE_WORD_A_OFFSET)
                 +*(u32 *)(raw+SCRIPT_FRAME_SOURCE_WORD_B_OFFSET);
 VM->state->dispatchState=(u32)frame;
 VM->state->dispatchIndex=0;
}
/** Dispatch the VM's current frame once.
 * @return The dispatch's reported work, or 0 if there is no current frame. */
AT("0007ED70") s32 ScriptDispatchCurrentFrame(void)
{
 struct ScriptFrame *frame=VM->state->frame;
 s32 result;
 if(frame) result=sub_08080070(frame);
 else result=0;
 return result;
}
AT("0007ED70") const u8 ScriptDispatchCurrentFrameTail[2]={0,0};
/** Repeatedly dispatch the current frame until it stops reporting work or
 * the VM's per-slice step budget is reached. Always attempts one dispatch,
 * unlike ScriptRunSlice, which checks the pending counter first.
 * @return Total work accumulated this batch. */
AT("0007ED90") u32 ScriptRunWorkBatch(void)
{
 u32 work=0;
 s32 status;
 do {
  status=ScriptDispatchCurrentFrame();
  work+=status;
 } while(work < VM->state->stepBudget && status>0);
 return work;
}
/** Set one bit of the VM's current frame flags.
 * @param index Bit index, 0..7.
 * @return 0 on success; -1 if there is no current frame or index is out of
 * range. */
AT("0007EDC0") s32 ScriptSetFrameFlag(u32 index)
{
 struct ScriptFrame *frame=VM->state->frame;
 if(frame && index<=7)
 {
  frame->flags |= 1<<index;
  return 0;
 }
 return -1;
}
/** Set the VM's per-slice step budget, without validating the caller's
 * frame state. A value of 0 is replaced with the default budget of 10.
 * @param value New step budget.
 * @return Nothing. */
AT("0007EDF0") void ScriptSetStepBudgetUnchecked(u32 value)
{
 u32 *limit=&VM->state->stepBudget;
 *limit=value;
 if(!value) *limit=10;
}
/** Restore the VM's parent frame, then free the popped frame's allocations
 * (storage, both work tables, and its optional owned resource) and the frame
 * itself. The optional resource is freed from heap zero; the other blocks
 * from the VM heap.
 * @return Nothing. */
AT("0007EFB8") void ScriptPopFrame(void)
{
 struct ScriptExecutionState *state;
 struct ScriptFrame *frame;
 ScriptFrameReleasePools();
 state=VM->state;
 frame=state->frame;
 state->frame=frame->parent;
 if(frame->storage) HeapFree(state->heap,frame->storage);
 if(frame->table038) HeapFree(VM->state->heap,frame->table038);
 if(frame->table03C) HeapFree(VM->state->heap,frame->table03C);
 if(frame->ownsResource && frame->resource) HeapFree(0,frame->resource);
 HeapFree(VM->state->heap,frame);
 state=VM->state;
 state->dispatchState=0;
 state->dispatchIndex=-1;
}
