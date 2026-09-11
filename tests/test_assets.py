import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import build_assets
import gfx
import icons
import lz77
import named_scripts
import ncd
import sprite_sources
from functools import lru_cache
import scenes
from unittest.mock import patch
import nfp
import rle
import text_codec

ROOT = ncd.ROOT


@lru_cache(maxsize=None)
def original_container(stem):
    rom=(ROOT/'baserom.gba').read_bytes()
    e=next(e for e in nfp.entries(rom) if e['name']==stem+'.NCD')
    return rom[e['rom_offset']:e['end']]


class AssetTest(unittest.TestCase):
    def test_named_scripts_all_roundtrip(self):
        for e in json.loads((ROOT/'scripts/nfp/manifest.json').read_text()):
            original=(ROOT/e['path']).read_bytes()
            self.assertEqual(named_scripts.rebuild(original,named_scripts.edits(ROOT/e['text'])),original,e['name'])

    def test_private_character_bytes(self):
        for raw in (b'\xF0\x56\x82\x71\x82\x6C',b'\xF0\x40',b'\x80\x01',b' text  '):
            self.assertEqual(text_codec.encode(text_codec.decode_lossless(raw)),raw)

    def test_portrait_pixel_changes_only_its_cell_byte(self):
        original=original_container('SYSTEM')
        e=json.loads((ROOT/'graphics/portraits/manifest.json').read_text())[0]
        px,pal=gfx.read_png(str(ROOT/e['path']))
        self.assertEqual(gfx.pixels_to_tiles(px,4),original[e['start']:e['end']])
        with tempfile.TemporaryDirectory() as tmp:
            image=Path(tmp)/'portrait.png';manifest=Path(tmp)/'manifest.json'
            px[0][0]^=1
            gfx.write_png(str(image),px,pal)
            manifest.write_text(json.dumps([dict(e,path=str(image))]))
            result=icons.apply(bytearray(original),original,manifest)
            self.assertEqual([i for i,(a,b) in enumerate(zip(original,result)) if a!=b],[e['start']])
            self.assertEqual(original[e['start']]^result[e['start']],1)
            conflict=bytearray(original);conflict[e['start']]^=2
            with self.assertRaises(ValueError):icons.apply(conflict,original,manifest)

    def test_scene_edit_roundtrip_and_conflict(self):
        original=original_container('CHR')
        info=ncd.layout(original)
        px,owners,pal,meta=scenes.render(original,info,0)
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);folder=root/'graphics/battle/characters'
            folder.mkdir(parents=True)
            source=folder/'frame.png'
            (folder/'manifest.json').write_text(json.dumps({'frames':[{'id':0,'path':'frame.png'}]}))
            gfx.write_png(str(source),px,pal,transparent_index=0)
            with patch.object(scenes,'ROOT',root):
                self.assertEqual(scenes.merge(bytearray(original),original,'CHR'),original)
                y,x=next((y,x) for y,row in enumerate(px) for x,v in enumerate(row) if v)
                address,shift,bank=owners[y][x]
                value=(px[y][x]%16)%15+1
                px[y][x]=bank*16+value
                gfx.write_png(str(source),px,pal,transparent_index=0)
                result=scenes.merge(bytearray(original),original,'CHR')
                self.assertEqual([i for i,(a,b) in enumerate(zip(original,result)) if a!=b],[address])
                self.assertEqual((result[address]>>shift)&15,value)
                conflict=bytearray(original)
                other=value%15+1
                if other==((original[address]>>shift)&15):other=other%15+1
                conflict[address]=(conflict[address]&~(15<<shift))|(other<<shift)
                with self.assertRaises(ValueError):scenes.merge(conflict,original,'CHR')

    def test_unchanged_pixel_hash_cannot_hide_frame_resize(self):
        original=original_container('CHR');info=ncd.layout(original)
        px,_,pal,meta=scenes.render(original,info,0)
        flat=[v for row in px for v in row];width=meta['width']*2
        resized=[flat[i:i+width] for i in range(0,len(flat),width)]
        self.assertEqual(sprite_sources.pixel_hash(px),sprite_sources.pixel_hash(resized))
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);folder=root/'graphics/battle/characters';folder.mkdir(parents=True)
            gfx.write_png(str(folder/'frame.png'),resized,pal)
            (folder/'manifest.json').write_text(json.dumps({'frames':[dict(meta,id=0,path='frame.png',baseline_pixels=sprite_sources.pixel_hash(px))]}))
            with patch.object(scenes,'ROOT',root):
                with self.assertRaisesRegex(ValueError,'dimensions changed'):scenes.merge(bytearray(original),original,'CHR')

    def test_scene_consolidation_requires_same_cells_and_file_bytes(self):
        blob=bytearray(24+40)
        for index,cell_index in enumerate((0,0,1)):
            struct.pack_into('<IHH',blob,index*8,cell_index,1,index+1)
        blob[44]=1  # Different cell data even though the PNG looks identical.
        info={'offsets':[0,0,0,24]}
        with tempfile.TemporaryDirectory() as tmp:
            folder=Path(tmp)
            for name in ('a.png','b.png','c.png'):
                gfx.write_png(str(folder/name),[[0]*8 for _ in range(8)],[(0,0,0)])
            data={'frames':[{'id':i,'path':name} for i,name in enumerate(('a.png','b.png','c.png'))]}
            with patch.object(scenes,'ROOT',folder):
                removed=scenes.consolidate(data,folder,blob,info)
                self.assertEqual([r['path'] for r in removed],['b.png'])
                self.assertEqual([f['path'] for f in data['frames']],['a.png','a.png','c.png'])
                # Distinct edits must not disappear during a subsequent cleanup.
                (folder/'b.png').write_bytes(b'different edited source')
                data['frames'][1]['path']='b.png'
                self.assertEqual(scenes.consolidate(data,folder,blob,info),[])

    def test_all_ncd_cells_and_palettes(self):
        for e in json.loads((ROOT/'graphics/sprite_containers.json').read_text()):
            blob=original_container(e['stem']);info=ncd.layout(blob)
            cells=[ncd.cell(blob,info,i) for i in range(info['cells'])]
            self.assertEqual(max(c['end'] for c in cells),len(blob))
            for i in range(info['palettes']):
                colors=gfx.read_jasc(str(sprite_sources.folder(e['stem'])/'palettes'/f'{i:03}.pal'))
                p=info['offsets'][4]+32*i
                self.assertEqual(gfx.palette_to_bytes(colors),blob[p:p+32])

    def test_ncd_reconstruction_without_original_containers(self):
        for stem in sprite_sources.CATEGORIES:
            self.assertEqual(sprite_sources.compile(stem),original_container(stem),stem)

    def test_cell_pixels_and_animation_metadata_reach_reconstructed_ncd(self):
        base=sprite_sources.folder('CHR')
        original=original_container('CHR');info=ncd.layout(original)
        with tempfile.TemporaryDirectory() as tmp:
            folder=Path(tmp);(folder/'source').mkdir()
            for path in (base/'source').glob('*.json'):
                (folder/'source'/path.name).write_bytes(path.read_bytes())
            (folder/'palettes').symlink_to(base/'palettes',target_is_directory=True)
            images=json.loads((folder/'source/images.json').read_text())
            for e in images:e['path']=str(base/e['path'])
            e=images[0];px,pal=gfx.read_png(e['path']);px[0][0]^=1
            image=folder/'edited.png';gfx.write_png(str(image),px,pal)
            e['path']=str(image)
            (folder/'source/images.json').write_text(json.dumps(images))
            frames=json.loads((folder/'source/frames.json').read_text());frames[0]['duration']^=1
            (folder/'source/frames.json').write_text(json.dumps(frames))
            with patch.object(sprite_sources,'folder',return_value=folder):
                result=sprite_sources.compile('CHR')
            self.assertEqual([i for i,(a,b) in enumerate(zip(original,result)) if a!=b],
                             [info['offsets'][2]+6,info['offsets'][5]+e['tile']*32])
            image.unlink()
            with patch.object(sprite_sources,'folder',return_value=folder):
                with self.assertRaises(FileNotFoundError):sprite_sources.compile('CHR')

    def test_missing_png_cannot_silently_reuse_original(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder=Path(tmp)
            (folder/'assets.json').write_text(json.dumps([{'path':'original','kind':'sprites'}]))
            result=subprocess.run([sys.executable,str(ROOT/'tools/build_assets.py')],cwd=folder,capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0)
            self.assertIn('Missing editable graphics source',result.stderr)
            self.assertFalse((folder/'build/original.lz').exists())

    def test_image_compression_edits_and_fixed_span(self):
        raw=bytes(512)
        compressed=lz77.compress(raw)
        entry=dict(path='test',kind='tilesets',bpp=4,width_tiles=16,raw_size=512,compressed_size=len(compressed)+128)
        px=gfx.tiles_to_pixels(raw,4,16)
        result=build_assets.compress_raw(entry,build_assets.pack_pixels(entry,px))
        self.assertEqual(lz77.decompress(result)[0],raw)
        px[0][0]=1
        result=build_assets.compress_raw(entry,build_assets.pack_pixels(entry,px))
        self.assertEqual(len(result),entry['compressed_size'])
        self.assertEqual(lz77.decompress(result)[0],bytes([1])+raw[1:])
        px[0][0]=16
        with self.assertRaises(ValueError):build_assets.pack_pixels(entry,px)
        with self.assertRaises(ValueError):build_assets.compress_raw(dict(entry,compressed_size=1),raw)


if __name__ == '__main__':unittest.main()
