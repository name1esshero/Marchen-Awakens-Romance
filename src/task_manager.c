/* Priority-list storage for engine tasks. Destruction frees every queued node
 * and then the queue array; it does not run task callbacks or free the manager.
 * Initialization preserves the original lack of allocation-failure checks.
 */
#include "task_manager.h"
#include "rom_section.h"
extern void HeapFree(struct Heap *, void *);

/**
 * @brief Allocate a manager's priority queue array and initialize every
 * queue empty. Preserves the original lack of an allocation-failure check.
 * @param manager Manager to initialize.
 * @param heap Heap the queue array and future tasks are allocated from.
 * @param count Number of priority queues to allocate.
 * @return Nothing.
 */
AT("0007A5FC")
void TaskManagerInit(struct TaskManager *manager, struct Heap *heap, u32 count)
{
    u32 i;

    manager->queues = HeapAlloc(heap, count * 12);
    manager->queueCount = count;
    for (i = 0; i < count; i++)
        ListInit(&manager->queues[i]);
    manager->heap = heap;
    manager->taskCount = 0;
}
AT("0007A5FC") const u8 TaskManagerInitTail[2] = {0, 0};

/**
 * @brief Free every queued task node and then the queue array itself.
 * Does not run task callbacks or free the manager struct.
 * @param manager Manager to tear down.
 * @return Nothing.
 */
AT("0007A644")
void TaskManagerDestroy(struct TaskManager *manager)
{
    u32 i;
    ListNode *next, *node;

    for (i = 0; i < manager->queueCount; i++) {
        next = manager->queues[i].head;
        while (next) {
            node = next;
            next = next->next;
            HeapFree(manager->heap, node);
        }
    }
    HeapFree(manager->heap, manager->queues);
    manager->queues = 0;
    manager->taskCount = 0;
}

/** @return The manager's live task count. */
AT("0007A868")
u32 TaskManagerCount(struct TaskManager *manager)
{
    return manager->taskCount;
}

/* Allocate a task header plus zeroed caller payload. The allocation-failure
    * handler remains assembly; preserve its original 0x00600000 argument. */
extern void CpuFill(void *,u32,u32);
/**
 * @brief Allocate a task header plus a zeroed caller payload and append it
 * to the end of one priority queue.
 * @param manager Owning task manager.
 * @param callback Function TaskManagerRun() invokes each pass while the task
 * is running.
 * @param priority Index of the queue to append to.
 * @param completion Optional pointer the caller polls for completion; reset
 * to 0 here if given.
 * @param size Size of the caller payload following the task header.
 * @return The new task, or NULL if the allocation fails.
 */
AT("0007A688")
struct EngineTask *TaskCreateInQueue(struct TaskManager *manager,void (*callback)(struct EngineTask *),u32 priority,u32 *completion,u32 size)
{
    struct EngineTask *task;
    u32 totalSize;
    task = HeapAlloc(manager->heap,totalSize = size + 32);
    if (!task) {
        HeapGetFreeBytes((struct Heap *)0x00600000);
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
/**
 * @brief Allocate a task header plus a zeroed caller payload and insert it
 * into another task's queue immediately before that task.
 * @param manager Owning task manager.
 * @param callback Function TaskManagerRun() invokes each pass while the task
 * is running.
 * @param at Existing task to insert before; also supplies the target queue.
 * @param completion Optional pointer the caller polls for completion; reset
 * to 0 here if given.
 * @param size Size of the caller payload following the task header.
 * @return The new task, or NULL if the allocation fails.
 */
AT("0007A700")
struct EngineTask *TaskCreateBefore(struct TaskManager *manager,void (*callback)(struct EngineTask *),struct EngineTask *at,u32 *completion,u32 size)
{
    struct EngineTask *task;
    u32 totalSize;
    task = HeapAlloc(manager->heap,totalSize = size + 32);
    if (!task) {
        HeapGetFreeBytes((struct Heap *)0x00600000);
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

/** Dispatch visits queues in ascending order and observes next links after
 * callbacks, allowing newly inserted following tasks to run in this pass. */
AT("0007A77C")
struct EngineTask *CreateTask(struct TaskManager *manager,void (*callback)(struct EngineTask *),u32 queueOrTask,u32 *completion,u32 size)
{
 if(queueOrTask >= manager->queueCount)
  return TaskCreateBefore(manager,callback,(struct EngineTask *)queueOrTask,completion,size);
 else
  return TaskCreateInQueue(manager,callback,queueOrTask,completion,size);
}
AT("0007A77C") const u8 CreateTaskTail[2]={0,0};

/** Only a running task can transition to the scheduler's removal state. */
AT("0007A7AC")
void FinishTask(struct EngineTask *task)
{
    if (task->state == 1)
        task->state = -1;
}

AT("0007A7AC") const u8 FinishTaskTail[2] = {0, 0};

extern void HeapFree(struct Heap *,void *);
AT("0007A7C4")
void TaskManagerRun(struct TaskManager *manager)
{
 u32 i;
 struct EngineTask *next,*task;
 for(i=0;i<manager->queueCount;i++) {
  next=(struct EngineTask *)manager->queues[i].head;
  while(next) {
   void (*callback)(struct EngineTask *)=next->callback;
   if(next->state == -1) {
    task=next;
    next=(struct EngineTask *)next->node.next;
    task->manager->taskCount--;
    ListRemove(&task->manager->queues[task->priority],&task->node);
    HeapFree(task->manager->heap,task);
   } else {
    callback(next);
    task=next;
    next=(struct EngineTask *)next->node.next;
    if(task->state == -1) {
     task->manager->taskCount--;
     ListRemove(&task->manager->queues[task->priority],&task->node);
     HeapFree(task->manager->heap,task);
    }
   }
  }
 }
}
