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
 * The precise mechanism: the ROM's `CreateTask` call setup uses r1 as a
 * scratch register for the stack-passed size argument (`movs r1,#16;
 * str r1,[sp]`) before overwriting r1 with the callback pointer
 * (`mov r1, sl`) for the actual call -- so the callback pointer, loaded
 * earlier, has to survive that scratch use somewhere else, and the ROM
 * preserves it in sl (r10) for that window. This candidate's compiled
 * output picks r2 as the stack-argument scratch instead of r1 (confirmed
 * with the size argument pulled into its own local, declared before the
 * call, which changes nothing), so the callback pointer never needs to
 * move out of r1 in the first place and sl is never touched -- one fewer
 * high register preserved across the whole function (`push {r6,r7}` for
 * r8/r9 only, not the ROM's `push {r5,r6,r7}` for r8/r9/sl). Like
 * sound_idle_wait.c's index/switch-temp coalescing, this looks like a
 * genuine difference in how many registers the compiler decides it needs,
 * not an instruction-order or expression-grouping question; forcing r1 to
 * be used as the scratch would mean steering the allocator rather than
 * recovering the original source shape. Keep this readable candidate here
 * until the original source lifetime that explains the ROM's choice is
 * recovered.
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
