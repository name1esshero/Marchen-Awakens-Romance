/* Constructors shared by several battle-effect state machines. Although the
 * callbacks differ, their front ends use one layout: participant identity,
 * the participant-zero side flag, a caller-owned resource, and an s32 slot
 * initialized to -1. Keeping the offsets in this table-like macro makes the
 * otherwise duplicated ABI visible.
 */
#include "gba/types.h"

#include "runtime_state.h"
#include "rom_section.h"

extern u8 *CreateTask(void *, void *, u32, s32 *, u32);

extern void sub_0802BB7C(void *task);
extern void sub_0802D940(void *task);
extern void sub_08030938(void *task);
extern void sub_08035078(void *task);
extern void sub_0803564C(void *task);
extern void sub_08039814(void *task);
extern void sub_0803E2A8(void *task);
extern void sub_0803ECA4(void *task);
extern void sub_08044DBC(void *task);
extern void sub_08046478(void *task);
extern void sub_08048DA0(void *task);
extern void sub_0804B010(void *task);
extern void sub_0804CBBC(void *task);
extern void sub_080503C8(void *task);
extern void sub_08035BC0(void *task);
extern void sub_08031CBC(void *task);
extern void sub_080378CC(void *task);
extern void sub_0803FF9C(void *task);
extern void sub_0804DD58(void *task);

#define DEFINE_BATTLE_TASK(address, name, callback, stateSize, ownerOffset,  \
                           slotOffset, flagOffset, resourceOffset,            \
                           emptyStartOffset, emptyEndOffset)                  \
AT(address) u8 *name(s32 owner, s32 slot, void *resource, s32 *result)       \
{                                                                           \
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,       \
                          (void *)(callback), 0, result, (stateSize));        \
    u8 *state;                                                               \
    s32 *end;                                                                \
    s32 *cursor;                                                             \
    s32 empty;                                                               \
    if (task == 0) {                                                         \
        if (result != 0)                                                     \
            *result = -1;                                                    \
        return 0;                                                            \
    }                                                                        \
    state = task + 32;                                                       \
    *(s32 *)(state + (ownerOffset)) = owner;                                 \
    *(s32 *)(state + (slotOffset)) = slot;                                   \
    *(s32 *)(state + (flagOffset)) = owner == 0;                             \
    *(void **)(state + (resourceOffset)) = resource;                         \
    end = (s32 *)(task + (emptyEndOffset));                                  \
    empty = -1;                                                              \
    cursor = (s32 *)(task + (emptyStartOffset));                             \
    do {                                                                     \
        *cursor = empty;                                                     \
        cursor--;                                                            \
    } while ((s32)cursor >= (s32)end);                                       \
    return task;                                                             \
}

#define BATTLE_TASK_TAIL(address, name) AT(address) const u8 name##Tail[2] = {0};

DEFINE_BATTLE_TASK("0002BB0C", CreateBattleFamilyTaskBB, sub_0802BB7C,
                   120, 64, 68, 72, 112, 148, 148)
BATTLE_TASK_TAIL("0002BB0C", CreateBattleFamilyTaskBB)
DEFINE_BATTLE_TASK("0002D8D0", CreateBattleFamilyTaskD8, sub_0802D940,
                   52, 0, 4, 8, 44, 80, 80)
BATTLE_TASK_TAIL("0002D8D0", CreateBattleFamilyTaskD8)
DEFINE_BATTLE_TASK("000308C8", CreateBattleFamilyTask308, sub_08030938,
                   104, 64, 68, 72, 96, 132, 132)
BATTLE_TASK_TAIL("000308C8", CreateBattleFamilyTask308)
DEFINE_BATTLE_TASK("00035004", CreateBattleFamilyTask350, sub_08035078,
                   144, 64, 68, 72, 136, 172, 172)
DEFINE_BATTLE_TASK("000355D4", CreateBattleFamilyTask355, sub_0803564C,
                   176, 128, 132, 136, 168, 204, 204)
DEFINE_BATTLE_TASK("000397A0", CreateBattleFamilyTask397, sub_08039814,
                   156, 64, 68, 72, 140, 184, 176)
DEFINE_BATTLE_TASK("0003E238", CreateBattleFamilyTask3E2, sub_0803E2A8,
                   116, 64, 68, 72, 108, 144, 144)
