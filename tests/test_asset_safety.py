import pathlib
import sys
import tempfile
import unittest
from unittest.mock import patch
ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import asset_safety
import scenes

class AssetSafetyTests(unittest.TestCase):
    def test_cleanup_protects_english_and_its_owner(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            original, english = root/'one.png', root/'one_en.png'
            original.write_bytes(b'original')
            english.write_bytes(b'edited English')
            for path in (original, english):
                with self.assertRaisesRegex(ValueError, 'Refusing'):
                    asset_safety.unlink_generated(path)
            self.assertEqual(english.read_bytes(), b'edited English')
            disposable = root/'unused.png'
            disposable.write_bytes(b'generated')
            asset_safety.unlink_generated(disposable)
            self.assertFalse(disposable.exists())

    def test_duplicate_frames_keep_their_english_companion_paths(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            for name in ('a.png','b.png'):
                (root/name).write_bytes(b'identical Japanese pixels')
            (root/'b_en.png').write_bytes(b'English edit')
            data = dict(frames=[dict(id=0,path='a.png'),dict(id=1,path='b.png')])
            with patch.object(scenes,'cell_key',return_value=b'identical cells'):
                removed = scenes.consolidate(data, root, b'', {})
            self.assertEqual(removed, [])
            self.assertEqual(data['frames'][1]['path'], 'b.png')
            self.assertTrue((root/'b_en.png').exists())

if __name__ == '__main__':
    unittest.main()
