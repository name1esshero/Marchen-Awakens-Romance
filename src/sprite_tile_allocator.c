/* Allocation bookkeeping for the renderer's 8 KiB OBJ tile staging area. */
#include "gba/types.h"
#include "heap.h"

#include "rom_section.h"
#ifdef __GNUC__
#define TARGET_REGISTER(name)
#define ALLOCATOR_MEMORY_BARRIER() ((void)0)
#define ALLOCATOR_VALUE_BARRIER(value) ((void)0)
#else
#define TARGET_REGISTER(name) asm(name)
#define ALLOCATOR_MEMORY_BARRIER() asm volatile("" ::: "memory")
#define ALLOCATOR_VALUE_BARRIER(value) asm volatile("" : "+r"(value))
#endif

extern void CpuFill(void *destination, u32 size, u32 value);

struct SpriteTileBlock {
    u16 link;
    u16 sizeAndFlags;
    u32 unknown04;
};

struct SpriteTileAllocator {
    struct SpriteTileBlock *blocks;
    struct Heap *heap;
    u16 cursor;
    u16 tileBase;
    u16 tileCount;
    s16 mode;
};

extern u32 SpriteTileBlockIndex(struct SpriteTileBlock *base,
                                struct SpriteTileBlock *block);

AT("0007B32C")
void *SpriteTileAllocatorInit(struct Heap *heap,
                              struct SpriteTileAllocator *allocator,
                              u32 tileBase, u32 tileCount)
{
    struct Heap *owner = heap;
    struct SpriteTileAllocator *state = allocator;
    u32 base = tileBase;
    u32 count = tileCount;
    register u32 allocationSize TARGET_REGISTER("r8");
    register u16 *blocks TARGET_REGISTER("r2");
    register u32 sentinel TARGET_REGISTER("r1");

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

AT("0007B64C")
u32 SpriteTileAllocatorFreeTotal(struct SpriteTileAllocator *allocator)
{
    register struct SpriteTileBlock *block TARGET_REGISTER("r4") = allocator->blocks;
    register u32 total TARGET_REGISTER("r5") = 0;
    register u32 sizeMask TARGET_REGISTER("r12") = 0x1FFF;
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

AT("0007B384")
void SpriteTileAllocatorReset(struct SpriteTileAllocator *allocator)
{
    if (allocator->blocks != 0)
        HeapFree(allocator->heap, allocator->blocks);
    CpuFill(allocator, 16, 0);
}

/* Release an OBJ-tile allocation and merge it with adjacent free spans.  The
 * packed high bits mark allocated/sentinel blocks; the low 13 bits are the
 * span in 8-byte block records. */
AT("0007B52C")
void SpriteTileAllocatorRelease(struct SpriteTileAllocator *allocator, s32 tile)
{
    register struct SpriteTileBlock *block TARGET_REGISTER("r4");
    struct SpriteTileBlock *previous;
    struct SpriteTileBlock *next;
    u32 flags;
    u32 size;
    u32 last;
    register u32 offset TARGET_REGISTER("r0");
    register u32 sizeMask TARGET_REGISTER("r12");
    register u32 maskValue TARGET_REGISTER("r0");
    register struct SpriteTileBlock *base TARGET_REGISTER("r1");
    u32 link;
    u32 previousFlags;
    register u32 previousSize TARGET_REGISTER("r0");
    register u32 nextFlags TARGET_REGISTER("r1");
    u32 nextSize;
    u32 combined;

    if (allocator->mode != 0)
        goto releaseFixedBlock;
    ALLOCATOR_MEMORY_BARRIER();
    offset = tile - allocator->tileBase;
    offset <<= 3;
    base = allocator->blocks;
    block = (struct SpriteTileBlock *)((u8 *)base + offset);
    block->sizeAndFlags &= 0x3FFF;
    flags = block->sizeAndFlags;
    maskValue = 0x1FFF;
    ALLOCATOR_VALUE_BARRIER(maskValue);
    sizeMask = maskValue;
    size = flags & sizeMask;
    link = block->link;
    previous = allocator->blocks + link;
    next = block + size;
    if (link != 0xFFFF) {
        previousFlags = previous->sizeAndFlags;
        if ((previousFlags & 0xC000) == 0) {
            block = previous;
            previousSize = sizeMask;
            previousSize &= previousFlags;
            size += previousSize;
            last = flags & 0x2000;
            block->sizeAndFlags = size | last;
            if (last != 0)
                goto updateCursor;
            next->link = SpriteTileBlockIndex(allocator->blocks, block);
        }
    }
    last = flags & 0x2000;
    if (last == 0) {
        nextFlags = next->sizeAndFlags;
        if ((nextFlags & 0xC000) == 0) {
            nextSize = nextFlags & 0x1FFF;
            size = (u16)(size + nextSize);
            combined = (nextFlags & 0x2000) | size;
            block->sizeAndFlags = combined;
            if ((combined & 0x2000) == 0) {
                next = block + size;
                next->link = SpriteTileBlockIndex(allocator->blocks, block);
            }
        }
    }
updateCursor:
    allocator->cursor = SpriteTileBlockIndex(allocator->blocks, block);
    return;

releaseFixedBlock:
    offset = tile - allocator->tileBase;
    offset <<= 3;
    base = allocator->blocks;
    block = (struct SpriteTileBlock *)((u8 *)base + offset);
    block->sizeAndFlags &= 0xBFFF;
}
