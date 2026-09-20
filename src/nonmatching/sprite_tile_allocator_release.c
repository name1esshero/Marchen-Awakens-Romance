/*
 * Clean reference implementation of the OBJ-tile free-list release path.
 *
 * This source is intentionally not linked into the matching ROM.  agbcc
 * assigns the previous-span mask to r2 here, while the original instruction
 * stream assigns it to r0.  The generated code is otherwise equivalent.  The
 * exact routine remains in asm/code/code_0780C0.s until a natural matching C
 * shape is found.
 */
#include "sprite_tile_allocator.h"

void SpriteTileAllocatorRelease(struct SpriteTileAllocator *allocator, s32 tile)
{
    struct SpriteTileBlock *block;
    struct SpriteTileBlock *previous;
    struct SpriteTileBlock *next;
    u32 flags;
    u32 size;
    u32 last;
    u32 sizeMask;
    u32 maskValue;
    u32 link;
    u32 previousFlags;
    u32 previousSize;
    u32 nextFlags;
    u32 nextSize;
    u32 combined;

    if (allocator->mode != 0)
        goto releaseFixedBlock;
    block = allocator->blocks + (tile - allocator->tileBase);
    block->sizeAndFlags &= 0x3FFF;
    flags = block->sizeAndFlags;
    maskValue = 0x1FFF;
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
    block = allocator->blocks + (tile - allocator->tileBase);
    block->sizeAndFlags &= 0xBFFF;
}
