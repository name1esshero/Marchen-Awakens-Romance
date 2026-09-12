/* Matching VM frame operations. WorkBatch always attempts one dispatch and
 * returns accumulated work, unlike ScriptRunSlice, which checks the pending
 * counter first and returns a status. Do not merge these two entry points.
 * Pop restores the parent before freeing frame-owned allocations; the optional
 * resource belongs to heap zero, while the other blocks use the VM heap.
 * Unknown table roles remain offset-named in the recovered layout. */
#include "script_vm.h"
#define AT(x) __attribute__((section(".rom." x)))
#define VM (*(struct ScriptContext **)0x0300611C)
extern s32 sub_08080070(struct ScriptFrame *);
AT("0007ED70") s32 ScriptDispatchCurrentFrame(void)
{
 struct ScriptFrame *frame=VM->state->frame;
 s32 result;
 if(frame) result=sub_08080070(frame);
 else result=0;
 return result;
}
AT("0007ED70") const u8 ScriptDispatchCurrentFrameTail[2]={0,0};
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
AT("0007EDF0") void ScriptSetStepBudgetUnchecked(u32 value)
{
 u32 *limit=&VM->state->stepBudget;
 *limit=value;
 if(!value) *limit=10;
}
extern void sub_0807EC58(void);
extern void HeapFree(void *,void *);
AT("0007EFB8") void ScriptPopFrame(void)
{
 struct ScriptExecutionState *state;
 struct ScriptFrame *frame;
 sub_0807EC58();
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
