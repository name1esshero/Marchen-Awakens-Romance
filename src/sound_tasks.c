/* Script-visible sound transitions run as engine tasks so dialogue can wait
 * for a fade to finish. */
#include "game_state.h"
#include "sound.h"
#include "task_manager.h"

#define AT(x) __attribute__((section(".rom." x)))

extern struct SoundPlayerEntry gSoundPlayerTable[];
extern struct SoundSongEntry gSongTable[];
extern void sub_0807915C(struct SoundPlayer *player, const void *song);
extern void ScriptCompletePendingTasks(u32 count);

enum {
    SOUND_FADE_RUNNING = 0x1000
};

struct SoundFadeTaskData {
    s32 countdown;
    s32 playerIndex;
    s32 completePendingOnFinish;
    s32 stopPlayerOnFinish;
};

struct SoundWaitTaskData {
    u8 header00[24];
    s32 *completion;
    u8 header1C[8];
    s32 playerIndex;
};

/* Complete a script wait once the selected sound player becomes idle. */
AT("0000581C") void SoundWaitTask(struct SoundWaitTaskData *task)
{
    if (GameStateGetField425C(task->playerIndex) != 0)
        return;

    ScriptCompletePendingTasks(1);
    if (task->completion != 0)
        *task->completion = -1;
    FinishTask((struct EngineTask *)task);
}

/* This task family stores a 16-bit phase at +0E rather than using the generic
 * byte-sized state field at +0C. Its task-specific data begins at +20. */
struct SoundFadeTask {
    u8 header00[14];
    u16 phase;
    u8 header10[8];
    s32 *completion;
    u8 header1C[4];
    struct SoundFadeTaskData data;
};

AT("00005720") void SoundFadeTask(struct SoundFadeTask *task)
{
    struct SoundFadeTaskData *data = &task->data;

    if (task->phase != 0) {
        if (task->phase == SOUND_FADE_RUNNING)
            goto processFade;
        goto done;
    }
    if (GameStateGetField425C(data->playerIndex) != 0)
        goto done;

    GameStateSetField425C(data->playerIndex, 1);
    ScriptCompletePendingTasks(1);
    SoundFadeOut(gSoundPlayerTable[data->playerIndex].player,
                 (u16)(data->countdown << 12 >> 16));
    task->phase = SOUND_FADE_RUNNING;

processFade:
    data->countdown--;
    if (data->countdown >= 0)
        goto done;
    if (data->completePendingOnFinish)
        ScriptCompletePendingTasks(1);
    if (data->stopPlayerOnFinish)
        SoundPlayerStop(gSoundPlayerTable[data->playerIndex].player);
    GameStateSetField425C(data->playerIndex, 0);
    if (task->completion)
        *task->completion = -1;
    FinishTask((struct EngineTask *)task);

done:
    return;
}

struct SoundStartTaskData {
    s32 delay;
    s32 playerIndex;
    s32 songIndex;
};

struct SoundStartTask {
    u8 header00[14];
    u16 phase;
    u8 header10[8];
    s32 *completion;
    u8 header1C[4];
    struct SoundStartTaskData data;
};

/* Wait four scheduler ticks before installing a song in its selected player,
 * then finish on the following callback. */
AT("00005AC4") void SoundStartTask(struct SoundStartTask *task)
{
    struct SoundStartTaskData *data = &task->data;

    if (task->phase != 0) {
        if (task->phase == 16)
            goto finish;
        goto done;
    }
    data->delay++;
    if (data->delay <= 4)
        goto done;
    sub_0807915C(gSoundPlayerTable[data->playerIndex].player,
                 gSongTable[data->songIndex].header);
    task->phase = 16;
    goto done;

finish:
    ScriptCompletePendingTasks(1);
    if (task->completion)
        *task->completion = -1;
    FinishTask((struct EngineTask *)task);

done:
    return;
}
AT("00005AC4") const u8 SoundStartTaskTail[2] = {0};
