/* Small task constructors recovered from the engine front ends. Task headers
 * are 32 bytes; state offsets below are therefore expressed from the returned
 * allocation exactly as the callbacks consume them.
 */
#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))
#define SECONDARY_RUNTIME (*(u8 **)0x03004020)

extern u8 *CreateTask(void *manager, void *callback, u32 priority,
                      s32 *result, u32 stateSize);
extern void ScriptAddPendingTasks(s32 count);
extern char *strcpy(char *, const char *);
extern void sub_08080BD8(void *task);
extern void sub_08080BDC(void *task);
extern void sub_08080BD4(void *task);
extern void sub_0805615C(void);
extern void sub_08009728(s32, s32);
extern s32 SpriteResourceFindGroup(s32, const char *);

AT("000037D8") void ScheduleVramFillTask(void *destination, u32 value, u32 size)
{
    u8 *task = CreateTask((void *)0x030032D4, (void *)0x0800380D,
                          0, 0, 12);
    u8 *state = task + 32;
    *(void **)(task + 32) = destination;
    *(u32 *)(state + 4) = value;
    *(u32 *)(state + 8) = size;
}

AT("00005378") s32 CreateInputWaitTask(s32 keyMask, s32 repeatMask,
                                        s32 *result)
{
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x080053B5,
                          0, result, 4);
    if (task != 0) {
        *(u16 *)(task + 32) = keyMask;
        *(u16 *)(task + 34) = repeatMask;
        ScriptAddPendingTasks(1);
    }
    return 0x7fff;
}

AT("00006050") u8 *CreateSceneTask(s32 *result)
{
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x08006079,
                          1, result, 88);
    if (task == 0)
        return 0;
    return task;
}

#ifdef NONMATCHING
AT("000061F0") u8 *CreateSceneModeTask(s32 mode, s32 *result)
{
    u8 *task;
    mode = (s16)mode;
    task = CreateTask((void *)0x030032C4, (void *)0x08006229,
                      0, result, 112);
    if (task == 0)
        goto failed;
    *(u16 *)(task + 142) = (s16)mode;
    goto done;
failed:
    task = 0;
done:
    return task;
}
#endif

AT("000275C4") u8 *CreateBattleMotionTask(s32 valueA, s32 owner, s32 valueB,
                                           void *resource, void *context,
                                           s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x0802761D, 0, result, 104);
    u8 *state = task + 32;
    *(u16 *)(task + 120) = valueA;
    *(u16 *)(task + 114) = valueB;
    *(void **)(state + 72) = resource;
    *(void **)(state + 76) = context;
    task[113] = owner;
    return task;
}

extern s32 RuntimeObjectGetField1A(s32 owner, s32 slot);
extern s32 RuntimeActorGetField352(s32 owner);
AT("000256F8") u8 *CreateBattleTrackingTask(s32 owner, s32 slot,
                                             s32 unused, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x08025755, 0, result, 52);
    u8 *state = task + 32;
    task[71] = owner;
    task[72] = slot;
    *(s32 *)(state + 44) = (s16)RuntimeObjectGetField1A((s8)task[71],
                                                        (s8)task[72]);
    *(s32 *)(state + 48) = RuntimeActorGetField352(owner);
    return task;
}

#ifdef NONMATCHING
AT("0001097C") u8 *CreateSpriteResetTask(s32 sprite, s32 mode, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + 64, (void *)0x080109C5,
                          0, result, 8);
    *(s32 *)(task + 32) = sprite;
    *(s32 *)(task + 36) = mode;
    ScriptAddPendingTasks(1);
    if (mode == 0)
        sub_08080BDC(task);
    return task;
}
#endif

AT("000301BC") u8 *CreateBattleResourceTask(s32 owner, s32 slot,
                                             void *resource, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32 + slot * 16,
                          (void *)0x08030219, 0, result, 24);
    if (task == 0) {
        if (result)
            *result = -1;
        return 0;
    }
    {
        u8 *state = task + 32;
        *(s32 *)(task + 32) = owner;
        *(s32 *)(state + 4) = slot;
        *(void **)(state + 16) = resource;
    }
    if (result)
        *result = 0;
    return task;
}

AT("0002A0B8") u8 *CreateBattleObjectTask(s32 owner, s32 slot,
                                           void *resource, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x0802A119, 0, result, 32);
    if (task == 0) {
        if (result)
            *result = -1;
        return 0;
    }
    {
        u8 *state = task + 32;
        *(s32 *)(task + 32) = owner;
        *(s32 *)(state + 4) = slot;
        *(s32 *)(state + 8) = owner == 0;
        *(u16 *)(state + 12) = owner * 4 + slot;
        *(void **)(state + 28) = resource;
    }
    return task;
}

#ifdef NONMATCHING
AT("00027894") u8 *CreateBattleNamedTaskA(s32 owner, s32 slot,
                                           s32 unused, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x080278F5, 0, result, 116);
    task[104] = owner;
    task[106] = slot;
    task[105] = owner == 0;
    task[107] = 0;
    *(u16 *)(task + 112) = SpriteResourceFindGroup(1, (const char *)0x080877A8);
    return task;
}
#endif

#ifdef NONMATCHING
AT("00026140") u8 *CreateBattleNamedTaskB(s32 owner, s32 slot,
                                           s32 unused, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x080261A5, 0, result, 240);
    task[251] = owner;
    task[253] = slot;
    task[252] = owner == 0;
    task[254] = 0;
    *(u16 *)(task + 258) = SpriteResourceFindGroup(1, (const char *)0x08087778);
    return task;
}
#endif

