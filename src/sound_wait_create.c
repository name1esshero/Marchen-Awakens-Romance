#include "game_state.h"
#include "task_manager.h"
/* The original source reused argument registers across CreateTask.  Keeping
 * those lvalues preserves agbcc's allocation while the control flow stays C. */
#define AT(x) __attribute__((section(".rom." x)))
extern void SoundWaitTask(struct EngineTask *task);
extern void ScriptAddPendingTasks(s32 count);
extern void sub_08080BD4(void *task);
struct SoundWaitRecord { u8 header[36]; s32 playerIndex; };
/* Return the current busy state immediately, or create a script-visible wait
 * task when the caller requests asynchronous completion. */
AT("000057C0") struct EngineTask *CreateSoundWaitTask(
    s32 playerIndex, s32 wait, s32 *result, s32 *completion)
{
    s32 status = GameStateGetField425C(playerIndex);
    if (status != 0) {
        if (wait != 0) {
            void *manager = &gMainTaskManager;
            result = (s32 *)SoundWaitTask;
            wait = (s32)CreateTask((struct TaskManager *)manager,
                                   (void (*)(struct EngineTask *))result,
                                   0, (u32 *)completion, 12);
            ((struct SoundWaitRecord *)wait)->playerIndex = playerIndex;
            ScriptAddPendingTasks(1);
            sub_08080BD4((void *)wait);
            return (struct EngineTask *)wait;
        }
        status = GameStateGetField425C(playerIndex);
    }
    *result = status;
    return 0;
}
AT("000057C0") const u8 CreateSoundWaitTaskTail[2] = {0};
