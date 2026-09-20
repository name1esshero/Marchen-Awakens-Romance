#include "sound.h"
#include "task_manager.h"

extern void ScriptAddPendingTasks(u32 count);
extern void sub_08080BD4(void *task);
extern void SoundPlayerIdleTask(struct EngineTask *task);

struct SoundIdleTaskRecord
{
    u8 header[36];
    s32 playerIndex;
};

/**
 * @brief Query a sound player or create a task that waits for it to become idle.
 * @param playerIndex Index in the engine's nine-player set.
 * @param wait Nonzero to create an asynchronous wait task.
 * @param result Receives the immediate busy result when no task is created.
 * @param completion Optional task completion word.
 * @return The wait task, or null for an immediate result or inactive player.
 *
 * This natural reconstruction has the correct behavior and size. The precise
 * mechanism: the ROM preserves three values across the whole function
 * (`push {r4,r5,r6,lr}`) -- `index` in r6 for its entire lifetime (from the
 * top down to `((SoundIdleTaskRecord*)waitValue)->playerIndex = index;`,
 * which is reached after the `bl CreateTask` that clobbers r0-r3), `wait` in
 * r4 (later reused for the returned task pointer), and a transient scratch
 * in r5 used only within the task-creation branch for CreateTask's manager/
 * callback arguments. This candidate's `index` and the switch's per-player
 * pointer temporary do not have overlapping live ranges (the switch's use
 * ends before `index` is read again), so agbcc coalesces them into the same
 * register (r5) and needs only two preserved registers (`push {r4,r5,lr}`)
 * instead of three. The ROM's original source apparently did not permit that
 * coalescing. This looks like a genuine compiler liveness-analysis choice
 * rather than an instruction-order or expression-grouping difference, and
 * none of the shapes in this project's playbook (statement splitting,
 * declaration order, combined expressions) target that kind of difference;
 * deliberately extending a variable's live range purely to prevent a
 * coalescing the optimizer would otherwise make is compiler-steering of the
 * same kind PRET_STANDARDS.md forbids elsewhere in this project (the
 * rejected SpriteAffineAllocate/ScriptResourceSet register-forcing unions),
 * so it was not attempted here either; a genuine fix would need to come
 * from recovering why the ROM's original source kept these three values
 * separate, not from a source-level trick aimed at the register allocator.
 * The exact symbolic implementation remains in assembly until that is
 * found.
 */
struct EngineTask *CreateSoundPlayerIdleWait(
    u32 playerIndex, u32 wait, s32 *result, u32 *completion)
{
    struct SoundPlayer *player;
    u32 status;
    u32 index;
    u32 waitValue;
    void (*callback)(struct EngineTask *);
    struct EngineTask *returnTask;

    index = playerIndex;
    waitValue = wait;
    switch (index) {
    case 0: player = &gSoundPlayer0; break;
    case 1: player = &gSoundPlayer1; break;
    case 2: player = &gSoundPlayer2; break;
    case 3: player = &gSoundPlayer3; break;
    case 4: player = &gSoundPlayer4; break;
    case 5: player = &gSoundPlayer5; break;
    case 6: player = &gSoundPlayer6; break;
    case 7: player = &gSoundPlayer7; break;
    case 8: player = &gSoundPlayer8; break;
    default: player = &gSoundPlayer0; break;
    }

    status = player->status;
    if ((s32)status < 0) {
        *result = 0;
        return 0;
    }
    if (waitValue != 0) {
        callback = SoundPlayerIdleTask;
        waitValue = (u32)CreateTask(&gMainTaskManager, callback, 0,
                                    completion, 12);
        ((struct SoundIdleTaskRecord *)waitValue)->playerIndex = index;
        ScriptAddPendingTasks(1);
        sub_08080BD4((void *)waitValue);
        returnTask = (struct EngineTask *)waitValue;
    } else {
        status >>= 31;
        status ^= 1;
        *result = status;
        returnTask = 0;
    }
    return returnTask;
}
