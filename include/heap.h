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
struct Heap *HeapInitDefault(void *memory, u32 size);
struct Heap *HeapCreate(void *memory, u32 size);
void *HeapAlloc(struct Heap *heap, u32 size);
#endif
