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
 * This natural reconstruction has the correct behavior and size, but agbcc
 * assigns three live values to different registers from the original ROM.
 * The exact symbolic implementation remains in assembly until the original
 * source lifetimes are recovered.
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
