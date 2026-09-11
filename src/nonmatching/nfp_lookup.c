/* NfpFindEntryIndex, from 0x0807ACC4.
 *
 * The reading is certain and the translation compiles to exactly the right
 * size, 136 bytes. What differs is register allocation: the original holds
 * the NUL it writes into the key buffer in r5, a callee-saved register, and
 * the stack pointer copy in r0. agbcc picks r0 and r1 instead, and the choice
 * ripples through the rest of the function, so "hi" ends up in r5 rather than
 * r4. Every instruction is otherwise the same, in the same order.
 *
 * Declaration order was tried both ways and does not move the allocator.
 * Nothing in the source appears to reach this, so it is recorded rather than
 * guessed at further.
 *
 * The behaviour this documents is worth keeping either way:
 *
 *   - the directory is sorted by name, which is what makes a binary search
 *     valid here; all 830 entries in the shipped archive are in order;
 *   - names are a fixed 12 bytes with no terminator when a name fills the
 *     field, so the search copies each candidate out and terminates it
 *     itself rather than comparing in place;
 *   - the loop narrows until the bounds meet and then tests that last
 *     candidate separately, so a hit can return from either place.
 */

#include "gba/types.h"
#include "nfp.h"

/* Find a member by name, or -1.
 *
 * A binary search, which the directory being sorted by name makes valid; all
 * 830 entries in the shipped archive are in order. The loop narrows until the
 * bounds meet and then tests that last candidate separately, so an exact hit
 * can return from either place.
 *
 * Names are copied out rather than compared in place because the field is a
 * fixed 12 bytes with no terminator when a name fills it.
 */
s32 NfpFindEntryIndexImpl(s32 handle, const char *name)
{
    char key[13];
    struct NfpEntry *dir;
    s32 count;
    s32 lo;
    s32 hi;
    s32 mid;
    int cmp;

    key[NFP_NAME_SIZE] = 0;

    dir = NfpGetDirectory(handle);
    count = NfpGetEntryCount(handle);
    if (count < 0)
        return -1;

    lo = 0;
    hi = count - 1;

    while (lo != hi)
    {
        mid = (lo + hi) / 2;
        CpuCopy(key, &dir[mid], NFP_NAME_SIZE);

        cmp = strcmp(name, key);
        if (cmp == 0)
            return mid;

        if (cmp < 0)
            hi = mid;
        else
            lo = mid + 1;
    }

    CpuCopy(key, &dir[lo], NFP_NAME_SIZE);
    if (strcmp(name, key) == 0)
        return lo;

    return -1;
}

