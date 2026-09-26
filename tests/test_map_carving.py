"""Exercise the matching corridor-carving helpers from src/mapping.c.

mapping.c depends on much of the engine, so the three helpers are extracted
from the real source text and compiled on their own against the project
headers. A small reference model written independently in the test checks
every direction, the interior margin, and the neighbor-shape codes.
"""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
FUNCTIONS = (
    'GeneratedMapIsValidStartCell',
    'GeneratedMapHasCarveDirection',
    'GeneratedMapCanCarveTwoCellStep',
    'GeneratedMapGetPathNeighborShape',
)


def extract(source, name):
    """Return the AT(...) definition of one function, through its closing brace."""
    match = re.search(r'^AT\("[0-9A-F]+"\)\n[^\n]*\b' + name + r'\(', source, re.M)
    if match is None:
        raise AssertionError(name + ' not found in src/mapping.c')
    end = source.index('\n}\n', match.start()) + 3
    return source[match.start():end]


TEST_PROGRAM = r'''
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static int reference_carve(int w, int h, const u8 *cells, int index, int dir)
{
    int x = index % w, y = index / w, dest;
    switch (dir) {
    case 0: if (x >= w - 3) return 0; dest = index + 2; break;
    case 1: if (y >= h - 3) return 0; dest = index + 2 * w; break;
    case 2: if (x <= 2) return 0; dest = index - 2; break;
    case 3: if (y <= 2) return 0; dest = index - 2 * w; break;
    default: return 0;
    }
    return cells[dest] == 0;
}

static int reference_mask(int w, int h, const u8 *cells, int index)
{
    int x = index % w, y = index / w, mask = 0;
    if (x > 0 && cells[index - 1]) mask |= 0x0001;
    if (x < w - 1 && cells[index + 1]) mask |= 0x0010;
    if (y > 0 && cells[index - w]) mask |= 0x0100;
    if (y < h - 1 && cells[index + w]) mask |= 0x1000;
    return mask;
}

static int reference_start(int w, int h, int cell)
{
    int row = cell / w, column = cell % w;
    return cell % 2 == 0 && row % 2 == 1 && column > 0 && column <= w - 2
        && row > 0 && row <= h - 2;
}

static int reference_shape(int mask)
{
    switch (mask) {
    case 0x1100: return 3;
    case 0x0011: return 4;
    case 0x0110: return 5;
    case 0x0101: return 6;
    case 0x1001: return 7;
    case 0x1010: return 8;
    default: return 9;
    }
}

int main(void)
{
    static const int sizes[][2] = {{7, 7}, {9, 13}, {16, 5}, {31, 17}};
    struct GeneratedFieldMap map;
    unsigned seed = 12345;
    int s, trial, index, dir;

    for (s = 0; s < 4; s++) {
        int w = sizes[s][0], h = sizes[s][1];
        u8 *cells = malloc(w * h);
        memset(&map, 0, sizeof(map));
        map.width = w;
        map.height = h;
        for (trial = 0; trial < 40; trial++) {
            for (index = 0; index < w * h; index++) {
                seed = seed * 1103515245u + 12345u;
                cells[index] = (seed >> 16) % 3 == 0 ? (u8)(seed >> 8) | 1 : 0;
            }
            for (index = 0; index < w * h; index++) {
                int any = 0, mask;
                for (dir = 0; dir < 4; dir++) {
                    int expected = reference_carve(w, h, cells, index, dir);
                    assert(GeneratedMapCanCarveTwoCellStep(&map, cells, index, dir) == expected);
                    any |= expected;
                }
                /* Out-of-range directions are never carvable. */
                assert(GeneratedMapCanCarveTwoCellStep(&map, cells, index, 4) == FALSE);
                assert(GeneratedMapCanCarveTwoCellStep(&map, cells, index, 255) == FALSE);
                assert(GeneratedMapHasCarveDirection(&map, cells, index) == any);

                mask = reference_mask(w, h, cells, index);
                assert(GeneratedMapGetPathNeighborShape(&map, cells, index, 1) == (u32)mask);
                assert(GeneratedMapGetPathNeighborShape(&map, cells, index, 0)
                       == (u32)reference_shape(mask));
                assert(GeneratedMapIsValidStartCell(&map, index)
                       == reference_start(w, h, index));
            }
        }
        /* Negative and past-the-end requests are rejected, not wrapped. */
        assert(GeneratedMapIsValidStartCell(&map, -w - 1) == FALSE);
        assert(GeneratedMapIsValidStartCell(&map, w * h + w + 1) == FALSE);
        free(cells);
    }
    return 0;
}
'''


class MapCarvingTests(unittest.TestCase):
    def test_carving_probes_and_neighbor_shapes_match_reference(self):
        source = (ROOT / 'src/mapping.c').read_text()
        body = '\n'.join(extract(source, name) for name in FUNCTIONS)
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)
            (folder / 'test.c').write_text(
                '#include "gba/types.h"\n#include "map_generation.h"\n'
                + body + TEST_PROGRAM)
            subprocess.run(['gcc', '-O2', '-D', 'AT(x)=', '-I' + str(ROOT / 'include'),
                            str(folder / 'test.c'), '-o', str(folder / 'test')],
                           check=True)
            subprocess.run([str(folder / 'test')], check=True)

    def test_every_helper_is_declared_in_the_header(self):
        header = (ROOT / 'include/map_generation.h').read_text()
        for name in FUNCTIONS:
            self.assertIn(name + '(', header)


if __name__ == '__main__':
    unittest.main()
