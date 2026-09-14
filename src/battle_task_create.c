/* Common constructors behind the fixed battle group/variant entry tables.
 * A battle participant owns a 32-byte bank of 16-byte task managers.  Each
 * constructor creates its implementation-specific state block, then records
 * the participant, slot, resource and the two table-selected s16 values in
 * the shared late-state layout.
 */
#include "gba/types.h"

#include "runtime_state.h"
#include "rom_section.h"

extern u8 *CreateTask(void *manager, void *callback, u32 priority,
                      s32 *result, u32 stateSize);

extern void sub_0802C21C(void *task);
extern void sub_0802FD34(void *task);
extern void sub_08032FE8(void *task);
extern void sub_08033D98(void *task);
extern void sub_0804083C(void *task);
extern void sub_08045170(void *task);
extern void sub_0804921C(void *task);
extern void sub_0804F4D4(void *task);
extern void sub_0804E700(void *task);
extern void sub_08048460(void *task);
extern void sub_0804AD08(void *task);
extern void sub_0802D464(void *task);
extern void sub_0804A7C4(void *task);

/** Implement create battle mode task2 c1 using the recovered battle task layout. */
AT("0002C198")
u8 *CreateBattleModeTask2C1(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 selectedMode)
{
    void *savedResource = resource;
    s32 mode = (s16)selectedMode;
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                          (void *)sub_0802C21C, 0, result, 116);
    u8 *state;
    s32 *empty;
    s32 *cursor;
    s32 emptyValue;

    if (task == 0) {
        if (result != 0)
            *result = -1;
        return 0;
    }
    state = task + 32;
    *(s32 *)(state + 64) = owner;
    *(s32 *)(state + 68) = slot;
    *(s32 *)(state + 72) = owner == 0;
    *(void **)(state + 108) = savedResource;
    *(s16 *)(task + 134) = mode;
    empty = (s32 *)(task + 144);
    emptyValue = -1;
    cursor = empty;
    do {
        *cursor = emptyValue;
        cursor--;
    } while ((s32)cursor >= (s32)empty);
    return task;
}
AT("0002C198") const u8 CreateBattleModeTask2C1Tail[2] = {0};

/** Implement create battle mode task2 f c using the recovered battle task layout. */
AT("0002FCA0")
u8 *CreateBattleModeTask2FC(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 selectedMode)
{
    void *savedResource = resource;
    s32 mode = (s16)selectedMode;
    u8 *task;
    s32 *empty;
    s32 *cursor;
    s32 emptyValue;

    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                      (void *)sub_0802FD34, 0, result, 236);
    if (task == 0) {
        if (result != 0)
            *result = -1;
        return 0;
    }
    *(s32 *)(task + 224) = owner;
    *(s32 *)(task + 228) = slot;
    *(s32 *)(task + 232) = owner == 0;
    *(void **)(task + 260) = savedResource;
    *(s16 *)(task + 246) = mode;
    empty = (s32 *)(task + 264);
    emptyValue = -1;
    cursor = empty;
    do {
        *cursor = emptyValue;
        cursor--;
    } while ((s32)cursor >= (s32)empty);
    return task;
}

/** Implement create battle mode task32 f using the recovered battle task layout. */
AT("00032F64")
u8 *CreateBattleModeTask32F(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 selectedMode)
{
    void *savedResource = resource;
    s32 mode = (s16)selectedMode;
    u8 *task;

    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                      (void *)sub_08032FE8, 0, result, 200);
    if (task == 0) {
        if (result != 0)
            *result = -1;
        return 0;
    }
    *(s32 *)(task + 160) = owner;
    *(s32 *)(task + 164) = slot;
    *(s32 *)(task + 168) = owner == 0;
    *(void **)(task + 224) = savedResource;
    *(s16 *)(task + 216) = mode;
    if (result != 0)
        *result = 0;
    return task;
}

#define DEFINE_SEQUENTIAL_MODE_TASK(address, name, callback, stateSize,    \
                                    ownerOffset, resourceOffset, modeOffset,\
                                    emptyOffset)                           \
AT(address)                                                               \
u8 *name(s32 owner, s32 slot, void *resource, s32 *result, s32 selectedMode)\
{                                                                          \
    void *savedResource = resource;                                         \
    s32 mode = (s16)selectedMode;                                           \
    u8 *task;                                                              \
    s32 *empty;                                                             \
    s32 *cursor;                                                            \
    s32 emptyValue;                                                         \
    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,             \
                      (void *)(callback), 0, result, (stateSize));          \
    if (task == 0) {                                                        \
        if (result != 0)                                                    \
            *result = -1;                                                   \
        return 0;                                                           \
    }                                                                       \
    *(s32 *)(task + (ownerOffset)) = owner;                                 \
    *(s32 *)(task + (ownerOffset) + 4) = slot;                              \
    *(s32 *)(task + (ownerOffset) + 8) = owner == 0;                        \
    *(void **)(task + (resourceOffset)) = savedResource;                    \
    *(s16 *)(task + (modeOffset)) = mode;                                   \
    empty = (s32 *)(task + (emptyOffset));                                  \
    emptyValue = -1;                                                        \
    cursor = empty;                                                         \
    do {                                                                    \
        *cursor = emptyValue;                                               \
        cursor--;                                                           \
    } while ((s32)cursor >= (s32)empty);                                    \
    return task;                                                            \
}

