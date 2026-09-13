/* Small task constructors.  The address suffix remains until the callbacks'
 * gameplay roles are known; their queue, work-area size, and payload layout
 * are fully represented here. */
#include "task_adapters.h"
#include "ncd.h"

#define AT(x) __attribute__((section(".rom." x)))
#define SECONDARY_RUNTIME (*(u8 **)0x03004020)

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

AT("000078E4") struct EngineTask *CreateTask078E4(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_08007908,0,completion,76);
}
AT("0006C8F0") struct EngineTask *CreateTask6C8F0(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_0806C918,0,completion,0x4A14);
}
AT("0006E748") struct EngineTask *CreateTask6E748(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_0806E76C,0,completion,20);
}
AT("0006EEF0") struct EngineTask *CreateTask6EEF0(u32 *completion)
{
 return CreateTask(&gMainTaskManager,sub_0806EF18,0,completion,0x3F34);
}

AT("0000D664") struct EngineTask *CreateActorTaskD664(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+64),sub_0800D690,1,completion,84);
 *(u32 *)((u8 *)task+104)=value;
 return task;
}
AT("0000F0F0") struct EngineTask *CreateActorTaskF0F0(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+96),sub_0800F11C,1,completion,20);
 *(u32 *)((u8 *)task+32)=value;
 return task;
}
AT("00010704") struct EngineTask *CreateActorTask10704(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+64),sub_08010730,1,completion,12);
 *(u32 *)((u8 *)task+32)=value;
 return task;
}
AT("00011484") struct EngineTask *CreateActorTask11484(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+64),sub_080114B0,1,completion,12);
 *(u32 *)((u8 *)task+32)=value;
 return task;
}

AT("00003784") void CreateCopyTask(void *source,void *destination,u32 size)
{
 struct EngineTask *task=CreateTask(&gAuxTaskManager,CopyThenFinishTask,0,0,12);
 u8 *payload=(u8 *)task+32;
 *(void **)payload=source;
 *(void **)(payload+4)=destination;
 *(u32 *)(payload+8)=size;
}
AT("0000B1B8") struct EngineTask *CreateCoordinateTask(u32 first,u32 second,u32 third,u32 *completion)
{
 struct EngineTask *task=CreateTask(&gMainTaskManager,sub_0800B1EC,0,completion,40);
 u8 *payload=(u8 *)task+32;
 *(u16 *)payload=first;
 *(u16 *)(payload+2)=second;
 *(u16 *)(payload+4)=third;
 return task;
}
AT("0000D98C") struct EngineTask *CreatePendingTaskD98C(u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+64),sub_0800D9C0,0,completion,4);
 ScriptAddPendingTasks(1);
 return task;
}
AT("0000FD3C") struct EngineTask *CreatePendingTaskFD3C(u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+64),sub_0800FD70,0,completion,4);
 ScriptAddPendingTasks(1);
 return task;
}
AT("0000F9B4") struct EngineTask *CreateSpriteTaskF9B4(u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+64),sub_0800F9EC,1,completion,84);
 u8 *payload=(u8 *)task+32;
 *(u32 *)(payload+72)=value;
 NcdSpriteContainerReset(payload);
 return task;
}
AT("0000E610") struct EngineTask *CreateIndexedPendingTask(u32 index,u32 value,u32 *completion)
{
 struct EngineTask *task=CreateTask((struct TaskManager *)(SECONDARY_RUNTIME+index*32),sub_0800E64C,1,completion,8);
 *(u32 *)((u8 *)task+32)=index;
 *(u32 *)((u8 *)task+36)=value;
 ScriptAddPendingTasks(1);
 return task;
}

AT("000037B8") void CopyThenFinishTask(struct EngineTask *task)
{
 u8 *payload=(u8 *)task+32;
 CpuCopy(*(void **)payload,*(void **)(payload+4),*(u32 *)(payload+8));
 FinishTask(task);
}
AT("000037B8") const u8 CopyThenFinishTaskTail[2]={0};
