#ifndef TASK_CONSTRUCTORS_H
#define TASK_CONSTRUCTORS_H

#include "gba/types.h"

struct EngineTask;

struct EngineTask *CreateMapCoordinateTask(s16 mode, s16 coordinate,
    s32 *result);

#endif /* TASK_CONSTRUCTORS_H */
