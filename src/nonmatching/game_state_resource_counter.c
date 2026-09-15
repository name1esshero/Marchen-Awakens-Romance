/* sub_08056290 and sub_080562C8, called from
 * ScriptNativeQueryResourceState()/ScriptNativeSetResourceState() in
 * src/resource_native.c. Both read or write a u32 counter at the game root's
 * +0x38BC, capped at 999999 (0xF423E is 999998, the "not yet at the cap"
 * threshold both functions compare against) -- almost certainly an on-screen
 * currency or score total, given the classic six-nines RPG max. Named by
 * that behavior rather than by offset, since the cap is strong, if not
 * certain, evidence, but left without a stronger name (e.g. "gold") since no
 * caller ties it to a specific game-visible quantity yet.
 *
 * Both are logically confirmed against every instruction, branch, and
 * literal in the disassembly, but do not yet byte-match. Both keep the game
 * root's own *address* (gIwramBase + gMapGenerationRootOffset, i.e. what
 * src/mapping.c's GAME_ROOT macro also computes, not the dereferenced
 * pointer) live in one register and the 0x38BC field offset live in another
 * across the whole function, re-deriving the field's address from that pair
 * fresh at each use rather than caching a single pointer. Every ordinary C
 * shape tried for that pair -- named locals in either declaration order, a
 * GNU statement-expression macro matching src/runtime_accessors.c's
 * GAME_STATE_BASE style, folding the offset into a pointer up front -- lands
 * the two values in the opposite pair of registers from the ROM (wants the
 * address in r4 and the offset in r3; agbcc's own allocator picks r3 for the
 * address and r4 for the offset every time). This is the same failure mode,
 * and likely the same underlying agbcc allocator quirk, documented for
 * sub_08004DA8 in src/nonmatching/runtime_link_status.c and for the
 * `register`-pinned functions audited in docs/PRET_AUDIT.md. Per project
 * direction, no register-forcing hint is added here to paper over it.
 *
 * Also checked against old_agbcc (the snapshot src/sound_m4a.c and
 * src/sound_cgb_update.c build with): identical mismatch. Not a
 * wrong-compiler-snapshot issue for this function.
 */

#include "gba/types.h"

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

#define GAME_ROOT_RESOURCE_COUNTER_OFFSET 0x38BC
#define GAME_ROOT_RESOURCE_COUNTER_MAX 999999

u32 GameStateGetResourceCounter(void)
{
    u8 **root = (u8 **)(gIwramBase + (u32)gMapGenerationRootOffset);
    u32 offset = GAME_ROOT_RESOURCE_COUNTER_OFFSET;

    if (*(u32 *)(*root + offset) > GAME_ROOT_RESOURCE_COUNTER_MAX - 1)
        *(u32 *)(*root + offset) = GAME_ROOT_RESOURCE_COUNTER_MAX;

    return *(u32 *)(*root + offset);
}

void GameStateAddResourceCounter(u32 value)
{
    u8 **root = (u8 **)(gIwramBase + (u32)gMapGenerationRootOffset);
    u32 offset = GAME_ROOT_RESOURCE_COUNTER_OFFSET;

    *(u32 *)(*root + offset) += value;
    if (*(u32 *)(*root + offset) > GAME_ROOT_RESOURCE_COUNTER_MAX - 1)
        *(u32 *)(*root + offset) = GAME_ROOT_RESOURCE_COUNTER_MAX;
}
