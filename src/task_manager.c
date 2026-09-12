/* Priority-list storage for engine tasks. Destruction frees every queued node
 * and then the queue array; it does not run task callbacks or free the manager.
 * Initialization preserves the original lack of allocation-failure checks.
 */
#include "task_manager.h"
#define AT(x) __attribute__((section(".rom." x)))
extern void HeapFree(struct Heap *, void *);

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

AT("0007A868")
u32 TaskManagerCount(struct TaskManager *manager)
{
    return manager->taskCount;
}
