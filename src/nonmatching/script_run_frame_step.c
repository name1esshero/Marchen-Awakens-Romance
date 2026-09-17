/* sub_08080070, called as `sub_08080070(frame)` from
 * ScriptDispatchCurrentFrame() in src/script_frames.c. The passed `frame`
 * argument is dead code in the ROM: the function re-derives
 * VM->state->frame itself via the global VM pointer instead of using r0,
 * even though the caller always passes the same value. Kept in the
 * signature to match the existing extern declaration and call site.
 *
 * If bit 0 of the frame's +0xA8 halfword is clear, walks the frame's 8
 * deferred-callback bits at +0xAA: for the first set bit, clears it and,
 * if the corresponding function pointer in the 8-entry table at +0x88 is
 * non-null, dispatches it through ScriptPushFrameAndJump(bit, callback)
 * and returns immediately (does not check the remaining bits). This is
 * the callback array ScriptFrameReleasePools() (src/script_frames.c)
 * frees/clears, and +0xAA is the same
 * halfword src/script_frames.c's ScriptSetFrameFlag() sets bits in --
 * this is `frame->flags`.
 *
 * Otherwise (or if no callback bit was set/dispatched), records the
 * dispatching frame and its +0x44 field (the same field ScriptReadNextU8()
 * reads as the bytecode program counter -- see src/script_bytecode.c) into
 * VM->state->dispatchState/dispatchIndex, then reads one opcode byte and
 * tail-calls its handler out of the 256-entry gScriptOpcodeHandlers table
 * (include/script.h) with no arguments, per that table's established
 * calling convention (see src/script_bytecode.c's ScriptCmd* functions).
 *
 * Logically confirmed correct and very close: the loop structure, the
 * mask-vs-value AND (once written as `u32 mask = 1 << bit;` matching the
 * ROM's own `movs r1,#1 / lsls r1,r5` -- writing the check as a bare
 * `value & (1 << bit)` instead makes agbcc shift the *value* right by
 * `bit` and test bit 0, which is a different, wrong-shaped instruction
 * sequence, not merely a register difference), the early-return dispatch,
 * and the whole opcode-dispatch tail (including reproducing the compiler's
 * own two absolute-address literal pool entries, decoded from the ROM's
 * unresolved raw `bl` bytes to ScriptPushFrameAndJump and ScriptReadNextU8
 * respectively) all match. Two differences remain:
 *   - Clearing the bit (`value & ~mask`, stored back) compiles in-place
 *     (`bic r1,r1,r3`) since `value` is dead after; the ROM copies value
 *     into a fresh register first, then `bics` (`adds r0,r3,#0` /
 *     `bics r0,r1`) -- one more instance of the destination-register ties
 *     documented in docs/AGBCC_CODEGEN.md, not reproduced by any spelling
 *     tried for this store.
 *   - Fetching the +0x88 table's base a second time: the ROM re-derives
 *     `VM->state->frame` from scratch (two more loads through the global)
 *     for this second use, where agbcc's ordinary CSE reuses the frame
 *     pointer already loaded for the flags check just above, whether or
 *     not the source repeats the `VM->state->frame` expression textually.
 *     No shape tried defeats that CSE for this one access.
 */

#include "gba/types.h"
#include "script_vm.h"

#define VM gScriptContext
extern s32 ScriptPushFrameAndJump(u32 callbackIndex, u32 destination);
extern u32 ScriptReadNextU8(void);

typedef s32 (*ScriptOpcodeHandler)(void);
#define SCRIPT_OPCODE_COUNT 256
extern const ScriptOpcodeHandler gScriptOpcodeHandlers[SCRIPT_OPCODE_COUNT];

s32 ScriptRunFrameStep(struct ScriptFrame *frame)
{
    u8 *raw;
    s32 bit;

    if (!(*(u16 *)((u8 *)VM->state->frame + 0xA8) & 1)) {
        for (bit = 0; bit <= 7; bit++) {
            u32 mask = 1 << bit;
            u16 value;
            raw = (u8 *)VM->state->frame;
            value = *(u16 *)(raw + 0xAA);
            if (mask & value) {
                *(u16 *)(raw + 0xAA) = value & ~mask;
                raw = (u8 *)VM->state->frame;
                if (((void **)(raw + 0x88))[bit])
                    return ScriptPushFrameAndJump(bit, (u32)((void **)(raw + 0x88))[bit]);
            }
        }
    }

    VM->state->dispatchState = (u32)VM->state->frame;
    VM->state->dispatchIndex = *(s32 *)((u8 *)VM->state->frame + 0x44);
    return gScriptOpcodeHandlers[ScriptReadNextU8()]();
}
