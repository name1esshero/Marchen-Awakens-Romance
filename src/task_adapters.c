/* Small task constructors.  The address suffix remains until the callbacks'
 * gameplay roles are known; their queue, work-area size, and payload layout
 * are fully represented here. */
#include "task_adapters.h"
#include "ncd.h"

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
extern void sub_0800E64C(struct EngineTask *);
extern void ScriptAddPendingTasks(u32 count);
extern void CpuCopy(const void *source,void *destination,u32 size);

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
AT("00003784") void CreateCopyTask(void *source,void *destination,u32 size)
{
 struct EngineTask *task=CreateTask(&gAuxTaskManager,CopyThenFinishTask,0,0,12);
 u8 *payload=(u8 *)task+32;
 *(void **)payload=source;
 *(void **)(payload+4)=destination;
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
 * sub_0800E64C(), storing index and value at payload +0/+4, and mark one
 * script wait pending. */
AT("0000E610") struct EngineTask *CreateIndexedPendingTask(u32 index,u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(gSecondaryRuntime+index*32),sub_0800E64C,1,completion,8);
 *(u32 *)((u8 *)task+32)=index;
 *(u32 *)((u8 *)task+36)=value;
 ScriptAddPendingTasks(1);
 return task;
}

/** CreateCopyTask()'s worker: copy the payload's source/destination/size,
 * then finish the task. @return Nothing. */
AT("000037B8") void CopyThenFinishTask(struct EngineTask *task)
{
 u8 *payload=(u8 *)task+32;
 CpuCopy(*(void **)payload,*(void **)(payload+4),*(u32 *)(payload+8));
 FinishTask(task);
}
AT("000037B8") const u8 CopyThenFinishTaskTail[2]={0};
