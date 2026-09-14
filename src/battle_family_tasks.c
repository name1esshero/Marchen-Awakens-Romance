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

DEFINE_BATTLE_TASK("0002BB0C", CreateBattleFamilyTaskBB, 0x0802BB7D,
                   120, 64, 68, 72, 112, 148, 148)
BATTLE_TASK_TAIL("0002BB0C", CreateBattleFamilyTaskBB)
DEFINE_BATTLE_TASK("0002D8D0", CreateBattleFamilyTaskD8, 0x0802D941,
                   52, 0, 4, 8, 44, 80, 80)
BATTLE_TASK_TAIL("0002D8D0", CreateBattleFamilyTaskD8)
DEFINE_BATTLE_TASK("000308C8", CreateBattleFamilyTask308, 0x08030939,
                   104, 64, 68, 72, 96, 132, 132)
BATTLE_TASK_TAIL("000308C8", CreateBattleFamilyTask308)
DEFINE_BATTLE_TASK("00035004", CreateBattleFamilyTask350, 0x08035079,
                   144, 64, 68, 72, 136, 172, 172)
DEFINE_BATTLE_TASK("000355D4", CreateBattleFamilyTask355, 0x0803564D,
                   176, 128, 132, 136, 168, 204, 204)
DEFINE_BATTLE_TASK("000397A0", CreateBattleFamilyTask397, 0x08039815,
                   156, 64, 68, 72, 140, 184, 176)
DEFINE_BATTLE_TASK("0003E238", CreateBattleFamilyTask3E2, 0x0803E2A9,
                   116, 64, 68, 72, 108, 144, 144)
BATTLE_TASK_TAIL("0003E238", CreateBattleFamilyTask3E2)
DEFINE_BATTLE_TASK("0003EC34", CreateBattleFamilyTask3EC, 0x0803ECA5,
                   116, 64, 68, 72, 108, 144, 144)
BATTLE_TASK_TAIL("0003EC34", CreateBattleFamilyTask3EC)
DEFINE_BATTLE_TASK("00044D4C", CreateBattleFamilyTask44D, 0x08044DBD,
                   72, 0, 4, 8, 64, 100, 100)
BATTLE_TASK_TAIL("00044D4C", CreateBattleFamilyTask44D)
DEFINE_BATTLE_TASK("00046408", CreateBattleFamilyTask464, 0x08046479,
                   104, 64, 68, 72, 96, 132, 132)
BATTLE_TASK_TAIL("00046408", CreateBattleFamilyTask464)
DEFINE_BATTLE_TASK("00048D30", CreateBattleFamilyTask48D, 0x08048DA1,
                   128, 64, 68, 72, 120, 156, 156)
BATTLE_TASK_TAIL("00048D30", CreateBattleFamilyTask48D)
DEFINE_BATTLE_TASK("0004AF98", CreateBattleFamilyTask4AF, 0x0804B011,
                   180, 128, 132, 136, 172, 208, 208)
DEFINE_BATTLE_TASK("0004CB44", CreateBattleFamilyTask4CB, 0x0804CBBD,
                   168, 128, 132, 136, 160, 196, 196)
DEFINE_BATTLE_TASK("00050358", CreateBattleFamilyTask503, 0x080503C9,
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
    register u8 *task asm("r1");                                          \
    register s32 primaryOffset asm("r2");                                 \
    register s32 work asm("r4") = (s32)result;                            \
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
DEFINE_LARGE_BATTLE_TASK("00035B34", CreateLargeBattleTask35B, 0x08035BC1,
                         1256, 1184, 1188, 1192, 1280, 1284, 1)

DEFINE_LARGE_BATTLE_TASK("00031C34", CreateLargeBattleTask31C, 0x08031CBD,
                         428, 416, 420, 424, 452, 456, 1)
DEFINE_LARGE_BATTLE_TASK("00037844", CreateLargeBattleTask378, 0x080378CD,
                         308, 288, 292, 296, 332, 336, 1)
DEFINE_LARGE_BATTLE_TASK("0003FF14", CreateLargeBattleTask3FF, 0x0803FF9D,
                         392, 352, 356, 360, 416, 420, 1)
DEFINE_LARGE_BATTLE_TASK("0004DCD0", CreateLargeBattleTask4DC, 0x0804DD59,
                         372, 352, 356, 360, 392, 400, 2)
#endif
