#ifndef NCD_H
#define NCD_H
#include "gba/types.h"

/* NCD (C)2004 NOBODY. The six offsets are resolved at 0807B96C.
 * All offsets are relative to the start of this header. */
struct NcdHeader
{
    u8 signature[0x1C];
    u32 formatWord; /* 0x3F828F5C in all three containers; meaning unconfirmed */
    u32 groupsOffset, animationsOffset, framesOffset, cellsOffset;
    u32 palettesOffset, tilesOffset;
    u32 reserved_38[2];
    u32 groupCount, animationCount, frameCount, cellCount;
    u32 cellCountCopy, paletteCount;
    /* Verified across all cells in CHR/EFFECT/SYSTEM. */
    u32 referencedTileCount; /* sum(width * height / 64), including reuse */
    u32 storedTileCount;     /* unique tile-pool length / 32 */
    u8 reserved_60[0x20];
};
struct NcdGroup { char name[8]; u32 firstAnimation, animationCount; };
struct NcdAnimation { u32 firstFrame, frameCount; };
struct NcdFrame { u32 firstCell; u16 cellCount, duration; };
struct NcdCell
{
    u16 attr0, attr1; /* OAM-shaped position/size fields; all cells are 4bpp */
    u32 paletteIndex;
    u32 tileIndex;    /* multiply by 32, then add tilesOffset */
    u32 unk_0C, unk_10;
};
#endif