DEFINE_SEQUENTIAL_MODE_TASK("00033D00", CreateBattleModeTask33D,
                            sub_08033D98, 244, 224, 268, 262, 272)
AT("00033D00") const u8 CreateBattleModeTask33DTail[2] = {0};

DEFINE_SEQUENTIAL_MODE_TASK("000407AC", CreateBattleModeTask407,
                            sub_0804083C, 192, 160, 216, 208, 220)
AT("000407AC") const u8 CreateBattleModeTask407Tail[2] = {0};

DEFINE_SEQUENTIAL_MODE_TASK("000450E0", CreateBattleModeTask450,
                            sub_08045170, 200, 160, 224, 214, 228)
AT("000450E0") const u8 CreateBattleModeTask450Tail[2] = {0};

DEFINE_SEQUENTIAL_MODE_TASK("00049184", CreateBattleModeTask491,
                            sub_0804921C, 248, 224, 272, 262, 276)
AT("00049184") const u8 CreateBattleModeTask491Tail[2] = {0};

DEFINE_SEQUENTIAL_MODE_TASK("0004F444", CreateBattleModeTask4F4,
                            sub_0804F4D4, 196, 160, 220, 214, 224)
AT("0004F444") const u8 CreateBattleModeTask4F4Tail[2] = {0};

#ifdef NONMATCHING
AT("0004E668")
u8 *CreateBattleModeTask4E6(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 selectedMode)
{
    void *savedResource = resource;
    s32 mode = (s16)selectedMode;
    u8 *task;
    s32 *end;
    s32 *cursor;
    s32 emptyValue;

    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                      (void *)sub_0804E700, 0, result, 256);
    if (task == 0) {
        if (result != 0)
            *result = -1;
        return 0;
    }
    *(s32 *)(task + 224) = owner;
    *(s32 *)(task + 228) = slot;
    *(s32 *)(task + 232) = owner == 0;
    result = savedResource;
    *(void **)(task + 276) = result;
    *(s16 *)(task + 266) = mode;
    result = (s32 *)280;
    end = (s32 *)(task + (s32)result);
    emptyValue = -1;
    result = (s32 *)((s32)result + 4);
    cursor = (s32 *)(task + (s32)result);
    do {
        *cursor = emptyValue;
        cursor--;
    } while ((s32)cursor >= (s32)end);
    return task;
}
#endif

/** Implement create battle mode task483 using the recovered battle task layout. */
AT("000483D8")
u8 *CreateBattleModeTask483(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 selectedMode)
{
    void *savedResource = resource;
    s32 mode = (s16)selectedMode;
    u8 *task;
    u8 *state;
    s32 *empty;
    s32 *cursor;
    s32 emptyValue;

    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                      (void *)sub_08048460, 0, result, 108);
    if (task == 0) {
        if (result != 0)
            *result = -1;
        return 0;
    }
    state = task + 32;
    *(s32 *)(state + 64) = owner;
    *(s32 *)(state + 68) = slot;
    *(s32 *)(state + 72) = owner == 0;
    *(void **)(state + 100) = savedResource;
    *(s16 *)(task + 112) = mode;
    empty = (s32 *)(task + 136);
    emptyValue = -1;
    cursor = empty;
    do {
        *cursor = emptyValue;
        cursor--;
    } while ((s32)cursor >= (s32)empty);
    return task;
}

#ifdef NONMATCHING
AT("0004AC8C")
u8 *CreateBattleModeTask4AC(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 selectedMode)
{
    void *savedResource = resource;
    s32 mode = (s16)selectedMode;
    u8 *task;
    u8 *state;
    s16 *modeField;
    s32 *cursor;
    s32 emptyValue;
    s32 remaining;

    task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,
                      (void *)sub_0804AD08, 0, result, 104);
    if (task == 0) {
        if (result != 0)
            *result = -1;
        return 0;
    }
    state = task + 32;
    *(s32 *)(state + 64) = owner;
    *(s32 *)(state + 68) = slot;
    *(void **)(state + 96) = savedResource;
    modeField = (s16 *)(state + 88);
    *modeField = mode;
    emptyValue = -1;
    remaining = 0;
    cursor = (s32 *)((u8 *)modeField + 12);
    do {
        *cursor = emptyValue;
        cursor--;
        remaining--;
    } while (remaining >= 0);
    return task;
}
#endif

#define CREATE_BATTLE_TASK(callback)                                      \
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32 + slot * 16,       \
                          (callback), 0, result, 300);                    \
    if (task == 0) {                                                      \
        if (result != 0)                                                  \
            *result = -1;                                                 \
        return 0;                                                         \
    }                                                                     \
    *(s32 *)(task + 288) = owner;                                         \
    *(s32 *)(task + 292) = slot;                                          \
    *(void **)(task + 324) = resource;                                    \
    *(s16 *)(task + 312) = group;                                         \
    *(s16 *)(task + 314) = variant;                                       \
    *(s32 *)(task + 328) = -1;                                            \
    return task

#ifdef NONMATCHING
AT("0002D3C0")
void *BattleObjectCreateTask(s32 owner, s32 slot, void *resource, s32 *result,
                             s32 group, s32 variant)
{
    CREATE_BATTLE_TASK((void *)sub_0802D464);
}

AT("0004A720")
void *BattleTaskACreateTask(s32 owner, s32 slot, void *resource, s32 *result,
                            s32 group, s32 variant)
{
    CREATE_BATTLE_TASK((void *)sub_0804A7C4);
}
#endif
