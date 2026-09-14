/* Constructor for the five-part encounter sprite task. The task owns five
 * 72-byte NCD sprite containers followed by selection and completion state.
 */
#include "gba/types.h"

#include "runtime_state.h"
#include "rom_section.h"

extern u8 *CreateTask(void *, void *, u32, s32 *, u32);
extern void NcdSpriteContainerReset(void *);
extern void RuntimeObjectSetField4A(s32, s32, s32);
extern void RuntimeObjectSetField4C(s32, s32, s32);

AT("00012FD0") u8 *CreateEncounterSpriteTask(s32 owner, s32 slot,
                                              void *context, s32 *result)
{
    u8 *task = CreateTask(gSecondaryRuntime + owner * 32,
                          (void *)0x080130A1, 0, result, 668);
    u8 *state = task + 32;
    s32 objectIndex;
    s32 zero;
    u8 *sprite;
    s32 remaining;
    task[392] = owner;
    task[393] = slot;
    objectIndex = owner * 4 + slot;
    task[652] = objectIndex;
    sprite = state;
    zero = 0;
    remaining = 4;
    do {
        NcdSpriteContainerReset(sprite);
        *(s32 *)(sprite + 28) = zero;
        sprite += 72;
    } while (--remaining >= 0);
    RuntimeObjectSetField4A(owner, slot, 512);
    RuntimeObjectSetField4C(owner, slot, 512);
    {
        s32 empty = -1;
        for (remaining = 4; remaining >= 0; remaining--)
            *(s32 *)(state + 556 + remaining * 4) = empty;
    }
    *(s32 *)(state + 652) = -1;
    *(s32 *)(state + 648) = -1;
    *(void **)(state + 644) = context;
    return task;
}
