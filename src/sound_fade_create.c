#include "sound.h"
#include "task_manager.h"

#include "rom_section.h"

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

AT("000056AC") struct EngineTask *CreateSoundFadeTask(
    s32 countdown, s32 playerIndex, s32 completePendingOnFinish,
    s32 stopPlayerOnFinish, u32 *completion)
{
    struct EngineTask *task;
    struct SoundFadeData *data;
    register void (*callback)(struct EngineTask *) asm("r10");
    struct TaskManager *manager;

    manager = &gMainTaskManager;
    callback = SoundFadeTask;
    task = CreateTask(manager, callback, 0, completion, 16);
    if (task == 0)
        return 0;

    data = (struct SoundFadeData *)((u8 *)task + 32);
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
