/* Natural-C candidate for GameStateAddResourceCounter (0x080562C8).
 *
 * The ROM reloads the root pointer after updating the counter. agbcc removes
 * that reload from this typed form, shortening the function by eight bytes.
 * A pointer/integer union can force the reload, but that merely steers code
 * generation and is not an authentic data model. Keep the exact function in
 * assembly until surrounding state ownership reveals a natural aliasing shape.
 */
#include "gba/types.h"

#include "game_state.h"

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

#define GAME_STATE_RESOURCE_COUNTER_OFFSET 0x38BC
#define GAME_STATE_RESOURCE_COUNTER_MAX 999999

struct GameStateResourceCounter
{
    u8 padding[GAME_STATE_RESOURCE_COUNTER_OFFSET];
    u32 value;
};

void GameStateAddResourceCounterCandidate(u32 value)
{
    struct GameStateResourceCounter **root;
    struct GameStateResourceCounter *state;

    root = (struct GameStateResourceCounter **)(gIwramBase +
                                                 (u32)gMapGenerationRootOffset);
    state = *root;
    state->value += value;
    if ((*root)->value >= GAME_STATE_RESOURCE_COUNTER_MAX)
        (*root)->value = GAME_STATE_RESOURCE_COUNTER_MAX;
}
