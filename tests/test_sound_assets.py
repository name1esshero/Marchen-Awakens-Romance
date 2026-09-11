import json
from pathlib import Path
import sys
import tempfile
import unittest
import wave
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import sound_assets


class SoundAssetTest(unittest.TestCase):
    def test_all_driver_samples_match_and_have_references(self):
        rom=(sound_assets.ROOT/'baserom.gba').read_bytes()
        samples=json.loads((sound_assets.OUT/'samples/manifest.json').read_text())
        self.assertEqual(len(samples),115)
        for e in samples:
            self.assertTrue(e['references'])
            self.assertEqual(sound_assets.compile_sample(e),rom[e['rom_offset']:e['rom_offset']+e['size']])

    def test_loop_edit_changes_sample_and_interpolation_guard(self):
        e=next(e for e in json.loads((sound_assets.OUT/'samples/manifest.json').read_text()) if e['status']==0x4000)
        original=sound_assets.compile_sample(e)
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);path=root/'edited.wav'
            raw=bytearray(original[16:16+e['sample_count']]);raw[e['loop_start']]^=1
            with wave.open(str(path),'wb') as out:
                out.setnchannels(1);out.setsampwidth(1);out.setframerate(e['frequency']//1024)
                out.writeframes(bytes(b^128 for b in raw))
            entry=dict(e,wav='edited.wav')
            edited=sound_assets.compile_sample(entry,root)
            self.assertEqual([i for i,(a,b) in enumerate(zip(original,edited)) if a!=b],[16+e['loop_start'],16+e['sample_count']])
            path.unlink()
            with self.assertRaises(FileNotFoundError):sound_assets.compile_sample(entry,root)

if __name__=='__main__':unittest.main()