AT("0000E788") u8 *CreateFieldEffectTask(s32 owner, s32 slot, s32 a, s32 b,
                                          s32 c, s32 d, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x0800E7ED, 0, result, 32);
    u8 *state = task + 32;
    *(s32 *)(task + 32) = owner;
    *(s32 *)(state + 4) = slot;
    *(s32 *)(state + 8) = a;
    *(s32 *)(state + 16) = 0;
    *(s32 *)(state + 20) = b;
    *(s32 *)(state + 12) = c;
    *(s32 *)(state + 24) = d;
    ScriptAddPendingTasks(1);
    return task;
}

#ifdef NONMATCHING
AT("0000ECF8") u8 *CreateFieldCommandTask(s32 owner, s32 a, s32 b, s32 c,
                                           s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + owner * 32,
                          (void *)0x0800ED5D, 0, result, 16);
    u8 *state = task + 32;
    *(s32 *)(task + 32) = owner;
    *(s32 *)(state + 4) = a;
    *(s32 *)(state + 8) = b;
    *(s32 *)(state + 12) = c;
    ScriptAddPendingTasks(1);
    sub_08080BD4(task);
    return task;
}
#endif

#define CREATE_PENDING_TASK(address, name, manager, callback)              \
AT(address) u8 *name(s32 value, s32 *result)                               \
{                                                                          \
    u8 *task = CreateTask((manager), (callback), 0, result, 4);             \
    *(s32 *)(task + 32) = value;                                            \
    ScriptAddPendingTasks(1);                                               \
    return task;                                                            \
}

CREATE_PENDING_TASK("0000D8EC", StartPendingEffectB,
                    SECONDARY_RUNTIME + 64, (void *)0x0800D925)
CREATE_PENDING_TASK("0000E6A0", StartPendingFieldEffect,
                    SECONDARY_RUNTIME + value * 32, (void *)0x0800E6D9)
CREATE_PENDING_TASK("0000FC90", StartPendingEffectA,
                    SECONDARY_RUNTIME + 64, (void *)0x0800FCC9)

AT("00007134") u8 *CreateNamedRuntimeTask(const char *name, s32 value,
                                           s32 other, s32 *result)
{
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x08007179,
                          0, result, 44);
    u8 *state = task + 32;
    *(s32 *)(state + 8) = value;
    *(s32 *)(state + 36) = other;
    strcpy((char *)state + 12, name);
    return task;
}

#ifdef NONMATCHING
AT("00010A2C") u8 *CreateSpriteWaitTask(s32 mode, s32 *result)
{
    u8 *task = CreateTask(SECONDARY_RUNTIME + 64, (void *)0x08010A71,
                          0, result, 8);
    *(s32 *)(task + 36) = mode;
    ScriptAddPendingTasks(1);
    if (mode == 0)
        sub_08080BD8(task);
    return task;
}
#endif

AT("0006F620") u8 *CreateEncounterTransitionTask(s32 *result)
{
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x0806F665,
                          0, result, 32);
    *(u16 *)(SECONDARY_RUNTIME + 0x0F24) = 1;
    ScriptAddPendingTasks(1);
    return task;
}

#ifdef NONMATCHING
AT("0006EFCC") u8 *CreateEncounterResetTask(s32 *result)
{
    s32 i;
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x0806F019,
                          0, result, 0x3F34);
    for (i = 0; i <= 3; i++)
        sub_08009728(i, 0);
    ScriptAddPendingTasks(1);
    return task;
}
#endif

#ifdef NONMATCHING
AT("0001B7FC") u8 *CreateObjectMotionTaskA(u8 *object, s32 *result)
{
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x0801B83D,
                          0, result, 16);
    u8 *state = task + 32;
    *(u8 **)(task + 32) = object;
    *(u16 *)(object + 24) = 120;
    *(u16 *)(object + 26) = -32;
    *(u16 *)(state + 4) = -32;
    *(u16 *)(state + 8) = 24;
    return task;
}
#endif

#ifdef NONMATCHING
AT("0001B8AC") u8 *CreateObjectMotionTaskB(u8 *object, s32 *result)
{
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x0801B8ED,
                          0, result, 16);
    u8 *state = task + 32;
    *(u8 **)(task + 32) = object;
    object[39] |= 4;
    *(u16 *)(state + 4) = 256;
    *(u16 *)(state + 8) = 0;
    return task;
}
#endif

#ifdef NONMATCHING
AT("0006C7AC") u8 *CreateMapCoordinateTask(s32 x, s32 y, s32 *result)
{
    u8 *task;
    x = (s16)x;
    y = (s16)y;
    task = CreateTask((void *)0x030032C4, (void *)0x0806C7ED,
                      0, result, 16);
    *(u16 *)(task + 32) = x;
    *(u16 *)(task + 36) = y;
    ScriptAddPendingTasks(1);
    return task;
}
#endif

#ifdef NONMATCHING
AT("0006FA94") u8 *CreateEncounterSetupTask(s32 value, s32 *result)
{
    register s32 savedValue asm("r4") = value;
    u8 *task = CreateTask((void *)0x030032C4, (void *)0x0806FAC5,
                          0, result, 368);
    *(s32 *)(task + 368) = savedValue;
    return task;
}
#endif
