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

/* The regular renderer adds ((paletteBankOffset << 12) | tileIndexOffset)
 * to each source u16 screen entry; it does not special-case tile 1023.
 * These offsets do not declare a second tileset: each KMP names one KCG/KCL.
 *
 * Partial viewport; the existing Entity accessors refer to this map state.
 * The complete allocated slot is 0xFC bytes. */
struct KmpViewport
{
    const struct KmpHeader *data;
    u16 *screenBuffer;
    u8 renderMode, background, plane;
    u8 reserved0B;
    u16 paletteBankOffset;          /* 0C: added to KMP destination palette bank */
    u16 tileIndexOffset;            /* 0E: added to each screen entry */
    u8 reserved10[8];
    u32 widthFixed, heightFixed;     /* 18, 1C: 16.16 pixel dimensions */
    u32 clipX, clipY, clipWidth, clipHeight;
    u8 reserved30[0xCC];             /* complete viewport slot is 0xFC bytes */
};

/* Fixed IWRAM viewport slots, indexed by struct size (0xFC bytes each).
 * Index 0 is the primary field viewport that collision/attribute probing
 * reads from; KmpLoadField also uses index 1 for the second rendered plane. */
#define gKmpViewports ((struct KmpViewport *)0x03003BC4)

void KmpInitViewport(struct KmpViewport *, const struct KmpHeader *, u16 *, u32, u32, u32);
void KmpRenderViewport(struct KmpViewport *, s32 xFixed, s32 yFixed);
void KmpLoadResource(const char *name, void *tileDestination, s32 slot, s32 plane,
                     s32 paletteOffset, s32 tileOffset, s32 flags);
u32 KmpGetCompressedTileAllocationSize(const char *mapResource);
void KmpLoadField(const char *name, s16 x, s16 y);
void KmpSetClip(struct KmpViewport *, u32 x, u32 y, u32 width, u32 height);
void KmpResetClip(struct KmpViewport *);
u16 *KmpAttributeAddress(struct KmpViewport *, u32, u32);
s32 KmpReadAttribute(struct KmpViewport *, s32 pixelX, s32 pixelY);
struct HitBounds;
s32 MapAttributeProbeDirection(s32 direction, s32 x, s32 y,
                               const struct HitBounds *bounds);
void MapGenerationSetProbeDirection(s32 direction, s32 x, s32 y,
                                    const struct HitBounds *bounds);
#endif
