#ifndef SPRITE_ENGINE_H
#define SPRITE_ENGINE_H

#include "gba/types.h"

struct SpriteResourceHeader {
    u8 unknown00[64];
    u32 entryCount;
};

struct SpriteResourceLevel0 {
    u8 unknown00[8];
    u32 childBase;
    u8 unknown0C[4];
};

struct SpriteResourceLevel1 {
    u32 childBase;
    u32 unknown04;
};

struct SpriteResourceLevel2 {
    u32 childBase;
    u32 unknown04;
};

struct SpriteResourceLevel3 {
    u8 unknown00[4];
    u32 table24Index;
    u32 table28Index;
    u8 unknown0C[8];
};

struct SpriteResourceDescriptor {
    struct SpriteResourceHeader *header;
    s8 *bindingIndices;
    struct SpriteResourceLevel0 *level0;
    struct SpriteResourceLevel1 *level1;
    struct SpriteResourceLevel2 *level2;
    struct SpriteResourceLevel3 *level3;
    u8 *table24;
    u8 *table28;
};

/* Partial layout of the global sprite-rendering state. The 8-byte records are
 * consumed as OAM-shaped work entries. The exact roles of boundaries 1 and 2
 * remain under investigation, so their indexed representation is preserved. */
struct SpriteEngineState {
    u8 oamBoundaries[3];             /* 0x000; boundary 0 is allocated count */
    u8 pad03;
    u8 *oamEntries;                  /* 0x004; 8 bytes per entry */
    u8 unknown008[0x144];
    struct SpriteResourceBinding {
        struct SpriteBindingOwner *owner;
        s32 index;
    } bindings[16];                  /* 0x14C: back-references by group */
    u8 counters[16][4];             /* 0x1CC */
    u16 flags20C;
    u8 unknown20E[0x402];
    s16 value610;                    /* 0x610: meaning not yet established */
    s16 value612;                    /* 0x612: meaning not yet established */
    u8 unknown614[8];
    struct SpriteResourceDescriptor *resources; /* 0x61C; 32-byte records */
};

/* Partial 16-byte resource record. The handle at +14 controls whether the
 * object at +0 is released; bytes +4..+13 remain unknown. */
struct SpriteResource {
    void *data;
    u8 unknown04[10];
    s16 handle;
};

/* Partial owner record used by bindings[]. Its signed-byte array records the
 * group assigned to each owner entry; other owner fields remain unknown. */
struct SpriteBindingOwner {
    u8 unknown00[4];
    s8 *bindingIndices;
};

#ifndef gSpriteEngineState
#define gSpriteEngineState (*(struct SpriteEngineState **)0x03006118)
#endif

void *SpriteEngineGetOamEntry(u32 index);
void *SpriteEngineAllocateOamEntry(void);
u32 SpriteEngineGetOamBoundary(s32 index);
void SpriteEngineSetOamBoundary(u32 value, s32 index);
void SpriteEngineSetValue610(s16 value);
void SpriteEngineSetValue612(s16 value);
s32 SpriteEngineGetValue610(void);
s32 SpriteEngineGetValue612(void);
void SpriteResourceRelease(struct SpriteResource *resource);
void SpriteResourceSetHandle(struct SpriteResource *resource, s32 handle);
s32 SpriteResourceGetHandle(struct SpriteResource *resource);
void SpriteEngineSetFlag20C(u8 index, s32 set);
u32 SpriteEngineTestFlag20C(u8 index);
u32 SpriteEngineGetFlags20C(void);
u32 SpriteEngineGetCounter(u8 group, u32 index);
void SpriteEngineSetCounter(u8 group, u32 index, u8 value);
void SpriteEngineReleaseBinding(u8 group);
void SpriteEngineIncrementCounter(u8 group, u32 index);
void SpriteEngineDecrementCounter(u8 group, u32 index);
struct SpriteResourceLevel0 *SpriteResourceGetLevel0(u32 resource, u32 index);
u32 SpriteResourceGetEntryCount(u32 resource);
struct SpriteResourceLevel1 *SpriteResourceGetLevel1(u32 resource, u32 index0, u32 offset);
struct SpriteResourceLevel2 *SpriteResourceGetLevel2(u32 resource, u32 index0, u32 offset1, u32 offset2);
struct SpriteResourceLevel3 *SpriteResourceGetLevel3(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3);
void *SpriteResourceGetTable24(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3);
void *SpriteResourceGetTable28(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3);

#endif
