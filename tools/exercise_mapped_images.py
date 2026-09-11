#!/usr/bin/env python3
"""Clean build, prove OP_05_A image/map edits reach ROM, then restore sources.

Run alone after tests finish. Original-ROM Python reads are blocked in build
subprocesses. A finally block restores both edited sources and the matching ROM.
"""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import gfx
import lz77
import mapped_images as mi
import build_assets


def main():
    root = mi.ROOT
    report_dir = root/'reports/build'
    base = (root/'baserom.gba').read_bytes()
    entry = next(e for e in mi.read(root/'assets.json') if e.get('archive_name')=='OP_05_A.KCG')
    layout_path = root/entry['image_layout']
    png = root/(entry['path']+'.png')
    layout = mi.read(layout_path)
    plane = layout['layers'][0]
    assert plane['entries'][0]&1023 == 2 and plane['entries'][2]&1023 == 4
    assert sum((v&1023)==4 for layer in layout['layers'] for v in layer['entries']) == 1
    map_entry = next(e for e in mi.read(root/'maps/nfp/manifest.json') if e['name']=='OP_05_A.KMP')
    map_byte = map_entry['rom_offset'] + plane['offset'] + 5
    original_tiles = lz77.decompress(base,entry['rom_offset'])[0]
    expected = bytearray(original_tiles)
    expected[64] ^= 1
    tile = mi.tiles_from_bytes(original_tiles)[4]
    expected[128:160] = gfx.pixels_to_tiles([row[::-1] for row in tile],4)
    saved = {p:p.read_bytes() for p in (png,layout_path)}
    # The previous tile-3 flip needs more space with the correct VRAM-safe
    # encoder. Reject overflow explicitly rather than weakening that encoder.
    overflowing = bytearray(original_tiles)
    overflowing[64] ^= 1
    old_tile = mi.tiles_from_bytes(original_tiles)[3]
    overflowing[96:128] = gfx.pixels_to_tiles([row[::-1] for row in old_tile],4)
    try:
        build_assets.compress_raw(entry, bytes(overflowing))
    except ValueError as error:
        assert 'does not fit' in str(error)
    else:
        raise AssertionError('Oversized stream was silently accepted')
    loose_lz = [str(p.relative_to(root)) for folder in ('graphics','scripts') for p in (root/folder).rglob('*.lz')]
    assert not loose_lz
    report = dict(oversized_mutation_rejected=True, source_lz_files=loose_lz)
    with tempfile.TemporaryDirectory(prefix='mar-map-proof-') as temp:
        Path(temp,'sitecustomize.py').write_text('import os,sys\nfrom pathlib import Path\nblocked=Path(os.environ["MAR_BLOCK_ROM"]).resolve()\ndef audit(event,args):\n if event=="open" and isinstance(args[0],(str,bytes)) and Path(os.fsdecode(args[0])).resolve()==blocked:raise RuntimeError("Original ROM read blocked during build")\nsys.addaudithook(audit)\n')
        env = dict(os.environ,MAR_BLOCK_ROM=str(root/'baserom.gba'))
        env['PYTHONPATH'] = temp+os.pathsep+env.get('PYTHONPATH','')
        def build(label,clean=False):
            with (report_dir/(label+'.log')).open('w') as log:
                if clean:subprocess.run(['make','clean'],cwd=root,stdout=log,stderr=subprocess.STDOUT,check=True)
                subprocess.run(['make','-j4','all'],cwd=root,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
            return (root/'mar.gba').read_bytes()
        assert build('mapped-clean-build',True)==base
        report['clean_rebuild'] = dict(byte_matching=True,python_original_rom_reads='blocked',legacy_folder_absent=not (root/'reports/graphics/legacy').exists())
        print('Clean mapped-image build matches',flush=True)
        try:
            pixels,colors = gfx.read_png(str(png));pixels[0][0] ^= 1
            gfx.write_png(str(png),pixels,colors)
            plane['entries'][2] ^= 0x400
            layout_path.write_text(json.dumps(layout,indent=2)+'\n')
            modified = build('mapped-source-mutations')
            assert len(modified)==len(base)
            decoded = lz77.decompress(modified,entry['rom_offset'])[0]
            assert decoded==expected,'Image edits did not compile into the expected tile bytes'
            differences = [i for i,(a,b) in enumerate(zip(base,modified)) if a!=b]
            outside = [i for i in differences if not entry['rom_offset']<=i<entry['rom_offset']+entry['compressed_size']]
            assert outside==[map_byte] and modified[map_byte]==base[map_byte]^4
            result = subprocess.run([sys.executable,'tools/compare.py','baserom.gba','mar.gba'],cwd=root,capture_output=True,text=True)
            assert result.returncode==1
            (report_dir/'mapped-mutation-compare.log').write_text(result.stdout)
            report['mutations'] = dict(files=[str(p.relative_to(root)) for p in saved],differing_bytes=len(differences),
                differing_offsets=[f'{i:06X}' for i in differences],map_byte=f'{map_byte:06X}',decoded_tiles_match_expected=True,compare_exit_code=1)
            print('Image pixel and independent map flip reached final ROM',flush=True)
        finally:
            for path,raw in saved.items():path.write_bytes(raw)
            restored = build('mapped-restored-build')
            assert restored==base,'Restored sources no longer match'
            report['restored'] = dict(byte_matching=True,sha1=hashlib.sha1(restored).hexdigest())
            (report_dir/'mapped-image-proof.json').write_text(json.dumps(report,indent=2)+'\n')
            print('Sources restored; ROM matches',flush=True)


if __name__=='__main__':main()
