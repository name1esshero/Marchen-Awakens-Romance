/* Sprite-runtime block accessors.
 *
 * The whole sprite runtime lives in one 0x8CC-byte IWRAM block whose base
 * pointer is cached in gSpriteRuntime. Field 0x800 is a 32-bit flag word, and
 * the three u16 fields at 0x8C4/0x8C6/0x8C8 hold a triple of 9-bit values
 * that the scene runtime reads and writes together (see scene_native.c).
 */
#include "sprite_engine.h"

#include "rom_section.h"

/* Cached base of the sprite runtime block. */
#define SPRITE_RUNTIME_BLOCK gSpriteRuntime

#define SPRITE_RUNTIME_BLOCK_SIZE 0x8CC
#define SPRITE_RUNTIME_FLAGS 0x800
#define SPRITE_RUNTIME_FIELD_8C4 0x8C4
#define SPRITE_RUNTIME_FIELD_8C6 0x8C6
#define SPRITE_RUNTIME_FIELD_8C8 0x8C8

/* The 0x8C4 triple is stored as 9-bit values; 0x100 is the neutral setting. */
#define SPRITE_RUNTIME_FIELD_8C4_MASK 0x1FF
#define SPRITE_RUNTIME_FIELD_8C4_DEFAULT 0x100

extern void CpuFill(void *destination,u32 size,u32 value);
extern void SpriteRuntimeSetAllFlags800(u32 enabled);

void SpriteRuntimeSetFields8C4(s32 first,s32 second,s32 third);

/** Zero the sprite runtime block, install it as the cached base, reset the
 * 0x8C4 triple to its neutral default, and enable all 0x800 flags. */
AT("000804BC") void SpriteRuntimeInit(void *block)
{
 CpuFill(block,SPRITE_RUNTIME_BLOCK_SIZE,0);
 SPRITE_RUNTIME_BLOCK=block;
 SpriteRuntimeSetFields8C4(SPRITE_RUNTIME_FIELD_8C4_DEFAULT,
                           SPRITE_RUNTIME_FIELD_8C4_DEFAULT,
                           SPRITE_RUNTIME_FIELD_8C4_DEFAULT);
 SpriteRuntimeSetAllFlags800(1);
}

/** Set the sprite runtime's 0x8C4 triple, masking each value to 9 bits.
 * See SpriteRuntimeInit() and its unrecovered getter's note above. */
AT("00080504") void SpriteRuntimeSetFields8C4(s32 first,s32 second,s32 third)
{
 u8 *block;
 first&=SPRITE_RUNTIME_FIELD_8C4_MASK;
 second&=SPRITE_RUNTIME_FIELD_8C4_MASK;
 third&=SPRITE_RUNTIME_FIELD_8C4_MASK;
 block=SPRITE_RUNTIME_BLOCK;
 *(u16 *)(block+SPRITE_RUNTIME_FIELD_8C4)=first;
 *(u16 *)(block+SPRITE_RUNTIME_FIELD_8C6)=second;
 *(u16 *)(block+SPRITE_RUNTIME_FIELD_8C8)=third;
}

/** The matching getter for the 0x8C4 triple (0x0808053C) is still in
 * asm/code/code_0800C0.s: the ROM reloads the block pointer before the third
 * field, which agbcc common-subexpression-eliminates away in every natural
 * C spelling tried so far. */

AT("00080574") void SpriteRuntimeSetFlag800(u8 bit,u32 enabled)
{
 if (enabled)
  *(u32 *)(SPRITE_RUNTIME_BLOCK+SPRITE_RUNTIME_FLAGS)|=1<<bit;
 else
  *(u32 *)(SPRITE_RUNTIME_BLOCK+SPRITE_RUNTIME_FLAGS)&=~(1<<bit);
}

/** @return Whether a bit is set in the sprite runtime's 0x800 flag word.
 * See SpriteRuntimeSetFlag800(). */
AT("000805B4") u32 SpriteRuntimeTestFlag800(u8 bit)
{
 u32 *flags=(u32 *)(SPRITE_RUNTIME_BLOCK+SPRITE_RUNTIME_FLAGS);
 return *flags&(1<<bit);
}
