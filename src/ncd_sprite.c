/* NCD instance initialization at 0807BC2C, instruction-matched with agbcc. */
#include "ncd.h"
#include "heap.h"
#include "sprite_engine.h"
extern void CpuFill(void *,u32,u32);
extern void CpuCopy(void *,const void *,u32);
__attribute__((section(".rom.0007BC2C"))) void NcdInitSprite(struct NcdSprite *sprite,s32 pool)
{
 CpuFill(sprite,52,0);
 sprite->animation=-1;
 sprite->frame=-1;
 sprite->scaleX=256;
 sprite->scaleY=256;
 sprite->container=0xFFFF;
 sprite->allocationPool=pool;
 sprite->flag27 = 1;
 sprite->flag24 = 1;
 sprite->flag28 = 1;
}

__attribute__((section(".rom.0007BF60")))
void NcdSpriteCopy(struct NcdSprite *destination, const struct NcdSprite *source)
{
 CpuCopy(destination,source,52);
 destination->copyMode27=1;
}

/* Register an NCD container and resolve its ROM-relative table offsets once.
 * Its signed-byte binding table has one slot per palette and starts unassigned. */
__attribute__((section(".rom.0007B96C")))
void NcdRegisterResource(struct NcdHeader *header, u32 resource)
{
 struct SpriteResourceDescriptor *descriptor;

 descriptor = &gSpriteEngineState->resources[resource];
 descriptor->header = (struct SpriteResourceHeader *)header;
 descriptor->bindingIndices = HeapAlloc(gSpriteEngineState->heap11C,
                                        header->paletteCount);
 CpuFill(descriptor->bindingIndices, header->paletteCount, -1);
 descriptor->level0 = (void *)((u8 *)header + header->groupsOffset);
 descriptor->level1 = (void *)((u8 *)header + header->animationsOffset);
 descriptor->level2 = (void *)((u8 *)header + header->framesOffset);
 descriptor->level3 = (void *)((u8 *)header + header->cellsOffset);
 descriptor->table24 = (u8 *)header + header->palettesOffset;
 descriptor->table28 = (u8 *)header + header->tilesOffset;
}

/* Clone the instance and give the copy its own per-cell handle table. */
__attribute__((section(".rom.0007BF80")))
void NcdSpriteDeepCopy(struct NcdSprite *destination, const struct NcdSprite *source)
{
 CpuCopy(destination, source, 52);
 destination->cellHandles = (u32)HeapAlloc(gSpriteEngineState->heap11C,
                                           destination->cellCount * 4);
 if (destination->cellHandles != 0) {
  CpuCopy((void *)destination->cellHandles, (void *)source->cellHandles,
          destination->cellCount * 4);
  destination->copyMode27 = 2;
 }
}
