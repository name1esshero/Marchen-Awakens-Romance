"""English frame overrides must never leak into the matching Japanese build."""
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import gfx
import ncd
import scenes
from sprite_sources import pixel_hash

class EnglishGraphicsTests(unittest.TestCase):
    def test_language_selection_removal_and_palette_validation(self):
        original=bytearray(100)
        info=dict(frames=1,cells=1,palettes=1,offsets=[0,0,0,8,28,60])
        struct.pack_into('<IHH',original,0,0,1,1)
        struct.pack_into('<HHIIII',original,8,0,0,0,0,0,0)
        original[60:92]=bytes([0x11])*32
        original=bytes(original)
        px,_,pal,meta=scenes.render(original,info,0)
        record=dict(meta,id=0,path='frames/0000.png',baseline_pixels=pixel_hash(px))
        with tempfile.TemporaryDirectory() as temp:
            folder=Path(temp)/'graphics/ui';(folder/'frames').mkdir(parents=True)
            path=folder/record['path'];variant=path.with_stem('0000_en')
            gfx.write_png(str(path),px,pal,transparent_index=0)
            (folder/'manifest.json').write_text(json.dumps(dict(frames=[record])))
            px[0][0]=2
            gfx.write_png(str(variant),px,pal,transparent_index=0)
            with patch.object(scenes,'ROOT',Path(temp)),patch.object(ncd,'layout',return_value=info):
                self.assertEqual(scenes.merge(bytearray(original),original,'SYSTEM'),original)
                english=scenes.merge(bytearray(original),original,'SYSTEM',english=True)
                self.assertEqual(english[60],0x12)
                self.assertEqual(english[:60],original[:60])
                badpal=list(pal);badpal[1]=(255,0,0)
                gfx.write_png(str(variant),px,badpal,transparent_index=0)
                with self.assertRaisesRegex(ValueError,'palette'):
                    scenes.merge(bytearray(original),original,'SYSTEM',english=True)
                gfx.write_png(str(variant),px,pal,transparent_indices=[0,1])
                with self.assertRaisesRegex(ValueError,'transparency'):
                    scenes.merge(bytearray(original),original,'SYSTEM',english=True)
                variant.unlink()
                self.assertEqual(scenes.merge(bytearray(original),original,'SYSTEM',english=True),original)

    def test_batch_four_preserves_controls_dialogue_borders_and_floor_marker(self):
        root=Path(__file__).resolve().parents[1];ui=root/'graphics/ui'
        recipes=[r for r in json.loads((ui/'text_labels.json').read_text()) if r.get('batch')==4]
        self.assertEqual(len(recipes),17)
        for r in recipes:
            source=ui/r['source'];dest=source.with_stem(source.stem+'_en')
            before,pal=gfx.read_png(str(source));after,colors=gfx.read_png(str(dest))
            self.assertEqual(pal,colors);self.assertEqual(gfx.png_alpha(source),gfx.png_alpha(dest))
            if r['kind']=='button':
                for a,b in zip(before,after):self.assertEqual(a[:r.get('left',8)],b[:r.get('left',8)])
            if r['kind']=='message':
                left,top,right,bottom=r['text_rect']
                for y,(a,b) in enumerate(zip(before,after)):
                    self.assertEqual([v==0 for v in a],[v==0 for v in b])
                    if not top<=y<bottom:self.assertEqual(a,b)
                    else:self.assertEqual(a[:left],b[:left]);self.assertEqual(a[right:],b[right:])
        link,_=gfx.read_png(str(ui/'frames/1841_en.png'))
        for row in link:self.assertNotIn(15,row[21:32]) # no leftover Japanese ink
        a,_=gfx.read_png(str(ui/'frames/1802_en.png'));b,_=gfx.read_png(str(ui/'frames/1803_en.png'))
        for x,y in zip(a,b):self.assertEqual(x[:32],y[:32])
        for number in range(1561,1565):
            source=ui/f'frames/{number}.png';dest=source.with_stem(source.stem+'_en')
            before,pal=gfx.read_png(str(source));after,colors=gfx.read_png(str(dest))
            self.assertEqual(pal,colors);self.assertEqual(gfx.png_alpha(source),gfx.png_alpha(dest))
            self.assertEqual((len(before),len(before[0])),(len(after),len(after[0])))
            if number==1563:
                for a,b in zip(before,after):self.assertEqual(a[56:],b[56:])

    def test_batch_three_icons_frames_and_layered_backdrops(self):
        root=Path(__file__).resolve().parents[1]
        for number in (1801,1804,1805,1807,1808,1809,1810):
            before,_=gfx.read_png(str(root/f'graphics/ui/frames/{number}.png'))
            after,_=gfx.read_png(str(root/f'graphics/ui/frames/{number}_en.png'))
            for a,b in zip(before,after):self.assertEqual(a[:8],b[:8])
        for number in (1686,1687):
            before,_=gfx.read_png(str(root/f'graphics/ui/frames/{number}.png'))
            after,_=gfx.read_png(str(root/f'graphics/ui/frames/{number}_en.png'))
            for y,(a,b) in enumerate(zip(before,after)):
                if y<3 or y>=12:self.assertEqual(a,b)
                else:self.assertEqual(a[:32],b[:32]);self.assertEqual(a[160:],b[160:])
        for number in list(range(1893,1898))+list(range(1902,1907)):
            path=root/f'graphics/ui/frames/{number}.png'
            variant=path.with_stem(path.stem+'_en')
            before,pal=gfx.read_png(str(path));after,colors=gfx.read_png(str(variant))
            self.assertEqual(pal,colors)
            self.assertEqual(gfx.png_alpha(path),gfx.png_alpha(variant))
            for a,b in zip(before,after):
                self.assertEqual(a[:20],b[:20]);self.assertEqual(a[108:],b[108:])
        for tile in (33791,33799,33807):
            self.assertFalse((root/f'graphics/ui/cells/T_TTL05/tile_{tile}_en.png').exists())

    def test_name_batch_preserves_transparency_and_opaque_backgrounds(self):
        root=Path(__file__).resolve().parents[1]
        records=[r for r in json.loads((root/'graphics/ui/text_labels.json').read_text()) if all(1820<=f<=1839 for f in r['frames'])]
        self.assertEqual(sum(len(r['frames']) for r in records),20)
        for record in records:
            source=root/'graphics/ui'/record['source'];variant=source.with_stem(source.stem+'_en')
            before,palette=gfx.read_png(str(source));after,colors=gfx.read_png(str(variant))
            self.assertEqual(palette,colors)
            self.assertEqual(gfx.png_alpha(source),gfx.png_alpha(variant))
            self.assertEqual([[v==0 for v in row] for row in before],[[v==0 for v in row] for row in after])
            self.assertNotEqual(before,after)
            if record['kind']=='shop':
                for a,b in zip(before,after):self.assertEqual(a[32:],b[32:])
            else:
                for a,b in zip(before,after):self.assertEqual(a[:5],b[:5])

    def test_batch_five_preserves_popup_borders_floor_markers_and_palettes(self):
        root=Path(__file__).resolve().parents[1]/'graphics/ui'
        records=[r for r in json.loads((root/'text_labels.json').read_text()) if r.get('batch')==5]
        self.assertEqual(len(records),40)
        for record in records:
            source=root/record['source'];variant=source.with_stem(source.stem+'_en')
            before,palette=gfx.read_png(str(source));after,colors=gfx.read_png(str(variant))
            self.assertEqual(palette,colors)
            self.assertEqual(gfx.png_alpha(source),gfx.png_alpha(variant))
            self.assertEqual((len(before),len(before[0])),(len(after),len(after[0])))
            if record['kind']=='message':
                for a,b in zip(before,after):
                    self.assertEqual(a[:12],b[:12]);self.assertEqual(a[64:],b[64:])
                    self.assertEqual([v==0 for v in a],[v==0 for v in b])
        for n in range(1570,1579):
            if n==1573:continue
            before,_=gfx.read_png(str(root/f'frames/{n}_en.png'))
            original,_=gfx.read_png(str(root/f'frames/{n}.png'))
            left=128 if n<1573 else 72
            for a,b in zip(original,before):self.assertEqual(a[left:],b[left:])

    def test_all_title_variants_preserve_end_ornaments(self):
        root=Path(__file__).resolve().parents[1]/'graphics/ui/frames'
        for number in list(range(1878,1893))+list(range(1898,1902)):
            source=root/f'{number:04}.png';variant=root/f'{number:04}_en.png'
            original,pal=gfx.read_png(str(source));english,colors=gfx.read_png(str(variant))
            self.assertEqual(pal,colors)
            self.assertEqual(gfx.png_alpha(source),gfx.png_alpha(variant))
            self.assertEqual((len(original),len(original[0])),(len(english),len(english[0])))
            edge=32 if len(original[0])==240 else 16
            for a,b in zip(original,english):
                self.assertEqual(a[:edge],b[:edge]);self.assertEqual(a[-edge:],b[-edge:])

    def test_story_menu_variants_preserve_original_rings(self):
        root=Path(__file__).resolve().parents[1]/'graphics/ui/frames'
        for number in range(1889,1893):
            original,pal=gfx.read_png(str(root/f'{number:04}.png'))
            english,colors=gfx.read_png(str(root/f'{number:04}_en.png'))
            self.assertEqual(pal,colors)
            self.assertEqual((len(english),len(english[0])),(32,240))
            self.assertNotEqual(original,english)
            self.assertEqual(gfx.png_alpha(root/f'{number:04}_en.png'),bytes([0])+bytes([255])*255)
            # The M's upper stroke was missing in the initial generated draft.
            for y in range(8,15):
                self.assertTrue(all(english[y][x]!=0 for x in range(133,144)))
            for a,b in zip(original,english):
                self.assertEqual(a[:32],b[:32])
                self.assertEqual(a[208:],b[208:])

if __name__=='__main__':unittest.main()
