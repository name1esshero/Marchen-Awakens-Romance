"""Check recovered hit-region lifetime, fixed-width fields, and mode retention."""
import ctypes
import random
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class Region(ctypes.Structure):
    _fields_ = [('active', ctypes.c_int32), ('x', ctypes.c_int16), ('y', ctypes.c_int16),
                ('width', ctypes.c_int16), ('height', ctypes.c_int16), ('mode', ctypes.c_int32)]

class Bounds(ctypes.Structure):
    _fields_ = [(name, ctypes.c_int16) for name in ('left', 'top', 'right', 'bottom')]

class HitRegionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        folder = Path(cls.temp.name)
        source = (ROOT / 'src/hit_region.c').read_text()
        (folder / 'hit.c').write_text(source)
        # GameStateGetHitRegion now reads through gIwramBase's real IWRAM-root
        # layout (+0x3FDC gameState pointer, +0x1090 table) rather than a
        # stand-in wrapper. Point the fictional "game state" at
        # regions[]-0x1090 so regions[id] is exactly what the real accessor
        # computes, while keeping the regions[] symbol itself directly
        # readable by ctypes. The write must go through the same
        # IwramGameStateRootLayout struct type the real accessor reads
        # through, not a raw +0x3FDC byte offset: a host u8* needs 8-byte
        # alignment, so the compiler pads gameState to a different real
        # offset than 0x3FDC (which is only 4-byte aligned, fine on the
        # 32-bit GBA target but not here) -- writing/reading through
        # mismatched offsets silently produces a garbage pointer.
        (folder / 'mock.c').write_text(
            '#include "hit_region.h"\n'
            '#include "game_state.h"\n'
            'u8 gIwramBase[sizeof(struct IwramGameStateRootLayout)];\n'
            'struct HitRegion regions[17];\n'
            '__attribute__((constructor)) static void wireGameState(void) {\n'
            '    struct IwramGameStateRootLayout *iwram ='
            ' (struct IwramGameStateRootLayout *)gIwramBase;\n'
            '    iwram->gameState = (u8 *)regions - 0x1090;\n'
            '}\n')
        subprocess.run(['gcc', '-shared', '-fPIC', '-O2', '-D', 'AT(x)=', '-I'+str(ROOT/'include'), str(folder/'hit.c'), str(folder/'mock.c'), '-o', str(folder/'hit.so')], check=True)
        cls.lib = ctypes.CDLL(str(folder/'hit.so'))
        cls.lib.HitRegionTest.argtypes = [ctypes.c_int16, ctypes.c_int16, ctypes.POINTER(Bounds)]
        cls.regions = (Region * 17).in_dll(cls.lib, 'regions')
        for name in ('HitRegionInit', 'HitRegionSetRect'):
            getattr(cls.lib, name).argtypes = [ctypes.c_int32]*5

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def setUp(self):
        ctypes.memset(ctypes.addressof(self.regions), 0, ctypes.sizeof(self.regions))

    def test_init_truncates_geometry_and_resets_mode(self):
        self.regions[3].mode = 1
        self.lib.HitRegionInit(3, 65535, 32768, 65538, -65533)
        r = self.regions[3]
        self.assertEqual((r.active, r.x, r.y, r.width, r.height, r.mode), (1, -1, -32768, 2, 3, 0))
        self.assertEqual(self.regions[2].active, 0)

    def test_disable_and_reactivate_preserve_mode(self):
        self.lib.HitRegionInit(7, 10, 20, 30, 40)
        self.regions[7].mode = 1
        self.lib.HitRegionDisable(7)
        self.assertEqual((self.regions[7].active, self.regions[7].width, self.regions[7].mode), (0, 30, 1))
        self.lib.HitRegionSetRect(7, -5, 9, 18, 24)
        r = self.regions[7]
        self.assertEqual((r.active, r.x, r.y, r.width, r.height, r.mode), (1, -5, 9, 18, 24, 1))

    def test_disable_all_exactly_sixteen_records(self):
        for i, r in enumerate(self.regions):
            r.active = 1; r.x = i; r.mode = i+1
        self.lib.HitRegionDisableAll()
        self.assertEqual([r.active for r in self.regions], [0]*16+[1])
        self.assertEqual([r.x for r in self.regions], list(range(17)))
        self.assertEqual([r.mode for r in self.regions], list(range(1,18)))

    def test_overlap_containment_and_first_match(self):
        bounds = Bounds(0, 0, 10, 10)
        self.lib.HitRegionInit(3, 10, 10, 20, 20)
        test = lambda x, y: self.lib.HitRegionTest(x, y, ctypes.byref(bounds))
        self.assertEqual(test(0, 10), 0)  # Right edge touches left edge.
        self.assertEqual(test(1, 10), 4)  # One pixel of overlap.
        self.assertEqual(test(30, 10), 0)
        self.regions[3].mode = 1
        self.assertEqual(test(10, 11), 0)  # Containment excludes shared edges.
        self.assertEqual(test(11, 11), 4)
        self.assertEqual(test(20, 11), 0)
        self.lib.HitRegionInit(0, 0, 0, 100, 100)
        self.assertEqual(test(11, 11), 1)  # First active matching region wins.
        self.regions[0].mode = 2
        self.assertEqual(test(11, 11), 4)  # Unknown modes never hit.
        self.regions[3].active = 0
        self.assertEqual(test(11, 11), 0)

    def test_randomized_collision_against_rectangle_oracle(self):
        rng = random.Random(0x18C4C)
        for _ in range(500):
            x, y = (rng.randrange(-32768, 32768) for _ in range(2))
            bounds = Bounds(*(rng.randrange(-32768, 32768) for _ in range(4)))
            left, top = x + bounds.left, y + bounds.top
            right, bottom = x + bounds.right, y + bounds.bottom
            expected = 0
            for i, r in enumerate(self.regions[:16]):
                r.active = rng.choice([0, 1, -1])
                r.mode = rng.randrange(3)
                r.x, r.y, r.width, r.height = (rng.randrange(-32768, 32768) for _ in range(4))
                if r.mode == 0:
                    hit = left < r.x+r.width and r.x < right and top < r.y+r.height and r.y < bottom
                else:
                    hit = r.mode == 1 and left > r.x and right < r.x+r.width and top > r.y and bottom < r.y+r.height
                if not expected and r.active and hit:
                    expected = i+1
            self.assertEqual(self.lib.HitRegionTest(x, y, ctypes.byref(bounds)), expected)

if __name__ == '__main__':
    unittest.main()
