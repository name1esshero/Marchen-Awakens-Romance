#ifndef MAP_EVENTS_H
#define MAP_EVENTS_H

#include "task_manager.h"

#define FIELD_EVENT_TASK_DATA_SIZE 160

/** Field-event payload after the task header; offsets describe the GBA ABI. */
struct FieldEventTaskData
{
    u8 unknown00[0x78];
    void *objectData;           /* 078: task +098 */
    s16 fourthCoordinate;      /* 07C: task +09C */
    s16 firstCoordinate;       /* 07E: task +09E */
    u8 unknown80[2];
    s16 secondCoordinate;      /* 082: task +0A2 */
    s16 thirdCoordinate;       /* 084: task +0A4 */
    u8 unknown86[12];
    s16 value;                 /* 092: task +0B2; meaning not yet recovered */
    u8 unknown94[12];
};

struct EngineTask *CreateFieldEventTask(s16 first, s16 second, s16 third,
    s16 fourth, s16 value, void *objectData, u32 *completion);
void FieldActorUpdateForMode(void *actorState);

#endif
