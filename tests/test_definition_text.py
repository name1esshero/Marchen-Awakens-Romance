import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

import definition_text
import extract_text


class DefinitionTextTests(unittest.TestCase):
    def test_definition_tables_have_explicit_nonoverlapping_ranges(self):
        ranges = definition_text.ranges()
        self.assertEqual(ranges, [(0x1B096C, 0x1BE7EC), (0x1BE82C, 0x1BEC3C)])
        self.assertLessEqual(ranges[0][1], ranges[1][0])
        excludes = extract_text.load_excludes()
        for source_range in ranges:
            self.assertIn(source_range, excludes)

    def test_all_definition_text_has_english_mapping(self):
        for table in definition_text.TABLES:
            lines = (ROOT / table['path']).read_text().splitlines()
            records = [line for line in lines if line.startswith('@')]
            self.assertTrue(records)
            self.assertTrue(all('  // EN:' in line for line in records))

    def test_definition_manifest_uses_structured_source_classes(self):
        manifest = json.loads((ROOT / 'text/text.json').read_text())
        by_kind = {row['kind']: row for row in manifest}
        self.assertEqual(by_kind['arm_definition_table']['path'], 'text/arm_definitions.txt')
        self.assertEqual(by_kind['arm_definition_table']['records'], 445)
        self.assertEqual(by_kind['item_definition_table']['path'], 'text/item_definitions.txt')
        self.assertEqual(by_kind['item_definition_table']['records'], 13)


if __name__ == '__main__':
    unittest.main()
