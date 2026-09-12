"""NCD cell centers must become OAM top-lefts before compositing."""
import sys
import struct
import unittest
import tempfile
import json
from unittest.mock import patch
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import ncd
import scenes
import gfx
from sprite_sources import pixel_hash

class SpriteCenterTests(unittest.TestCase):
    def test_mixed_cell_sizes_and_edit_pixel_ownership(self):
        # Geometry from CHR frame 61: body, legs, hair, with solid test tiles.
        blob=bytearray(1024)
        info=dict(frames=1,cells=3,palettes=1,offsets=[0,0,0,8,68,100])
        struct.pack_into('<IHH',blob,0,0,3,5)
        for i,(y,shape,size,tile) in enumerate([(-28,0,2,0),(-4,1,2,16),(-48,1,0,24)]):
            struct.pack_into('<HHIIII',blob,8+i*20,(y&255)|(shape<<14),size<<14,0,tile,0,0)
        blob[100:612]=bytes([0x11])*512
        blob[612:868]=bytes([0x22])*256
        blob[868:932]=bytes([0x33])*64
        pixels,owners,_,meta=scenes.render(blob,info,0)
        self.assertEqual((meta['x'],meta['y'],meta['width'],meta['height']),(-16,-52,32,56))
        self.assertEqual(pixels[0][8],3)  # hair centered over 32-pixel body
        self.assertEqual(pixels[8][0],1)  # body starts directly below hair
        self.assertEqual(owners[0][8],(868,0,0))
        # Editing that visible pixel writes the original hair tile nibble.
        address,shift,_=owners[0][8]
        blob[address]=(blob[address]&~(15<<shift))|(4<<shift)
        self.assertEqual(scenes.render(blob,info,0)[0][0][8],4)
        self.assertEqual(ncd.cell(blob,info,2)['y'],-48) # source center unchanged
        # Exercise the production PNG-to-tile merge, not just the renderer.
        original = bytes(blob)
        before,_,palette,meta = scenes.render(original,info,0)
        record = dict(meta,id=0,path='frames/0000.png',baseline_pixels=pixel_hash(before))
        before[0][8] = 5
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)/'graphics/battle/characters'
            (folder/'frames').mkdir(parents=True)
            gfx.write_png(str(folder/record['path']),before,palette,transparent_index=0)
            (folder/'manifest.json').write_text(json.dumps({'frames':[record]}))
            with patch.object(scenes,'ROOT',Path(temp)), patch.object(ncd,'layout',return_value=info):
                result = scenes.merge(bytearray(original),original,'CHR')
            self.assertEqual([i for i,(a,b) in enumerate(zip(original,result)) if a!=b],[868])
            self.assertEqual(scenes.render(result,info,0)[0],before)

if __name__=='__main__':unittest.main()
