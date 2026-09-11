/* Field accessors on the game-object struct.
 *
 * Each of these is a two-instruction leaf the original compiler emitted for a
 * plain field read. They are pinned to the ROM offsets they occupy so the
 * linker drops them exactly where the assembly used to be. */

#include "gba/types.h"
#include "entity.h"

#define AT_ROM(off) __attribute__((section(".rom." off), used))

AT_ROM("000032A8") u32 GetEntityField20(struct Entity *entity)
{
    return entity->unk_20;
}

AT_ROM("000032AC") u32 GetEntityField24(struct Entity *entity)
{
    return entity->unk_24;
}

AT_ROM("000032B0") u32 GetEntityField28(struct Entity *entity)
{
    return entity->unk_28;
}

AT_ROM("000032B4") u32 GetEntityField2C(struct Entity *entity)
{
    return entity->unk_2C;
}

/* sub_08003358 tests this word before building the render request passed to
 * sub_080869A8. When nonzero, it takes descriptor data from entity+0xF4 and
 * request fields from entity+0x44/+0x48 instead of the default descriptor.
 * Preserve the entire word: the setter does not normalize it to 0 or 1.
 * The getter's result is also forwarded in the request at stack offset 0x30.
 */
/* Both live in one section because they are contiguous in the ROM. That lets
 * the compiler's own ".align 2, 0" supply the two zero bytes that sit between
 * them, exactly as it did originally. Pinning each function to its own
 * section instead makes the assembler pad the tail of each one with a THUMB
 * nop (0xC046), which is not what the cartridge holds. */
AT_ROM("00003348") void SetEntityRenderOverride(struct Entity *entity, u32 enabled)
{
    entity->renderOverride = enabled;
}

AT_ROM("00003348") u32 GetEntityRenderOverride(struct Entity *entity)
{
    return entity->renderOverride;
}

/* The last two bytes are alignment, not instructions. They are declared here
 * so the section ends on a word boundary with zeros rather than a nop. */
AT_ROM("00003348") const u8 EntityRenderOverrideTail[2] = {0, 0};

/* The assembly that has not been decompiled yet still calls these by the
 * labels the splitter generated, so export those names as aliases. */
u32 sub_080032A8(struct Entity *) __attribute__((alias("GetEntityField20")));
u32 sub_080032AC(struct Entity *) __attribute__((alias("GetEntityField24")));
u32 sub_080032B0(struct Entity *) __attribute__((alias("GetEntityField28")));
u32 sub_080032B4(struct Entity *) __attribute__((alias("GetEntityField2C")));
void sub_08003348(struct Entity *, u32) __attribute__((alias("SetEntityRenderOverride")));
u32 sub_08003350(struct Entity *) __attribute__((alias("GetEntityRenderOverride")));
