/* Allocation bookkeeping for the renderer's 8 KiB OBJ tile staging area. */
#include "sprite_tile_allocator.h"

#include "rom_section.h"

extern void CpuFill(void *destination, u32 size, u32 value);

/**
 * @brief Allocate and initialize an 8 KiB OBJ-tile free-list for a tile range.
 * @param heap Heap the block table itself is allocated from.
 * @param allocator Allocator state to initialize.
 * @param tileBase First OBJ tile index this allocator manages.
 * @param tileCount Number of OBJ tiles this allocator manages.
 * @return The new block table, or NULL if the range is out of bounds or the
 * heap allocation fails.
 */
AT("0007B32C")
void *SpriteTileAllocatorInit(struct Heap *heap,
                              struct SpriteTileAllocator *allocator,
                              u32 tileBase, u32 tileCount)
{
    struct Heap *owner = heap;
    struct SpriteTileAllocator *state = allocator;
    u32 base = tileBase;
    u32 count = tileCount;
    u32 allocationSize;
    u16 *blocks;
    u32 sentinel;

    if (base + count > 1024)
        return 0;
    state->heap = owner;
    allocationSize = 8192;
    blocks = HeapAlloc(owner, allocationSize);
    state->blocks = (struct SpriteTileBlock *)blocks;
    if (blocks == 0)
        return 0;
    state->tileBase = base;
    state->tileCount = count;
    state->mode = 0;
    blocks[1] = count | allocationSize;
    sentinel = 0xFFFF;
    blocks[0] = sentinel;
    state->cursor = 0;
    return ((volatile struct SpriteTileAllocator *)state)->blocks;
}

/**
 * @brief Sum every free block's size across the whole allocator.
 * @param allocator Allocator to scan.
 * @return Total free space, in 8-byte block units.
 */
AT("0007B64C")
u32 SpriteTileAllocatorFreeTotal(struct SpriteTileAllocator *allocator)
{
    struct SpriteTileBlock *block = allocator->blocks;
    u32 total = 0;
    u32 sizeMask = 0x1FFF;
    u32 unavailableMask = 0xC000;
    u32 finalMask = 0x2000;

    do {
        u16 flags = block->sizeAndFlags;
        u32 size = flags & sizeMask;
        u32 span = size;
        if ((flags & unavailableMask) == 0)
            total += size;
        block += span;
        if ((flags & finalMask) != 0)
            break;
    } while (1);
    return total;
}

/**
 * @brief Find the single largest contiguous free block.
 * @param allocator Allocator to scan.
 * @return Size of the largest free block, in 8-byte block units.
 */
AT("0007B68C")
u32 SpriteTileAllocatorLargestFree(struct SpriteTileAllocator *allocator)
{
    struct SpriteTileBlock *block = allocator->blocks;
    u32 largest = 0;

    do {
        u16 flags = block->sizeAndFlags;
        u32 size = flags & 0x1FFF;
        if ((flags & 0xC000) == 0 && size > largest)
            largest = size;
        block += size;
        if ((flags & 0x2000) != 0)
            break;
    } while (1);
    return largest;
}

/**
 * @brief Free the allocator's block table and clear its state to zero.
 * @param allocator Allocator to reset.
 * @return Nothing.
 */
AT("0007B384")
void SpriteTileAllocatorReset(struct SpriteTileAllocator *allocator)
{
    if (allocator->blocks != 0)
        HeapFree(allocator->heap, allocator->blocks);
    CpuFill(allocator, 16, 0);
}
