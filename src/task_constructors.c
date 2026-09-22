/* Small task constructors recovered from the engine front ends. Task headers
 * are 32 bytes; state offsets below are therefore expressed from the returned
 * allocation exactly as the callbacks consume them.
 */
#include "gba/types.h"

#include "dialogue.h"
#include "input.h"
#include "runtime_state.h"
#include "rom_section.h"
#include "task_constructors.h"

#define SPRITE_RESET_MANAGER_OFFSET 64
#define SPRITE_RESET_TASK_SIZE 8
#define SPRITE_RESET_TASK_SPRITE_OFFSET 32
#define SPRITE_RESET_TASK_MODE_OFFSET 36

extern const char gBattleNamedTaskAResourceName[];
extern const char gBattleNamedTaskBResourceName[];
extern const char gResource77A03[];
extern const char gResourceSpBa04[];
extern const char gResourceTestE02[];
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
extern void sub_08007410(void *task);
extern void sub_08076C5C(void *task);
extern void sub_08077584(void *task);
extern void sub_0800D924(void *task);
extern void sub_0800E6D8(void *task);
extern void sub_0800ED5C(void *task);
extern void sub_0800FCC8(void *task);
extern void sub_0801B83C(void *task);
extern void sub_0801B8EC(void *task);
extern void sub_08065EB4(void *task);
extern void sub_08069E00(void *task);
extern void sub_0806F018(void *task);
extern void sub_0806F664(void *task);

extern void sub_0806FAC4(void *task);
extern void ScriptCompletePendingTasks(u32 count);
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

