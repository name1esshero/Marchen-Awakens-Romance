/* Small task constructors.  The address suffix remains until the callbacks'
 * gameplay roles are known; their queue, work-area size, and payload layout
 * are fully represented here. */
#include "task_adapters.h"
#include "ncd.h"

#include "runtime_accessors.h"
#include "runtime_state.h"
#include "rom_section.h"

extern void sub_08007908(struct EngineTask *);
extern void sub_0806C918(struct EngineTask *);
extern void sub_0806E76C(struct EngineTask *);
extern void sub_0806EF18(struct EngineTask *);
extern void sub_0800D690(struct EngineTask *);
extern void sub_0800F11C(struct EngineTask *);
extern void sub_08010730(struct EngineTask *);
extern void sub_080114B0(struct EngineTask *);
extern void sub_0800B1EC(struct EngineTask *);
extern void sub_0800D9C0(struct EngineTask *);
extern void sub_0800FD70(struct EngineTask *);
extern void sub_0800F9EC(struct EngineTask *);
extern void ActorPartInitTask(struct EngineTask *);
extern void sub_0800B49C(struct EngineTask *);
extern void sub_0801BB0C(struct EngineTask *);
extern void sub_08051EB8(struct EngineTask *);
extern void sub_08062384(struct EngineTask *);
extern void sub_080660B0(struct EngineTask *);
extern void sub_08068408(struct EngineTask *);
extern void sub_08068C3C(struct EngineTask *);
extern void sub_0806EB44(struct EngineTask *);
extern void sub_0805615C(void);
extern s16 sub_080565F8(s32 first, s32 second, void *resource);
extern void ScriptAddPendingTasks(u32 count);
extern void CpuCopy(const void *source,void *destination,u32 size);

enum
{
    TASK_B478_STATE_SIZE = 256,
    TASK_1BAD8_STATE_SIZE = 0x61C,
    TASK_1BAD8_VALUE_OFFSET = 0x5E8,
    TASK_51E84_STATE_SIZE = 0x7E8,
    TASK_51E84_VALUE_OFFSET = 0x7A4,
    TASK_62304_STATE_SIZE = 0x25F4,
    TASK_62304_OWNER_STAGE = 9,
    TASK_62304_OWNER_RESOURCE_OFFSET = 0xA90,
    TASK_62304_MAX_SELECTION = 5,
    TASK_62304_FALLBACK_SELECTION = 6,
    TASK_66068_STATE_SIZE = 0xC40,
    TASK_66068_OWNER_VARIANT_MASK = 0xC0,
    TASK_66068_OWNER_VARIANT_STAGE = 28,
    TASK_66068_OWNER_DEFAULT_STAGE = 12,
    TASK_683C4_STATE_SIZE = 0x238,
    TASK_683C4_OWNER_STAGE = 15,
    TASK_68C0C_STATE_SIZE = 0xAD0,
    TASK_6EAFC_STATE_SIZE = 0xB24,
    TASK_ADAPTER_GAME_STATE_MODE_OFFSET = 0x38B8,
    TASK_ADAPTER_GAME_STATE_MODE_2 = 2
};

struct TaskAdapterRuntime
{
    struct TaskManager mainTaskManager;
    u8 unknown010[0xD08];
    u8 *gameState;
};

struct TaskAdapterOwner
{
    u8 unknown00[20];
    u16 stage;
    u8 unknown16[2];
    u8 flags18;
};

struct Task66068State
{
    u8 unknown00[0xC30];
    struct TaskAdapterOwner *owner;
    u8 unknownC34[12];
};

struct Task683C4State
{
    u8 unknown00[0x218];
    struct TaskAdapterOwner *owner;
    u16 encounterValue;
    u8 unknown21E[0x1A];
};

struct Task62304State
{
    u8 unknown0000[0x2494];
    struct TaskAdapterOwner *owner;
    void *resource;
    u8 unknown249C[0x144];
    s16 selection;
    s16 fallbackSelection;
    u8 unknown25E4[0x10];
};

/** Create a 76-byte main-manager task running sub_08007908(). No payload
 * fields are set here beyond the task header. */
AT("000078E4") struct EngineTask *CreateTask078E4(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_08007908,0,completion,76);
}
/** Create a large (0x4A14-byte) main-manager task running sub_0806C918().
 * No payload fields are set here beyond the task header. */
AT("0006C8F0") struct EngineTask *CreateTask6C8F0(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_0806C918,0,completion,0x4A14);
}
/** Create a 20-byte main-manager task running sub_0806E76C(). No payload
 * fields are set here beyond the task header. */
