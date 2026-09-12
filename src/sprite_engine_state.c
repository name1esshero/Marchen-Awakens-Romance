/* Small accessors for the sprite renderer's global work state. */
#include "sprite_engine.h"

#define AT(x) __attribute__((section(".rom." x)))

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
