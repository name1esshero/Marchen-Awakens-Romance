/* Small accessors for the sprite renderer's global work state. */
#include "sprite_engine.h"

#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#else
#define TARGET_REGISTER(name) asm(name)
#endif

extern void CpuCopy(void *destination, const void *source, u32 size);
extern s32 __divsi3(s32 dividend, s32 divisor);
extern char *strcpy(char *destination, const char *source);
extern char *strupr(char *string);
extern s32 memcmp(const void *left, const void *right, u32 size);

AT("0007B224")
void *SpriteEngineGetOamEntry(u32 index)
{
    return gSpriteEngineState->oamEntries + index * 8;
}

AT("0007B238")
void *SpriteEngineAllocateOamEntry(void)
{
    gSpriteEngineState->oamBoundaries[0]++;
    return gSpriteEngineState->oamEntries
         + gSpriteEngineState->oamBoundaries[0] * 8 - 8;
}

AT("0007B254")
u32 SpriteEngineGetOamBoundary(s32 index)
{
    switch (index) {
    case 0: return gSpriteEngineState->oamBoundaries[0];
    case 1: return gSpriteEngineState->oamBoundaries[1];
    case 2: return gSpriteEngineState->oamBoundaries[2];
    default: return 0;
    }
}

AT("0007B254") const u8 SpriteEngineGetOamBoundaryTail[2] = {0, 0};

AT("0007B298")
void SpriteEngineSetOamBoundary(u32 value, s32 index)
{
    switch (index) {
    case 0: gSpriteEngineState->oamBoundaries[0] = value; break;
    case 1: gSpriteEngineState->oamBoundaries[1] = value; break;
    case 2: gSpriteEngineState->oamBoundaries[2] = value; break;
    }
}

AT("0007B2D8")
void SpriteEngineSetValue610(s16 value)
{
    gSpriteEngineState->value610 = value;
}

AT("0007B2EC")
void SpriteEngineSetValue612(s16 value)
{
    gSpriteEngineState->value612 = value;
}

AT("0007B300")
s32 SpriteEngineGetValue610(void)
{
    return gSpriteEngineState->value610;
}

AT("0007B314")
s32 SpriteEngineGetValue612(void)
{
    return gSpriteEngineState->value612;
}

extern void sub_080869B8(void *);

AT("0007B618")
void SpriteResourceRelease(struct SpriteResource *resource)
{
    if (resource->handle != 0)
        sub_080869B8(resource->data);
}

AT("0007B618") const u8 SpriteResourceReleaseTail[2] = {0, 0};

AT("0007B630")
void SpriteResourceSetHandle(struct SpriteResource *resource, s32 handle)
{
    SpriteResourceRelease(resource);
    resource->handle = handle;
}

AT("0007B630") const u8 SpriteResourceSetHandleTail[2] = {0, 0};

AT("0007B644")
s32 SpriteResourceGetHandle(struct SpriteResource *resource)
{
    return resource->handle;
}

AT("0007B644") const u8 SpriteResourceGetHandleTail[2] = {0, 0};

AT("0007B6C8")
void SpriteEngineSetFlag20C(u8 index, s32 set)
{
    if (set)
        gSpriteEngineState->flags20C |= 1 << index;
    else
        gSpriteEngineState->flags20C &= ~(1 << index);
}

AT("0007B708")
u32 SpriteEngineTestFlag20C(u8 index)
{
    return gSpriteEngineState->flags20C & (1 << index);
}

/* Enable or clear every one of the renderer's sixteen resource-group flags. */
AT("0007B728")
void SpriteEngineSetAllFlags20C(s32 set)
{
    gSpriteEngineState->flags20C = set ? 0xFFFF : 0;
}

AT("0007B74C")
u32 SpriteEngineGetFlags20C(void)
{
    return gSpriteEngineState->flags20C;
}

AT("0007B798")
u32 SpriteEngineGetCounter(u8 group, u32 index)
{
    return gSpriteEngineState->counters[group][index];
}

AT("0007B7B4")
void SpriteEngineSetCounter(u8 group, u32 index, u8 value)
{
    gSpriteEngineState->counters[group][index] = value;
}

AT("0007B760")
void SpriteEngineReleaseBinding(u8 group)
{
    struct SpriteResourceBinding *binding = &gSpriteEngineState->bindings[group];

    if (binding->owner != 0) {
        if (binding->index != -1) {
            binding->owner->bindingIndices[binding->index] = -1;
            binding->index = -1;
        }
        binding->owner = 0;
    }
}

