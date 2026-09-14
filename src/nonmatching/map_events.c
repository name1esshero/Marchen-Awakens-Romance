/* Decoded field-event construction. Still outside the matching build, but the
 * original blocker -- "agbcc's initial argument narrowing" -- is solved, and
 * the shape below reproduces the ROM's prologue instruction for instruction.
 *
 * Three narrowing shapes exist and they are not interchangeable:
 *
 *   s32 parameter, cast in place (`first = (s16)first;`)
 *       -> lsl r0, r6, #16 / asr r6, r0, #16   (sign-correct, scratch reg)
 *   s16 parameter
 *       -> lsl r5, r5, #16 / lsr r5, r5, #16   (in place, but ZERO-extends:
 *          every use here stores to a 16-bit field, so the upper bits are dead
 *          and agbcc is free to pick lsr)
 *   s32 parameter copied to a local, then cast (`a = first; a = (s16)a;`)
 *       -> lsl r5, r5, #16 / asr r5, r5, #16   (in place AND sign-correct)
 *
 * The third is what the ROM has, and it is what is written below.
 *
 * What still differs is register pressure around the call. The ROM preserves
 * both r8 and r9 (`mov r7,r9 / mov r6,r8 / push {r6,r7}`) and loads all three
 * stack arguments up front, holding objectData in r7 across CreateTask. agbcc
 * preserves only r8, loads two stack arguments, and rematerialises the rest
 * afterwards. Binding objectData to a local before the call does not change
 * this. See docs/AGBCC_CODEGEN.md.
 */
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

extern void sub_08061F20(void *task);

AT("00061EA8") struct EngineTask *CreateFieldEventTask(
    s32 first, s32 second, s32 third, s32 fourth, s32 value,
    void *objectData, u32 *completion)
{
    struct EngineTask *task;
    struct FieldEventTaskData *data;
    s32 a, b, c, d, v;

    a = first;  a = (s16)a;
    b = second; b = (s16)b;
    c = third;  c = (s16)c;
    d = fourth; d = (s16)d;
    v = value;  v = (s16)v;
    task = CreateTask(&gMainTaskManager,
                      (void (*)(struct EngineTask *))sub_08061F20,
                      0, completion, 160);
    data = (struct FieldEventTaskData *)task;
    data->firstCoordinate = a;
    data->fourthCoordinate = d;
    data->objectData = objectData;
    data->secondCoordinate = b;
    data->thirdCoordinate = c;
    data->value = v;
    return task;
}
