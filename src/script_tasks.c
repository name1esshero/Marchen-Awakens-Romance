/* Recovered task/VM coordination. These agbcc sections match the ROM.
 * 0300611C points to the active VM; +0x0C points to its execution state.
 * State +0x220 is the pending-operation counter used by asynchronous commands.
 * These helpers deliberately retain the original null checks and unsigned
 * arithmetic (including underflow behavior). They do not own task memory.
 * VramFillTask runs on the separate graphics task queue and fills its payload's
 * destination/byte count/pattern before marking itself for scheduler removal.
 */
#include "gba/types.h"
#define AT(x) __attribute__((section(".rom." x)))
extern void FinishTask(void *);
extern void CpuFill(void *,u32,u32);
AT("0000380C") void VramFillTask(void *task)
{
 u8 *payload = (u8 *)task + 32;
 CpuFill(*(void **)payload, *(u32 *)(payload+4), *(u32 *)(payload+8));
 FinishTask(task);
}
AT("0000380C") const u8 VramFillTaskTail[2]={0,0};
AT("0007E420") void ScriptAddPendingTasks(u32 count)
{
 u8 *context = *(u8 **)0x0300611C;
 u8 *state;
 if (context && (state = *(u8 **)(context+12)))
  *(u32 *)(state+0x220) += count;
}
AT("0007E448") void ScriptCompletePendingTasks(u32 count)
{
 u8 *context = *(u8 **)0x0300611C;
 u8 *state;
 if (context && (state = *(u8 **)(context+12)))
  *(u32 *)(state+0x220) -= count;
}

/* VM shutdown clears the active flag and execution-state pointer; storage is
 * released by the caller. The result word at context +8 is copied to the
 * script task's caller at 080052CE. Native command 0808045C sets that result
 * and unwinds frames. Frame +0x40 is the parent link: 0807EFB8 restores it
 * into execution state +12 before freeing the current frame. State +0x214
 * is the per-update work budget, compared against accumulated dispatch results
 * by ScriptRunSlice. Passing zero to its setter restores the default of ten.
 * 0807E3D4/0807E3F8 were previously emitted as opaque words; 0807E416 was
 * mistakenly marked as a function inside the getter's IWRAM literal pool.
 */
AT("0007E384") void ScriptDeactivate(void)
{
 u8 *context=*(u8 **)0x0300611C;
 *(u32 *)(context+4)=0;
 *(u32 *)(context+12)=0;
}
AT("0007E394") void ScriptSetResult(u32 value)
{
 u8 *context=*(u8 **)0x0300611C;
 *(u32 *)(context+8)=value;
}
AT("0007E3A0") u32 ScriptGetResult(void)
{
 u8 *context=*(u8 **)0x0300611C;
 return *(u32 *)(context+8);
}
AT("0007E3AC") void *ScriptGetParentFrame(void)
{
 u8 *context=*(u8 **)0x0300611C;
 u8 *state,*frame;
 if(context && (state=*(u8 **)(context+12)) && (frame=*(u8 **)(state+12)))
  return *(void **)(frame+64);
 return 0;
}
AT("0007E3AC") const u8 ScriptGetParentFrameTail[2]={0,0};
AT("0007E3D4") void ScriptSetPendingTasks(u32 count)
{
 u8 *context=*(u8 **)0x0300611C;
 u8 *state;
 if(context && (state=*(u8 **)(context+12)))
  *(u32 *)(state+0x220)=count;
}
AT("0007E3F8") u32 ScriptGetPendingTasks(void)
{
 u8 *context=*(u8 **)0x0300611C;
 u8 *state;
 if(context && (state=*(u8 **)(context+12)))
  return *(u32 *)(state+0x220);
 return 0;
}
AT("0007E3F8") const u8 ScriptGetPendingTasksTail[2]={0,0};
AT("0007E470") void ScriptSetStepBudget(u32 value)
{
 u8 *context=*(u8 **)0x0300611C;
 u8 *state;
 if(context && (state=*(u8 **)(context+12)))
 {
  u32 *limit=(u32 *)(state+0x214);
  if(value) *limit=value;
  else *limit=10;
 }
}
AT("0007E49C") u32 ScriptGetStepBudget(void)
{
 u8 *context=*(u8 **)0x0300611C;
 u8 *state;
 if(context && (state=*(u8 **)(context+12)))
  return *(u32 *)(state+0x214);
 return 0;
}
AT("0007E49C") const u8 ScriptGetStepBudgetTail[2]={0,0};

/* Execute a bounded VM slice. Pending asynchronous operations block dispatch.
 * Dispatch status is also accumulated as work; nonpositive status stops the
 * slice. Preserve the original unsigned accumulation and clamp only values
 * above one, leaving zero/negative termination codes intact. */
extern s32 ScriptDispatchCurrentFrame(void);

AT("0007F15C") s32 ScriptRunSlice(void)
{
 u32 processed=0;
 s32 status=1;
 u8 *state=*(u8 **)(*(u8 **)0x0300611C+12);
 while(processed < *(u32 *)(state+0x214) && status>0)
 {
  if(*(u32 *)(state+0x220)) break;
  status=ScriptDispatchCurrentFrame();
  processed+=status;
  state=*(u8 **)(*(u8 **)0x0300611C+12);
 }
 return status>1 ? 1 : status;
}
extern void ScriptSetResult(u32);
extern void *ScriptGetParentFrame(void);
extern void ScriptPopFrame(void);
/* Native return: publish the argument, pop through the parent chain, then
 * pop the root frame. Native command success is one. */
AT("0008045C") s32 ScriptCommandReturn(u32 count, const u32 *arguments)
{
 ScriptSetResult(arguments[0]);
 while(ScriptGetParentFrame()) ScriptPopFrame();
 ScriptPopFrame();
 return 1;
}