BATTLE_TASK_TAIL("0003E238", CreateBattleFamilyTask3E2)
DEFINE_BATTLE_TASK("0003EC34", CreateBattleFamilyTask3EC, sub_0803ECA4,
                   116, 64, 68, 72, 108, 144, 144)
BATTLE_TASK_TAIL("0003EC34", CreateBattleFamilyTask3EC)
DEFINE_BATTLE_TASK("00044D4C", CreateBattleFamilyTask44D, sub_08044DBC,
                   72, 0, 4, 8, 64, 100, 100)
BATTLE_TASK_TAIL("00044D4C", CreateBattleFamilyTask44D)
DEFINE_BATTLE_TASK("00046408", CreateBattleFamilyTask464, sub_08046478,
                   104, 64, 68, 72, 96, 132, 132)
BATTLE_TASK_TAIL("00046408", CreateBattleFamilyTask464)
DEFINE_BATTLE_TASK("00048D30", CreateBattleFamilyTask48D, sub_08048DA0,
                   128, 64, 68, 72, 120, 156, 156)
BATTLE_TASK_TAIL("00048D30", CreateBattleFamilyTask48D)
DEFINE_BATTLE_TASK("0004AF98", CreateBattleFamilyTask4AF, sub_0804B010,
                   180, 128, 132, 136, 172, 208, 208)
DEFINE_BATTLE_TASK("0004CB44", CreateBattleFamilyTask4CB, sub_0804CBBC,
                   168, 128, 132, 136, 160, 196, 196)
DEFINE_BATTLE_TASK("00050358", CreateBattleFamilyTask503, sub_080503C8,
                   128, 64, 68, 72, 120, 156, 156)
BATTLE_TASK_TAIL("00050358", CreateBattleFamilyTask503)

/* Larger state blocks use shifted immediates for their late fields and a
 * counted initialization loop. */
#define DEFINE_LARGE_BATTLE_TASK(address, name, callback, stateSize,       \
                                 ownerOffset, slotOffset, flagOffset,       \
                                 resourceOffset, emptyStartOffset,         \
                                 emptyCount)                               \
AT(address) u8 *name(s32 owner, s32 slot, void *resource, s32 *result)     \
{                                                                          \
    u8 *task;                                                               \
    s32 primaryOffset;                                                      \
    s32 work = (s32)result;                                                 \
    s32 *cursor;                                                            \
    s32 remaining;                                                         \
    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,          \
                      (void *)(callback), 0, (s32 *)work, (stateSize));     \
    if (task == 0) {                                                        \
        if (work != 0)                                                      \
            *(s32 *)work = -1;                                              \
        return 0;                                                           \
    }                                                                       \
    primaryOffset = (ownerOffset);                                          \
    *(s32 *)(task + primaryOffset) = owner;                                 \
    work = (slotOffset);                                                     \
    *(s32 *)(task + work) = slot;                                           \
    primaryOffset += (flagOffset) - (ownerOffset);                          \
    *(s32 *)(task + primaryOffset) = owner == 0;                            \
    work = (resourceOffset);                                                 \
    *(void **)(task + work) = resource;                                     \
    work += (emptyStartOffset) - (resourceOffset);                          \
    cursor = (s32 *)(task + work);                                          \
    remaining = (emptyCount) - 1;                                          \
    do {                                                                    \
        *cursor = -1;                                                       \
        cursor--;                                                           \
        remaining--;                                                        \
    } while (remaining >= 0);                                               \
    return task;                                                            \
}

#ifdef NONMATCHING
DEFINE_LARGE_BATTLE_TASK("00035B34", CreateLargeBattleTask35B, sub_08035BC0,
                         1256, 1184, 1188, 1192, 1280, 1284, 1)

DEFINE_LARGE_BATTLE_TASK("00031C34", CreateLargeBattleTask31C, sub_08031CBC,
                         428, 416, 420, 424, 452, 456, 1)
DEFINE_LARGE_BATTLE_TASK("00037844", CreateLargeBattleTask378, sub_080378CC,
                         308, 288, 292, 296, 332, 336, 1)
DEFINE_LARGE_BATTLE_TASK("0003FF14", CreateLargeBattleTask3FF, sub_0803FF9C,
                         392, 352, 356, 360, 416, 420, 1)
DEFINE_LARGE_BATTLE_TASK("0004DCD0", CreateLargeBattleTask4DC, sub_0804DD58,
                         372, 352, 356, 360, 392, 400, 2)
#endif
