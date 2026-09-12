/* Dispatch visits queues in ascending order and observes next links after
 * callbacks, allowing newly inserted following tasks to run in this pass. */
#include "task_manager.h"
#define AT(x) __attribute__((section(".rom." x)))
AT("0007A77C")
struct EngineTask *CreateTask(struct TaskManager *manager,void (*callback)(struct EngineTask *),u32 queueOrTask,u32 *completion,u32 size)
{
 if(queueOrTask >= manager->queueCount)
  return TaskCreateBefore(manager,callback,(struct EngineTask *)queueOrTask,completion,size);
 else
  return TaskCreateInQueue(manager,callback,queueOrTask,completion,size);
}
AT("0007A77C") const u8 CreateTaskTail[2]={0,0};

/* Only a running task can transition to the scheduler's removal state. */
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
