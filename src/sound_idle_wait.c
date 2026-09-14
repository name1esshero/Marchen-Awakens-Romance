#include "sound.h"
#include "task_manager.h"

#include "rom_section.h"

extern void ScriptAddPendingTasks(u32 count);
extern void sub_08080BD4(void *task);
extern void SoundPlayerIdleTask(struct EngineTask *task);
extern struct SoundPlayer gSoundPlayer0, gSoundPlayer1;

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
 */
AT("00005848") struct EngineTask *CreateSoundPlayerIdleWait(
    u32 playerIndex, u32 wait, s32 *result, u32 *completion)
{
    struct SoundPlayer *player;
    register u32 status asm("r0");
    u32 index;
    register u32 waitValue asm("r4");
    register void (*callback)(struct EngineTask *) asm("r5");
    register struct EngineTask *returnTask asm("r0");

    index = playerIndex;
    waitValue = wait;
    switch (index) {
    case 0: player = &gSoundPlayer0; break;
    case 1: player = &gSoundPlayer1; break;
    case 2: player = (struct SoundPlayer *)0x03005FF0; break;
    case 3: player = (struct SoundPlayer *)0x030060C0; break;
    case 4: player = (struct SoundPlayer *)0x03006030; break;
    case 5: player = (struct SoundPlayer *)0x03005EB0; break;
    case 6: player = (struct SoundPlayer *)0x03005EF0; break;
    case 7: player = (struct SoundPlayer *)0x03005F70; break;
    case 8: player = (struct SoundPlayer *)0x03006080; break;
    default: player = &gSoundPlayer0; break;
    }

    status = player->status;
    if ((s32)status < 0) {
        *result = 0;
        return 0;
    }
    if (waitValue != 0) {
        struct TaskManager *manager = &gMainTaskManager;
        callback = SoundPlayerIdleTask;
        waitValue = (u32)CreateTask(manager, callback, 0, completion, 12);
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

asm(".section .rom.00005848,\"ax\",%progbits\n.space 2, 0\n");
