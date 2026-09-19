/**
 * @file script_run_frame_step.c
 * @brief Nonmatching reconstruction of one script-interpreter step.
 *
 * The frame argument is unused in the ROM. The routine reads the active frame
 * from gScriptContext, dispatches at most one pending callback, records the
 * bytecode position, and invokes the next opcode handler.
 *
 * The VM was evidently manipulated through a shared word-storage view rather
 * than only through the later recovered ScriptContext/ScriptFrame types. A
 * partial union preserves that real alias relationship: agbcc then keeps the
 * context in r4, reloads state->frame after the flag store, and addresses the
 * flag at frame+0xAA exactly as the ROM does.
 *
 * Reusing one pointer scratch for the current context and the opcode table
 * reproduces the ROM's r4 lifetime. Reusing one scalar scratch for the bit
 * mask and callback address likewise puts it in r1 and the loaded flag word
 * in r3. One instruction-allocation difference remains: the ROM copies r3
 * to r0 and clears r0, while agbcc clears r3 in place. No register forcing,
 * volatile qualifier, or inline assembly is used.
 */

#include "gba/types.h"
#include "script.h"
#include "script_vm.h"

#define SCRIPT_DISPATCH_BLOCKED 1

extern s32 ScriptPushFrameAndJump(u32 callbackIndex, u32 destination);
extern u32 ScriptReadNextU8(void);

/* Shared storage view used by the original VM. Only fields reached by this
 * routine are exposed; the typed public layouts remain in script_vm.h. */
union ScriptRuntimeStorage
{
    u32 word;
    struct
    {
        u8 padding[12];
        union ScriptRuntimeStorage *next;
    } link;
    struct
    {
        u8 padding[136];
        u32 addresses[SCRIPT_FRAME_CALLBACK_COUNT];
    } callbacks;
};

struct ScriptRuntimeFlag
{
    u16 value;
};

union ScriptDispatchPointer
{
    union ScriptRuntimeStorage *runtime;
    const ScriptOpcodeHandler *handlers;
};

s32 ScriptRunFrameStep(struct ScriptFrame *unusedFrame)
{
    s32 bit;
    union ScriptDispatchPointer data;

    if (!(gScriptContext->state->frame->dispatchFlags & SCRIPT_DISPATCH_BLOCKED))
    {
        for (bit = 0; bit < SCRIPT_FRAME_CALLBACK_COUNT; bit++)
        {
            union ScriptRuntimeStorage *frame;
            struct ScriptRuntimeFlag *flags;
            u32 value;
            u16 flagWord;

            data.runtime = (union ScriptRuntimeStorage *)gScriptContext;
            frame = data.runtime->link.next->link.next;
            flags = (struct ScriptRuntimeFlag *)((u8 *)frame + 0xAA);
            value = 1 << bit;
            flagWord = flags->value;

            if (value & flagWord)
            {
                flags->value = flagWord & ~value;
                frame = data.runtime->link.next->link.next;
                value = frame->callbacks.addresses[bit];
                if (value != 0)
                    return ScriptPushFrameAndJump(bit, value);
            }
        }
    }

    gScriptContext->state->dispatchState =
        (u32)gScriptContext->state->frame;
    gScriptContext->state->dispatchIndex =
        gScriptContext->state->frame->programCounter;
    data.handlers = gScriptOpcodeHandlers;
    return data.handlers[ScriptReadNextU8()]();
}
