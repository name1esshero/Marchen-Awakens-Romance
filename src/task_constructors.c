/* Small task constructors recovered from the engine front ends. Task headers
 * are 32 bytes; state offsets below are therefore expressed from the returned
 * allocation exactly as the callbacks consume them.
 */
#include "gba/types.h"

#include "runtime_state.h"
#include "rom_section.h"
#include "task_constructors.h"

extern const char gBattleNamedTaskAResourceName[];
extern const char gBattleNamedTaskBResourceName[];
extern u8 *CreateTask(void *manager, void *callback, u32 priority,
                      s32 *result, u32 stateSize);
extern u8 gMainTaskManager;
extern u8 gAuxTaskManager;
extern void ScriptAddPendingTasks(s32 count);
extern char *strcpy(char *, const char *);
extern void sub_08080BD8(void *task);
extern void sub_08080BDC(void *task);
extern void sub_08080BD4(void *task);
extern void sub_0805615C(void);
extern void RuntimeSetFlagC0(u32, s32);
extern s32 SpriteResourceFindGroup(s32, const char *);
extern void VramFillTask(void *task);
extern void ScriptSpriteResetTask(void *task);
extern void ScriptSpriteResetAllTask(void *task);
extern void sub_080053B4(void *task);
extern void sub_08006078(void *task);
extern void sub_08006228(void *task);
extern void sub_0800E7EC(void *task);
extern void sub_08025754(void *task);
extern void sub_080261A4(void *task);
extern void sub_0802761C(void *task);
extern void sub_080278F4(void *task);
extern void sub_0802A118(void *task);
extern void sub_08030218(void *task);
extern void sub_08007178(void *task);
extern void sub_0800D924(void *task);
extern void sub_0800E6D8(void *task);
extern void sub_0800ED5C(void *task);
extern void sub_0800FCC8(void *task);
extern void sub_0801B83C(void *task);
extern void sub_0801B8EC(void *task);
extern void sub_0806F018(void *task);
extern void sub_0806F664(void *task);

extern void sub_0806FAC4(void *task);
extern void ScriptCompletePendingTasks(u32 count);
extern void sub_08011718(s32 mode, s32 enabled);
extern void FinishTask(void *task);
extern void *CreateFieldEventTask(s16 first, s16 second, s16 third,
    s16 fourth, s16 value, void *objectData, u32 *completion);

struct MapCoordinateTaskData
{
    s16 mode;
    s16 result;
    s16 coordinate;
    u16 unused06;
    u32 eventCompletion;
};

struct MapCoordinateTask
{
    u8 unknown00[14];
    u16 stage;
    u8 unknown10[8];
    s32 *completion;
    u8 unknown1C[4];
    struct MapCoordinateTaskData data;
};

enum
{
    MAP_COORDINATE_MODE_0,
    MAP_COORDINATE_MODE_1,
    MAP_COORDINATE_MODE_2,
    MAP_COORDINATE_MODE_3,
    MAP_COORDINATE_MODE_4,
    MAP_FIELD_EVENT_X = 204,
    MAP_FIELD_EVENT_Y = 92
};

static void MapCoordinateTask(void *rawTask);

/** Schedule an asynchronous VRAM fill on the aux task manager.
 * @param destination VRAM address to fill.
 * @param value Fill value.
 * @param size Number of bytes to fill.
 * @return Nothing. */
AT("000037D8") void ScheduleVramFillTask(void *destination, u32 value, u32 size)
{
    u8 *task = CreateTask(&gAuxTaskManager, (void *)((u32)VramFillTask + 1),
                          0, 0, 12);
    u8 *state = task + 32;
    *(void **)(task + 32) = destination;
    *(u32 *)(state + 4) = value;
    *(u32 *)(state + 8) = size;
}

/** Schedule a task that waits for the given keys, marking one script wait
 * pending if the task is created.
 * @param keyMask Keys to wait for.
 * @param repeatMask Keys eligible for repeat while held.
 * @param result Optional task completion word.
 * @return Always 0x7fff, regardless of whether the task was created. */
AT("00005378") s32 CreateInputWaitTask(s32 keyMask, s32 repeatMask,
                                        s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_080053B4,
                          0, result, 4);
    if (task != 0) {
        *(u16 *)(task + 32) = keyMask;
        *(u16 *)(task + 34) = repeatMask;
        ScriptAddPendingTasks(1);
    }
    return 0x7fff;
}

/** Create the main scene task.
 * @param result Optional task completion word.
 * @return The new task, or NULL if creation fails. */
