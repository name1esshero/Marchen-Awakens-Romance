/* NfpGetMountName, from 0x0807AA48.
 *
 * The logic is certain: return the mount's name, or NULL when the slot is
 * free. What does not reproduce is the branch layout. The original tests the
 * flag and falls straight through to the NULL case:
 *
 *     cmp  r0, #0
 *     bne  .active        @ active -> compute the name
 *     mov  r0, #0         @ inactive falls through to here
 *     b    .exit
 * .active:
 *     ...
 *
 * agbcc inverts that, emitting "beq" to a NULL case sunk to the end of the
 * function. Four source shapes were tried, an early return, an if/else, a
 * ternary, and a pre-initialised variable, at -O1, -O2 and -O3. All produce
 * the same inverted layout, so this is not something the source controls.
 *
 * Everything around it matches: NfpMountIsActive, NfpSetMountActive and
 * NfpFindArchive, which calls this one, are all byte-exact.
 */

#include "gba/types.h"
#include "nfp.h"

const char *NfpGetMountNameImpl(s32 handle)
{
    const char *name;

    if (NfpMountIsActive(handle))
        name = (const char *)&gNfpState->mounts[handle] + 1;
    else
        name = NULL;

    return name;
}
