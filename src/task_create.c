/* Allocate a task header plus zeroed caller payload. The allocation-failure
    * handler remains assembly; preserve its original 0x00600000 argument. */
#include "task_manager.h"
#define AT(x) __attribute__((section(".rom." x)))
extern void CpuFill(void *,u32,u32);
extern void sub_0807A4B8(void *);
AT("0007A688")
struct EngineTask *TaskCreateInQueue(struct TaskManager *manager,void (*callback)(struct EngineTask *),u32 priority,u32 *completion,u32 size)
{
    struct EngineTask *task;
    u32 totalSize;
    task = HeapAlloc(manager->heap,totalSize = size + 32);
    if (!task) {
        sub_0807A4B8((void *)0x00600000);
        return 0;
    }
    CpuFill(task,totalSize,0);
    ListAppend(&manager->queues[priority],&task->node);
    task->callback = callback;
    task->priority = priority;
    task->completion = completion;
    task->manager = manager;
    task->state = 1;
    if (completion)
        *completion = 0;
    manager->taskCount++;
    return task;
}
AT("0007A688") const u8 TaskCreateInQueueTail[2]={0,0};
AT("0007A700")
struct EngineTask *TaskCreateBefore(struct TaskManager *manager,void (*callback)(struct EngineTask *),struct EngineTask *at,u32 *completion,u32 size)
{
    struct EngineTask *task;
    u32 totalSize;
    task = HeapAlloc(manager->heap,totalSize = size + 32);
    if (!task) {
        sub_0807A4B8((void *)0x00600000);
        return 0;
    }
    CpuFill(task,totalSize,0);
    ListInsertBefore(&manager->queues[at->priority],&at->node,&task->node);
    task->callback = callback;
    task->priority = at->priority;
    task->completion = completion;
    task->manager = manager;
    task->state = 1;
    if (completion)
        *completion = 0;
    manager->taskCount++;
    return task;
}
