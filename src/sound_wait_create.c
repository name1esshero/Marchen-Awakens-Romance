#include "game_state.h"
#include "task_manager.h"
#include "rom_section.h"
extern void SoundWaitTask(struct EngineTask *task);
extern void ScriptAddPendingTasks(s32 count);
struct SoundWaitRecord { u8 header[36]; s32 playerIndex; };
/** Return the current busy state immediately, or create a script-visible wait
 * task when the caller requests asynchronous completion. */
AT("000057C0") struct EngineTask *CreateSoundWaitTask(
    s32 playerIndex, s32 wait, s32 *result, s32 *completion)
{
    struct TaskManager *manager;
    void (*callback)(struct EngineTask *);
    struct EngineTask *task;
    s32 status;

    status = GameStateGetField425C(playerIndex);
    if (status != 0) {
        if (wait != 0) {
            manager = &gMainTaskManager;
            callback = SoundWaitTask;
            task = CreateTask(manager, callback, 0, (u32 *)completion, 12);
            ((struct SoundWaitRecord *)task)->playerIndex = playerIndex;
            ScriptAddPendingTasks(1);
            callback(task);
            return task;
        }
        status = GameStateGetField425C(playerIndex);
    }
    *result = status;
    return 0;
}
AT("000057C0") const u8 CreateSoundWaitTaskTail[2] = {0};
