#ifndef HEAP_H
#define HEAP_H
#include "gba/types.h"

/* Original 32-bit heap layout. The low size bits are allocator flags:
 * bit 0 marks an occupied block, bit 1 marks the last block in the heap.
 * The allocator at 0807A54C scans blocks by their size, not by a linked list.
 */
struct HeapBlock {
    u32 sizeAndFlags;
    u32 metadata; /* Cleared at creation; remaining uses are not decoded here. */
};
struct Heap {
    u32 size;
    struct HeapBlock *scanStart;
};
#ifndef gDefaultHeap
#define gDefaultHeap (*(struct Heap **)0x03006110)
#endif
/* A second, fixed IWRAM heap handle used by the object manager and the
 * archive loader (src/nonmatching/archive.c has its own local copy of this
 * same address). Nothing so far has been found that sets it separately from
 * gDefaultHeap, but the two globals are read from distinct addresses in the
 * ROM and are kept distinct here rather than assumed identical.
 *
 * The ROM computes this address as two separate loads (0x03000000 and
 * 0x00003FB4) added at runtime, rather than one folded literal; no C shape
 * tried so far reproduces that under agbcc, which always folds a
 * compile-time-constant sum. See src/nonmatching/object_free.c. */
#ifndef gHeapHandle
#define gHeapHandle (*(struct Heap **)0x03003FB4)
#endif
struct Heap *HeapInitDefault(void *memory, u32 size);
struct Heap *HeapCreate(void *memory, u32 size);
void *HeapAlloc(struct Heap *heap, u32 size);
void HeapFree(struct Heap *heap, void *allocation);
u32 HeapGetFreeBytes(struct Heap *heap);
u32 HeapGetLargestFreeBlock(struct Heap *heap);
u32 HeapGetAllocationSize(const void *allocation);
#endif
