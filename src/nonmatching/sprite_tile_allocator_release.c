/*
 * Clean reference implementation of the OBJ-tile free-list release path.
 *
 * This source is intentionally not linked into the matching ROM. Verified
 * genuinely close, not just plausible: every instruction in this ~90-
 * instruction function matches the ROM byte-for-byte except one. At
 * `previousSize = sizeMask; previousSize &= previousFlags;`, the ROM moves
 * the mask (already held in r12 from an earlier `mov ip, r0`) into r0 and
 * ANDs it with the already-live `previousFlags` (r1), keeping the mask's
 * register as the destination (`mov r0, r12; ands r0, r1`). Every C shape
 * tried here -- the two-statement form, one combined
 * `sizeMask & previousFlags` expression (both operand orders), and reading
 * `previous->sizeAndFlags` fresh instead of the cached `previousFlags`
 * local -- keeps `previousFlags`'s own register (r1) as the destination
 * instead (`mov r2, ip; and r1, r1, r2`): the AND's logical operands
 * already match, only which one "survives" as the destination register
 * differs -- a genuine register allocation choice agbcc makes differently
 * from the ROM, not an instruction-order or expression-grouping gap. The
 * exact routine remains in asm/code/code_0780C0.s until a natural matching
 * C shape for this one instruction is found.
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
