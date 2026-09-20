#include "sound.h"
#include "task_manager.h"

extern void SoundFadeTask(struct EngineTask *task);
extern void ScriptAddPendingTasks(u32 count);
extern void sub_08080BE8(void *task);

struct SoundFadeData
{
    s32 countdown;
    s32 playerIndex;
    s32 completePendingOnFinish;
    s32 stopPlayerOnFinish;
};

/**
 * @brief Create a script-visible sound fade task.
 * @param countdown Fade duration consumed by the task callback.
 * @param playerIndex Sound player to fade.
 * @param completePendingOnFinish Whether completion releases an extra script wait.
 * @param stopPlayerOnFinish Whether the player stops after fading.
 * @param completion Optional task completion word.
 * @return The initialized task, or null when allocation fails.
 *
 * The exact ROM keeps the callback in sl while it stages the allocation size
 * through r1. agbcc instead passes the same call in fewer instructions from
 * this natural source. Keep this readable candidate here until the original
 * source lifetime that explains that preservation is recovered.
 */
struct EngineTask *CreateSoundFadeTask(
    s32 countdown, s32 playerIndex, s32 completePendingOnFinish,
    s32 stopPlayerOnFinish, u32 *completion)
{
    struct EngineTask *task;
    struct SoundFadeData *data;

    task = CreateTask(&gMainTaskManager, SoundFadeTask, 0, completion,
                      sizeof(*data));
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
    sub_08080BE8(task);
    return task;
}