struct InputWaitTask
{
    u8 unknown00[24];
    s32 *completion;
    u8 unknown1C[4];
    s16 inputSlot;
    u16 keyMask;
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

/* State owned by the field-effect task created at 0x08077370. The unknown
 * spans are retained until its callback is decompiled; the known members are
 * named from the constructor's writes and resource lookups. */
struct BattleSpriteEffectTaskState
{
    u8 unknown00[84];
    u32 value54;
    s32 owner;
    u32 value5C;
    s32 resourceGroup60;
    s32 resourceGroup64;
    s32 x;
    s32 y;
    u8 unknown70[52];
    u32 valueA4;
};

struct LargeBattleSpriteEffectTaskState
{
    u8 unknown00[84];
    u32 value54;
    s32 owner;
    u32 value5C;
    s32 resourceGroup60;
    s32 resourceGroup64;
    s32 x;
    s32 y;
    u8 unknown70[712];
};

/* Known fields of two battle tasks whose callbacks still own the preceding
 * state. Offsets are from the complete EngineTask allocation. */
struct BattleNamedTaskA
{
    u8 unknown00[104];
    u8 owner;
    u8 isPrimary;
    u8 slot;
    u8 reset;
    u8 unknown108[4];
    u16 resourceGroup;
};

struct BattleNamedTaskB
{
    u8 unknown00[251];
    u8 owner;
    u8 isPrimary;
    u8 slot;
    u8 reset;
    u8 unknown255[3];
    u16 resourceGroup;
};

struct MotionObject
{
    u8 unknown00[24];
    s16 field18;
    s16 field1A;
    u8 unknown1C[11];
    u8 flags27;
};

struct ObjectMotionTaskState
{
    struct MotionObject *object;
    s16 value04;
    u16 unknown06;
    s16 value08;
};

enum
{
    MOTION_OBJECT_FLAGS_OFFSET = 39,
    MOTION_OBJECT_FLAG_2 = 1 << 2
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

enum
{
    NAMED_RUNTIME_RECORD_TASK_STATE_SIZE = 32,
    NAMED_RUNTIME_RECORD_TASK_NAME_OFFSET = 36
};

enum
{
    RUNTIME_TASK_69DB4_PAYLOAD_SIZE = 0x7038,
    RUNTIME_TASK_69DB4_OWNER_STATE_OFFSET = 20,
    RUNTIME_TASK_69DB4_OWNER_NEXT_STATE = 20,
    RUNTIME_TASK_69DB4_OWNER_SLOT = 0x11CC,
    RUNTIME_TASK_69DB4_TARGET_SLOT = 0x11D0,
    RUNTIME_TASK_69DB4_OWNER_TARGET = 0xA90
};

/* The constructor accepts a parent scene-work block and creates a child task.
 * Both allocations are accessed through this partial union so the parent
 * state write may alias the child task's stored parent pointer. This matches
 * the ROM's required reload without volatile access or compiler hints. */
union RuntimeTask69DB4State
{
    struct
    {
        u8 unknown0000[RUNTIME_TASK_69DB4_OWNER_STATE_OFFSET];
        u16 state;
    } owner;
    struct
    {
        u8 unknown0000[RUNTIME_TASK_69DB4_OWNER_SLOT];
        union RuntimeTask69DB4State *owner;
        u8 *target;
    } child;
};

static void MapCoordinateTask(void *rawTask);
static void InputWaitTask(void *rawTask);

/** Schedule an asynchronous VRAM fill on the aux task manager.
 * @param destination VRAM address to fill.
 * @param size Number of bytes to fill.
 * @param value Fill value.
 * @return Nothing. */
AT("000037D8") void ScheduleVramFillTask(void *destination, u32 size, u32 value)
{
    u8 *task = CreateTask(&gAuxTaskManager, (void *)((u32)VramFillTask + 1),
                          0, 0, 12);
    u8 *state = task + 32;
    *(void **)(task + 32) = destination;
    *(u32 *)(state + 4) = size;
    *(u32 *)(state + 8) = value;
}

/** Schedule a task that waits for the given keys, marking one script wait
 * pending if the task is created.
 * @param inputSlot Key-input state slot to inspect.
 * @param keyMask Keys to consume from the slot's pressed state.
 * @param result Optional task completion word.
 * @return Always 0x7fff, regardless of whether the task was created. */
AT("00005378") s32 CreateInputWaitTask(s32 inputSlot, s32 keyMask,
                                        s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, InputWaitTask,
                          0, result, 4);
    if (task != 0) {
        *(u16 *)(task + 32) = inputSlot;
        *(u16 *)(task + 34) = keyMask;
        ScriptAddPendingTasks(1);
    }
    return 0x7fff;
}

/** Wait until a requested key is pressed, then complete the script wait and
 * release the task. */
AT("000053B4") static void InputWaitTask(void *rawTask)
{
    struct InputWaitTask *task = rawTask;

    if (KeyInputConsumePressed(task->keyMask, task->inputSlot))
    {
        ScriptCompletePendingTasks(1);
        if (task->completion != 0)
            *task->completion = -1;
        FinishTask(rawTask);
    }
}

/**
 * @brief Create a large scene child task and bind it to its parent work block.
 * @param owner Parent scene-work block. Its state changes to 20.
 * @param completion Optional completion word owned by the task system.
 * @return The newly allocated child task.
 */
AT("00069DB4")
u8 *CreateRuntimeTask69DB4(union RuntimeTask69DB4State *owner,
                           s32 *completion)
{
    union RuntimeTask69DB4State *task =
        (union RuntimeTask69DB4State *)CreateTask(
            &gMainTaskManager, sub_08069E00, 0, completion,
            RUNTIME_TASK_69DB4_PAYLOAD_SIZE);

    task->child.owner = owner;
    owner->owner.state = RUNTIME_TASK_69DB4_OWNER_NEXT_STATE;
    task->child.target = (u8 *)task->child.owner
        + RUNTIME_TASK_69DB4_OWNER_TARGET;
    return (u8 *)task;
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

/**
 * @brief Create the runtime task handled by sub_08007410 and copy its name.
 * @param resourceName Name stored at the start of the task-specific payload.
 * @param completion Optional completion word owned by the task system.
 * @return The newly allocated task.
 */
AT("000073DC") u8 *CreateNamedRuntimeRecordTask(const char *resourceName,
                                                 s32 *completion)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_08007410, 0, completion,
                          NAMED_RUNTIME_RECORD_TASK_STATE_SIZE);

