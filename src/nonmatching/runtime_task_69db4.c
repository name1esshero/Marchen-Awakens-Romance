/* sub_08069DB4 creates a 0x7038-byte task whose worker is sub_08069E00.
 *
 * The task stores the supplied object at +0x11CC, records a constant 20 in
 * the object's +20 halfword, and stores object +0xA90 at task +0x11D0. The
 * worker is still assembly, so the object and task fields retain offsets
 * rather than speculative structure members.
 *
 * This ordinary C accurately describes the observed data flow but does not
 * match yet. agbcc coalesces the two task-relative addresses and replaces
 * the ROM's reload through +0x11CC with the original object argument. The
 * original instead materializes +0x11CC and +0x11D0 independently and
 * reloads the object slot between them. Keep the canonical assembly until
 * decoding sub_08069E00 exposes a source-level relationship that naturally
 * yields that shape; do not use volatile, register pinning, or inline asm.
 */

#include "gba/types.h"
#include "ncd.h"
#include "task_manager.h"

extern void sub_08069E00(struct EngineTask *);

#define RUNTIME_TASK_69DB4_PAYLOAD_SIZE 0x7038
#define RUNTIME_TASK_69DB4_OBJECT_OFFSET 0x11CC
#define RUNTIME_TASK_69DB4_TARGET_OFFSET 0x11D0
#define RUNTIME_TASK_69DB4_OBJECT_STATE_OFFSET 20
#define RUNTIME_TASK_69DB4_OBJECT_TARGET_OFFSET 0xA90

u8 *CreateRuntimeTask69DB4(u8 *object, u32 *completion)
{
    u8 *task = (u8 *)CreateTask(&gMainTaskManager, sub_08069E00, 0,
                                 completion, RUNTIME_TASK_69DB4_PAYLOAD_SIZE);
    u8 **objectSlot = (u8 **)(task + RUNTIME_TASK_69DB4_OBJECT_OFFSET);
    u8 **targetSlot;

    *objectSlot = object;
    *(u16 *)(object + RUNTIME_TASK_69DB4_OBJECT_STATE_OFFSET) = 20;
    targetSlot = (u8 **)(task + RUNTIME_TASK_69DB4_TARGET_OFFSET);
    *targetSlot = *objectSlot + RUNTIME_TASK_69DB4_OBJECT_TARGET_OFFSET;
    return task;
}
