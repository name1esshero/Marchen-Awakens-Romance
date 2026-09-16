"""Exercise both object cleanup paths, including signed record counts."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ObjectReleaseTests(unittest.TestCase):
    def test_release_lifetime_and_record_offsets(self):
        with tempfile.TemporaryDirectory() as temp:
            source = Path(temp) / 'test.c'
            source.write_text(r'''
#include <assert.h>
#include <string.h>
#include "object.h"
#include "heap.h"
#include "ncd.h"

static struct Heap heap;
/* The host link maps the root offset to zero in this small mock IWRAM. */
_Alignas(void *) u8 gIwramBase[sizeof(void *)];
static void *released[4];
static unsigned releaseCount, freeCount, auxiliaryCount;
static void *expectedAllocation;

void NcdRuntimeSpriteReleaseAllocation(struct NcdSprite *sprite)
{
    assert(releaseCount < 4);
    released[releaseCount++] = sprite;
}

void SpriteAuxiliaryReset(void *state)
{
    auxiliaryCount++;
    NcdRuntimeSpriteReleaseAllocation(state);
}

void HeapFree(struct Heap *owner, void *allocation)
{
    assert(owner == &heap && allocation == expectedAllocation);
    freeCount++;
}

int main(void)
{
    unsigned char records[OBJECT_RECORD_SIZE * 3];
    struct Object object;
    unsigned mode, scenario, i;
    const u32 counts[] = {3, 0, 0xFFFFFFFF};
    struct Heap *heapPointer = &heap;

    memcpy(gIwramBase, &heapPointer, sizeof(heapPointer));

    for (mode = 0; mode < 2; mode++)
    {
        void (*release)(struct Object *) = mode
            ? ObjectFreeAuxiliaryResources : ObjectFreeNcdResources;
        for (scenario = 0; scenario < 6; scenario++)
        {
            memset(&object, 0, sizeof(object));
            object.record = records;
            object.unk_14 = 0x12345678;
            object.flags = OBJECT_ACTIVE;
            if (scenario < 3)
            {
                object.flags |= OBJECT_MULTIPLE_RECORDS;
                object.unk_18 = counts[scenario];
            }
            if (scenario == 4)
                object.record = NULL;
            if (scenario == 5)
                object.flags = OBJECT_MULTIPLE_RECORDS;
            expectedAllocation = object.record;
            releaseCount = freeCount = auxiliaryCount = 0;
            release(&object);
            assert(object.unk_14 == 0x12345678);
            if (scenario == 5)
            {
                assert(object.record == records);
                assert(object.flags == OBJECT_MULTIPLE_RECORDS);
                assert(freeCount == 0 && releaseCount == 0);
                continue;
            }
            assert(object.record == NULL && object.flags == 0);
            assert(freeCount == 1);
            assert(releaseCount == (scenario == 0 ? 3 : scenario == 3 ? 1 : 0));
            assert(auxiliaryCount == (mode ? releaseCount : 0));
            for (i = 0; i < releaseCount; i++)
                assert(released[i] == records + i * OBJECT_RECORD_SIZE
                    + (mode ? 0 : OBJECT_RECORD_SPRITE_OFFSET));
        }
    }
    return 0;
}
''')
            executable = Path(temp) / 'test'
            subprocess.run([
                'gcc', '-O2', '-no-pie', '-ffunction-sections', '-fdata-sections',
                '-DAT(x)=', '-I' + str(ROOT / 'include'), str(source),
                str(ROOT / 'src/object.c'), '-Wl,--gc-sections',
                '-Wl,--defsym=gObjectHeapRootOffset=0', '-o', str(executable),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(executable)], check=True)
