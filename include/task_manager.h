#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include "gba/types.h"
#include "heap.h"
#include "list.h"

/* Task creation indexes this list array by its priority argument. Task nodes
 * begin with intrusive next/previous links; their remaining header is separate.
 */
struct TaskManager {
    struct Heap *heap;
    List *queues;
    u32 queueCount;
    u32 taskCount;
};
void TaskManagerInit(struct TaskManager *manager, struct Heap *heap, u32 count);
void TaskManagerDestroy(struct TaskManager *manager);
u32 TaskManagerCount(struct TaskManager *manager);
struct EngineTask {
 ListNode node;
 struct TaskManager *manager;
 s8 state;
 u8 unknown[7];
 void (*callback)(struct EngineTask *);
 u32 *completion;
 u32 priority;
};
/* Queue index and insertion target must belong to this manager. */
struct EngineTask *TaskCreateInQueue(struct TaskManager *, void (*)(struct EngineTask *), u32, u32 *, u32);
struct EngineTask *TaskCreateBefore(struct TaskManager *, void (*)(struct EngineTask *), struct EngineTask *, u32 *, u32);
/* queueOrTask is a queue index when below queueCount, otherwise a task pointer. */
struct EngineTask *CreateTask(struct TaskManager *, void (*)(struct EngineTask *), u32 queueOrTask, u32 *, u32);
void FinishTask(struct EngineTask *task);
void TaskManagerRun(struct TaskManager *);
#endif
