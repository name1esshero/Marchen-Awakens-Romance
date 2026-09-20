import json
from pathlib import Path
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import rom_data_sections


class RomDataSectionTests(unittest.TestCase):
    def test_debug_monitor_padding_is_not_stored_in_binary(self):
        monitor = ROOT / "data/agb_debug_monitor.bin"
        self.assertEqual(monitor.stat().st_size, 0x3090)
        self.assertFalse((ROOT / "data/data_FE0000.bin").exists())

        manifest = json.loads((ROOT / "data/rom_data_sections.json").read_text())
        sections = {entry["start"]: entry for entry in manifest["sections"]}
        self.assertEqual(sections["00FE0000"]["kind"], "debug_monitor")
        self.assertEqual(sections["00FE0000"]["end"], "00FE3090")
        self.assertEqual(sections["00FE3090"]["fill"], 0)
        self.assertEqual(sections["00FE3090"]["end"], "00FE4000")

    def test_high_rom_pattern_is_declarative(self):
        manifest = json.loads((ROOT / "data/rom_data_sections.json").read_text())
        sections = {
            entry["start"]: entry
            for entry in manifest["sections"]
            if entry["group"] == "raw_data"
        }

        self.assertEqual(
            sections["00FFF000"],
            {
                "group": "raw_data",
                "start": "00FFF000",
                "end": "00FFF800",
                "kind": "fill",
                "fill": 0,
            },
        )
        self.assertEqual(sections["00FFF800"]["word_fill"], "09FFC000")
        self.assertFalse((ROOT / "data/data_FFF000.bin").exists())

    def test_word_fill_emits_little_endian_word_repetition(self):
        sections = [{
            "group": "raw_data",
            "start": "00000000",
            "end": "00000010",
            "kind": "word_fill",
            "word_fill": "09FFC000",
        }]
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "pattern.s"
            rom_data_sections.write_assembly(sections, output)
            self.assertIn(".fill 4, 4, 0x09FFC000", output.read_text())


if __name__ == "__main__":
    unittest.main()
