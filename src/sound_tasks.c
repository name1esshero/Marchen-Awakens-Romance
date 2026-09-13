/* Script-visible sound transitions run as engine tasks so dialogue can wait
 * for a fade to finish. */
#include "game_state.h"
#include "sound.h"
#include "task_manager.h"

#define AT(x) __attribute__((section(".rom." x)))

extern void ScriptCompletePendingTasks(u32 count);
extern void ScriptAddPendingTasks(u32 count);

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

/* A second wait form reads MusicPlayer2000's status word directly.  The
 * signed high bit marks an idle or paused player. */
AT("00005920") void SoundPlayerIdleTask(struct SoundWaitTaskData *task)
{
    struct SoundPlayer *player;

    switch (task->playerIndex) {
    case 0: player = (struct SoundPlayer *)0x03005F30; break;
    case 1: player = (struct SoundPlayer *)0x03005FB0; break;
    case 2: player = (struct SoundPlayer *)0x03005FF0; break;
    case 3: player = (struct SoundPlayer *)0x030060C0; break;
    case 4: player = (struct SoundPlayer *)0x03006030; break;
    case 5: player = (struct SoundPlayer *)0x03005EB0; break;
    case 6: player = (struct SoundPlayer *)0x03005EF0; break;
    case 7: player = (struct SoundPlayer *)0x03005F70; break;
    case 8: player = (struct SoundPlayer *)0x03006080; break;
    default: player = (struct SoundPlayer *)0x03005F30; break;
    }

    if ((s32)player->status < 0) {
        ScriptCompletePendingTasks(1);
        if (task->completion != 0)
            *task->completion = -1;
        FinishTask((struct EngineTask *)task);
    }
}

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

void SoundStartTask(struct SoundStartTask *task);

/* Start immediately when the selected player is idle.  When it is already
 * active, stop it and schedule SoundStartTask so scripts can wait through the
 * short transition. */
AT("000059C8") struct EngineTask *StartSongWithTransition(
    u32 playerIndex, u32 songIndex, u32 *completion)
{
    struct SoundPlayer *player;
    struct EngineTask *task;
    struct SoundStartTaskData *data;

    switch (playerIndex) {
    case 0: player = (struct SoundPlayer *)0x03005F30; break;
    case 1: player = (struct SoundPlayer *)0x03005FB0; break;
    case 2: player = (struct SoundPlayer *)0x03005FF0; break;
    case 3: player = (struct SoundPlayer *)0x030060C0; break;
    case 4: player = (struct SoundPlayer *)0x03006030; break;
    case 5: player = (struct SoundPlayer *)0x03005EB0; break;
    case 6: player = (struct SoundPlayer *)0x03005EF0; break;
    case 7: player = (struct SoundPlayer *)0x03005F70; break;
    case 8: player = (struct SoundPlayer *)0x03006080; break;
    default: player = (struct SoundPlayer *)0x03005F30; break;
    }

    if ((s32)player->status >= 0) {
        SoundPlayerStop(gSoundPlayerTable[playerIndex].player);
        task = CreateTask((struct TaskManager *)0x030032C4,
                          (void (*)(struct EngineTask *))SoundStartTask,
                          0, completion, sizeof(*data));
        if (task != 0) {
            data = (struct SoundStartTaskData *)((u8 *)task + 32);
            ScriptAddPendingTasks(1);
            data->playerIndex = playerIndex;
            data->songIndex = songIndex;
            return task;
        }
    } else {
        SoundPlayerStart(gSoundPlayerTable[playerIndex].player,
                         gSongTable[songIndex].header);
    }
    return 0;
}

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
    SoundPlayerStart(gSoundPlayerTable[data->playerIndex].player,
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

AT("00006034") void ResumeSoundPlayer(u32 playerIndex)
{
    SoundResumePlayer(gSoundPlayerTable[playerIndex].player);
}
