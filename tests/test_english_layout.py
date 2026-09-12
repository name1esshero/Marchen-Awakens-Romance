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

    def test_multiple_pages_keep_words_and_explicit_lines(self):
        text='One two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen.'
        for capacity in (2,3):
            pages=el.pages(text,self.mapping,capacity)
            self.assertGreater(len(pages),1)
            self.assertTrue(all(1<=len(page)<=capacity for page in pages))
            reverse={value:key for key,value in self.mapping.items()}
            decoded=[''.join(reverse[row[i:i+2]] for i in range(0,len(row)-1,2))
                     for page in pages for row in page]
            self.assertTrue(all(len(row)<=21 for row in decoded))
            self.assertEqual(' '.join(decoded),text)
        self.assertEqual([len(page) for page in el.pages('A\nB\nC\nD',self.mapping)], [3,1])

    def test_color_markup_has_no_width_and_survives_wrap(self):
        rows=el.wrap_lines('{color:0D04}'+ 'A'*21 + ' {color:0F04}B',self.mapping)
        self.assertEqual(rows[0],b'C0D04'+self.mapping['A']*21+b'\0')
        self.assertEqual(rows[1],b'C0F04'+self.mapping['B']+b'\0')
        self.assertEqual(el.wrap_lines('{speed:0002}Hi',self.mapping)[0][:5],b'T0002')
        with self.assertRaises(ValueError):el.wrap_lines('{color:FF04}Hi',self.mapping)

    def test_unknown_glyph_and_embedded_control_rejected(self):
        for text in ['hello\0world', 'hello\tworld', 'hello😀']:
            with self.assertRaises(ValueError):
                el.layout(text, self.mapping)


if __name__ == '__main__':
    unittest.main()
