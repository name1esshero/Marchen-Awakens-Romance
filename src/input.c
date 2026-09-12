/* Input state at 03006108. Opposite directions deliberately keep ONLY left
 * (0x20) or up (0x40), discarding other held buttons too. This preserves the
 * original behavior rather than silently fixing its directional filtering.
 * Poll reads active-low hardware keys; Set accepts an already-active mask.
 * Callers must select a valid slot; the original routines do not bounds-check.
 */
#include "input.h"
#define AT(x) __attribute__((section(".rom." x)))
extern void CpuFill(void *, u32, u32);

AT("0007A084")
void KeyInputInit(struct KeyState *states)
{
    gKeyStates = states;
    CpuFill(states, 64, 0);
}

AT("0007A09C")
void KeyInputPoll(u32 slot)
{
    struct KeyState *state = &gKeyStates[slot];

    state->previous = state->held;
    state->held = ~gKeyInputRegister & 0x3FF;
    if ((state->held & 0x30) == 0x30)
        state->held &= 0x20;
    if ((state->held & 0xC0) == 0xC0)
        state->held &= 0x40;
    state->pressed = state->held & ~state->previous;
}

AT("0007A0F0")
void KeyInputSet(u16 held, u32 slot)
{
    struct KeyState *state = &gKeyStates[slot];

    state->previous = state->held;
    state->held = held;
    if ((state->held & 0x30) == 0x30)
        state->held &= 0x20;
    if ((state->held & 0xC0) == 0xC0)
        state->held &= 0x40;
    state->pressed = state->held & ~state->previous;
}

AT("0007A134")
u32 KeyInputConsumePressed(u16 mask, u32 slot)
{
    struct KeyState *state = &gKeyStates[slot];
    u32 hit = state->pressed & mask;

    if (hit) {
        state->pressed &= ~mask;
        return hit;
    } else {
        return 0;
    }
}
AT("0007A134") const u8 KeyInputConsumePressedTail[2] = {0, 0};

AT("0007A160")
u32 KeyInputAnyHeld(u32 mask, u32 slot)
{
    struct KeyState *state = &gKeyStates[slot];

    if (state->held & mask)
        return 1;
    return 0;
}
AT("0007A160") const u8 KeyInputAnyHeldTail[2] = {0, 0};
