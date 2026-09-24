/*
 * Readable candidates for the actor fields at offsets 0x350..0x357.
 *
 * The ROM routines at 0x08009F04 and 0x08009F44 are still assembled. Their
 * callers pass the actor index in r0; the machine code confirms a 1672-byte
 * record stride and the field offsets represented below. The field meanings
 * are not established, so the candidate names retain the offsets. Neither
 * routine replaces its ROM implementation until it matches under ordinary
 * agbcc output.
 */
#include "gba/types.h"
#include "runtime_state.h"

#define ACTOR_RECORD_SIZE 1672
#define ACTOR_STATE_350_OFFSET 0x350

struct RuntimeActorState350
{
    u8 field350;
    u8 field351;
    s16 field352;
    u32 field354;
};

/**
 * @brief Set actor fields 0x350 and 0x351, then clear field 0x354.
 * @param actor Actor index in the secondary runtime table.
 * @param value Low byte stored in field 0x351.
 */
void RuntimeActorSetFields350_351Candidate(s32 actor, s32 value)
{
    struct RuntimeActorState350 *state;

    state = (struct RuntimeActorState350 *)(gSecondaryRuntime
        + actor * ACTOR_RECORD_SIZE + ACTOR_STATE_350_OFFSET);
    state->field350 = 1;
    state->field351 = value;
    state->field354 = 0;
}

/**
 * @brief Clear actor fields 0x350 through 0x357.
 * @param actor Actor index in the secondary runtime table.
 */
void RuntimeActorClearFields350_357Candidate(s32 actor)
{
    struct RuntimeActorState350 *state;

    state = (struct RuntimeActorState350 *)(gSecondaryRuntime
        + actor * ACTOR_RECORD_SIZE + ACTOR_STATE_350_OFFSET);
    state->field350 = 0;
    state->field352 = 0;
    state->field351 = 0;
    state->field354 = 0;
}
