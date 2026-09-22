#include "sound.h"
#include "task_manager.h"

#include "rom_section.h"

extern void SoundFadeTask(struct EngineTask *task);
extern void ScriptAddPendingTasks(u32 count);

struct SoundFadeData
{
    s32 countdown;
    s32 playerIndex;
    s32 completePendingOnFinish;
    s32 stopPlayerOnFinish;
};

/**
 * @brief Create a script-visible sound fade task.
 * @return The initialized task, or null when allocation fails.
 *
 * Assigning the callback as the CreateTask argument records the same function
 * for the immediate first update.  agbcc consequently preserves it across the
 * constructor in sl and emits the ROM's existing bx-sl call veneer naturally.
 */
AT("000056AC") struct EngineTask *CreateSoundFadeTask(
    s32 countdown, s32 playerIndex, s32 completePendingOnFinish,
    s32 stopPlayerOnFinish, u32 *completion)
{
    struct EngineTask *task;
    struct SoundFadeData *data;
    void (*callback)(struct EngineTask *);

    task = CreateTask(&gMainTaskManager, callback = SoundFadeTask, 0,
                      completion, sizeof(*data));
    if (task == 0)
        return 0;

    data = (struct SoundFadeData *)(task + 1);
    if (completePendingOnFinish != 0)
        ScriptAddPendingTasks(1);
    ScriptAddPendingTasks(1);
    data->countdown = countdown;
    data->playerIndex = playerIndex;
    data->completePendingOnFinish = completePendingOnFinish;
    data->stopPlayerOnFinish = stopPlayerOnFinish;
    callback(task);
    return task;
}
