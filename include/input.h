#ifndef INPUT_H
#define INPUT_H
#include "gba/types.h"

/* Eight 8-byte slots in the 64-byte block installed by KeyInputInit. */
struct KeyState {
    u16 held;
    u16 pressed;  /* Rising edges; consumers remove the bits they handle. */
    u16 previous;
    u16 unused;
};
#ifndef gKeyStates
#define gKeyStates (*(struct KeyState **)0x03006108)
#endif
#ifndef gKeyInputRegister
#define gKeyInputRegister (*(vu16 *)0x04000130)
#endif

void KeyInputInit(struct KeyState *states);
void KeyInputPoll(u32 slot);
void KeyInputSet(u16 held, u32 slot);
u32 KeyInputConsumePressed(u16 mask, u32 slot);
u32 KeyInputAnyHeld(u32 mask, u32 slot);
#endif