AT("0007B7D0")
void SpriteEngineIncrementCounter(u8 group, u32 index)
{
    u8 *counter = &gSpriteEngineState->counters[group][index];

    if (*counter != 0xFF)
        (*counter)++;
}

AT("0007B7F8")
void SpriteEngineDecrementCounter(u8 group, u32 index)
{
    u8 *counter = &gSpriteEngineState->counters[group][index];

    if (*counter != 0) {
        (*counter)--;
        if (*counter == 0)
            SpriteEngineReleaseBinding(group);
    }
}

/* Choose an unprotected resource group, preferring groups whose complete
 * four-level reference counter is empty before progressively weaker tests. */
AT("0007B830")
s32 SpriteEngineFindReusableGroup(void)
{
    u32 *counters = (u32 *)&gSpriteEngineState->counters[0][0];
    u32 *base = counters;
    u16 protected = SpriteEngineGetFlags20C();
    u16 bit = 1;
    s32 group = 0;

    while (group <= 15) {
        if (*counters == 0 && (protected & bit) == 0)
            goto found;
        group++;
        counters++;
        bit <<= 1;
    }
    bit = 1;
    counters = base;
    group = 0;
    while (group <= 15) {
        if ((*counters & 0x00FFFFFF) == 0 && (protected & bit) == 0)
            goto found;
        group++;
        counters++;
        bit <<= 1;
    }
    bit = 1;
    counters = base;
    group = 0;
    while (group <= 15) {
        if ((*counters & 0x0000FFFF) == 0 && (protected & bit) == 0)
            goto found;
        group++;
        counters++;
        bit <<= 1;
    }
    bit = 1;
    counters = base;
    group = 0;
    while (group <= 15) {
        if ((*counters & 0xFF) == 0 && (protected & bit) == 0)
            goto found;
        group++;
        counters++;
        bit <<= 1;
    }
    return -1;

found:
    SpriteEngineReleaseBinding((u8)group);
    return group;
}

/* Bind one palette/resource entry to a reusable renderer group.  The reverse
 * link lets the group's counters release the descriptor slot later. */
AT("0007B8F4")
s32 SpriteResourceBindGroup(u32 resource, u32 index)
{
    register u32 bindingIndex TARGET_REGISTER("r8") = index;
    register struct SpriteEngineState **global TARGET_REGISTER("r9") =
        (struct SpriteEngineState **)0x03006118;
    struct SpriteResourceDescriptor *descriptor =
        &(*global)->resources[resource];
    register u32 bindingOffset TARGET_REGISTER("r3") = bindingIndex;
    s8 *slot = &descriptor->bindingIndices[bindingOffset];
    s32 group = *slot;
    s32 previous = group;

    if (group == -1) {
        group = SpriteEngineFindReusableGroup();
        if (group != previous) {
            struct SpriteEngineState *state;
            SpriteEngineSetCounter((u8)group, 0, 0);
            *slot = group;
            state = *global;
            state->bindings[group].owner =
                (struct SpriteBindingOwner *)descriptor;
            state->bindings[group].index = bindingIndex;
        }
    }
    return group;
}

/* Group names occupy the first eight bytes of sorted 16-byte records. */
AT("0007BB98")
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
        register u32 byteOffset TARGET_REGISTER("r0") = low << 4;
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
AT("0007BB98") const u8 SpriteResourceFindGroupTail[2] = {0, 0};

AT("0007BA0C")
struct SpriteResourceLevel0 *SpriteResourceGetLevel0(u32 resource, u32 index)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    return &descriptor->level0[index];
}

AT("0007BA2C")
u32 SpriteResourceGetEntryCount(u32 resource)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    return descriptor->header->entryCount;
}

AT("0007BA48")
struct SpriteResourceLevel1 *SpriteResourceGetLevel1(u32 resource, u32 index0, u32 offset)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    struct SpriteResourceLevel0 *level0;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    index0 <<= 4;
    level0 = (struct SpriteResourceLevel0 *)((u8 *)descriptor->level0 + index0);
    return &descriptor->level1[level0->childBase + offset];
}

