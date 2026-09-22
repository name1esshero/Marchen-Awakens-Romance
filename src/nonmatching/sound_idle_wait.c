#include "sound.h"
#include "task_manager.h"

extern void ScriptAddPendingTasks(u32 count);
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
 * Behavior, size, and three of four register assignments now match exactly
 * (`index` in r6, `wait` in r4, the returned task in r4 reused after
 * `CreateTask`, and `callback` surviving in r5 for the immediate run --
 * `sub_08080BD4` is agbcc's own `bx r5` indirect-call veneer, not a real
 * function; calling the already-computed `callback` local through it
 * instead of the raw trampoline name is what fixed all three of those at
 * once, the same fix that resolved CreateSpriteResetTask and
 * CreateSoundFadeTask earlier this session). What remains is a single
 * instruction: the ROM loads the player's status in place
 * (`ldr r0, [r0, #4]`, overwriting the same register that already held the
 * player pointer from the jump-table case body), while every C shape tried
 * here loads it into a fresh register instead (`ldr r1, [r0, #4]`) --
 * tried with `player` as its own pointer variable and without one (folding
 * the switch to assign `status` directly per case, which agbcc still
 * correctly merges back into one shared load site), with `status` typed
 * `u32` and `s32`, and with the merge point reached from every case in the
 * jump table. None recovers the in-place overwrite. This is the same class
 * of narrow, register-choice-only gap as `sub_08070DA8`
 * (`GeneratedMapStartGeneration`) and `SpriteAffineWriteDispatch`'s
 * register swap elsewhere in this project -- not a logic error.
 */
struct EngineTask *CreateSoundPlayerIdleWait(
    u32 playerIndex, u32 wait, s32 *result, u32 *completion)
{
    struct SoundPlayer *player;
    u32 status;
    u32 index;
    void (*callback)(struct EngineTask *);
    struct EngineTask *task;

    index = playerIndex;
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
    if (wait == 0) {
        status >>= 31;
        status ^= 1;
        *result = status;
        return 0;
    }
    callback = SoundPlayerIdleTask;
    task = CreateTask(&gMainTaskManager, (void *)callback, 0, completion, 12);
    ((struct SoundIdleTaskRecord *)task)->playerIndex = index;
    ScriptAddPendingTasks(1);
    callback(task);
    return task;
}
