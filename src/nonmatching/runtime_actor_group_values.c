#include "gba/types.h"
#include "runtime_state.h"

/**
 * @brief A per-(actor,group) run of five signed halfwords.
 * @param actor Actor index; scaled by the 1672-byte actor-record stride.
 * @param group Group index; scaled by the 104-byte per-group stride.
 * @return Pointer to the five-entry s16 array at actor-record +0x538.
 *
 * Same actor-record base as the matching RuntimeGetActorPartRecord
 * (src/runtime_objects.c, +0x4DC, same 104-byte stride): both read
 * `gSecondaryRuntime[actor*1672+0x120] + group_or_part*104 + offset`. Only
 * the trailing offset (+0x538 here) differs.
 *
 * That different offset is exactly what blocks the match. 0x4DC needs a full
 * pool-constant load, so agbcc puts the running actor-record base in r2 and
 * the freshly dereferenced gSecondaryRuntime value in r0. 0x538 (= 167 << 3)
 * fits a `movs`+`lsls` immediate load instead, and with that cheaper
 * encoding available agbcc allocates the *opposite* way: the actor
 * accumulator stays in r0 and the dereferenced base moves to r2. Six
 * combinations of statement order and grouping were tried against the exact
 * shape below (splitting the dereference from the actor-offset add,
 * reordering which happens first, computing the group offset before or
 * after the base) and every one kept the dereferenced base in r0, never r2.
 *
 * Established by its own caller (RuntimeCountMatchingValues in
 * src/runtime_buffers.c, already matching): the second parameter is named
 * "group", not "part", and the return type is s16*, not void*.
 */
s16 *RuntimeGetActorGroupValuesCandidate(u32 actor, u32 group)
{
    u8 **root = &gSecondaryRuntime;
    u32 actorOffset = actor * 1672;
    u8 *base;
    u32 groupOffset;

    actorOffset += 0x120;
    base = *root + actorOffset;
    groupOffset = group * 104;
    groupOffset += 0x538;
    return (s16 *)(base + groupOffset);
}
