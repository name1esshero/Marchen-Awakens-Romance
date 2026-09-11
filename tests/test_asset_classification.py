import contextlib
import io
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import extract_assets
import extract_raw
import lz77


class ClassificationTests(unittest.TestCase):
    def test_reextract_preserves_existing_mapped_image_and_layout(self):
        packed = lz77.compress(bytes(64))
        _, consumed = lz77.decompress(packed)
        rom = bytearray(0x200)
        rom[0x100:0x100+len(packed)] = packed
        directory = [dict(name='TEST.KCG', rom_offset=0x100, end=0x180)]
        previous = Path.cwd()
        with tempfile.TemporaryDirectory() as temp:
            try:
                os.chdir(temp)
                Path('rom.gba').write_bytes(rom)
                Path('graphics/backgrounds').mkdir(parents=True)
                image = Path('graphics/backgrounds/TEST.KCG.png')
                layout = Path('graphics/backgrounds/TEST.KCG.json')
                image.write_bytes(b'edited image sentinel')
                layout.write_text('{"edited": true}')
                entry = dict(rom_offset=0x100, kind='mapped_image',
                             path='graphics/backgrounds/TEST.KCG', image_layout=str(layout))
                Path('assets.json').write_text(json.dumps([entry]))
                with patch.object(sys, 'argv', ['extract_assets.py', 'rom.gba']), \
                     patch.object(extract_assets.nfp, 'entries', return_value=directory), \
                     patch.object(extract_assets, 'find_streams', return_value=[(0x100, 64, consumed)]), \
                     contextlib.redirect_stdout(io.StringIO()):
                    extract_assets.main()
                self.assertEqual(image.read_bytes(), b'edited image sentinel')
                self.assertEqual(layout.read_text(), '{"edited": true}')
                self.assertEqual(json.loads(Path('assets.json').read_text())[0]['kind'], 'mapped_image')
            finally:
                os.chdir(previous)

    def test_valid_compression_inside_sprite_container_is_not_a_source(self):
        packed = lz77.compress(bytes(64))
        _, consumed = lz77.decompress(packed)
        rom = bytearray(0x500)
        for offset in (0x100, 0x200):
            rom[offset:offset + len(packed)] = packed
        directory = [dict(name='TEST.KCG', rom_offset=0x100, end=0x180),
                     dict(name='CHR.NCD', rom_offset=0x200, end=0x300),
                     dict(name='TEST.KCL', rom_offset=0x400, end=0x420,
                          size_with_padding=32)]
        previous = Path.cwd()
        with tempfile.TemporaryDirectory() as temp:
            try:
                os.chdir(temp)
                Path('rom.gba').write_bytes(rom)
                with patch.object(sys, 'argv', ['extract_assets.py', 'rom.gba']), \
                     patch.object(extract_assets.nfp, 'entries', return_value=directory), \
                     patch.object(extract_assets, 'find_streams', return_value=[
                         (0x100, 64, consumed), (0x200, 64, consumed)]), \
                     contextlib.redirect_stdout(io.StringIO()):
                    extract_assets.main()
                manifest = json.loads(Path('assets.json').read_text())
                self.assertEqual(len(manifest), 1)
                self.assertEqual(manifest[0]['kind'], 'tilesets')
                self.assertEqual(manifest[0]['path'], 'graphics/tilesets/TEST.KCG')
                candidates = json.loads(Path('reports/graphics/compression-candidates.json').read_text())
                self.assertEqual(candidates[0]['owners'], ['CHR.NCD'])
                self.assertFalse(Path('reports/graphics/legacy').exists())
            finally:
                os.chdir(previous)

    def test_script_scan_does_not_recreate_duplicate_lz_sources(self):
        raw=b'SCRP'+bytes(60);packed=lz77.compress(raw)
        _,consumed=lz77.decompress(packed)
        rom=bytearray(0x200);rom[0x100:0x100+len(packed)]=packed
        previous=Path.cwd()
        with tempfile.TemporaryDirectory() as temp:
            try:
                os.chdir(temp);Path('rom.gba').write_bytes(rom)
                with patch.object(sys,'argv',['extract_assets.py','rom.gba']), \
                     patch.object(extract_assets.nfp,'entries',return_value=[dict(name='TEST.SPC',rom_offset=0x100,end=0x180)]), \
                     patch.object(extract_assets,'find_streams',return_value=[(0x100,len(raw),consumed)]), \
                     contextlib.redirect_stdout(io.StringIO()):
                    extract_assets.main()
                self.assertEqual(json.loads(Path('assets.json').read_text()),[])
                self.assertEqual(list(Path('.').rglob('*.lz')),[])
                self.assertEqual(Path('scripts/script_000100.scrp').read_bytes(),raw)
            finally:os.chdir(previous)

    def test_raw_scan_only_writes_candidate_reports(self):
        previous = Path.cwd()
        with tempfile.TemporaryDirectory() as temp:
            try:
                os.chdir(temp)
                Path('rom.gba').write_bytes(bytes(range(256)) * 4)
                Path('runs.json').write_text(json.dumps([[0, 512, 'art'], [512, 1024, 'sound']]))
                with patch.object(sys, 'argv', ['extract_raw.py', 'rom.gba', 'runs.json']), \
                     contextlib.redirect_stdout(io.StringIO()):
                    extract_raw.main()
                self.assertEqual(len(json.loads(Path('reports/graphics/raw-candidates.json').read_text())), 1)
                self.assertEqual(len(json.loads(Path('reports/sound/pcm-candidates.json').read_text())), 1)
                self.assertFalse(Path('assets_raw.json').exists())
                self.assertFalse(list(Path('.').rglob('*.png')))
                self.assertFalse(Path('reports/graphics/legacy').exists())
            finally:
                os.chdir(previous)


if __name__ == '__main__':
    unittest.main()
