"""English credit packing must stay isolated from Japanese assets and other UI."""
import copy
import json
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import english_credits
import gfx
import ncd

ROOT=Path(__file__).resolve().parents[1]

class EnglishCreditTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path=ROOT/'build/graphics/ncd/SYSTEM.ncd'
        if not path.exists():raise unittest.SkipTest('Build the Japanese SYSTEM container first')
        cls.original=path.read_bytes();cls.info=ncd.layout(cls.original)
        cls.layout=english_credits.load()

    def test_packing_preserves_other_assets_and_protected_metadata(self):
        result=english_credits.apply(bytearray(self.original))
        allowed=english_credits.metadata_offsets(self.info)
        start=self.info['offsets'][5];low,high=self.layout['tile_span']
        for i,(a,b) in enumerate(zip(self.original[:start],result[:start])):
            if i not in allowed:self.assertEqual(a,b,hex(i))
        self.assertEqual(self.original[start:start+low*32],result[start:start+low*32])
        self.assertEqual(self.original[start+high*32:],result[start+high*32:])
        # The compiler also verifies all 82 full renders against source PNGs.
        self.assertEqual(len(self.layout['frames']),82)
        self.assertEqual(len(result),len(self.original))

    def test_rejects_tile_pool_shared_with_untranslated_frame(self):
        blob=bytearray(self.original)
        first=struct.unpack_from('<I',blob,self.info['offsets'][2])[0]
        struct.pack_into('<I',blob,self.info['offsets'][3]+first*20+8,self.layout['tile_span'][0])
        with self.assertRaisesRegex(ValueError,'shared with another frame'):english_credits.apply(blob)

    def test_rejects_overflow_and_unpaintable_cell_gaps(self):
        layout=copy.deepcopy(self.layout)
        layout['frames'][-1]['cells'][-1]['tile']=layout['tile_span'][1]
        with patch.object(english_credits,'load',return_value=layout):
            with self.assertRaisesRegex(ValueError,'allocation'):english_credits.apply(bytearray(self.original))
        layout=copy.deepcopy(self.layout)
        layout['frames'][0]['cells'][0]['x']+=1
        with patch.object(english_credits,'load',return_value=layout):
            with self.assertRaisesRegex(ValueError,'bounds'):english_credits.apply(bytearray(self.original))

    def test_material_panel_preserves_counter_space_borders_and_opacity(self):
        before,palette=gfx.read_png(str(ROOT/'graphics/ui/frames/1685.png'))
        after,colors=gfx.read_png(str(ROOT/'graphics/ui/frames/1685_en.png'))
        self.assertEqual(palette,colors)
        for y,(a,b) in enumerate(zip(before,after)):
            for x,(v,w) in enumerate(zip(a,b)):
                self.assertEqual(v==0,w==0,(x,y))
                edited=(72<=x<112 and 3<=y<12) or (8<=x<70 and any(top<=y<top+8 for top in (16,32,48,64,80,96)))
                if not edited:self.assertEqual(v,w,(x,y))
        # No old quantity lettering may survive below the cyan tab.
        for row in after[12:16]:self.assertNotIn(15,row[72:112])

if __name__=='__main__':unittest.main()
