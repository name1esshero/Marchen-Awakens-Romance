#ifndef TASK_CONSTRUCTORS_H
#define TASK_CONSTRUCTORS_H

#include "gba/types.h"

struct EngineTask;

struct EngineTask *CreateMapCoordinateTask(s16 mode, s16 coordinate,
                                            s32 *result);
s32 CreateInputWaitTask(s32 inputSlot, s32 keyMask, s32 *result);
void ScheduleVramFillTask(void *destination, u32 size, u32 value);

#endif /* TASK_CONSTRUCTORS_H */
