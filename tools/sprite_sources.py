#!/usr/bin/env python3
"""Compile NCD containers from readable tables, cell PNGs and palette files.

Extraction alone reads the ROM. Compilation starts from an empty bytearray;
there is no original NCD, tile-sheet or binary-template input. Cell images
cover the tile pool exactly once. Frame/portrait/icon views can override their
visible pixels; unchanged views never erase edits to the cell sources.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import gfx
import ncd
import nfp

ROOT=ncd.ROOT
CATEGORIES={'CHR':'battle/characters','EFFECT':'battle/effects','SYSTEM':'ui'}
MANIFEST=ROOT/'graphics/sprite_containers.json'


def folder(stem):return ROOT/'graphics'/CATEGORIES[stem]
def read(path):return json.loads(path.read_text())
def write(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n')
def pixel_hash(px):return hashlib.sha256(b''.join(bytes(row) for row in px)).hexdigest()
def align(n):return (n+15)&~15


def extract():
    rom=(ROOT/'baserom.gba').read_bytes();manifest=[]
    for e in nfp.entries(rom):
        if e['name'] not in [s+'.NCD' for s in CATEGORIES]:continue
        stem=e['name'][:-4];base=folder(stem);source=base/'source';source.mkdir(parents=True,exist_ok=True)
        (base/'palettes').mkdir(exist_ok=True)
        blob=rom[e['rom_offset']:e['end']];info=ncd.layout(blob);off=info['offsets']
        groups=[];animations=[];frames=[];cells=[];owner={}
        for k in range(info['groups']):
            raw=blob[off[0]+k*16:off[0]+k*16+8];name=raw.rstrip(b'\0').decode('ascii')
            assert raw==name.encode().ljust(8,b'\0')
            start,count=struct.unpack_from('<II',blob,off[0]+k*16+8)
            groups.append(dict(name=name,first_animation=start,animation_count=count))
        for k in range(info['animations']):
            start,count=struct.unpack_from('<II',blob,off[1]+k*8)
            animations.append(dict(first_frame=start,frame_count=count))
        for k in range(info['frames']):
            start,count,duration=struct.unpack_from('<IHH',blob,off[2]+k*8)
            frames.append(dict(first_cell=start,cell_count=count,duration=duration))
        for g in groups:
            for a in animations[g['first_animation']:g['first_animation']+g['animation_count']]:
                for f in frames[a['first_frame']:a['first_frame']+a['frame_count']]:
                    for c in range(f['first_cell'],f['first_cell']+f['cell_count']):owner.setdefault(c,g['name'])
        for k in range(info['cells']):
            c=ncd.cell(blob,info,k);a0,a1,pal,tile,u0,u1=struct.unpack_from('<HHIIII',blob,off[3]+k*20)
            cells.append(dict(x=c['x'],y=c['y'],width=c['width'],height=c['height'],hflip=c['hflip'],vflip=c['vflip'],
                              attr0_flags=f'0x{a0 & 0x3F00:04X}',attr1_flags=f'0x{a1 & 0x0E00:04X}',palette=pal,tile=tile,
                              unknown_0c=f'0x{u0:08X}',unknown_10=f'0x{u1:08X}'))
        for bank in range(info['palettes']):
            path=base/'palettes'/f'{bank:03}.pal'
            if not path.exists():gfx.write_jasc(str(path),gfx.read_palette(blob,off[4]+bank*32,16))
        # Greedy coverage selects complete, nonoverlapping sprite cells. All
        # three original containers have exact coverage with no orphan tiles.
        candidates=sorted((ncd.cell(blob,info,k) for k in range(info['cells'])),key=lambda c:c['start'])
        cursor=off[5];j=0;parts=[]
        while cursor<len(blob):
            options=[]
            while j<len(candidates) and candidates[j]['start']<=cursor:options.append(candidates[j]);j+=1
            c=max(options,key=lambda c:c['end'])
            if c['start']!=cursor or c['end']<=cursor:raise ValueError('Tile pool needs an explicit uncovered/overlapping-cell source')
            name=owner.get(c['index'],'unassigned')
            path=Path('cells')/name/f'tile_{c["tile"]:05d}.png';dest=base/path;dest.parent.mkdir(parents=True,exist_ok=True)
            px=gfx.tiles_to_pixels(blob[c['start']:c['end']],4,c['width']//8)
            if not dest.exists():gfx.write_png(str(dest),px,gfx.read_palette(blob,off[4]+c['palette']*32,16),transparent_index=0)
            parts.append(dict(path=str(path),tile=c['tile'],width=c['width'],height=c['height'],palette=c['palette'],baseline_pixels=pixel_hash(px)))
            cursor=c['end']
        header=dict(signature='NCD (C)2004 NOBODY',word_1c=f'0x{struct.unpack_from("<I",blob,0x1c)[0]:08X}',
                    reserved_words={f'0x{p:02X}':f'0x{struct.unpack_from("<I",blob,p)[0]:08X}' for p in [0x38,0x3c]+list(range(0x60,0x80,4))},
                    palette_count=info['palettes'],tile_count=(len(blob)-off[5])//32,rom_size=len(blob))
        for name,value in [('container',header),('groups',groups),('animations',animations),('frames',frames),('cells',cells),('images',parts)]:write(source/(name+'.json'),value)
        # Hashes select edited views; they are never used to recover pixels.
        import scenes
        view=read(base/'manifest.json') if (base/'manifest.json').exists() else None
        if view is None:
            (base/'frames').mkdir(exist_ok=True);records=[];canonical={}
            for index in range(info['frames']):
                px,_,pal,meta=scenes.render(blob,info,index)
                key=scenes.cell_key(blob,info,index);path=canonical.setdefault(key,f'frames/{index:04}.png')
                if not (base/path).exists():gfx.write_png(str(base/path),px,pal,transparent_index=0)
                records.append(dict(meta,id=index,path=path))
            named=[]
            for index,g in enumerate(groups):
                named.append(dict(name=g['name'],index=index,animations=[dict(index=j,frames=list(range(a['first_frame'],a['first_frame']+a['frame_count']))) for j,a in enumerate(animations[g['first_animation']:g['first_animation']+g['animation_count']])]))
            view=dict(container=stem,category=CATEGORIES[stem],frames=records,groups=named)
        for f in view['frames']:
            px,_,_,_=scenes.render(blob,info,f['id']);f['baseline_pixels']=pixel_hash(px)
        scenes.save_gallery(base,view)
        manifest.append(dict(name=e['name'],stem=stem,category=CATEGORIES[stem],rom_offset=e['rom_offset'],size=len(blob)))
        print(stem,len(parts),'individual cell sources',flush=True)
    write(MANIFEST,manifest)
    for name in ('icons','portraits'):
        path=ROOT/'graphics'/name/'manifest.json'
        if not path.exists():
            import importlib
            importlib.import_module(name).extract()
        records=read(path)
        for e in records:
            px,_=gfx.read_png(str(ROOT/e['path']));e['baseline_pixels']=pixel_hash(px)
        write(path,records)


def compile(stem,views=True):
    base=folder(stem);source=base/'source';header=read(source/'container.json')
    groups=read(source/'groups.json');animations=read(source/'animations.json');frames=read(source/'frames.json');cells=read(source/'cells.json')
    counts=[len(groups),len(animations),len(frames),len(cells)]
    offsets=[128]
    for count,size in zip(counts,(16,8,8,20)):offsets.append(align(offsets[-1]+count*size))
    offsets.append(offsets[-1]+header['palette_count']*32)
    size=offsets[5]+header['tile_count']*32
    if size!=header['rom_size']:raise ValueError('NCD size changed; update archive layout before resizing')
    blob=bytearray(size)
    signature=header['signature'].encode('ascii')
    if len(signature)>28:raise ValueError('NCD signature too long')
    blob[:28]=signature.ljust(28,b'\0');struct.pack_into('<I',blob,0x1c,int(header['word_1c'],0))
    struct.pack_into('<6I',blob,0x20,*offsets)
    for p,value in header['reserved_words'].items():
        pos=int(p,0)
        if pos not in [0x38,0x3c]+list(range(0x60,0x80,4)):raise ValueError('Reserved field overlaps a decoded field')
        struct.pack_into('<I',blob,pos,int(value,0))
    struct.pack_into('<8I',blob,0x40,*counts,len(cells),header['palette_count'],sum(c['width']*c['height']//64 for c in cells),header['tile_count'])
    for k,g in enumerate(groups):
        name=g['name'].encode('ascii')
        if len(name)>8 or g['first_animation']+g['animation_count']>len(animations):raise ValueError('Invalid group')
        struct.pack_into('<8sII',blob,offsets[0]+k*16,name,g['first_animation'],g['animation_count'])
    for k,a in enumerate(animations):
        if a['first_frame']+a['frame_count']>len(frames):raise ValueError('Invalid animation')
        struct.pack_into('<II',blob,offsets[1]+k*8,a['first_frame'],a['frame_count'])
    for k,f in enumerate(frames):
        if f['first_cell']+f['cell_count']>len(cells):raise ValueError('Invalid frame')
        struct.pack_into('<IHH',blob,offsets[2]+k*8,f['first_cell'],f['cell_count'],f['duration'])
    for k,c in enumerate(cells):
        shapes=[(shape,s) for shape,row in enumerate(ncd.DIMS) for s,(w,h) in enumerate(row) if (w,h)==(c['width'],c['height'])]
        if not shapes or not -256<=c['x']<=255 or not -128<=c['y']<=127:raise ValueError('Invalid cell geometry')
        shape,s=shapes[0];flags0=int(c['attr0_flags'],0);flags1=int(c['attr1_flags'],0)
        if flags0&~0x1F00 or flags1&~0x0E00:raise ValueError('Invalid cell flags or unsupported 8bpp')
        if not 0<=c['palette']<header['palette_count'] or c['tile']<0 or c['tile']+c['width']*c['height']//64>header['tile_count']:raise ValueError('Invalid cell tile/palette')
        a0=(c['y']&255)|(shape<<14)|flags0;a1=(c['x']&511)|(s<<14)|flags1|(int(c['hflip'])<<12)|(int(c['vflip'])<<13)
        struct.pack_into('<HHIIII',blob,offsets[3]+k*20,a0,a1,c['palette'],c['tile'],int(c['unknown_0c'],0),int(c['unknown_10'],0))
    for bank in range(header['palette_count']):
        colors=gfx.read_jasc(str(base/'palettes'/f'{bank:03}.pal'))
        if len(colors)!=16 or any(not 0<=v<=255 for rgb in colors for v in rgb):raise ValueError('Invalid palette')
        blob[offsets[4]+bank*32:offsets[4]+(bank+1)*32]=gfx.palette_to_bytes(colors)
    cursor=0;changed=set()
    for e in read(source/'images.json'):
        if e['tile']!=cursor:raise ValueError('Cell image coverage has a gap or overlap')
        px,_=gfx.read_png(str(base/e['path']))
        if len(px)!=e['height'] or any(len(row)!=e['width'] or any(v>15 for v in row) for row in px):raise ValueError('Invalid cell image: '+e['path'])
        data=gfx.pixels_to_tiles(px,4);start=offsets[5]+cursor*32
        if start+len(data)>len(blob):raise ValueError('Cell image exceeds tile pool')
        blob[start:start+len(data)]=data
        if pixel_hash(px)!=e['baseline_pixels']:changed.update(range(start,start+len(data)))
        cursor+=len(data)//32
    if cursor!=header['tile_count']:raise ValueError('Cell images do not cover complete tile pool')
    if views:
        import icons,scenes
        baseline=bytes(blob)
        if stem=='SYSTEM':
            for name in ('icons','portraits'):blob=icons.apply(blob,baseline,ROOT/'graphics'/name/'manifest.json',changed)
        blob=scenes.merge(blob,baseline,stem,changed)
    return bytes(blob)


def build(stem=None):
    for stem in ([stem] if stem else CATEGORIES):
        blob=compile(stem);dest=ROOT/'build/graphics/ncd'/(stem+'.ncd');dest.parent.mkdir(parents=True,exist_ok=True)
        dest.write_bytes(blob)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('command',choices=['extract','build']);p.add_argument('--stem',choices=list(CATEGORIES));a=p.parse_args();extract() if a.command=='extract' else build(a.stem)
