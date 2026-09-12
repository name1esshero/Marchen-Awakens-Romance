#!/usr/bin/env python3
"""Legible, editable NCD frames and local animation browsers.

Reconstruct visible pixels with explicit cell/palette ownership. Edits map
back only to existing sprite cells. Shared-byte conflicts and paint outside
cell coverage are rejected. Palette changes use each category’s palette files.
"""
import asset_safety
import argparse
import bisect
import hashlib
import json
from pathlib import Path
import struct

import gfx
import ncd

ROOT=ncd.ROOT
CATEGORIES={'CHR':'battle/characters','EFFECT':'battle/effects','SYSTEM':'ui'}


def render(blob, info, frame):
    start,count,duration=struct.unpack_from('<IHH',blob,info['offsets'][2]+frame*8)
    cells=[ncd.cell(blob,info,start+i) for i in range(count)]
    if not cells:raise ValueError('Empty frame')
    banks=sorted({c['palette'] for c in cells})
    if len(banks)*16>256:raise ValueError('Frame exceeds indexed PNG palette capacity')
    x0=min(c['left'] for c in cells);y0=min(c['top'] for c in cells)
    w=max(c['left']+c['width'] for c in cells)-x0;h=max(c['top']+c['height'] for c in cells)-y0
    if w*h>1024*1024:raise ValueError('Unexpected frame extent')
    pixels=[[0]*w for _ in range(h)];owners=[[None]*w for _ in range(h)]
    colors=[]
    for bank in banks:colors+=gfx.read_palette(blob,info['offsets'][4]+bank*32,16)
    for c in reversed(cells):
        bank=banks.index(c['palette'])
        source=gfx.tiles_to_pixels(blob[c['start']:c['end']],4,c['width']//8)
        for y in range(c['height']):
            sy=c['height']-1-y if c['vflip'] else y
            for x in range(c['width']):
                sx=c['width']-1-x if c['hflip'] else x
                py=c['top']-y0+y;px=c['left']-x0+x;value=source[sy][sx]
                tile=(sy//8)*(c['width']//8)+(sx//8)
                address=c['start']+tile*32+(sy%8)*4+(sx%8)//2
                owner=(address,(sx&1)*4,bank)
                if value:
                    pixels[py][px]=bank*16+value;owners[py][px]=owner
                elif owners[py][px] is None:
                    owners[py][px]=owner
    return pixels,owners,colors,dict(x=x0,y=y0,width=w,height=h,duration=duration,banks=banks)


def cell_key(blob, info, frame):
    """Exact cell records, including tile addresses, palettes, flags and order.

    Equal-looking pixels at different ROM addresses are independent assets.
    Duration is per frame and deliberately excluded from the shared PNG key.
    """
    start,count,_=struct.unpack_from('<IHH',blob,info['offsets'][2]+frame*8)
    return bytes(blob[info['offsets'][3]+start*20:info['offsets'][3]+(start+count)*20])


def save_gallery(folder, data):
    (folder/'manifest.json').write_text(json.dumps(data,separators=(',',':'))+'\n')
    page=TEMPLATE.replace('GRAPHICS_INDEX', '../'*len(data['category'].split('/'))+'index.html').replace('DATA_JSON',json.dumps(data,separators=(',',':')).replace('</','<\\/'))
    (folder/'index.html').write_text(page)


def consolidate(data, folder, blob, info):
    """Update aliases, returning duplicate files safe to remove after saving.

    Both original cell identity and current file bytes must match. Differently
    edited copies are retained so normal rebuild conflict checks still apply.
    """
    canonical={};removed={}
    for frame in data['frames']:
        path=folder/frame['path']
        if asset_safety.has_english_art(path):
            continue  # Keep this path: its sibling is an English build input.
        raw=path.read_bytes()
        key=(cell_key(blob,info,frame['id']),raw)
        if key not in canonical:
            canonical[key]=(frame['id'],frame['path']);continue
        first,target=canonical[key]
        if target==frame['path']:continue
        removed[frame['path']]=dict(path=str(path.relative_to(ROOT)),replacement=str((folder/target).relative_to(ROOT)),bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest())
        frame['path']=target
        frame['source_frame']=first
    referenced={frame['path'] for frame in data['frames']}
    return [record for old,record in removed.items() if old not in referenced]


def tidy():
    report=[]
    for stem,category in CATEGORIES.items():
        folder=ROOT/'graphics'/category
        data=json.loads((folder/'manifest.json').read_text())
        blob=ncd.source(stem)
        removed=consolidate(data,folder,blob,ncd.layout(blob))
        # Publish every alias before removing its now-unreferenced file.
        save_gallery(folder,data)
        for record in removed:
            path=ROOT/record['path']
            if hashlib.sha256(path.read_bytes()).hexdigest()!=record['sha256']:
                raise ValueError('Source changed during consolidation: '+str(path))
            asset_safety.unlink_generated(path)
        report.extend(removed)
    dest=ROOT/'reports/graphics/consolidated.json'
    previous=json.loads(dest.read_text()) if dest.exists() else []
    dest.write_text(json.dumps(previous+report,indent=2)+'\n')
    print(json.dumps(dict(removed=len(report),bytes_saved=sum(r['bytes'] for r in report))))


def extract():
    totals={}
    for stem,category in CATEGORIES.items():
        folder=ROOT/'graphics'/category;(folder/'frames').mkdir(parents=True,exist_ok=True)
        blob=ncd.source(stem);info=ncd.layout(blob)
        previous=json.loads((folder/'manifest.json').read_text()) if (folder/'manifest.json').exists() else {'frames':[]}
        previous_records={f['id']:f for f in previous['frames']}
        previous_paths={f['id']:f['path'] for f in previous['frames']}
        from sprite_sources import pixel_hash
        parts=json.loads((folder/'source/images.json').read_text())
        starts=[p['tile'] for p in parts]
        canonical={}
        frames=[]
        for f in range(info['frames']):
            px,_,pal,meta=render(blob,info,f)
            key=cell_key(blob,info,f)
            path=previous_paths.get(f,canonical.get(key,'frames/%04d.png'%f))
            canonical.setdefault(key,path)
            source=folder/path
            # Refresh unchanged views after cell/palette edits; preserve authored edits.
            refresh=not source.exists()
            if source.exists():
                current,colors=gfx.read_png(str(source))
                old_hash=previous_records.get(f,{}).get('baseline_pixels')
                refresh=old_hash==pixel_hash(current) and (current!=px or colors[:len(pal)]!=pal)
            if refresh:gfx.write_png(str(source),px,pal,transparent_index=0)
            first,count,_=struct.unpack_from('<IHH',blob,info['offsets'][2]+f*8)
            images=set()
            for c in (ncd.cell(blob,info,first+i) for i in range(count)):
                i=max(0,bisect.bisect_right(starts,c['tile'])-1)
                end=c['tile']+c['width']*c['height']//64
                while i<len(parts) and starts[i]<end:
                    images.add(parts[i]['path']);i+=1
            frames.append(dict(meta,id=f,path=path,baseline_pixels=pixel_hash(px),cell_images=sorted(images)))
        groups=[]
        for i in range(info['groups']):
            pos=info['offsets'][0]+i*16;name=blob[pos:pos+8].split(b'\0')[0].decode('ascii')
            first,count=struct.unpack_from('<II',blob,pos+8);animations=[]
            for a in range(count):
                start,n=struct.unpack_from('<II',blob,info['offsets'][1]+(first+a)*8)
                if start+n>len(frames):raise ValueError('Animation exceeds frame table')
                animations.append(dict(index=a,frames=list(range(start,start+n))))
            groups.append(dict(name=name,index=i,animations=animations))
        data=dict(container=stem,category=category,frames=frames,groups=groups)
        save_gallery(folder,data)
        totals[stem]=dict(groups=len(groups),frames=len(frames),animations=sum(len(g['animations']) for g in groups))
    print(json.dumps(totals))


def merge(blob, original, stem, blocked_addresses=None, english=False):
    path=ROOT/'graphics'/CATEGORIES[stem]/'manifest.json'
    info=ncd.layout(original);data=json.loads(path.read_text());writes={};changed=[];seen={}
    import english_credits
    credit_frames=english_credits.frame_ids(ROOT) if english and stem=='SYSTEM' else set()
    for f in data['frames']:
        if f['id'] in credit_frames:continue # Compiled separately with explicit English OAM layouts.
        key=cell_key(original,info,f['id'])
        if f['path'] in seen:
            if seen[f['path']]!=key:raise ValueError('PNG shared by independent sprite cells: '+f['path'])
            continue
        seen[f['path']]=key
        image_path=path.parent/f['path']
        if english:
            variant=image_path.with_name(image_path.stem+'_en.png')
            if variant.exists():image_path=variant
        current,colors=gfx.read_png(str(image_path))
        if image_path.stem.endswith('_en'):
            _,expected_colors=gfx.read_png(str(path.parent/f['path']))
            if colors!=expected_colors:raise ValueError('English frame must preserve indexed palette: '+str(image_path))
            if gfx.png_alpha(image_path)!=gfx.png_alpha(path.parent/f['path']):raise ValueError('English PNG must preserve transparency indices: '+str(image_path))
        if f.get('baseline_pixels'):
            if len(current)!=f['height'] or any(len(row)!=f['width'] for row in current):raise ValueError('Frame dimensions changed: '+f['path'])
            from sprite_sources import pixel_hash
            if pixel_hash(current)==f['baseline_pixels']:continue
        old,owners,_,meta=render(original,info,f['id'])
        if len(current)!=meta['height'] or any(len(row)!=meta['width'] for row in current):raise ValueError('Frame dimensions changed: '+f['path'])
        if current==old:continue
        for y,row in enumerate(current):
            for x,value in enumerate(row):
                if value==old[y][x]:continue
                owner=owners[y][x]
                if owner is None:raise ValueError('Cannot paint outside original cell bounds: '+f['path'])
                address,shift,bank=owner
                if blocked_addresses and address in blocked_addresses:raise ValueError('Edit either the cell image or its assembled frame: '+f['path'])
                if value and (value//16!=bank or value%16==0):raise ValueError('Use the original cell palette bank: '+f['path'])
                nibble=value%16
                previous=(blob[address]>>shift)&15;baseline=(original[address]>>shift)&15
                key=(address,shift)
                if (key in writes and writes[key]!=nibble) or previous not in (baseline,nibble):raise ValueError('Conflicting scene/tile/portrait edits: '+f['path'])
                writes[key]=nibble
        changed.append((f,current))
    for (address,shift),value in writes.items():blob[address]=(blob[address]&~(15<<shift))|(value<<shift)
    for f,expected in changed:
        actual,_,_,_=render(blob,info,f['id'])
        if actual!=expected:raise ValueError('Edit changes occluded/shared pixels; edit the individual cell image instead: '+f['path'])
    return blob


TEMPLATE='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MAR sprite animation viewer</title>
<style>*{box-sizing:border-box}body{margin:0;background:#151b25;color:#e9eff8;font:16px system-ui}header{padding:20px 28px;background:#202a38}h1{margin:0 0 10px}a{color:#95c6ff}main{display:grid;grid-template-columns:280px 1fr;gap:24px;padding:24px}nav{max-height:80vh;overflow:auto}button,input,select{font:inherit;padding:7px;border:1px solid #53657f;border-radius:5px;background:#233248;color:inherit}nav button{display:block;width:100%;text-align:left;margin:4px 0}button{cursor:pointer}#stage{position:relative;margin:24px 0;overflow:hidden;background:conic-gradient(#242e3c 25%,#303c4c 0 50%,#242e3c 0 75%,#303c4c 0) 0 0/16px 16px;image-rendering:pixelated}#stage img{position:absolute;image-rendering:pixelated}#strip{display:flex;overflow:auto;gap:6px;max-width:100%;padding:8px;background:#202a38}#strip img{image-rendering:pixelated;max-height:96px;max-width:128px;object-fit:contain}#strip button.active{outline:2px solid #b2d7ff}.toolbar{display:flex;flex-wrap:wrap;gap:12px;align-items:center}#details{max-width:900px;overflow-wrap:anywhere}section{min-width:0}@media(max-width:700px){main{grid-template-columns:1fr}nav{max-height:200px}}[hidden]{display:none!important}</style>
<header><h1 id="title"></h1><p>Original resource names, complete frame sequences, and original palette colors. Pixel PNGs below are editable build sources.</p><a href="GRAPHICS_INDEX">Graphics index</a></header>
<main><aside><input id="search" placeholder="Filter resource name"><p id="total"></p><nav id="groups"></nav></aside><section><h2 id="name"></h2><div class="toolbar"><label>Artwork <select id="language"><option value="jp">Japanese</option><option value="en">English overrides</option></select></label><label>Animation <select id="animation"></select></label><button id="play">Play</button><label>Preview FPS <input id="fps" type="number" min="1" max="60" value="8" size="3" style="width:70px"></label><label>Zoom <select id="zoom"><option>1</option><option selected>2</option><option>3</option><option>4</option></select></label></div><p id="details"></p><div id="stage"><img id="sprite" alt="Selected sprite frame"></div><div id="strip"></div><p><a id="source">Open editable frame PNG</a></p><p id="palettes"></p><details><summary>Editable cell images and animation tables</summary><p><a href="source/groups.json">Named groups</a> · <a href="source/animations.json">Animations</a> · <a href="source/frames.json">Frame durations and layouts</a> · <a href="source/cells.json">Cell positions and flags</a> · <a href="source/container.json">Container metadata</a></p><div id="cells"></div></details><p>Playback FPS is a viewing control, not a claim about game timing. Stored duration values are shown per frame. Affine transformations and blending are not reproduced. Edit linked palette files for palette changes; preserve PNG dimensions and palette indices.</p></section></main>
<script>const data=DATA_JSON;const $=id=>document.getElementById(id);let group,animation,index=0,timer=null;let base='';const depth=data.category.split('/').length;base='palettes/';$('title').textContent=data.container+' — '+(data.container==='SYSTEM'?'UI, item art, and panels':'character and battle animations');$('total').textContent=data.groups.length+' resources / '+data.frames.length+' frames';
function displayFrame(i){const f=data.frames[i];return $('language').value==='en'&&f.english?{...f,...f.english}:f}function stop(){clearInterval(timer);timer=null;$('play').textContent='Play'}function show(){const ids=animation.frames;if(!ids.length)return;const frame=displayFrame(ids[index]),all=ids.map(displayFrame),scale=Number($('zoom').value);const x=Math.min(...all.map(f=>f.x)),y=Math.min(...all.map(f=>f.y));const w=Math.max(...all.map(f=>f.x+f.width))-x,h=Math.max(...all.map(f=>f.y+f.height))-y;$('stage').style.width=w*scale+'px';$('stage').style.height=h*scale+'px';$('sprite').src=frame.path;$('sprite').style.cssText='left:'+(frame.x-x)*scale+'px;top:'+(frame.y-y)*scale+'px;width:'+frame.width*scale+'px;height:'+frame.height*scale+'px';$('details').textContent='Frame '+(index+1)+' / '+ids.length+' · table index '+frame.id+' · '+frame.width+'×'+frame.height+' · stored duration '+frame.duration;$('source').href=frame.path;$('cells').replaceChildren();for(const path of frame.cell_images||[]){const a=document.createElement('a'),im=document.createElement('img');a.href=path;im.src=path;im.alt=path;im.title=path;im.style.cssText='image-rendering:pixelated;margin:8px;max-width:128px;max-height:128px';a.append(im);$('cells').append(a)}$('palettes').replaceChildren('Palette banks: ');for(const bank of frame.banks){const a=document.createElement('a');a.href=base+String(bank).padStart(3,'0')+'.pal';a.textContent=bank+' ';$('palettes').append(a)}[...$('strip').children].forEach((b,i)=>b.classList.toggle('active',i===index))}
function selectAnimation(){stop();animation=group.animations[Number($('animation').value)];index=0;$('strip').replaceChildren();animation.frames.forEach((f,i)=>{const b=document.createElement('button'),im=document.createElement('img');im.src=displayFrame(f).path;im.loading='lazy';im.alt='Frame '+i;b.title='Frame '+i;b.append(im);b.onclick=()=>{stop();index=i;show()};$('strip').append(b)});show()}
function selectGroup(g){group=g;$('name').textContent=g.name;$('animation').replaceChildren();g.animations.forEach((a,i)=>{const o=document.createElement('option');o.value=i;o.textContent=i+' ('+a.frames.length+' frames)';$('animation').append(o)});selectAnimation()}
for(const g of data.groups){const b=document.createElement('button');b.textContent=g.name;b.onclick=()=>selectGroup(g);$('groups').append(b)}$('search').oninput=()=>{[...$('groups').children].forEach(b=>b.hidden=!b.textContent.toLowerCase().includes($('search').value.toLowerCase()))};$('language').onchange=selectAnimation;$('animation').onchange=selectAnimation;$('zoom').onchange=show;$('play').onclick=()=>{if(timer){stop();return}$('play').textContent='Pause';timer=setInterval(()=>{index=(index+1)%animation.frames.length;show()},1000/Math.max(1,Math.min(60,Number($('fps').value)||8)))};selectGroup(data.groups[0]);</script></html>'''


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('command',choices=['extract','tidy']);a=p.parse_args();extract() if a.command=='extract' else tidy()
