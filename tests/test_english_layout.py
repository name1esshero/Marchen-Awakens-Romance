import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import english_layout as el


class EnglishLayoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.mapping = el.load_mapping()

    def test_actual_font_and_ascii_command_safety(self):
        rows = el.layout('CAT test!', self.mapping)
        self.assertEqual(rows[0], 'ＣＡＴ　ｔｅｓｔ！'.encode('shift_jis') + b'\0')
        self.assertEqual(rows[1:], [b'\0', b'\0'])
        self.assertEqual(el.layout('Ä♥♪', self.mapping)[0], bytes.fromhex('F056F04081F400'))

    def test_width_and_row_boundaries(self):
        self.assertEqual(len(el.layout('A' * 21, self.mapping)[0]), 43)
        for text, rows in [('A' * 22, 3), ('A\nB\nC\nD', 3), ('A\nB\nC', 2)]:
            with self.assertRaises(ValueError):
                el.layout(text, self.mapping, rows)
        result = el.layout('A' * 20 + ' B', self.mapping)
        self.assertEqual(result[1], self.mapping['B'] + b'\0')

    def test_unknown_glyph_and_embedded_control_rejected(self):
        for text in ['hello\0world', 'hello\tworld', 'hello😀']:
            with self.assertRaises(ValueError):
                el.layout(text, self.mapping)


if __name__ == '__main__':
    unittest.main()
