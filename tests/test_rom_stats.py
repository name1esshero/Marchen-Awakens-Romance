import tempfile
import unittest
from pathlib import Path

from tools.rom_stats import rom_occupancy
from tools.ram_layout import audit_layout, resolve_address


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

    def test_ram_layout_covers_both_physical_regions_without_overlap(self):
        report = audit_layout()
        self.assertEqual(report['totals']['EWRAM']['size'], 256 * 1024)
        self.assertEqual(report['totals']['EWRAM']['kinds']['heap_arena'],
                         256 * 1024)
        self.assertEqual(report['totals']['IWRAM']['size'], 32 * 1024)
        self.assertEqual(report['totals']['IWRAM']['kinds']['unassigned'], 968)
        self.assertGreaterEqual(report['symbol_count'], 60)

    def test_ram_address_resolution_prefers_nested_objects(self):
        result = resolve_address('03003CC4')
        self.assertEqual(result['partition']['name'], 'gMainRuntime')
        self.assertEqual(result['symbols'][0]['name'], 'gKmpViewport1')
        self.assertEqual(result['symbols'][0]['offset'], 4)


if __name__=='__main__':
    unittest.main()
