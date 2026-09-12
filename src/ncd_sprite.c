/* NCD instance initialization at 0807BC2C, instruction-matched with agbcc. */
#include "ncd.h"
extern void CpuFill(void *,u32,u32);
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
