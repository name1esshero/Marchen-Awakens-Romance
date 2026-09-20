/*
 * Clean reference implementation of the sprite-resource group lookup.
 *
 * This source is intentionally not linked into the matching ROM. The
 * original blocker (a manually-computed `u32 byteOffset = low << 4; ...`
 * address, compiled with the opposite operand order from the ROM) is fixed
 * below by writing plain array indexing, `descriptor->level0[low]`,
 * matching the loop's own already-correct `descriptor->level0[middle]`
 * pattern exactly. This is the real fix, confirmed at the instruction-byte
 * level (`0x1840` for the old shape's `adds r0,r0,r1` versus `0x1808` for
 * this shape's still-not-quite-right add -- see below -- both genuinely
 * different encodings, not a display artifact of equivalent bytes).
 *
 * What remains open is narrower than before: the ROM computes the shift
 * (`low << 4`) into r0 and loads `level0` into r1 at this specific
 * post-loop site (`lsls r0,r6,#4` then `ldr r1,[r7,#8]`), while every C
 * shape tried here -- direct array indexing, and copying `low` into a
 * fresh local before indexing with it -- has agbcc compute the shift into
 * r1 and load `level0` into r0 instead (the registers swapped, though the
 * add's *logical* operands, level0 as the first addend and the shift as
 * the second, already match). The loop's own equivalent computation
 * matches byte-for-byte with the identical array-indexing C, which is why
 * this is likely tied to `low` being a long-lived loop-counter register at
 * this specific program point (alive since before the loop started) rather
 * than a freshly-derived value like the loop's own `middle`, not a flaw in
 * the array-indexing fix itself.
 */
#include "sprite_engine.h"

extern char *strcpy(char *destination, const char *source);
extern char *strupr(char *string);
extern s32 memcmp(const void *left, const void *right, u32 size);

s32 SpriteResourceFindGroup(u32 resource, const char *name)
{
    u32 keyWords[3];
    char *key;
    struct SpriteResourceDescriptor *descriptor;
    s32 low;
    s32 high;
    s32 middle;
    s32 order;

    keyWords[2] = 0;
    keyWords[1] = 0;
    keyWords[0] = 0;
    strcpy((char *)keyWords, name);
    strupr((char *)keyWords);
    key = (char *)keyWords;
    descriptor = &gSpriteEngineState->resources[resource];
    low = 0;
    high = descriptor->header->entryCount - 1;
    while (low != high) {
        middle = (low + high) / 2;
        order = memcmp(descriptor->level0[middle].name, key, 8);
        if (order == 0)
            goto middleFound;
        if (order > 0)
            high = middle;
        else
            low = middle + 1;
    }
    if (memcmp(descriptor->level0[low].name, key, 8) == 0)
        goto lowFound;
    return -1;
middleFound:
    return middle;
lowFound:
    return low;
}