    strcpy((char *)task + NAMED_RUNTIME_RECORD_TASK_NAME_OFFSET, resourceName);
    return task;
}

/**
 * @brief Create a field-event mode task.
 * @param mode Signed 16-bit mode stored in the task state.
 * @param completion Optional completion word owned by the task system.
 * @return The newly allocated task.
 */
AT("00065E88") u8 *CreateFieldEventModeTask(s16 mode, s32 *completion)
{
    u8 *task;
    s32 signedMode = mode;

    task = CreateTask(&gMainTaskManager, sub_08065EB4, 0, completion, 32);
    *(u16 *)(task + 52) = signedMode;
    return task;
}

/**
 * Create the field-effect task that uses the 77A03 and SP_BA04 sprite groups.
 * Positions arrive as 16.16 fixed-point values and are stored as pixels.
 */
AT("00077370") u8 *CreateBattleSpriteEffectTask(s32 *completion, u32 value54,
    s32 owner, u32 value5C, s32 x, s32 y, u32 valueA4)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_08076C5C, 0, completion,
                          sizeof(struct BattleSpriteEffectTaskState));
    struct BattleSpriteEffectTaskState *state = (void *)(task + 32);

    state->value54 = value54;
    state->owner = owner;
    state->value5C = value5C;
    state->x = x >> 16;
    state->y = y >> 16;
    state->valueA4 = valueA4;
    state->resourceGroup60 = SpriteResourceFindGroup(2, gResource77A03);
    state->resourceGroup64 = SpriteResourceFindGroup(1, gResourceSpBa04);
    return task;
}

/**
 * Create the larger battle sprite-effect task that pairs the 77A03 character
 * graphics with the TEST_E02 effect group. Positions are 16.16 fixed-point.
 */
AT("00077B44") u8 *CreateLargeBattleSpriteEffectTask(s32 *completion,
    u32 value54, s32 owner, u32 value5C, s32 x, s32 y)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          sub_08077584, 0, completion,
                          sizeof(struct LargeBattleSpriteEffectTaskState));
    struct LargeBattleSpriteEffectTaskState *state = (void *)(task + 32);

    state->value54 = value54;
    state->owner = owner;
    state->value5C = value5C;
    state->x = x >> 16;
    state->y = y >> 16;
    state->resourceGroup60 = SpriteResourceFindGroup(2, gResource77A03);
    state->resourceGroup64 = SpriteResourceFindGroup(1, gResourceTestE02);
    return task;
}

/** Create a scene-mode task and store its signed mode in the task state. */
AT("000061F0") u8 *CreateSceneModeTask(s16 mode, s32 *result)
{
    s32 signedMode = mode;
    u8 *task = CreateTask(&gMainTaskManager, sub_08006228,
                          0, result, 112);

    if (task == 0)
        return 0;
    *(u16 *)(task + 142) = signedMode;
    return task;
}
AT("000061F0") const u8 CreateSceneModeTaskTail[2] = {0};

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

/** Create an 8-byte sprite-reset task running ScriptSpriteResetTask().
 * @param sprite Sprite index, stored at task +32.
 * @param mode Stored at task +36; mode 0 also runs the task's callback
 * immediately instead of waiting for the scheduler.
 * @param result Optional task completion word.
 * @return The new task. */
AT("0001097C") u8 *CreateSpriteResetTask(s32 sprite, s32 mode, s32 *result)
{
    void *manager;
    void (*callback)(void *);
    u8 *task;

    manager = gSecondaryRuntime + SPRITE_RESET_MANAGER_OFFSET;
    callback = ScriptSpriteResetTask;
    task = CreateTask(manager, (void *)callback, 0, result,
                      SPRITE_RESET_TASK_SIZE);
    *(s32 *)(task + SPRITE_RESET_TASK_SPRITE_OFFSET) = sprite;
    *(s32 *)(task + SPRITE_RESET_TASK_MODE_OFFSET) = mode;
    ScriptAddPendingTasks(1);
    if (mode == 0)
        callback(task);
    return task;
}

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

