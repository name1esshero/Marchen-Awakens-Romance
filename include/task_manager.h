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
#endif
