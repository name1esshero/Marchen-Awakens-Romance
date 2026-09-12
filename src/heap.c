/* Heap construction and the first-block allocation adapter. The allocator
 * itself remains assembly. Null HeapCreate storage allocates from the default
 * heap; HeapInitDefault instead requires caller-supplied storage.
 */
#include "heap.h"
#define AT(x) __attribute__((section(".rom." x)))
extern void CpuFill(void *, u32, u32);
extern void *sub_0807A54C(struct Heap *, u32, u32);

AT("0007A270")
struct Heap *HeapInitDefault(void *memory, u32 size)
{
    struct Heap *heap;

    if (!memory)
        return 0;
    CpuFill(memory, 16, 0);
    heap = HeapCreate(memory, size);
    if (!heap)
        return 0;
    gDefaultHeap = heap;
    return heap;
}

AT("0007A2A8")
struct Heap *HeapCreate(void *memory, u32 size)
{
    struct Heap *heap = memory;
    struct HeapBlock *block;
    u32 alignedSize = (size + 3) & ~3;

    if (alignedSize <= 24)
        return 0;
    if (!heap) {
        heap = HeapAlloc(0, alignedSize);
        if (!heap)
            return 0;
    }
    heap->size = alignedSize;
    block = (struct HeapBlock *)((u8 *)heap + 8);
    block->sizeAndFlags = (alignedSize - 8) | 2;
    block->metadata = 0;
    heap->scanStart = block;
    return heap;
}

AT("0007A2EC")
void *HeapAlloc(struct Heap *heap, u32 size)
{
    return sub_0807A54C(heap, size, 0);
}
