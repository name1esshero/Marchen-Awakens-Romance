"""Exercise matching pool and map-query C at their caller-visible boundaries."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class MapPoolBatchTests(unittest.TestCase):
    def test_draws_scan_limits_and_inclusive_mirrored_bounds(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'harness.c'
            source.write_text(r'''
#include <assert.h>
#include <stdlib.h>
#include "random.h"
#include "heap.h"
#include "kmp.h"
#include "map_generation.h"
#include "hit_region.h"

static u32 randomCalls, allocations;
static u32 randomValue;
u32 Random(void) { randomCalls++; return randomValue; }
void *HeapAlloc(struct Heap *heap, u32 size)
{
    assert(heap == 0); allocations++; return malloc(size);
}

static const s32 attributes[6] = {400, 0, 400, 0, 400, 0};
static s32 probes;
s32 KmpReadAttribute(struct KmpViewport *view, s32 x, s32 y)
{
    assert(view == 0);
    assert(x % KMP_TILE_SIZE == 0 && y % KMP_TILE_SIZE == 0);
    probes++;
    x /= KMP_TILE_SIZE; y /= KMP_TILE_SIZE;
    if (x < 0 || y < 0 || x >= 3 || y >= 2) return -1;
    return attributes[y * 3 + x];
}

static s32 states[4], fields[4], rectangles[4][4];
s32 MapGenerationGetEntryState(u32 i) { return states[i]; }
s32 MapGenerationGetFieldId(u32 i) { return fields[i]; }
s32 MapGenerationGetVectorValue0(u32 i) { return rectangles[i][0]; }
s32 MapGenerationGetVectorValue2(u32 i) { return rectangles[i][1]; }
s32 MapGenerationGetVectorValue4(u32 i) { return rectangles[i][2]; }
s32 MapGenerationGetVectorValue6(u32 i) { return rectangles[i][3]; }
u32 GameStateGetField4256(void) { return 11; }

int main(void)
{
    u16 storage[16], count, *pool, taken;
    u32 size, mask, before;
    s32 xs[4], ys[4];
    struct HitBounds bounds = {-2, -1, 5, 2};
    for (size = 1; size <= 16; size++)
    {
        count = size;
        assert(RandomPoolInitialize(storage, &count) == storage);
        assert(count == size && allocations == 0);
        for (before = 0; before < size; before++) assert(storage[before] == before);
        mask = 0;
        while (count)
        {
            /* Alternate between removal from the first and last live slot. */
            randomValue = count % 2 ? 0 : count - 1;
            before = count;
            taken = RandomPoolTake(storage, &count);
            assert(taken < size && !(mask & (1u << taken)));
            mask |= 1u << taken;
            assert(count == before - 1);
        }
        assert(mask == (1u << size) - 1);
        before = randomCalls;
        assert(RandomPoolTake(storage, &count) == storage[0]);
        assert(count == 0 && randomCalls == before);
    }
    count = 5;
    pool = RandomPoolInitialize(0, &count);
    assert(allocations == 1 && count == 5 && pool[4] == 4);
    free(pool);

    xs[2] = ys[2] = 12345;
    assert(MapCollectAttributePositions(0, xs, ys, 0, 0, 3, 2, 400, 2) == 1);
    assert(xs[0] == 0 && ys[0] == 0 && xs[1] == 2 && ys[1] == 0);
    assert(xs[2] == 12345 && ys[2] == 12345 && probes == 3);
    assert(MapCollectAttributePositions(0, xs, ys, 0, 0, 3, 2, 400, 4) == 1);
    assert(xs[2] == 1 && ys[2] == 1);
    xs[0] = ys[0] = 12345;
    assert(MapCollectAttributePositions(0, xs, ys, 0, 0, 3, 2, 99, 4) == 0);
    assert(xs[0] == 12345 && ys[0] == 12345);
    before = probes;
    assert(MapCollectAttributePositions(0, xs, ys, 3, 0, 3, 2, 400, 4) == 0);
    assert(probes == before);
    assert(MapCollectAttributePositions(0, xs, ys, -1, 0, 0, 1, -1, 1) == 1);
    assert(xs[0] == -1 && ys[0] == 0);

    states[0] = 1; fields[0] = 11;
    rectangles[0][0] = 13; rectangles[0][1] = 9;
    rectangles[0][2] = 0; rectangles[0][3] = 2;
    assert(MapGenerationFindOverlappingEntry(0, &bounds, 10, 10, 0) == 11);
    assert(MapGenerationFindOverlappingEntry(0, &bounds, 10, 10, 3) == 0);
    rectangles[0][0] = 15; rectangles[0][1] = 12;
    assert(MapGenerationFindOverlappingEntry(0, &bounds, 10, 10, 0) == 11);
    rectangles[0][0] = 16;
    assert(MapGenerationFindOverlappingEntry(0, &bounds, 10, 10, 0) == 0);
    rectangles[0][0] = 13; fields[0] = 12;
    assert(MapGenerationFindOverlappingEntry(0, &bounds, 10, 10, 0) == 0);
    fields[0] = 11; states[0] = 0x10000;
    assert(MapGenerationFindOverlappingEntry(0, &bounds, 10, 10, 0) == 0);
    return 0;
}
''')
            executable = Path(directory) / 'harness'
            subprocess.run([
                'gcc', '-O2', '-ffunction-sections', '-fdata-sections', '-DAT(x)=',
                '-I' + str(ROOT / 'include'), str(source),
                str(ROOT / 'src/random_pool.c'), str(ROOT / 'src/map_attributes.c'),
                '-Wl,--gc-sections', '-o', str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)
