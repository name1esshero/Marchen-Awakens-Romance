"""The encoder must generate original streams from pixels, with no LZ inputs."""
import json
from pathlib import Path
import random
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import build_assets
import lz77

ROOT=Path(__file__).resolve().parents[1]


class Lz77Tests(unittest.TestCase):
    def test_vram_encoder_excludes_distance_one(self):
        self.assertEqual(lz77.compress(bytes(8))[:9],bytes.fromhex('10 08 00 00 20 00 00 30 01'))
        self.assertEqual(lz77.compress(bytes(8),vram_safe=False)[:8],bytes.fromhex('10 08 00 00 40 00 40 00'))

    def test_fast_match_search_matches_brute_force(self):
        rng=random.Random(719)
        data=bytes(rng.randrange(4) for _ in range(512))
        for pos in range(len(data)):
            for minimum in (1,2):
                best=(0,0)
                for candidate in range(pos-minimum,max(-1,pos-4097),-1):
                    length=0
                    while length<min(18,len(data)-pos) and data[candidate+length]==data[pos+length]:length+=1
                    if length>=3 and length>best[0]:best=length,pos-candidate
                self.assertEqual(lz77._find_match(data,pos,len(data),minimum),best)

    def test_every_graphic_recompresses_exactly_from_editable_sources(self):
        rom=(ROOT/'baserom.gba').read_bytes()
        entries=json.loads((ROOT/'assets.json').read_text())
        self.assertEqual(len(entries),104)
        for entry in entries:
            self.assertFalse((ROOT/(entry['path']+'.lz')).exists())
            encoded=build_assets.compile_entry(entry,ROOT)
            original=rom[entry['rom_offset']:entry['rom_offset']+entry['compressed_size']]
            self.assertEqual(encoded,original,entry['archive_name'])


if __name__=='__main__':unittest.main()