AT("00006050") u8 *CreateSceneTask(s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_08006078,
                          1, result, 88);
    if (task == 0)
        return 0;
    return task;
}

#ifdef NONMATCHING
AT("000061F0") u8 *CreateSceneModeTask(s32 mode, s32 *result)
{
    u8 *task;
    mode = (s16)mode;
    task = CreateTask(&gMainTaskManager, sub_08006228,
                      0, result, 112);
    if (task == 0)
        goto failed;
    *(u16 *)(task + 142) = (s16)mode;
    goto done;
failed:
    task = 0;
done:
    return task;
}
#endif

/** Create a battle actor's motion task on its owner's per-actor task slot.
 * @param valueA Stored at +120; meaning unresolved.
 * @param owner Battle actor slot; selects the task manager and is recorded
 * at task byte 113.
 * @param valueB Stored at +114; meaning unresolved.
 * @param resource Motion resource, stored at state +72.
 * @param context Motion context, stored at state +76.
 * @param result Optional task completion word.
 * @return The new task. */
AT("000275C4") u8 *CreateBattleMotionTask(s32 valueA, s32 owner, s32 valueB,
                                           void *resource, void *context,
                                           s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_0802761C, 0, result, 104);
    u8 *state = task + 32;
    *(u16 *)(task + 120) = valueA;
    *(u16 *)(task + 114) = valueB;
    *(void **)(state + 72) = resource;
    *(void **)(state + 76) = context;
    task[113] = owner;
    return task;
}

extern s32 RuntimeObjectGetField1A(s32 owner, s32 slot);
extern s32 RuntimeActorGetField352(s32 owner);
/** Create a battle actor-tracking task, snapshotting the actor's current
 * +0x1A field and +0x352 field into the task state at creation time.
 * @param owner Battle actor slot; selects the task manager.
 * @param slot Stored at task byte 72.
 * @param unused Not read by this constructor.
 * @param result Optional task completion word.
 * @return The new task. */
AT("000256F8") u8 *CreateBattleTrackingTask(s32 owner, s32 slot,
                                             s32 unused, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_08025754, 0, result, 52);
    u8 *state = task + 32;
    task[71] = owner;
    task[72] = slot;
    *(s32 *)(state + 44) = (s16)RuntimeObjectGetField1A((s8)task[71],
                                                        (s8)task[72]);
    *(s32 *)(state + 48) = RuntimeActorGetField352(owner);
    return task;
}

#ifdef NONMATCHING
AT("0001097C") u8 *CreateSpriteResetTask(s32 sprite, s32 mode, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + 64, (void *)((u32)ScriptSpriteResetTask + 1),
                          0, result, 8);
    *(s32 *)(task + 32) = sprite;
    *(s32 *)(task + 36) = mode;
    ScriptAddPendingTasks(1);
    if (mode == 0)
        sub_08080BDC(task);
    return task;
}
#endif

/** Create a battle resource task on a per-owner, per-slot task manager.
 * @param owner Battle actor slot; selects the task manager together with
 * slot.
 * @param slot Sub-slot within the owner's task managers.
 * @param resource Resource pointer stored at state +16.
 * @param result Set to 0 on success, -1 if creation fails.
 * @return The new task, or NULL if creation fails. */
AT("000301BC") u8 *CreateBattleResourceTask(s32 owner, s32 slot,
                                             void *resource, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                          sub_08030218, 0, result, 24);
    if (task == 0) {
        if (result)
            *result = -1;
        return 0;
    }
    {
        u8 *state = task + 32;
        *(s32 *)(task + 32) = owner;
        *(s32 *)(state + 4) = slot;
        *(void **)(state + 16) = resource;
    }
    if (result)
        *result = 0;
    return task;
}

/** Create a battle display-object task on the owner's per-actor task slot.
 * @param owner Battle actor slot; selects the task manager and whether this
 * is the player side (owner == 0).
 * @param slot Stored at state +4; also folded into the +12 id (owner*4+slot).
 * @param resource Resource pointer stored at state +28.
 * @param result Optional task completion word, set to -1 if creation fails.
 * @return The new task, or NULL if creation fails. */
AT("0002A0B8") u8 *CreateBattleObjectTask(s32 owner, s32 slot,
                                           void *resource, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_0802A118, 0, result, 32);
    if (task == 0) {
        if (result)
            *result = -1;
        return 0;
    }
    {
        u8 *state = task + 32;
        *(s32 *)(task + 32) = owner;
        *(s32 *)(state + 4) = slot;
        *(s32 *)(state + 8) = owner == 0;
        *(u16 *)(state + 12) = owner * 4 + slot;
        *(void **)(state + 28) = resource;
    }
    return task;
}

