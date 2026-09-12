import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import gfx
import lz77
import mapped_images as mi
import affine_images
import regular_images


class MappedImagesTests(unittest.TestCase):
    def fixture(self, root):
        tiles = [[[((y*3+x) % 16) for x in range(8)] for y in range(8)],
                 [[(y % 2) for x in range(8)] for y in range(8)],
                 [[2]*8 for _ in range(8)]]
        words = [0, 0x1400, 0x801]
        colors = [(i*7, i*7, i*7) for i in range(32)]
        pixels = mi.render(tiles, words, 3, 1)
        gfx.write_png(str(root/'picture.png'), pixels, colors)
        gfx.write_png(str(root/'unused.png'), tiles[2], colors[:16])
        layout = dict(width_tiles=3, height_tiles=1, tile_count=3,
                      layers=[dict(index=0, offset=0xC0, entries=words, image='picture.png')],
                      unused_tiles=[2], baseline_tiles=[mi.digest(t) for t in tiles])
        (root/'layout.json').write_text(json.dumps(layout))
        entry = dict(path='picture', bpp=4, raw_size=96, palette_banks=2,
                     image_layout='layout.json', unused_tiles_image='unused.png')
        return entry, tiles, pixels, colors, layout

    def test_all_migrated_pixels_and_map_planes_match_original_sources(self):
        rom = (mi.ROOT/'baserom.gba').read_bytes()
        maps = {e['name']:e for e in json.loads((mi.ROOT/'maps/nfp/manifest.json').read_text())}
        entries = [e for e in json.loads((mi.ROOT/'assets.json').read_text()) if e['kind']=='mapped_image']
        self.assertEqual(len(entries), 91)
        for e in entries:
            original, _ = lz77.decompress(rom, e['rom_offset'])
            self.assertEqual(mi.compile_image(e), original, e['archive_name'])
            layout = mi.read(mi.ROOT/e['image_layout'])
            if layout.get('format') in ('affine_tsc_u8','regular_tsc_u16'):
                for layer in layout['layers']:
                    source = next(m for m in maps.values() if m['path']==layer['map_path'])
                    builder=regular_images.build_plane if layout['format']=='regular_tsc_u16' else affine_images.build_plane
                    self.assertEqual(builder(layout,layer), rom[source['rom_offset']:source['rom_offset']+source['size']])
            else:
                source = maps[e['archive_name'][:-3]+'KMP']
                raw = (mi.ROOT/source['path']).read_bytes()
                self.assertEqual(mi.build_map(raw, layout), rom[source['rom_offset']:source['rom_offset']+source['size']])

    def test_flipped_alias_edit_and_unused_tile_survive(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp);entry, tiles, pixels, colors, _ = self.fixture(root)
            original = b''.join(gfx.pixels_to_tiles(t,4) for t in tiles)
            self.assertEqual(mi.compile_image(entry,root), original)
            pixels[0][8] ^= 1  # H-flipped second occurrence -> original tile's x=7.
            gfx.write_png(str(root/'picture.png'),pixels,colors)
            changed = mi.compile_image(entry,root)
            expected = bytearray(original);expected[3] ^= 0x10
            self.assertEqual(changed, bytes(expected))
            self.assertEqual(changed[64:], original[64:])
            (root/'unused.png').unlink()
            with self.assertRaises(FileNotFoundError):mi.compile_image(entry,root)

    def test_conflicting_aliases_and_wrong_palette_are_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp);entry, _, pixels, colors, _ = self.fixture(root)
            pixels[0][0] ^= 1
            pixels[0][8] ^= 1
            gfx.write_png(str(root/'picture.png'),pixels,colors)
            with self.assertRaisesRegex(ValueError,'Conflicting edits'):mi.compile_image(entry,root)
            entry, _, pixels, colors, _ = self.fixture(root)
            pixels[0][0] = 16
            gfx.write_png(str(root/'picture.png'),pixels,colors)
            with self.assertRaisesRegex(ValueError,'palette bank'):mi.compile_image(entry,root)

    def test_affine_byte_indices_and_8bpp_pixels_survive_edits(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)
            tiles=[[[192+x+y for x in range(8)] for y in range(8)], [[0]*8 for _ in range(8)]]
            colors=[(i,i,i) for i in range(256)]
            pixels=mi.render(tiles,[0,1],2,1,8)
            gfx.write_png(str(root/'image.png'),pixels,colors)
            layout=dict(format='affine_tsc_u8',width_tiles=2,height_tiles=1,tile_count=2,
                        layers=[dict(index=0,entries=[0,1],image='image.png')],
                        unused_tiles=[],baseline_tiles=[mi.digest(t) for t in tiles])
            (root/'layout.json').write_text(json.dumps(layout))
            entry=dict(bpp=8,raw_size=128,palette_banks=3,image_layout='layout.json')
            raw=b''.join(gfx.pixels_to_tiles(t,8) for t in tiles)
            self.assertEqual(mi.compile_image(entry,root),raw)
            pixels[0][0]=201
            gfx.write_png(str(root/'image.png'),pixels,colors)
            self.assertEqual(mi.compile_image(entry,root),bytes([201])+raw[1:])
            layer=layout['layers'][0];layer['entries']=[1,0]
            self.assertEqual(affine_images.build_plane(layout,layer),b'\x01\x00')
            layer['entries'][0]=0x400
            with self.assertRaises(ValueError):affine_images.build_plane(layout,layer)

    def test_regular_screenblock_global_palette_banks(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)
            tile=[[x for x in range(8)] for _ in range(8)]
            words=[0xC000]*1024
            pixels=mi.render([tile],words,32,32)
            colors=[(i,i,i) for i in range(256)]
            gfx.write_png(str(root/'frame.png'),pixels,colors)
            layout=dict(format='regular_tsc_u16',width_tiles=32,height_tiles=32,tile_count=1,
                        layers=[dict(index=0,entries=words,image='frame.png')],unused_tiles=[],baseline_tiles=[mi.digest(tile)])
            (root/'layout.json').write_text(json.dumps(layout))
            entry=dict(path='frame',bpp=4,raw_size=32,palette_banks=3,palette_bank_base=12,image_layout='layout.json')
            self.assertEqual(mi.compile_image(entry,root),gfx.pixels_to_tiles(tile,4))
            self.assertEqual(regular_images.build_plane(layout,layout['layers'][0]),b'\x00\xc0'*1024)
            pixels[0][0]=193
            gfx.write_png(str(root/'frame.png'),pixels,colors)
            expected=bytearray(gfx.pixels_to_tiles(tile,4));expected[0]^=1
            self.assertEqual(mi.compile_image(entry,root),expected)
            layout['layers'][0]['entries'][0]=0x8000
            (root/'layout.json').write_text(json.dumps(layout))
            with self.assertRaises(ValueError):mi.compile_image(entry,root)

    def test_readable_map_entry_edit_reaches_map_bytes(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp);_, _, _, _, layout = self.fixture(root)
            original = bytearray(0xD0)
            struct.pack_into('<2I',original,0x14,3,1)
            struct.pack_into('<I',original,0x9C,0xC0)
            struct.pack_into('<3H',original,0xC0,*layout['layers'][0]['entries'])
            self.assertEqual(mi.build_map(original,layout),original)
            layout['layers'][0]['entries'][0] ^= 0x400
            changed = mi.build_map(original,layout)
            self.assertEqual([i for i,(a,b) in enumerate(zip(original,changed)) if a!=b],[0xC1])


if __name__=='__main__':unittest.main()
