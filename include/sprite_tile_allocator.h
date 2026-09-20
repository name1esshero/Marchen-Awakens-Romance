#ifndef SPRITE_TILE_ALLOCATOR_H
#define SPRITE_TILE_ALLOCATOR_H

#include "gba/types.h"
#include "heap.h"

struct SpriteTileBlock
{
    u16 link;
    u16 sizeAndFlags;
    u32 unknown04;
};

struct SpriteTileAllocator
{
    struct SpriteTileBlock *blocks;
    struct Heap *heap;
    u16 cursor;
    u16 tileBase;
    u16 tileCount;
    s16 mode;
};

void *SpriteTileAllocatorInit(struct Heap *heap,
                              struct SpriteTileAllocator *allocator,
                              u32 tileBase, u32 tileCount);
void SpriteTileAllocatorReset(struct SpriteTileAllocator *allocator);
void SpriteTileAllocatorRelease(struct SpriteTileAllocator *allocator,
                                s32 tile);
u32 SpriteTileAllocatorFreeTotal(struct SpriteTileAllocator *allocator);
u32 SpriteTileAllocatorLargestFree(struct SpriteTileAllocator *allocator);
u32 SpriteTileBlockIndex(struct SpriteTileBlock *base,
                         struct SpriteTileBlock *block);

#endif /* SPRITE_TILE_ALLOCATOR_H */
