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
    u16 attr0, attr1; /* OAM-shaped size/flags, but X/Y encode centers; all cells are 4bpp */
    u32 paletteIndex;
    u32 tileIndex;    /* multiply by 32, then add tilesOffset */
    u32 unk_0C, unk_10;
};
/* Runtime instance, 52 bytes. Address fields remain explicit GBA u32s.
 * Flag names encode offsets where their complete semantics are still unknown.
 * x/y are screen pixels after the higher-level object subtracts the camera.
 * scaleX/scaleY use 0x100 for identity; remaining is the frame countdown.
 */
struct NcdSprite {
 u32 next, previous;
 s16 container, allocationPool;
 s32 group, animation, frame;
 s16 x,y,offsetX,offsetY;
 u16 remaining;
 u8 frameCount,cellCount;
 u8 reserved24:4, flag24:1, rest24:3;
 u8 flags25,flags26;
 u8 copyMode27:2, reserved27:1, flag27:1, rest27:4;
 u8 flag28:1, rest28:7;
 u8 reserved29;
 s16 angle,scaleX,scaleY;
 u32 cellHandles;
};
void NcdInitSprite(struct NcdSprite *sprite, s32 pool);
void NcdSpriteCopy(struct NcdSprite *destination, const struct NcdSprite *source);
void NcdSpriteDeepCopy(struct NcdSprite *destination, const struct NcdSprite *source);
void NcdRegisterResource(struct NcdHeader *header, u32 resource);
void NcdSpriteContainerReset(void *container);
#endif
