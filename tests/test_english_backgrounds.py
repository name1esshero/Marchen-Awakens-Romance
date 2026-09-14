"""Verify English tile maps display the authored PNGs and fallback is exact."""
import json
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import english_backgrounds as bg
import gfx
import graphics_catalog
import lz77
import mapped_images

class EnglishBackgroundTests(unittest.TestCase):
    def test_catalog_exposes_active_english_overrides(self):
        graphics_catalog.main([])
        page = (ROOT/'graphics/backgrounds/index.html').read_text()
        for name in bg.NAMES:
            self.assertIn(name + '_en.png', page)
        self.assertEqual(page.count('>English override</a>'), len(bg.NAMES))

    def test_authored_pixels_survive_compression_and_map_repacking(self):
        assets = {e.get('archive_name'): e for e in json.loads((ROOT/'assets.json').read_text())}
        for name in bg.NAMES:
            with self.subTest(name=name):
                entry = assets[name]
                data, map_data, overrides = bg.compile_entry(entry)
                self.assertTrue(overrides)
                self.assertEqual(len(data), entry['compressed_size'])
                raw, _ = lz77.decompress(data)
                # These layers share character VRAM with later scenes.  The
                # English compiler must never grow beyond the source layer's
                # allocation merely because its temporary heap buffer can.
                self.assertEqual(len(raw), entry['raw_size'])
                layout = json.loads((ROOT/entry['image_layout']).read_text())
                tiles = mapped_images.tiles_from_bytes(raw)
                for layer in layout['layers']:
                    width, height = layout['width_tiles'], layout['height_tiles']
                    words = struct.unpack_from('<'+'H'*(width*height), map_data, layer['offset'])
                    pixels = mapped_images.render(tiles, words, width, height)
                    expected, palette = gfx.read_png(str(ROOT/overrides[layer['image']]))
                    original, original_palette = gfx.read_png(str(ROOT/layer['image']))
                    self.assertEqual(palette, original_palette)
                    # A 4bpp map stores one palette bank per tile.  Local
                    # colour zero therefore renders as 0, 16, 32, ... in the
                    # editable global-index view, even though every one is
                    # the same transparent/background slot.  English art is
                    # free to use any equivalent bank-zero index.
                    canonical = lambda rows: [[0 if v % 16 == 0 else v for v in row]
                                               for row in rows]
                    self.assertEqual(canonical(pixels), canonical(expected))
                    self.assertNotEqual(pixels, original)

    @unittest.skipUnless((ROOT/'baserom.gba').exists(),
                         'baserom.gba is required for ROM comparison')
    def test_missing_overrides_restore_original_tiles_and_map(self):
        assets = {e.get('archive_name'): e for e in json.loads((ROOT/'assets.json').read_text())}
        rom = (ROOT/'baserom.gba').read_bytes()
        for name in bg.NAMES:
            with self.subTest(name=name), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                entry = assets[name]
                layout = json.loads((ROOT/entry['image_layout']).read_text())
                paths = [entry['image_layout'], layout['map_path']]
                paths += [layer['image'] for layer in layout['layers']]
                if 'unused_tiles_image' in entry:
                    paths.append(entry['unused_tiles_image'])
                for path in paths:
                    dest = root/path
                    dest.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(ROOT/path, dest)
                data, map_data, overrides = bg.compile_entry(entry, root)
                self.assertFalse(overrides)
                start = entry['rom_offset']
                self.assertEqual(data, rom[start:start+entry['compressed_size']])
                self.assertEqual(map_data, (ROOT/layout['map_path']).read_bytes())

if __name__ == '__main__':
    unittest.main()
