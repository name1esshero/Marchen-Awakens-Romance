/*
 * Clean reference implementation of the sprite-resource group lookup.
 *
 * This source is intentionally not linked into the matching ROM.  The clean
 * expression makes agbcc encode the final address addition as r0 + r1; the
 * original encodes the commutative operation as r1 + r0.  The exact routine
 * remains in asm/code/code_0780C0.s until a natural matching C shape is found.
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
    {
        u32 byteOffset = low << 4;
        byteOffset = (u32)descriptor->level0 + byteOffset;
        if (memcmp((void *)byteOffset, key, 8) == 0)
            goto lowFound;
    }
    return -1;
middleFound:
    return middle;
lowFound:
    return low;
}
