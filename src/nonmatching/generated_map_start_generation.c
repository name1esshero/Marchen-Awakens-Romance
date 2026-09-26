/* sub_08070DA8, the real logic behind DungGenStart
 * (ScriptNativeMapClearDState, which calls it as
 * GeneratedMapStartGeneration(GameStateGetBuffer38C0())).
 *
 * Logically confirmed correct against a direct force-thumb disassembly of
 * the raw ROM bytes (not the stale symbol-grouped ELF disassembly): looks
 * up the room record for the generation state's current cell, loads that
 * room's name-string field into the field-graphic KMP renderer, creates the
 * map-generation task (immediately running its callback), normalizes the
 * +0x644 field to 0 or 8 depending on what that callback left there, and
 * creates 8 sprite-reset tasks with IDs [that value .. that value + 7].
 *
 * Every field access, call argument, and branch condition matches. What
 * remains is a single register-allocation swap: the ROM keeps the incoming
 * state pointer in r5 and the computed &state->field644 pointer in r4;
 * every C shape tried here puts them the other way around (state in r4,
 * the field pointer in r5). Tried: the field pointer computed before vs.
 * after the room lookup; with and without a separate `room` local; with
 * the parameter typed as `struct GeneratedFieldMap *` directly vs. a `void
 * *state` parameter with a typed local assigned from it; and the field
 * pointer dereferenced once into a saved scalar before the room lookup
 * (which also pulled in an extra register, r7, making it worse, not
 * better). All were re-checked with the branch direction corrected first
 * (`if (*field644 == 0) *field644 = 8; else *field644 = 0;`, not the
 * reversed condition, which alone fixes an unrelated beq/bne mismatch).
 * None recovers the r4/r5 assignment. This is the same class of gap as
 * SpriteAffineWriteDispatch's register swap and GameStateSetEncounterValue
 * earlier this session -- a genuine agbcc allocator choice, not a logic
 * error.
 */
#include "gba/types.h"
#include "map_generation.h"

#define GENERATED_FIELD_MAP_FIELD644_OFFSET 0x644

extern void KmpLoadField(const char *name, s16 x, s16 y);
extern u8 *CreateMapGenerationTask(s32 state, s32 mode, s32 field644,
                                   s32 cell, s32 arg5, s32 arg6, s32 *result);
extern u8 *CreateSpriteResetTask(s32 sprite, s32 mode, s32 *result);

void GeneratedMapStartGeneration(struct GeneratedFieldMap *map)
{
    struct GeneratedMapRoomRecord *room;
    s32 *field644;
    s32 i;

    room = &map->rooms[map->cellRoomIndices[map->currentCell]];
    KmpLoadField(room->name, 0, 0);

    field644 = (s32 *)((u8 *)map + GENERATED_FIELD_MAP_FIELD644_OFFSET);
    /* PRET_PTR_INT_OK: operation=store the state pointer as a raw word in the new task's payload; evidence=CreateMapGenerationTask stores its first argument unchanged via `str r7,[r1,#28]`, never dereferencing it; typed=the constructor's signature takes s32 to match that raw-word store */
    CreateMapGenerationTask((s32)map, 1, *field644, map->currentCell, 0, 0, 0);
    if (*field644 == 0)
        *field644 = 8;
    else
        *field644 = 0;

    for (i = 0; i <= 7; i++)
        CreateSpriteResetTask(*field644 + i, 0, 0);
}