AT("0006E748") struct EngineTask *CreateTask6E748(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_0806E76C,0,completion,20);
}
/** Create a large (0x3F34-byte) main-manager task running sub_0806EF18().
 * No payload fields are set here beyond the task header. */
AT("0006EEF0") struct EngineTask *CreateTask6EEF0(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_0806EF18,0,completion,0x3F34);
}

/** Create the main-manager task handled by sub_0800B49C(). */
AT("0000B478") struct EngineTask *CreateTaskB478(u32 *completion)
{
    return CreateTask(&gMainTaskManager, sub_0800B49C, 0, completion,
                      TASK_B478_STATE_SIZE);
}

/** Create the task handled by sub_0801BB0C() and retain its owner value. */
AT("0001BAD8") struct EngineTask *CreateTask1BAD8(u32 value, u32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, sub_0801BB0C, 0, completion,
        TASK_1BAD8_STATE_SIZE);

    *(u32 *)((u8 *)task + TASK_1BAD8_VALUE_OFFSET) = value;
    return task;
}

/** Create the task handled by sub_08051EB8() and retain its owner value. */
AT("00051E84") struct EngineTask *CreateTask51E84(u32 value, u32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, sub_08051EB8, 0, completion,
        TASK_51E84_STATE_SIZE);

    *(u32 *)((u8 *)task + TASK_51E84_VALUE_OFFSET) = value;
    return task;
}

/**
 * @brief Create the task handled by sub_08062384() and choose its resource.
 */
AT("00062304")
struct EngineTask *CreateTask62304(struct TaskAdapterOwner *owner,
                                   u32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, sub_08062384, 0, completion,
        TASK_62304_STATE_SIZE);
    struct Task62304State *state = (struct Task62304State *)(task + 1);

    state->owner = owner;
    owner->stage = TASK_62304_OWNER_STAGE;
    state->resource =
        (u8 *)state->owner + TASK_62304_OWNER_RESOURCE_OFFSET;
    state->selection = sub_080565F8(0, 1, state->resource);
    if (state->selection <= TASK_62304_MAX_SELECTION)
        state->fallbackSelection = state->selection;
    else
        state->fallbackSelection = TASK_62304_FALLBACK_SELECTION;
    return task;
}

/**
 * @brief Create the task handled by sub_080660B0() and attach its owner.
 *
 * Two high owner flags select the initial stage used by the worker.
 */
AT("00066068")
struct EngineTask *CreateTask66068(struct TaskAdapterOwner *owner,
                                   u32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, sub_080660B0, 0, completion,
        TASK_66068_STATE_SIZE);
    struct Task66068State *state = (struct Task66068State *)(task + 1);

    state->owner = owner;
    if (owner->flags18 & TASK_66068_OWNER_VARIANT_MASK)
        owner->stage = TASK_66068_OWNER_VARIANT_STAGE;
    else
        owner->stage = TASK_66068_OWNER_DEFAULT_STAGE;
    return task;
}
AT("00066068") const u8 CreateTask66068Tail[2] = {0, 0};

/**
 * @brief Create the task handled by sub_08068408() and cache encounter state.
 */
AT("000683C4")
struct EngineTask *CreateTask683C4(struct TaskAdapterOwner *owner,
                                   u32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, sub_08068408, 0, completion,
        TASK_683C4_STATE_SIZE);
    struct Task683C4State *state = (struct Task683C4State *)(task + 1);

    state->owner = owner;
    owner->stage = TASK_683C4_OWNER_STAGE;
    state->encounterValue = GameStateGetEncounterValue();
    return task;
}

/** Create the task handled by sub_08068C3C() and reset shared runtime state. */
AT("00068C0C") struct EngineTask *CreateTask68C0C(u32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, sub_08068C3C, 0, completion,
        TASK_68C0C_STATE_SIZE);

    sub_0805615C();
    return task;
}

/** Create the task handled by sub_0806EB44() and select game-state mode 2. */
AT("0006EAFC")
struct EngineTask *CreateTask6EAFC(u32 *completion)
{
    struct TaskAdapterRuntime *runtime =
        (struct TaskAdapterRuntime *)&gMainTaskManager;
    struct EngineTask *task = CreateTask(
        &runtime->mainTaskManager, sub_0806EB44, 0, completion,
        TASK_6EAFC_STATE_SIZE);

    runtime->gameState[TASK_ADAPTER_GAME_STATE_MODE_OFFSET] =
        TASK_ADAPTER_GAME_STATE_MODE_2;
    sub_0805615C();
    return task;
}