#ifdef NONMATCHING
AT("00027894") u8 *CreateBattleNamedTaskA(s32 owner, s32 slot,
                                           s32 unused, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_080278F4, 0, result, 116);
    task[104] = owner;
    task[106] = slot;
    task[105] = owner == 0;
    task[107] = 0;
    *(u16 *)(task + 112) = SpriteResourceFindGroup(1, gBattleNamedTaskAResourceName);
    return task;
}
#endif

#ifdef NONMATCHING
AT("00026140") u8 *CreateBattleNamedTaskB(s32 owner, s32 slot,
                                           s32 unused, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_080261A4, 0, result, 240);
    task[251] = owner;
    task[253] = slot;
    task[252] = owner == 0;
    task[254] = 0;
    *(u16 *)(task + 258) = SpriteResourceFindGroup(1, gBattleNamedTaskBResourceName);
    return task;
}
#endif

/** Create a field-effect task on the owner's per-actor task slot and mark
 * one script wait pending.
 * @param owner Field object slot; selects the task manager.
 * @param slot Stored at state +4.
 * @param a Stored at state +8.
 * @param b Stored at state +20.
 * @param c Stored at state +12.
 * @param d Stored at state +24.
 * @param result Optional task completion word.
 * @return The new task. */
AT("0000E788") u8 *CreateFieldEffectTask(s32 owner, s32 slot, s32 a, s32 b,
                                          s32 c, s32 d, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_0800E7EC, 0, result, 32);
    u8 *state = task + 32;
    *(s32 *)(task + 32) = owner;
    *(s32 *)(state + 4) = slot;
    *(s32 *)(state + 8) = a;
    *(s32 *)(state + 16) = 0;
    *(s32 *)(state + 20) = b;
    *(s32 *)(state + 12) = c;
    *(s32 *)(state + 24) = d;
    ScriptAddPendingTasks(1);
    return task;
}

#ifdef NONMATCHING
AT("0000ECF8") u8 *CreateFieldCommandTask(s32 owner, s32 a, s32 b, s32 c,
                                           s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_0800ED5C, 0, result, 16);
    u8 *state = task + 32;
    *(s32 *)(task + 32) = owner;
    *(s32 *)(state + 4) = a;
    *(s32 *)(state + 8) = b;
    *(s32 *)(state + 12) = c;
    ScriptAddPendingTasks(1);
    sub_08080BD4(task);
    return task;
}
#endif

#define CREATE_PENDING_TASK(address, name, manager, callback)              \
AT(address) u8 *name(s32 value, s32 *result)                               \
{                                                                          \
    u8 *task = CreateTask((manager), (callback), 0, result, 4);             \
    *(s32 *)(task + 32) = value;                                            \
    ScriptAddPendingTasks(1);                                               \
    return task;                                                            \
}

CREATE_PENDING_TASK("0000D8EC", StartPendingEffectB,
                    gSecondaryRuntime + 64, sub_0800D924)
CREATE_PENDING_TASK("0000E6A0", StartPendingFieldEffect,
                    gSecondaryRuntime + value * 32, sub_0800E6D8)
CREATE_PENDING_TASK("0000FC90", StartPendingEffectA,
                    gSecondaryRuntime + 64, sub_0800FCC8)

/** Create a runtime task carrying a copied name string and two values.
 * @param name Copied into the task's inline name buffer at state +12.
 * @param value Stored at state +8.
 * @param other Stored at state +36.
 * @param result Optional task completion word.
 * @return The new task. */
AT("00007134") u8 *CreateNamedRuntimeTask(const char *name, s32 value,
                                           s32 other, s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_08007178,
                          0, result, 44);
    u8 *state = task + 32;
    *(s32 *)(state + 8) = value;
    *(s32 *)(state + 36) = other;
    strcpy((char *)state + 12, name);
    return task;
}

#ifdef NONMATCHING
AT("00010A2C") u8 *CreateSpriteWaitTask(s32 mode, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + 64, (void *)((u32)ScriptSpriteResetAllTask + 1),
                          0, result, 8);
    *(s32 *)(task + 36) = mode;
    ScriptAddPendingTasks(1);
    if (mode == 0)
        sub_08080BD8(task);
    return task;
}
#endif

/** Create the encounter-transition task and set the secondary runtime's
 * 0x0F24 flag, marking one script wait pending.
 * @param result Optional task completion word.
 * @return The new task. */
