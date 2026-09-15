/* sub_0807EC58, called as `sub_0807EC58()` (no arguments) from
 * ScriptPopFrame() in src/script_frames.c, before that function frees
 * table038/table03C's own backing allocations and the frame itself. The
 * trailing label sub_0807ED60 is the tail end of this function's own
 * epilogue (the compiler's usual r8-r10 push/pop-via-low-register dance),
 * not a real function -- confirmed with `grep -rn` that it has no external
 * caller.
 *
 * Each table038/table03C slot is an 8-byte {count, data} pair. table038's
 * `data` is a single allocation, freed outright when count > 1. table03C's
 * `data` is itself an array of `count` pointers, each freed individually
 * before the array allocation itself is freed. Both tables' slots are then
 * zeroed (but not freed as tables -- ScriptPopFrame does that afterward).
 * The struct offsets 0x28/0x2C/0x84 (added and stored back) and the block
 * from 0x44 to 0xA9 (zeroed via two CpuFill calls, a u32 store, and two u16
 * stores) fall inside struct ScriptFrame's unknown044[0x66] and the tail
 * end of resourceName; their meaning is not recovered, so they are
 * addressed here as raw offsets rather than invented field names. The
 * final two stores (frame pointer, then 0) land in
 * VM->state->dispatchState/dispatchIndex -- the same fields ScriptPopFrame
 * overwrites with 0/-1 immediately after this returns, so they have no
 * visible effect at the one real call site, but are included since they
 * are genuinely what the ROM does.
 *
 * Logically confirmed correct (every read/write offset and free accounted
 * for) and matches instruction-for-instruction *except* inside table03C's
 * loop body: the ROM reads the slot's count into r0, precomputes the next
 * slot's address into r1 (saved to r8), and -- once past the `count > 1`
 * gate -- loads the slot's `data` pointer directly into r4 for the inner
 * loop. Every C shape tried for that one loop (named locals for the count
 * and/or the next-slot pointer in various declaration/statement orders;
 * the count read as a bare `entry->count` in the `if` versus hoisted to a
 * local first; the loop written as `for` with the advance in the header
 * versus a `while` with an explicit `next` pointer and the advance at the
 * bottom) reproduces the surrounding tables038 loop and the rest of this
 * function exactly, but for this one loop always either swaps which of
 * count/next-slot lands in r0 vs r1, or adds an extra register-to-register
 * copy (`adds r4, r0, #0`) before the data pointer reaches r4 -- sometimes
 * both, sometimes trading one register mismatch for a new stack spill
 * instead. One more instance of GCC 2.9's allocator not being steerable
 * from plain C for this shape, in the same family as the documented
 * SpriteResourceFindGroup/OR-vs-AND register ties, not a logic error. The
 * table038 loop just above it, structurally identical apart from not
 * having the inner per-element loop, matches exactly.
 */

#include "gba/types.h"
#include "script_vm.h"

#define VM (*(struct ScriptContext **)0x0300611C)
extern void HeapFree(void *, void *);
extern void CpuFill(void *, u32, u32);

struct ScriptFramePoolEntry {
    u32 count;
    void *data;
};

void ScriptFrameReleasePools(void)
{
    struct ScriptFrame *frame = VM->state->frame;
    struct ScriptFramePoolEntry *entry;
    s32 i;
    u8 *raw;

    if (!frame)
        return;

    entry = frame->table038;
    for (i = 0; i < frame->count034; i++, entry++) {
        if (entry->count > 1)
            HeapFree(VM->state->heap, entry->data);
        entry->count = 0;
        entry->data = 0;
    }

    entry = frame->table03C;
    for (i = 0; i < frame->count036; i++, entry++) {
        if (entry->count > 1) {
            void **array = entry->data;
            u32 j;
            for (j = 0; j < entry->count; j++) {
                if (array[j])
                    HeapFree(VM->state->heap, array[j]);
            }
        }
        if (entry->data)
            HeapFree(VM->state->heap, entry->data);
        entry->count = 0;
        entry->data = 0;
    }

    raw = (u8 *)frame;
    *(u32 *)(raw + 0x44) = 0;
    {
        void *p0 = raw + 0x48;
        void *p1 = raw + 0x88;
        void *p2 = raw + 0xA8;
        void *p3 = raw + 0xAA;
        void *p4 = raw + 0x84;
        CpuFill(p0, 64, 0);
        CpuFill(p1, 32, 0);
        *(u16 *)p2 = 0;
        *(u16 *)p3 = 0;
        *(u32 *)p4 = *(u32 *)(raw + 0x28) + *(u32 *)(raw + 0x2C);
    }

    VM->state->dispatchState = (u32)frame;
    VM->state->dispatchIndex = 0;
}
