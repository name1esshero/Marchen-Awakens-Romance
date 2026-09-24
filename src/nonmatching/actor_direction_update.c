/*
 * Readable candidate for sub_08017D48, the input-to-direction helper used by
 * the large field actor update routine at 0x080130A0. The ROM implementation
 * remains assembled until this ordinary C reproduces its bytes.
 */
#include "gba/io_reg.h"
#include "gba/types.h"
#include "input.h"

#define ACTOR_FACING_UP 0
#define ACTOR_FACING_DOWN 1
#define ACTOR_FACING_RIGHT 2
#define ACTOR_FACING_LEFT 3

#define ACTOR_MOVE_NONE 0
#define ACTOR_MOVE_UP 1
#define ACTOR_MOVE_UP_RIGHT 2
#define ACTOR_MOVE_RIGHT 3
#define ACTOR_MOVE_DOWN_RIGHT 4
#define ACTOR_MOVE_DOWN 5
#define ACTOR_MOVE_DOWN_LEFT 6
#define ACTOR_MOVE_LEFT 7
#define ACTOR_MOVE_UP_LEFT 8

#define ACTOR_AUXILIARY_CODE_PRIMARY 3
#define ACTOR_AUXILIARY_CODE_SECONDARY 7

/**
 * @brief Update actor direction fields from the held directional keys.
 * @param facing Signed facing value, updated when the input changes facing.
 * @param movementDirection Receives an eight-way movement code or zero.
 * @param auxiliaryCode Receives 3 or 7 for diagonal and horizontal input;
 *                      vertical-only input leaves it unchanged.
 * @param keySlot Input-state slot passed to KeyInputAnyHeld.
 * @return One when a direction is held, otherwise zero.
 */
u32 ActorUpdateDirectionFromInputCandidate(s8 *facing,
                                           u8 *movementDirection,
                                           u8 *auxiliaryCode,
                                           u32 keySlot)
{
    if (KeyInputAnyHeld(KEY_UP, keySlot))
    {
        if (KeyInputAnyHeld(KEY_LEFT, keySlot))
        {
            if (*facing != ACTOR_FACING_LEFT)
                *facing = ACTOR_FACING_UP;
            *movementDirection = ACTOR_MOVE_UP_LEFT;
            *auxiliaryCode = ACTOR_AUXILIARY_CODE_SECONDARY;
            return 1;
        }
        if (KeyInputAnyHeld(KEY_RIGHT, keySlot))
        {
            if (*facing != ACTOR_FACING_RIGHT)
                *facing = ACTOR_FACING_UP;
            *movementDirection = ACTOR_MOVE_UP_RIGHT;
            *auxiliaryCode = ACTOR_AUXILIARY_CODE_PRIMARY;
            return 1;
        }
        *facing = ACTOR_FACING_UP;
        *movementDirection = ACTOR_MOVE_UP;
        return 1;
    }

    if (KeyInputAnyHeld(KEY_DOWN, keySlot))
    {
        if (KeyInputAnyHeld(KEY_LEFT, keySlot))
        {
            if (*facing != ACTOR_FACING_LEFT)
                *facing = ACTOR_FACING_DOWN;
            *movementDirection = ACTOR_MOVE_DOWN_LEFT;
            *auxiliaryCode = ACTOR_AUXILIARY_CODE_SECONDARY;
            return 1;
        }
        if (KeyInputAnyHeld(KEY_RIGHT, keySlot))
        {
            if (*facing != ACTOR_FACING_RIGHT)
                *facing = ACTOR_FACING_DOWN;
            *movementDirection = ACTOR_MOVE_DOWN_RIGHT;
            *auxiliaryCode = ACTOR_AUXILIARY_CODE_PRIMARY;
            return 1;
        }
        *facing = ACTOR_FACING_DOWN;
        *movementDirection = ACTOR_MOVE_DOWN;
        return 1;
    }

    if (KeyInputAnyHeld(KEY_LEFT, keySlot))
    {
        *facing = ACTOR_FACING_LEFT;
        *movementDirection = ACTOR_MOVE_LEFT;
        *auxiliaryCode = ACTOR_AUXILIARY_CODE_PRIMARY;
        return 1;
    }
    if (KeyInputAnyHeld(KEY_RIGHT, keySlot))
    {
        *facing = ACTOR_FACING_RIGHT;
        *movementDirection = ACTOR_MOVE_RIGHT;
        *auxiliaryCode = ACTOR_AUXILIARY_CODE_PRIMARY;
        return 1;
    }

    *movementDirection = ACTOR_MOVE_NONE;
    return 0;
}
