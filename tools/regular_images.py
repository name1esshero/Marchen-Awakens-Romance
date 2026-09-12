#!/usr/bin/env python3
"""Editable 32x32 regular TSC screenblocks with palette banks 12..14.

Only complete, single-screenblock resources are migrated. Multi-screenblock
maps need their BG size/orientation decoded first. Bank-zero transparent tiles
are retained; nontransparent references outside the named palette are rejected.
"""
import asset_safety
import json
import re
from pathlib import Path
import struct
import gfx
import lz77
import mapped_images as mi
import build_assets

ROOT=mi.ROOT


def build_plane(layout,layer):
    if (layout['width_tiles'],layout['height_tiles'])!=(32,32):
        raise ValueError('Regular TSC profile requires one 32x32 screenblock')
    words=layer['entries']
    if len(words)!=1024 or any(not isinstance(v,int) or not 0<=v<=65535 for v in words):
        raise ValueError('Regular screenblock requires 1024 u16 entries')
    return struct.pack('<1024H',*words)


def save_progress(manifest,converted,skipped):
    (ROOT/'assets.json').write_text(json.dumps(manifest,indent=2)+'\n')
    for p in (ROOT/'asm/data').glob('*.s'):
        text=p.read_text();out=text
        for e in converted:out=out.replace(e['old_path']+'.lz',e['new_path']+'.lz')
        if out!=text:p.write_text(out)
    (ROOT/'reports/graphics/regular-image-migration.json').write_text(json.dumps(dict(converted=converted,skipped=skipped),indent=2)+'\n')


def main():
    manifest=mi.read(ROOT/'assets.json');maps=mi.read(ROOT/'maps/nfp/manifest.json')
    rom=(ROOT/'baserom.gba').read_bytes()
    previous=ROOT/'reports/graphics/regular-image-migration.json'
    converted=mi.read(previous)['converted'] if previous.exists() else []
    skipped=[]
    for entry in manifest:
        if asset_safety.has_english_art(ROOT/(entry['path']+'.png')):
            continue  # Do not migrate paths with authored English companions.
        if entry['kind']=='mapped_image' or entry['bpp']!=4:continue
        name=entry['archive_name'];stem=name.partition('.')[0]
        # Numeric screen suffixes only: BA06_BG belongs to BA06_BG, not BA06.
        sources=[m for m in maps if re.fullmatch(re.escape(stem) + r'(?:_?\d+)?\.TSC', m['name'])]
        if not sources:continue
        if any(m['size']!=2048 for m in sources):
            skipped.append(dict(name=name,reason='Multiple screenblocks: BG orientation not decoded'));continue
        old=entry['path'];pixels,_=gfx.read_png(str(ROOT/(old+'.png')))
        raw=build_assets.pack_pixels(entry,pixels)
        if raw!=lz77.decompress(rom,entry['rom_offset'])[0]:raise ValueError('Preserve source edits before migration')
        tiles=mi.tiles_from_bytes(raw);layers=[];used=set()
        problem=None
        for i,m in enumerate(sorted(sources,key=lambda m:m['name'])):
            words=list(struct.unpack('<1024H',(ROOT/m['path']).read_bytes()))
            for word in words:
                index=word&1023;bank=word>>12
                if index>=len(tiles):
                    problem=f'{m["name"]} references tile {index}, but the source has {len(tiles)} tiles'
                    break
                if not 12<=bank<12+entry['palette_banks'] and any(v for row in tiles[index] for v in row):
                    problem=m['name']+': nontransparent tile references an unavailable palette'
                    break
            if problem:break
            used.update(w&1023 for w in words)
            layers.append(dict(index=i,map_path=m['path'],entries=words))
        if problem:
            skipped.append(dict(name=name,reason=problem));continue
        base='graphics/backgrounds/'+name
        colors=[(0,0,0)]*192+gfx.read_jasc(str(ROOT/entry['palette_path']))
        for layer in layers:
            layer['image']=base+('' if layer['index']==0 else '.layer'+str(layer['index']))+'.png'
            if (ROOT/layer['image']).exists():raise ValueError('Refusing to overwrite an existing frame')
            gfx.write_png(str(ROOT/layer['image']),mi.render(tiles,layer['entries'],32,32),colors,transparent_indices=range(0,256,16))
        unused=sorted(set(range(len(tiles)))-used)
        entry.update(kind='mapped_image',path=base,image_layout=base+'.json',palette_bank_base=12,palette_base=192)
        if unused:
            entry['unused_tiles_image']='graphics/backgrounds/unused/'+name+'.png'
            gfx.write_png(str(ROOT/entry['unused_tiles_image']),[row for i in unused for row in tiles[i]],colors[192:208])
        layout=dict(format='regular_tsc_u16',width_tiles=32,height_tiles=32,tile_count=len(tiles),layers=layers,unused_tiles=unused,baseline_tiles=[mi.digest(t) for t in tiles])
        (ROOT/entry['image_layout']).write_text(json.dumps(layout,indent=2)+'\n')
        if build_assets.compile_entry(entry,ROOT)!=rom[entry['rom_offset']:entry['rom_offset']+entry['compressed_size']]:raise ValueError('Recompression differs after migration')
        for layer in layers:
            if build_plane(layout,layer)!=(ROOT/layer['map_path']).read_bytes():raise ValueError('TSC round-trip failed')
        converted.append(dict(name=name,old_path=old,new_path=base,frames=len(layers),width=256,height=256))
        save_progress(manifest,converted,skipped)
        asset_safety.unlink_generated(ROOT/(old+'.png'))
    save_progress(manifest,converted,skipped)
    print(len(converted),'resources,',sum(e['frames'] for e in converted),'regular image frames;',len(skipped),'unsupported map cases deferred')


if __name__=='__main__':main()
