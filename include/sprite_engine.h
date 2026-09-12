#ifndef SPRITE_ENGINE_H
#define SPRITE_ENGINE_H

#include "gba/types.h"

/* Partial layout of the global sprite-rendering state. The 8-byte records are
 * consumed as OAM-shaped work entries. The exact roles of boundaries 1 and 2
 * remain under investigation, so their indexed representation is preserved. */
struct SpriteEngineState {
    u8 oamBoundaries[3];             /* 0x000; boundary 0 is allocated count */
    u8 pad03;
    u8 *oamEntries;                  /* 0x004; 8 bytes per entry */
    u8 unknown008[0x1C4];
    u8 counters[16][4];             /* 0x1CC */
    u16 flags20C;
    u8 unknown20E[0x402];
    s16 value610;                    /* 0x610: meaning not yet established */
    s16 value612;                    /* 0x612: meaning not yet established */
};

/* Partial 16-byte resource record. The handle at +14 controls whether the
 * object at +0 is released; bytes +4..+13 remain unknown. */
struct SpriteResource {
    void *data;
    u8 unknown04[10];
    s16 handle;
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

#endif
