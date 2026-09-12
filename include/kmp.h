#ifndef KMP_H
#define KMP_H
#include "gba/types.h"

/* KMP header fields traced through 08002650, 08003104 and 08003178.
 * Offsets are relative to the KMP member. Unknown fields stay reserved. */
struct KmpHeader
{
    u8 reserved00[0x14];
    u32 widthTiles;                  /* 14 */
    u32 heightTiles;                 /* 18 */
    char tileResource[64];           /* 1C: KCG name */
    char paletteResource[64];        /* 5C: KCL name */
    u32 planeOffsets[4];             /* 9C: row-major u16 tilemap words */
    u32 attributesOffset;            /* AC */
    u32 paletteBaseBank;             /* B0: destination bank, each 32 bytes */
    u32 paletteBankCount;            /* B4 */
    s16 compressedTiles;             /* B8: nonzero selects decompression */
    s16 wordAttributes;              /* BA: nonzero selects u16, else u8 */
    u32 reservedBC;
};

/* Partial viewport; the existing Entity accessors refer to this map state.
 * The complete allocated slot is 0xFC bytes. */
struct KmpViewport
{
    const struct KmpHeader *data;
    u16 *screenBuffer;
    u8 renderMode, background, plane;
    u8 reserved0B[0x0D];
    u32 widthFixed, heightFixed;     /* 18, 1C: 16.16 pixel dimensions */
    u32 clipX, clipY, clipWidth, clipHeight;
};
void KmpInitViewport(struct KmpViewport *, const struct KmpHeader *, u16 *, u32, u32, u32);
void KmpRenderViewport(struct KmpViewport *, s32 xFixed, s32 yFixed);
void KmpLoadField(const char *name, s16 x, s16 y);
void KmpSetClip(struct KmpViewport *, u32 x, u32 y, u32 width, u32 height);
void KmpResetClip(struct KmpViewport *);
u16 *KmpAttributeAddress(struct KmpViewport *, u32, u32);
s32 KmpReadAttribute(struct KmpViewport *, s32 pixelX, s32 pixelY);
#endif
