/* ObjectFreeNcdResources (0x080281E0, 108 bytes) and
 * ObjectFreeAuxiliaryResources (0x0802824C, 104 bytes).
 *
 * The logic below is certain: both are no-ops on an inactive object
 * (struct Object, see include/object.h), both still free and zero the
 * record when it is non-null even if the release loop is skipped, and they
 * differ only in what per-entry release call they use before freeing:
 * ObjectFreeNcdResources calls NcdRuntimeSpriteReleaseAllocation directly on
 * each 72-byte entry (at offset +8 within it); ObjectFreeAuxiliaryResources
 * calls SpriteAuxiliaryReset on each entry instead (which itself calls
 * NcdRuntimeSpriteReleaseAllocation on entry+8). Which subsystem picks one
 * over the other is not yet known.
 *
 * What blocks a byte-exact match is the heap handle load. Both functions
 * read it as:
 *
 *   08028228: ldr r0, =0x03000000
 *   0802822A: ldr r1, =0x00003FB4
 *   0802822C: adds r0, r0, r1
 *   0802822E: ldr r0, [r0, #0]
 *
 * i.e. two literal-pool words added at runtime, then dereferenced -- not
 * one folded 0x03003FB4 literal. Every C shape tried collapses to the
 * single-literal form instead, because agbcc constant-folds any compile-time
 * sum regardless of how it is written (tested directly against agbcc:
 * `*(T**)(A+B)`, a local pointer variable split across two statements, a
 * struct-with-filler member access through a large offset, and an extern
 * array symbol plus a byte offset -- all four fold to one .word). The two
 * loads only reproduce if at least one operand is not a compile-time
 * constant to the compiler, which nothing in this file's own state
 * (`gHeapHandle`, from include/heap.h) can express. Whatever the original
 * source did to keep these apart, it is not visible from here.
 *
 * Sizes are otherwise exact: both compile to 12 bytes short of the ROM
 * range (96 of 108, and 92 of 104), consistent with losing exactly this one
 * extra load+add pair.
 */

#include "object.h"
#include "heap.h"
#include "ncd.h"

extern void HeapFree(struct Heap *heap, void *allocation);
extern void NcdRuntimeSpriteReleaseAllocation(struct NcdSprite *sprite);
extern void SpriteAuxiliaryReset(void *state);

void ObjectFreeNcdResourcesImpl(struct Object *object)
{
    s32 i;
    if (object->flags & OBJECT_ACTIVE)
    {
        if (object->record != 0)
        {
            if (object->flags & 0x0200)
            {
                for (i = 0; i < object->unk_18; i++)
                    NcdRuntimeSpriteReleaseAllocation((struct NcdSprite *)((u8 *)object->record + i * 72 + 8));
            }
            else
            {
                NcdRuntimeSpriteReleaseAllocation((struct NcdSprite *)((u8 *)object->record + 8));
            }
        }
        HeapFree(gHeapHandle, object->record);
        object->record = 0;
        object->flags = 0;
    }
}

void ObjectFreeAuxiliaryResourcesImpl(struct Object *object)
{
    s32 i;
    if (object->flags & OBJECT_ACTIVE)
    {
        if (object->record != 0)
        {
            if (object->flags & 0x0200)
            {
                for (i = 0; i < object->unk_18; i++)
                    SpriteAuxiliaryReset((u8 *)object->record + i * 72);
            }
            else
            {
                SpriteAuxiliaryReset(object->record);
            }
        }
        HeapFree(gHeapHandle, object->record);
        object->record = 0;
        object->flags = 0;
    }
}
