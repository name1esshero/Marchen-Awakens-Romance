/* NCD instance initialization at 0807BC2C, instruction-matched with agbcc. */
#include "ncd.h"
#include "heap.h"
#include "list.h"
#include "sprite_engine.h"
extern void CpuFill(void *,u32,u32);
extern void CpuCopy(void *,const void *,u32);
extern void HeapFree(struct Heap *,void *);
#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#define NCD_ALLOCATION_BARRIER(value) ((void)0)
#else
#define TARGET_REGISTER(name) asm(name)
#define NCD_ALLOCATION_BARRIER(value) asm volatile("" : "+r"(value))
#endif

extern void SpriteTileAllocatorRelease(void *allocator, s32 tile);

/* Release one resource's palette-binding table and clear its four descriptor
 * slots.  NCD resources reserve 0x80 bytes here even though one descriptor is
 * 0x20 bytes. */
AT("0007B9CC") void NcdResetResource(u32 resource)
{
 u32 offset=resource;
 struct SpriteEngineState **global=
     (struct SpriteEngineState **)0x03006118;
 u32 resourcesOffset;
 struct SpriteEngineState *state=*global;
 struct Heap *heap=*(struct Heap **)((u8 *)state+0x11C);
 struct SpriteResourceDescriptor *resources;

 resourcesOffset=0x61C;
 resources=*(struct SpriteResourceDescriptor **)((u8 *)state+resourcesOffset);
 offset <<=5;
 {
  u32 slot=offset+(u32)resources;
  HeapFree(heap,*(void **)(slot+4));
 }
 {
  struct SpriteResourceDescriptor *resetBase;
  resetBase=*(struct SpriteResourceDescriptor **)
      ((u8 *)*global+resourcesOffset);
  CpuFill((u8 *)resetBase+offset,128,0);
 }
}

/* Sort one sprite into its flag-selected render queue and account for the
 * queued object.  Each priority bucket is a 12-byte List. */
AT("0007BDAC") void NcdQueueSprite(struct NcdSprite *sprite, u32 priority)
{
 register u8 *object TARGET_REGISTER("r4");
 register u8 **global TARGET_REGISTER("r5");
 register u8 *state TARGET_REGISTER("r3");
 register u32 rawFlags TARGET_REGISTER("r2");
 register u32 group TARGET_REGISTER("r0");
 register u8 *countState TARGET_REGISTER("r1");
 List *queue;
 register u32 offset TARGET_REGISTER("r2");
 object = (u8 *)sprite;
 global = (u8 **)0x03006118;
 state = *global;
 rawFlags = object[38];
 group = rawFlags & 12;
 state += 288;
 state += group;
 offset = priority;
 offset <<= 1;
 offset += priority;
 offset <<= 2;
 queue = *(List **)state;
 ListAppend((List *)((u8 *)queue + offset), (ListNode *)object);
 countState = *global;
 countState += 320;
 (*(u32 *)countState)++;
}
AT("0007BC2C") void NcdInitSprite(struct NcdSprite *sprite,s32 pool)
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

AT("0007BF60")
void NcdSpriteCopy(struct NcdSprite *destination, const struct NcdSprite *source)
{
 CpuCopy(destination,source,52);
 destination->copyMode27=1;
}

/* Register an NCD container and resolve its ROM-relative table offsets once.
 * Its signed-byte binding table has one slot per palette and starts unassigned. */
AT("0007B96C")
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
AT("0007BF80")
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

/* Reset the 72-byte owner record and initialize its embedded NCD sprite at
 * offset eight.  The four trailing halfwords are renderer bookkeeping. */
AT("00008A70") void NcdSpriteContainerReset(void *container)
{
 u8 *record=container;
 u32 zero=0;
 *(u16 *)(record+60)=zero;
 *(u16 *)(record+68)=zero;
 *(u16 *)(record+62)=zero;
 *(u16 *)(record+66)=zero;
 *(u32 *)record=zero;
 *(u32 *)(record+4)=zero;
 NcdInitSprite((struct NcdSprite *)(record+8),0);
}
AT("00008A70") const u8 NcdSpriteContainerResetTail[2]={0};

/* Byte-oriented view used while the remaining NCD runtime flags are decoded. */
struct NcdRuntimeAllocation {
 u8 unknown00[10];
 s16 resourceIndex;
 u8 unknown0C[23];
 u8 partCount;
 u8 unknown24[3];
 u8 flags27;
 u8 unknown28[8];
 void *allocation;
};

/* Release every per-cell OBJ-tile handle owned by a multipart sprite.  Copy
 * mode one borrows storage; copy mode two owns only its allocation array. */
AT("0007BEC0")
void NcdRuntimeSpriteReleaseAllocation(struct NcdSprite *sprite)
{
 struct NcdRuntimeAllocation *self = (struct NcdRuntimeAllocation *)sprite;
 u8 *allocation = self->allocation;
 u32 mode;
 struct Heap *heap;
 void *toFree;

 if (allocation == 0)
  return;
 mode = self->flags27;
 mode <<= 30;
 mode >>= 30;
 NCD_ALLOCATION_BARRIER(mode);
 switch ((s32)mode) {
 case 0:
  {
   u8 *state = *(u8 **)0x03006118;
   void *allocator = *(void **)(state + 0x618)
                   + self->resourceIndex * 16;
   u8 *part = allocation;
   u16 i = 0;
   register u8 *countTemp TARGET_REGISTER("r0") = (u8 *)self + 35;
   u8 *count;
   NCD_ALLOCATION_BARRIER(countTemp);
   count = countTemp;
   while (i < *count) {
    SpriteTileAllocatorRelease(allocator, *(s16 *)part);
    i++;
    part += 4;
   }
   heap = *(struct Heap **)(*(u8 **)0x03006118 + 0x11C);
   toFree = self->allocation;
  }
  break;
 case 1:
  return;
 case 2:
  heap = *(struct Heap **)(*(u8 **)0x03006118 + 0x11C);
  toFree = allocation;
  break;
 default:
  return;
 }
 HeapFree(heap, toFree);
 self->allocation = 0;
}