/** Create an 84-byte actor task (queue 1 of the actor at +64) running
 * sub_0800D690(), storing value at payload +72. */
AT("0000D664") struct EngineTask *CreateActorTaskD664(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+64),sub_0800D690,1,completion,84);
 *(u32 *)((u8 *)task+104)=value;
 return task;
}
/** Create a 20-byte actor task (queue 1 of the actor at +96) running
 * sub_0800F11C(), storing value at payload +0. */
AT("0000F0F0") struct EngineTask *CreateActorTaskF0F0(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+96),sub_0800F11C,1,completion,20);
 *(u32 *)((u8 *)task+32)=value;
 return task;
}
/** Create a 12-byte actor task (queue 1 of the actor at +64) running
 * sub_08010730(), storing value at payload +0. */
AT("00010704") struct EngineTask *CreateActorTask10704(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+64),sub_08010730,1,completion,12);
 *(u32 *)((u8 *)task+32)=value;
 return task;
}
/** Create a 12-byte actor task (queue 1 of the actor at +64) running
 * sub_080114B0(), storing value at payload +0. */
AT("00011484") struct EngineTask *CreateActorTask11484(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+64),sub_080114B0,1,completion,12);
 *(u32 *)((u8 *)task+32)=value;
 return task;
}

/** Schedule an asynchronous memory copy on the aux task manager, running
 * CopyThenFinishTask() with the copy's arguments as its payload.
 * @return Nothing. */
AT("00003784") void CreateCopyTask(void *destination,void *source,u32 size)
{
 struct EngineTask *task=CreateTask(&gAuxTaskManager,CopyThenFinishTask,0,0,12);
 u8 *payload=(u8 *)task+32;
 *(void **)payload=destination;
 *(void **)(payload+4)=source;
 *(u32 *)(payload+8)=size;
}
/** Create a 40-byte main-manager task running sub_0800B1EC(), storing three
 * u16 values at payload +0/+2/+4. */
AT("0000B1B8") struct EngineTask *CreateCoordinateTask(u32 first,u32 second,u32 third,u32 *completion)
{
 struct EngineTask *task=CreateTask(&gMainTaskManager,sub_0800B1EC,0,completion,40);
 u8 *payload=(u8 *)task+32;
 *(u16 *)payload=first;
 *(u16 *)(payload+2)=second;
 *(u16 *)(payload+4)=third;
 return task;
}
/** Create a 4-byte actor task (queue 0 of the actor at +64) running
 * sub_0800D9C0() and mark one script wait pending. */
AT("0000D98C") struct EngineTask *CreatePendingTaskD98C(u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+64),sub_0800D9C0,0,completion,4);
 ScriptAddPendingTasks(1);
 return task;
}
/** Create a 4-byte actor task (queue 0 of the actor at +64) running
 * sub_0800FD70() and mark one script wait pending. */
AT("0000FD3C") struct EngineTask *CreatePendingTaskFD3C(u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+64),sub_0800FD70,0,completion,4);
 ScriptAddPendingTasks(1);
 return task;
}
/** Create an 84-byte actor task (queue 1 of the actor at +64) running
 * sub_0800F9EC(), storing value at payload +72 and resetting the payload's
 * embedded NCD sprite container. */
AT("0000F9B4") struct EngineTask *CreateSpriteTaskF9B4(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+64),sub_0800F9EC,1,completion,84);
 u8 *payload=(u8 *)task+32;
 *(u32 *)(payload+72)=value;
 NcdSpriteContainerReset(payload);
 return task;
}
/** Create an 8-byte actor task (queue 1 of the actor at index*32) running
 * ActorPartInitTask(), storing index and value at payload +0/+4, and mark
 * one script wait pending. */
AT("0000E610") struct EngineTask *CreateIndexedPendingTask(u32 index,u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+index*32),ActorPartInitTask,1,completion,8);
 *(u32 *)((u8 *)task+32)=index;
 *(u32 *)((u8 *)task+36)=value;
 ScriptAddPendingTasks(1);
 return task;
}

/** CreateCopyTask()'s worker: copy the payload's destination/source/size,
 * then finish the task. @return Nothing. */
AT("000037B8") void CopyThenFinishTask(struct EngineTask *task)
{
 u8 *payload=(u8 *)task+32;
 CpuCopy(*(void **)payload,*(void **)(payload+4),*(u32 *)(payload+8));
 FinishTask(task);
}
AT("000037B8") const u8 CopyThenFinishTaskTail[2]={0};
