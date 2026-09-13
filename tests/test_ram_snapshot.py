import struct
import unittest

from tools.ram_snapshot import inspect_snapshot


class RamSnapshotTests(unittest.TestCase):
    def test_counts_live_blocks_and_fragmentation(self):
        ewram = bytearray(0x40000)
        iwram = bytearray(0x8000)
        heap = 0x02000000
        struct.pack_into("<I", iwram, 0x3FB4, heap)
        struct.pack_into("<II", ewram, 0, 64, heap + 32)
        # 24-byte allocated block, then 32-byte final free block.
        struct.pack_into("<II", ewram, 8, 24 | 1, 0)
        struct.pack_into("<II", ewram, 32, 32 | 2, heap + 8)

        report = inspect_snapshot(ewram, iwram)
        first = report["heaps"][0]
        self.assertEqual(first["allocated_payload"], 16)
        self.assertEqual(first["free_payload"], 24)
        self.assertEqual(first["largest_free_block"], 24)
        self.assertEqual(first["allocator_overhead"], 24)
        self.assertEqual(first["block_count"], 2)

    def test_rejects_corrupt_block_size(self):
        ewram = bytearray(0x40000)
        iwram = bytearray(0x8000)
        struct.pack_into("<I", iwram, 0x3FB4, 0x02000000)
        struct.pack_into("<II", ewram, 0, 64, 0x02000008)
        struct.pack_into("<I", ewram, 8, 4 | 2)
        with self.assertRaisesRegex(ValueError, "corrupt block"):
            inspect_snapshot(ewram, iwram)


if __name__ == "__main__":
    unittest.main()
