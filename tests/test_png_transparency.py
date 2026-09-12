"""Per-bank GBA transparency must not rewrite indexed source pixels."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import gfx


class PngTransparencyTest(unittest.TestCase):
    def test_palette_zero_entries_are_transparent_without_index_changes(self):
        pixels=[[0,192,193,208,209,224,225,240]]
        palette=[(i,i,i) for i in range(256)]
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'frame.png'
            gfx.write_png(str(path),pixels,palette,transparent_indices=range(0,256,16))
            self.assertEqual(gfx.read_png(str(path)),(pixels,palette))
            blob=path.read_bytes();offset=8;alpha=None
            while offset<len(blob):
                size=struct.unpack_from('>I',blob,offset)[0]
                if blob[offset+4:offset+8]==b'tRNS':alpha=blob[offset+8:offset+8+size]
                offset+=12+size
            self.assertEqual([i for i,v in enumerate(alpha) if v==0],list(range(0,256,16)))
            self.assertEqual(alpha[193],255)
            with self.assertRaises(ValueError):
                gfx.write_png(str(path),pixels,palette,transparent_index=0,transparent_indices=[192])


if __name__=='__main__':unittest.main()
