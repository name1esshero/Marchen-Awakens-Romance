#ifndef SPRITE_ENGINE_H
#define SPRITE_ENGINE_H

#include "gba/types.h"

typedef void (*SpriteCopyCallback)(void *destination, const void *source, u32 size);

struct SpriteResourceHeader {
    u8 unknown00[64];
    u32 entryCount;
};

struct SpriteResourceLevel0 {
    char name[8];
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
    void *buffer8;
    void *bufferC;
    u32 flags10;
    u32 flags14;
    u32 affineSearchCursor;           /* 0x018 */
    struct SpriteAffineSlot {
        u16 key;
        u16 unused02;
        u32 transform;
    } affineSlots[32];               /* 0x01C */
    void *heap11C;                    /* 0x11C: sprite/NCD allocation heap */
    u8 unknown120[0x2C];
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
    SpriteCopyCallback copyCallback620;
    SpriteCopyCallback copyCallback624;
};

/* Partial 16-byte resource record. The handle at +14 controls whether the
 * object at +0 is released; bytes +4..+13 remain unknown. */
struct SpriteResource {
    void *data;
    u8 unknown04[10];
    s16 handle;
};

/* Renderer vector. Rotations use signed 18.14 fixed-point components; the
 * projection helper interprets z as the point scale. */
struct SpriteVector3 {
    s32 x;
    s32 y;
    s32 z;
};

/* Affine sprite work record. Matrix coefficients are signed 18.14 values;
 * angle is one turn per 4096 units and each scale uses the engine's signed
 * fixed-point reciprocal helper. */
struct SpriteAffineTransform {
    u16 packedYLow;
    u16 packedXAndFlags;
    u32 packedCoordinateBits;
    s16 pa;
    s16 pb;
    s16 pc;
    s16 pd;
    s16 centerX;
    s16 centerY;
    s16 halfWidth;
    s16 halfHeight;
    u16 angle;
    s16 scaleX;
    s16 scaleY;
};

struct SpriteInterpolation {
    s32 count;
    s32 *x;
    s32 *y;
    s32 *segmentLength;
    s32 *segmentScale;
    s32 *work5;
    s32 *work6;
    s32 *work7;
    s32 *work8;
};

struct SpriteInterpolationPair {
    s32 base;
    s32 xStart;
    s32 yStart;
    s32 step;
    s32 xEnd;
    s32 yEnd;
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
void SpriteEngineSetAllFlags20C(s32 set);
u32 SpriteEngineGetFlags20C(void);
u32 SpriteEngineGetCounter(u8 group, u32 index);
void SpriteEngineSetCounter(u8 group, u32 index, u8 value);
void SpriteEngineReleaseBinding(u8 group);
void SpriteEngineIncrementCounter(u8 group, u32 index);
void SpriteEngineDecrementCounter(u8 group, u32 index);
s32 SpriteEngineFindReusableGroup(void);
s32 SpriteResourceBindGroup(u32 resource, u32 index);
s32 SpriteResourceFindGroup(u32 resource, const char *name);
struct SpriteResourceLevel0 *SpriteResourceGetLevel0(u32 resource, u32 index);
u32 SpriteResourceGetEntryCount(u32 resource);
struct SpriteResourceLevel1 *SpriteResourceGetLevel1(u32 resource, u32 index0, u32 offset);
struct SpriteResourceLevel2 *SpriteResourceGetLevel2(u32 resource, u32 index0, u32 offset1, u32 offset2);
struct SpriteResourceLevel3 *SpriteResourceGetLevel3(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3);
void *SpriteResourceGetTable24(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3);
void *SpriteResourceGetTable28(u32 resource, u32 index0, u32 offset1, u32 offset2, u32 offset3);
void *SpriteEngineGetBuffer8(void);
void SpriteEngineSetBuffer8(void *buffer);
void *SpriteEngineGetBufferC(void);
void SpriteEngineSetBufferC(void *buffer);
void *SpriteEngineGetBuffer4(void);
void SpriteEngineSetBuffer4(void *buffer);
void SpriteEngineSetCopyCallback620(SpriteCopyCallback callback);
SpriteCopyCallback SpriteEngineGetCopyCallback620(void);
void SpriteEngineDefaultCopy620(void *destination, const void *source, u32 size);
void SpriteEngineSetCopyCallback624(SpriteCopyCallback callback);
SpriteCopyCallback SpriteEngineGetCopyCallback624(void);
void SpriteEngineDefaultCopy624(void *destination, const void *source, u32 size);
void SpriteEngineCopyToBuffer8(u16 index, const void *source, u32 count);
void SpriteEngineCopyToBufferC(u8 index, const void *source);
void SpriteEngineSetFlag10(u8 index, u8 set);
u32 SpriteEngineTestFlag10(u8 index);
void SpriteEngineSetAllFlags10(s32 set);
u32 SpriteEngineGetFlags10(void);
void SpriteEngineSetFlag14(u8 index, s32 set);
u32 SpriteEngineTestFlag14(u8 index);
void SpriteEngineSetAllFlags14(s32 set);
u32 SpriteEngineGetFlags14(void);
s32 SpriteAffineFind(u16 key, u32 high, s16 low);
s32 SpriteAffineAllocate(u16 key, u32 high, s16 low);
void SpriteEngineSetAffineWork(void *work);
void *SpriteEngineGetAffineWork(void);
void SpriteGetViewportOrigin(u16 *x, u16 *y);
void *SpriteEngineGetBuffer4Entry(u32 index);
s32 SpriteMathDivide65536ByS16(s32 value);
s32 SpriteFixed8Multiply(s32 left, s32 right);
s32 SpriteFixed8Divide(s32 dividend, s32 divisor);
s32 SpriteVectorLengthFixed(s32 x, s32 y);
s32 SpriteFixedSqrt(s32 value);
u32 SpriteRecordSizeForCount(u32 count);
void SpriteVectorRotateX(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle);
void SpriteVectorRotateY(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle);
void SpriteVectorRotateZ(struct SpriteVector3 *out,
                         const struct SpriteVector3 *in, s32 angle);
void SpriteProjectPoint(struct SpriteVector3 *point);
void SpritePackAffinePosition(struct SpriteAffineTransform *transform);
void SpriteBuildAffineMatrix(struct SpriteAffineTransform *transform);
void SpriteInterpolationInit(struct SpriteInterpolation *state, s32 *storage,
                             const s16 *x, const s16 *y, volatile s32 count);
void SpriteInterpolationEvaluatePair(s32 position, s16 *outX, s16 *outY,
                                     const struct SpriteInterpolationPair *pair);
void SpriteInterpolationEvaluatePairClamped(
    s32 position, s16 *outX, s16 *outY,
    const struct SpriteInterpolationPair *pair);

#endif
