/* Heap construction and the first-block allocation adapter. The allocator
 * itself remains assembly. Null HeapCreate storage allocates from the default
 * heap; HeapInitDefault instead requires caller-supplied storage.
 */
#include "heap.h"
#include "rom_section.h"
extern void CpuFill(void *, u32, u32);
extern void *sub_0807A54C(struct Heap *, u32, u32);

/** Heap init default in the engine heap allocator. */
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

/** Heap create in the engine heap allocator. */
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

/** Heap alloc in the engine heap allocator. */
AT("0007A2EC")
void *HeapAlloc(struct Heap *heap, u32 size)
{
    return sub_0807A54C(heap, size, 0);
}

/** Walk the boundary-tag blocks. The stored size includes the eight-byte
 * header; low bit 0 marks an allocation and low bit 1 marks the final block. */
AT("0007A4B8")
u32 HeapGetFreeBytes(struct Heap *heap)
{
    struct HeapBlock *block;
    u32 freeBytes;
    u32 taggedSize;
    u32 size;

    if (!heap)
        heap = gDefaultHeap;
    block = (struct HeapBlock *)((u8 *)heap + 8);
    freeBytes = 0;
    do {
        taggedSize = block->sizeAndFlags;
        size = taggedSize & ~3;
        if (!(taggedSize & 1))
            freeBytes += size - 8;
        block = (struct HeapBlock *)((u8 *)block + size);
    } while (!(taggedSize & 2));
    return freeBytes;
}

/** Heap get largest free block in the engine heap allocator. */
AT("0007A4F8")
u32 HeapGetLargestFreeBlock(struct Heap *heap)
{
    struct HeapBlock *block;
    u32 largest;
    u32 taggedSize;
    u32 size;

    if (!heap)
        heap = gDefaultHeap;
    block = (struct HeapBlock *)((u8 *)heap + 8);
    largest = 0;
    do {
        taggedSize = block->sizeAndFlags;
        size = taggedSize & ~3;
        if (!(taggedSize & 1)) {
            u32 payloadSize = size - 8;
            if (payloadSize > largest)
                largest = payloadSize;
        }
        block = (struct HeapBlock *)((u8 *)block + size);
    } while (!(taggedSize & 2));
    return largest;
}

/** Heap get allocation size in the engine heap allocator. */
AT("0007A53C")
u32 HeapGetAllocationSize(const void *allocation)
{
    const struct HeapBlock *block =
        (const struct HeapBlock *)((const u8 *)allocation - 8);
    return (block->sizeAndFlags & ~3) - 8;
}
AT("0007A53C") const u8 HeapGetAllocationSizeTail[2] = {0, 0};
