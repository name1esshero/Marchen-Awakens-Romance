#ifndef SCRIPT_VM_H
#define SCRIPT_VM_H
#include "gba/types.h"

/* Partial layouts recovered from the VM's allocation, dispatch and teardown.
 * Offsets describe the 32-bit GBA ABI; unknown regions stay explicitly opaque. */
struct ScriptFrame
{
    u8 unknown000[0x10];
    char resourceName[0x20];      /* active SPC/resource name */
    void *storage;                 /* 030: allocated while loading frame data */
    u16 count034, count036;
    void *table038;
    void *table03C;
    struct ScriptFrame *parent;    /* 040: restored when this frame is popped */
    u8 unknown044[0x66];
    u16 flags;                    /* 0AA: setter accepts bit indices 0..7 */
    void *resource;                /* 0AC: freed through heap zero if owned */
    u32 ownsResource;              /* 0B0 */
};
struct ScriptExecutionState
{
    void *heap;
    u8 unknown004[8];
    struct ScriptFrame *frame;     /* 00C */
    u8 unknown010[0x204];
    u32 stepBudget;                /* 214 */
    u32 dispatchState;             /* 218: reset to zero after popping */
    s32 dispatchIndex;             /* 21C: reset to -1 after popping */
    u32 pendingTasks;              /* 220 */
};
struct ScriptContext
{
    void *workspace;
    u32 active;
    u32 result;
    struct ScriptExecutionState *state;
};
s32 ScriptDispatchCurrentFrame(void);
u32 ScriptRunWorkBatch(void);
s32 ScriptSetFrameFlag(u32 index);
void ScriptSetStepBudgetUnchecked(u32 value);
void ScriptPopFrame(void);
s32 ScriptNativeChain(u32 count, const u32 *arguments, u32 *result);
s32 ScriptNativeExec(u32 count, const u32 *arguments, u32 *result);
s32 ScriptNativeCall(u32 count, const u32 *arguments, u32 *result);
s32 ScriptNativeResurn(u32 count, const u32 *arguments, u32 *result);
s32 ScriptNativeExit(u32 count, const u32 *arguments);
#endif