AT("0007BA78")
struct SpriteResourceLevel2 *SpriteResourceGetLevel2(u32 resource, u32 index0, u32 offset1, u32 offset2)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    struct SpriteResourceLevel0 *level0;
    struct SpriteResourceLevel1 *level1;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    index0 <<= 4;
    level0 = (struct SpriteResourceLevel0 *)((u8 *)descriptor->level0 + index0);
    level1 = &descriptor->level1[level0->childBase + offset1];
    return &descriptor->level2[level1->childBase + offset2];
}

AT("0007BAB0")
struct SpriteResourceLevel3 *SpriteResourceGetLevel3(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    struct SpriteResourceLevel0 *level0;
    struct SpriteResourceLevel1 *level1;
    struct SpriteResourceLevel2 *level2;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    index0 <<= 4;
    level0 = (struct SpriteResourceLevel0 *)((u8 *)descriptor->level0 + index0);
    level1 = &descriptor->level1[level0->childBase + offset1];
    level2 = &descriptor->level2[level1->childBase + offset2];
    return &descriptor->level3[level2->childBase + offset3];
}

AT("0007BAF8")
void *SpriteResourceGetTable24(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    struct SpriteResourceLevel0 *level0;
    struct SpriteResourceLevel1 *level1;
    struct SpriteResourceLevel2 *level2;
    struct SpriteResourceLevel3 *level3;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    index0 <<= 4;
    level0 = (struct SpriteResourceLevel0 *)((u8 *)descriptor->level0 + index0);
    level1 = &descriptor->level1[level0->childBase + offset1];
    level2 = &descriptor->level2[level1->childBase + offset2];
    level3 = &descriptor->level3[level2->childBase + offset3];
    return descriptor->table24 + level3->table24Index * 32;
}

AT("0007BB48")
void *SpriteResourceGetTable28(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3)
{
    struct SpriteResourceDescriptor *descriptor;
    struct SpriteResourceDescriptor **table = &gSpriteEngineState->resources;
    struct SpriteResourceLevel0 *level0;
    struct SpriteResourceLevel1 *level1;
    struct SpriteResourceLevel2 *level2;
    struct SpriteResourceLevel3 *level3;
    resource <<= 5;
    descriptor = (struct SpriteResourceDescriptor *)((u8 *)*table + resource);
    index0 <<= 4;
    level0 = (struct SpriteResourceLevel0 *)((u8 *)descriptor->level0 + index0);
    level1 = &descriptor->level1[level0->childBase + offset1];
    level2 = &descriptor->level2[level1->childBase + offset2];
    level3 = &descriptor->level3[level2->childBase + offset3];
    return descriptor->table28 + level3->table28Index * 32;
}

AT("0007D3C4")
void *SpriteEngineGetBuffer8(void)
{
    return gSpriteEngineState->buffer8;
}

AT("0007D3D0")
void SpriteEngineSetBuffer8(void *buffer)
{
    gSpriteEngineState->buffer8 = buffer;
}

AT("0007D3DC")
void *SpriteEngineGetBufferC(void)
{
    return gSpriteEngineState->bufferC;
}

AT("0007D3E8")
void SpriteEngineSetBufferC(void *buffer)
{
    gSpriteEngineState->bufferC = buffer;
}

AT("0007D3F4")
void *SpriteEngineGetBuffer4(void)
{
    return gSpriteEngineState->oamEntries;
}

AT("0007D400")
void SpriteEngineSetBuffer4(void *buffer)
{
    gSpriteEngineState->oamEntries = buffer;
}

AT("0007D40C")
void SpriteEngineSetCopyCallback620(SpriteCopyCallback callback)
{
    gSpriteEngineState->copyCallback620 = callback;
    if (callback == 0)
        gSpriteEngineState->copyCallback620 = SpriteEngineDefaultCopy620;
}

AT("0007D430")
SpriteCopyCallback SpriteEngineGetCopyCallback620(void)
{
    return gSpriteEngineState->copyCallback620;
}

AT("0007D47C")
void SpriteEngineDefaultCopy620(void *destination, const void *source, u32 size)
{
    CpuCopy(destination, source, size);
}

AT("0007D47C") const u8 SpriteEngineDefaultCopy620Tail[2] = {0, 0};

AT("0007D488")
void SpriteEngineSetCopyCallback624(SpriteCopyCallback callback)
{
    gSpriteEngineState->copyCallback624 = callback;
    if (callback == 0)
        gSpriteEngineState->copyCallback624 = SpriteEngineDefaultCopy624;
}

