#!/usr/bin/env python3
"""Export all ICON/ICONMINI frames using verified NCD cell and palette tables.

Per-icon indexed PNGs are editable sources. NCD build applies them only when
pixels differ from the extracted original, so unchanged copies do not mask
edits to individual SYSTEM cell sources. Conflicting edits are rejected.
"""
import json
from pathlib import Path
import struct
import gfx
import ncd
from sprite_sources import pixel_hash

ROOT=ncd.ROOT
DIR=ROOT/'graphics/icons'


def extract():
    DIR.mkdir(parents=True,exist_ok=True)
    blob=ncd.source('SYSTEM');info=ncd.layout(blob)
    records=[];sheets={}
    for group in range(info['groups']):
        pos=info['offsets'][0]+group*16
        name=blob[pos:pos+8].split(b'\0')[0].decode('ascii')
        if name not in ('ICON','ICONMINI'):continue
        first,count=struct.unpack_from('<II',blob,pos+8)
        sheet=[[(40,44,52)]*(7*40) for _ in range(count*40)]
        for animation in range(count):
            frame,nframes=struct.unpack_from('<II',blob,info['offsets'][1]+(first+animation)*8)
            for f in range(nframes):
                start,ncells,_=struct.unpack_from('<IHH',blob,info['offsets'][2]+(frame+f)*8)
                if ncells!=1:raise ValueError('Icon frame is not a single editable cell')
                cell=ncd.cell(blob,info,start)
                pixels=gfx.tiles_to_pixels(blob[cell['start']:cell['end']],4,cell['width']//8)
                colors=gfx.read_palette(blob,info['offsets'][4]+cell['palette']*32,16)
                path=f'graphics/icons/{name}_{animation}_{f}.png'
                gfx.write_png(str(ROOT/path),pixels,colors)
                records.append(dict(cell,path=path,group=name,animation=animation,frame=frame+f,baseline_pixels=pixel_hash(pixels)))
                preview=ncd.frame_preview(blob,info,frame+f)
                for y,row in enumerate(preview):
                    for x,rgb in enumerate(row):sheet[animation*40+y][f*40+x]=rgb
        sheets[name]=sheet
        ncd.write_rgb(DIR/(name+'_contact.png'),sheet)
    (DIR/'manifest.json').write_text(json.dumps(records,indent=2)+'\n')
    page='''<!doctype html><meta charset="utf-8"><title>MAR icons</title><style>body{font:18px system-ui;background:#282c34;color:white;margin:32px}img{image-rendering:pixelated}a{color:#9cf}</style><h1>Original MAR icon graphics</h1><p>42 ICON frames and 42 ICONMINI frames, with original palette colors. Rows are animations; columns are frames. Individual indexed PNGs in this folder are editable build inputs.</p>'''
    for name in sheets:page+=f'<h2>{name}</h2><img width="560" src="{name}_contact.png">'
    page+='<p>Palette sources: ../ui/palettes/NNN.pal; see manifest.json for each icon’s bank, cell, and tile offsets.</p>'
    (DIR/'index.html').write_text(page)
    print(f'Exported {len(records)} editable icon frames')


def apply(blob, original, manifest=None, blocked_addresses=None):
    manifest = manifest or DIR/'manifest.json'
    writes={}
    for e in json.loads(manifest.read_text()):
        px,_=gfx.read_png(str(ROOT/e['path']))
        if len(px)!=e['height'] or any(len(row)!=e['width'] or any(v>15 for v in row) for row in px):raise ValueError('Invalid icon dimensions or indices: '+e['path'])
        if e.get('baseline_pixels'):
            from sprite_sources import pixel_hash
            if pixel_hash(px)==e['baseline_pixels']:continue
        encoded=gfx.pixels_to_tiles(px,4)
        old=original[e['start']:e['end']]
        for offset,(before,after) in enumerate(zip(old,encoded),e['start']):
            if before==after:continue
            if blocked_addresses and offset in blocked_addresses:raise ValueError('Edit either the cell source or its icon/portrait view')
            if (offset in writes and writes[offset]!=after) or blob[offset] not in (before,after):
                raise ValueError('Conflicting icon/cell-image edit at '+hex(offset))
            writes[offset]=after
    for offset,value in writes.items():blob[offset]=value
    return blob


if __name__=='__main__':extract()