/** Create the smaller named battle task and resolve its sprite group. */
AT("00027894") u8 *CreateBattleNamedTaskA(s32 owner, s32 slot,
                                           s32 unused, s32 *result)
{
    struct BattleNamedTaskA *task = (struct BattleNamedTaskA *)CreateTask(
        gSecondaryRuntime + owner * 32, sub_080278F4, 0, result, 116);

    task->owner = owner;
    task->slot = slot;
    task->isPrimary = owner == 0;
    task->reset = 0;
    task->resourceGroup = SpriteResourceFindGroup(
        1, gBattleNamedTaskAResourceName);
    return (u8 *)task;
}

/** Create the larger named battle task and resolve its sprite group. */
AT("00026140") u8 *CreateBattleNamedTaskB(s32 owner, s32 slot,
                                           s32 unused, s32 *result)
{
    struct BattleNamedTaskB *task = (struct BattleNamedTaskB *)CreateTask(
        gSecondaryRuntime + owner * 32, sub_080261A4, 0, result, 240);

    task->owner = owner;
    task->slot = slot;
    task->isPrimary = owner == 0;
    task->reset = 0;
    task->resourceGroup = SpriteResourceFindGroup(
        1, gBattleNamedTaskBResourceName);
    return (u8 *)task;
}

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

/**
 * @brief Create the encounter reset task and clear all four runtime flags.
 *
 * The flag index is narrowed to the engine's signed 16-bit index width after
 * each increment. agbcc represents that recurrence in the high halfword of
 * its induction register, matching the original routine without hints.
 *
 * @param result Optional task completion word.
 * @return The newly created reset task.
 */
AT("0006EFCC") u8 *CreateEncounterResetTask(s32 *result)
{
    s32 i;
    u8 *task = CreateTask(&gMainTaskManager, sub_0806F018,
                          0, result, 0x3F34);
    for (i = 0; i <= 3; i = (s16)(i + 1))
        RuntimeSetFlagC0(i, 0);
    ScriptAddPendingTasks(1);
    return task;
}

/** Create the first object-motion task and initialize its vertical movement. */
AT("0001B7FC") u8 *CreateObjectMotionTaskA(u8 *object, s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_0801B83C,
                          0, result, 16);
    struct ObjectMotionTaskState *state =
        (struct ObjectMotionTaskState *)(task + 32);
    struct MotionObject *motionObject = (struct MotionObject *)object;

    state->object = motionObject;
    motionObject->field18 = 120;
    motionObject->field1A = -32;
    state->value04 = -32;
    state->value08 = 24;
    return task;
}

/** Create the second object-motion task and enable motion flag 2. */
AT("0001B8AC") u8 *CreateObjectMotionTaskB(u8 *object, s32 *result)
{
    u8 *task = CreateTask(&gMainTaskManager, sub_0801B8EC,
                          0, result, 16);
    struct ObjectMotionTaskState *state =
        (struct ObjectMotionTaskState *)(task + 32);

    state->object = (struct MotionObject *)object;
    object[MOTION_OBJECT_FLAGS_OFFSET] |= MOTION_OBJECT_FLAG_2;
    state->value04 = 256;
    state->value08 = 0;
    return task;
}

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
        DialogueLoadWindowGraphics(0, TRUE);
        if (task->completion != 0)
            *task->completion = data->result;
        FinishTask(task);
        break;
    }
}

/** Create the encounter setup task and store its initial mode in the final
 * payload word. */
AT("0006FA94") u8 *CreateEncounterSetupTask(s32 value, s32 *result)
{
    s32 savedValue = value;
    u8 *task = CreateTask(&gMainTaskManager, (void *)sub_0806FAC4,
                          0, result, 368);
    u8 *state = task + 32;

    *(s32 *)(state + 336) = savedValue;
    return task;
}
