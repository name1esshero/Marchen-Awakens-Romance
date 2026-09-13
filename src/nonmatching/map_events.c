/* Decoded field-event construction. This remains outside the matching build
 * until agbcc's initial argument narrowing is shaped like the ROM. */
#include "gba/types.h"
#include "task_manager.h"

#define AT(x) __attribute__((section(".rom." x)))

struct FieldEventTaskData {
    u8 unused00[152];
    void *objectData;
    s16 fourthCoordinate;
    s16 firstCoordinate;
    u8 unusedA0[2];
    s16 secondCoordinate;
    s16 thirdCoordinate;
    u8 unusedA6[14];
    s16 value;
};

AT("00061EA8") struct EngineTask *CreateFieldEventTask(
    s32 first, s32 second, s32 third, s32 fourth, s32 value,
    void *objectData, u32 *completion)
{
    struct EngineTask *task;
    struct FieldEventTaskData *data;

    first = (s16)first;
    second = (s16)second;
    third = (s16)third;
    fourth = (s16)fourth;
    value = (s16)value;
    task = CreateTask(&gMainTaskManager,
                      (void (*)(struct EngineTask *))0x08061F21,
                      0, completion, 160);
    data = (struct FieldEventTaskData *)task;
    data->firstCoordinate = first;
    data->fourthCoordinate = fourth;
    data->objectData = objectData;
    data->secondCoordinate = second;
    data->thirdCoordinate = third;
    data->value = value;
    return task;
}
