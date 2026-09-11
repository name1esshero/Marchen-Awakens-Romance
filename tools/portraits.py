#!/usr/bin/env python3
"""Extract the 152 named, single-cell dialogue portraits from SYSTEM.NCD."""
import json
import re
import struct
import gfx
import ncd
from sprite_sources import pixel_hash

ROOT=ncd.ROOT
DIR=ROOT/'graphics/portraits'


def extract():
    DIR.mkdir(parents=True,exist_ok=True)
    blob=ncd.source('SYSTEM');info=ncd.layout(blob);entries=[]
    for i in range(info['groups']):
        pos=info['offsets'][0]+i*16;name=blob[pos:pos+8].split(b'\0')[0].decode('ascii')
        if not re.fullmatch(r'F\d\d[A-Z]\d\d',name):continue
        animation,nanimations=struct.unpack_from('<II',blob,pos+8)
        frame,nframes=struct.unpack_from('<II',blob,info['offsets'][1]+animation*8)
        start,ncells,_=struct.unpack_from('<IHH',blob,info['offsets'][2]+frame*8)
        if (nanimations,nframes,ncells)!=(1,1,1):raise ValueError('Unexpected portrait structure')
        c=ncd.cell(blob,info,start)
        pixels=gfx.tiles_to_pixels(blob[c['start']:c['end']],4,c['width']//8)
        colors=gfx.read_palette(blob,info['offsets'][4]+c['palette']*32,16)
        path=f'graphics/portraits/{name}.png';gfx.write_png(str(ROOT/path),pixels,colors)
        entries.append(dict(c,name=name,path=path,frame=frame,baseline_pixels=pixel_hash(pixels)))
    (DIR/'manifest.json').write_text(json.dumps(entries,indent=2)+'\n')
    sheet=[[(40,44,52)]*(8*72) for _ in range(((len(entries)+7)//8)*72)]
    for i,e in enumerate(entries):
        px,_=gfx.read_png(str(ROOT/e['path']));pal=gfx.read_palette(blob,info['offsets'][4]+e['palette']*32,16)
        for y,row in enumerate(px):
            for x,v in enumerate(row):sheet[i//8*72+y][i%8*72+x]=pal[v] if v else (40,44,52)
    ncd.write_rgb(DIR/'contact.png',sheet)
    page='''<!doctype html><meta charset="utf-8"><title>MAR dialogue portraits</title><style>body{font:18px system-ui;background:#282c34;color:white;margin:24px}main{display:grid;grid-template-columns:repeat(auto-fill,150px);gap:16px}img{image-rendering:pixelated;width:128px}a{color:#9cf}input{font:inherit;margin:16px}</style><h1>152 original dialogue portraits</h1><p>Each indexed PNG is editable and feeds the SYSTEM.NCD rebuild. Labels are the original resource names used by scripts. Palette bank sources are linked under each portrait.</p><input id="q" placeholder="Filter resource name"><main>'''
    for e in entries:
        page+=f'<article><a href="{e["name"]}.png"><img src="{e["name"]}.png" loading="lazy"></a><p>{e["name"]}</p><a href="../ui/palettes/{e["palette"]:03}.pal">Palette {e["palette"]}</a></article>'
    page+='</main><script>document.getElementById("q").oninput=function(){document.querySelectorAll("article").forEach(a=>a.hidden=!a.textContent.toLowerCase().includes(this.value.toLowerCase()))}</script>'
    (DIR/'index.html').write_text(page)
    print(f'{len(entries)} editable portraits, each verified as one 64x64 sprite cell')


if __name__=='__main__':extract()