AT("0007D4AC")
SpriteCopyCallback SpriteEngineGetCopyCallback624(void)
{
    return gSpriteEngineState->copyCallback624;
}

AT("0007D4EC")
void SpriteEngineDefaultCopy624(void *destination, const void *source, u32 size)
{
    CpuCopy(destination, source, size);
}

AT("0007D4EC") const u8 SpriteEngineDefaultCopy624Tail[2] = {0, 0};

AT("0007D444")
void SpriteEngineCopyToBuffer8(u16 index, const void *source, u32 count)
{
    void *destination = (u8 *)SpriteEngineGetBuffer8() + index * 32;
    SpriteCopyCallback callback = SpriteEngineGetCopyCallback620();
    callback(destination, source, count * 32);
}

AT("0007D444") const u8 SpriteEngineCopyToBuffer8Tail[2] = {0, 0};

AT("0007D4C0")
void SpriteEngineCopyToBufferC(u8 index, const void *source)
{
    void *destination = (u8 *)SpriteEngineGetBufferC() + index * 32;
    SpriteCopyCallback callback = SpriteEngineGetCopyCallback624();
    callback(destination, source, 32);
}

AT("0007D4C0") const u8 SpriteEngineCopyToBufferCTail[2] = {0, 0};

AT("0007CCEC")
void SpriteEngineSetFlag10(u8 index, u8 set)
{
    if (set)
        gSpriteEngineState->flags10 |= 1 << index;
    else
        gSpriteEngineState->flags10 &= ~(1 << index);
}

AT("0007CD24")
u32 SpriteEngineTestFlag10(u8 index)
{
    return gSpriteEngineState->flags10 & (1 << index);
}

AT("0007CD3C")
void SpriteEngineSetAllFlags10(s32 set)
{
    struct SpriteEngineState *state = gSpriteEngineState;
    if (set)
        state->flags10 = -1;
    else
        state->flags10 = set;
}

AT("0007CD3C") const u8 SpriteEngineSetAllFlags10Tail[2] = {0, 0};

AT("0007CD5C")
u32 SpriteEngineGetFlags10(void)
{
    return gSpriteEngineState->flags10;
}

AT("0007CD68")
void SpriteEngineSetFlag14(u8 index, s32 set)
{
    if (set)
        gSpriteEngineState->flags14 |= 1 << index;
    else
        gSpriteEngineState->flags14 &= ~(1 << index);
}

AT("0007CD9C")
u32 SpriteEngineTestFlag14(u8 index)
{
    return gSpriteEngineState->flags14 & (1 << index);
}

AT("0007CDB4")
void SpriteEngineSetAllFlags14(s32 set)
{
    struct SpriteEngineState *state = gSpriteEngineState;
    if (set)
        state->flags14 = -1;
    else
        state->flags14 = set;
}

AT("0007CDB4") const u8 SpriteEngineSetAllFlags14Tail[2] = {0, 0};

AT("0007CDD4")
u32 SpriteEngineGetFlags14(void)
{
    return gSpriteEngineState->flags14;
}

/* The renderer keeps the active affine-work descriptor immediately before
 * the viewport origin.  Larger affine routines consume this pointer. */
AT("0007D01C")
void SpriteEngineSetAffineWork(void *work)
{
    *(void **)((u8 *)gSpriteEngineState + 0x144) = work;
}

AT("0007D030")
void *SpriteEngineGetAffineWork(void)
{
    return *(void **)((u8 *)gSpriteEngineState + 0x144);
}

AT("0007D23C")
void SpriteGetViewportOrigin(u16 *x, u16 *y)
{
    *x = *(u16 *)((u8 *)gSpriteEngineState + 0x148);
    *y = *(u16 *)((u8 *)gSpriteEngineState + 0x14A);
}

AT("0007CC04")
void *SpriteEngineGetBuffer4Entry(u32 index)
{
    return (u8 *)SpriteEngineGetBuffer4() + index * 32;
}

AT("0007CC04") const u8 SpriteEngineGetBuffer4EntryTail[2] = {0, 0};

AT("0007D92C")
s32 SpriteMathDivide65536ByS16(s32 value)
{
    s32 divisor = (s16)value;
    return (s16)__divsi3(0x10000, divisor);
}

AT("0007DB4C")
u32 SpriteRecordSizeForCount(u32 count)
{
    return (count + 1) * 32;
}

AT("0007DB4C") const u8 SpriteRecordSizeForCountTail[2] = {0, 0};
