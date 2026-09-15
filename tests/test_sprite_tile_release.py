"""Check tile release addressing and free-span coalescing in the actual C."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class SpriteTileReleaseTests(unittest.TestCase):
    def test_release_modes_and_adjacent_spans(self):
        with tempfile.TemporaryDirectory() as temp:
            source = Path(temp) / 'test.c'
            source.write_text(r'''
#include <assert.h>
#include <string.h>
#include "src/sprite_tile_allocator.c"

u32 SpriteTileBlockIndex(struct SpriteTileBlock *base,
                         struct SpriteTileBlock *block)
{
    return block - base;
}

int main(void)
{
    struct SpriteTileBlock blocks[9];
    struct SpriteTileAllocator allocator = {0};
    memset(blocks, 0, sizeof(blocks));
    allocator.blocks = blocks;
    allocator.tileBase = 100;
    allocator.mode = 1;
    allocator.cursor = 8;
    blocks[2].sizeAndFlags = 0x4567;
    blocks[3].sizeAndFlags = 0xC005;
    blocks[4].sizeAndFlags = 0x6789;
    SpriteTileAllocatorRelease(&allocator, 103);
    assert(blocks[3].sizeAndFlags == 0x8005);
    assert(blocks[2].sizeAndFlags == 0x4567);
    assert(blocks[4].sizeAndFlags == 0x6789);
    assert(allocator.cursor == 8);

    /* Release the middle allocation and merge both neighboring free spans. */
    memset(blocks, 0, sizeof(blocks));
    allocator.mode = 0;
    blocks[0].link = 0xFFFF;
    blocks[0].sizeAndFlags = 2;
    blocks[2].link = 0;
    blocks[2].sizeAndFlags = 0x4002;
    blocks[4].link = 2;
    blocks[4].sizeAndFlags = 3;
    blocks[7].link = 4;
    blocks[7].sizeAndFlags = 0x6001;
    blocks[8].unknown04 = 0x12345678;
    SpriteTileAllocatorRelease(&allocator, 102);
    assert(blocks[0].sizeAndFlags == 7);
    assert(blocks[7].link == 0);
    assert(blocks[7].sizeAndFlags == 0x6001);
    assert(blocks[8].unknown04 == 0x12345678);
    assert(allocator.cursor == 0);
    assert(SpriteTileAllocatorFreeTotal(&allocator) == 7);
    assert(SpriteTileAllocatorLargestFree(&allocator) == 7);

    /* A final span keeps its end marker and cannot merge into a busy predecessor. */
    memset(blocks, 0, sizeof(blocks));
    blocks[0].link = 0xFFFF;
    blocks[0].sizeAndFlags = 0x4002;
    blocks[2].link = 0;
    blocks[2].sizeAndFlags = 0x6003;
    SpriteTileAllocatorRelease(&allocator, 102);
    assert(blocks[0].sizeAndFlags == 0x4002);
    assert(blocks[2].sizeAndFlags == 0x2003);
    assert(allocator.cursor == 2);
    assert(SpriteTileAllocatorFreeTotal(&allocator) == 3);
    return 0;
}
''')
            executable = Path(temp) / 'test'
            subprocess.run([
                'gcc', '-O2', '-ffunction-sections', '-fdata-sections',
                '-DAT(x)=', '-I' + str(ROOT), '-I' + str(ROOT / 'include'),
                str(source), '-Wl,--gc-sections', '-o', str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)