AT("0006F620") u8 *CreateEncounterTransitionTask(s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_0806F664,
                          0, result, 32);
    *(u16 *)(gSecondaryRuntime + 0x0F24) = 1;
    ScriptAddPendingTasks(1);
    return task;
}

#ifdef NONMATCHING
AT("0006EFCC") u8 *CreateEncounterResetTask(s32 *result)
{
    s32 i;
    u8 *task = CreateTask(&gMainTaskManager, sub_0806F018,
                          0, result, 0x3F34);
    for (i = 0; i <= 3; i++)
        RuntimeSetFlagC0(i, 0);
    ScriptAddPendingTasks(1);
    return task;
}
#endif

#ifdef NONMATCHING
AT("0001B7FC") u8 *CreateObjectMotionTaskA(u8 *object, s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_0801B83C,
                          0, result, 16);
    u8 *state = task + 32;
    *(u8 **)(task + 32) = object;
    *(u16 *)(object + 24) = 120;
    *(u16 *)(object + 26) = -32;
    *(u16 *)(state + 4) = -32;
    *(u16 *)(state + 8) = 24;
    return task;
}
#endif

#ifdef NONMATCHING
AT("0001B8AC") u8 *CreateObjectMotionTaskB(u8 *object, s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_0801B8EC,
                          0, result, 16);
    u8 *state = task + 32;
    *(u8 **)(task + 32) = object;
    object[39] |= 4;
    *(u16 *)(state + 4) = 256;
    *(u16 *)(state + 8) = 0;
    return task;
}
#endif

/** Create the map task that processes a signed tile-coordinate pair. */
AT("0006C7AC") struct EngineTask *CreateMapCoordinateTask(s16 mode,
    s16 coordinate, s32 *result)
{
    s32 signedMode = mode;
    s32 signedCoordinate = coordinate;
    u8 *task;
    task = CreateTask(&gMainTaskManager, MapCoordinateTask,
                      0, result, 16);
    *(u16 *)((u8 *)task + 32) = signedMode;
    *(u16 *)((u8 *)task + 36) = signedCoordinate;
    ScriptAddPendingTasks(1);
    return (struct EngineTask *)task;
}

/** Advance a map-coordinate command through field-event launch, wait, and
 * script-completion stages. */
AT("0006C7EC") static void MapCoordinateTask(void *rawTask)
{
    struct MapCoordinateTask *task = rawTask;
    struct MapCoordinateTaskData *data = &task->data;

    switch (task->stage) {
    case 0:
        switch (data->mode) {
        case MAP_COORDINATE_MODE_0:
            CreateFieldEventTask(4, MAP_FIELD_EVENT_X, MAP_FIELD_EVENT_Y,
                data->coordinate, 3, 0,
                &data->eventCompletion);
            break;
        case MAP_COORDINATE_MODE_1:
            CreateFieldEventTask(4, MAP_FIELD_EVENT_X, MAP_FIELD_EVENT_Y,
                data->coordinate, 3, 0,
                &data->eventCompletion);
            break;
        case MAP_COORDINATE_MODE_2:
            CreateFieldEventTask(3, MAP_FIELD_EVENT_X, MAP_FIELD_EVENT_Y,
                data->coordinate, 3, 0,
                &data->eventCompletion);
            break;
        case MAP_COORDINATE_MODE_3:
            CreateFieldEventTask(2, MAP_FIELD_EVENT_X, MAP_FIELD_EVENT_Y,
                data->coordinate, 4, 0,
                &data->eventCompletion);
            break;
        case MAP_COORDINATE_MODE_4:
            CreateFieldEventTask(5, MAP_FIELD_EVENT_X, MAP_FIELD_EVENT_Y,
                data->coordinate, 2, 0,
                &data->eventCompletion);
            break;
        }
        task->stage = 1;
        break;
    case 1:
        if (data->eventCompletion != 0) {
            data->result = data->eventCompletion;
            task->stage = 2;
        }
        break;
    case 2:
        ScriptCompletePendingTasks(1);
        sub_08011718(0, 1);
        if (task->completion != 0)
            *task->completion = data->result;
        FinishTask(task);
        break;
    }
}

#ifdef NONMATCHING
AT("0006FA94") u8 *CreateEncounterSetupTask(s32 value, s32 *result)
{
    s32 savedValue = value;
    u8 *task = CreateTask(&gMainTaskManager, (void *)sub_0806FAC4,
                          0, result, 368);
    *(s32 *)(task + 368) = savedValue;
    return task;
}
#endif
