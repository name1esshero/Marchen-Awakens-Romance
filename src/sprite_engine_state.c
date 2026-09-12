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
