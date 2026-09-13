import tempfile
import unittest
from pathlib import Path

from tools.rom_stats import rom_occupancy


class RomStatsTests(unittest.TestCase):
    def test_counts_only_reusable_fill_runs_as_free(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'test.gba'
            path.write_bytes(b'code\0\xffdata'+b'\0'*32+b'used'+b'\xff'*40)
            used,free=rom_occupancy(path,128)
            self.assertEqual(free,128-path.stat().st_size+32+40)
            self.assertEqual(used+free,128)

    def test_rejects_image_larger_than_address_space(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'test.gba';path.write_bytes(b'x'*9)
            with self.assertRaisesRegex(ValueError,'exceeds'):
                rom_occupancy(path,8)


if __name__=='__main__':
    unittest.main()
