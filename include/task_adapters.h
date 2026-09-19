#ifndef TASK_ADAPTERS_H
#define TASK_ADAPTERS_H

#include "task_manager.h"

struct TaskAdapterOwner;

struct EngineTask *CreateTask078E4(u32 *completion);
struct EngineTask *CreateTask6C8F0(u32 *completion);
struct EngineTask *CreateTask6E748(u32 *completion);
struct EngineTask *CreateTask6EEF0(u32 *completion);
struct EngineTask *CreateTaskB478(u32 *completion);
struct EngineTask *CreateTask1BAD8(u32 value, u32 *completion);
struct EngineTask *CreateTask51E84(u32 value, u32 *completion);
struct EngineTask *CreateTask62304(struct TaskAdapterOwner *owner,
                                   u32 *completion);
struct EngineTask *CreateTask66068(struct TaskAdapterOwner *owner,
                                   u32 *completion);
struct EngineTask *CreateTask683C4(struct TaskAdapterOwner *owner,
                                   u32 *completion);
struct EngineTask *CreateTask68C0C(u32 *completion);
struct EngineTask *CreateTask6EAFC(u32 *completion);
struct EngineTask *CreateActorTaskD664(u32 value,u32 *completion);
struct EngineTask *CreateActorTaskF0F0(u32 value,u32 *completion);
struct EngineTask *CreateActorTask10704(u32 value,u32 *completion);
struct EngineTask *CreateActorTask11484(u32 value,u32 *completion);
void CreateCopyTask(void *destination,void *source,u32 size);
struct EngineTask *CreateCoordinateTask(u32 first,u32 second,u32 third,u32 *completion);
struct EngineTask *CreatePendingTaskD98C(u32 *completion);
struct EngineTask *CreatePendingTaskFD3C(u32 *completion);
struct EngineTask *CreateSpriteTaskF9B4(u32 value,u32 *completion);
struct EngineTask *CreateIndexedPendingTask(u32 index,u32 value,u32 *completion);
void CopyThenFinishTask(struct EngineTask *task);

#endif
