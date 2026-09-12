#!/usr/bin/env python3
"""Compile English credits from frame PNGs and explicit OAM layouts.

The Japanese compiler never calls this module. English credits reuse the
original frame/cell records and their isolated tile-pool span, but no longer
share Japanese surname tiles or leave unpaintable gaps inside Latin names.
No ROM reads, binary templates or byte-matching substitutions are used.
"""
import json
from pathlib import Path
import struct
import gfx
import ncd

ROOT=Path(__file__).resolve().parents[1]
LAYOUT='graphics/ui/source/english/credits_layouts.json'


def load(root=None):
    path=(root or ROOT)/LAYOUT
    return json.loads(path.read_text()) if path.exists() else {'frames':[]}


def frame_ids(root=None):
    return {f['frame'] for f in load(root)['frames']}


def metadata_offsets(info,layout=None):
    """Only position/shape and tile-index bytes may differ in credit records."""
    result=set(range(0x58,0x5c)) # Derived sum of cell tile references.
    for frame in (layout or load())['frames']:
        for cell in frame['cells']:
            start=info['offsets'][3]+cell['id']*20
            result.update(range(start,start+4))
            result.update(range(start+8,start+12))
    return result


def apply(blob,root=None):
    root=root or ROOT;layout=load(root)
    if not layout['frames']:return blob
    info=ncd.layout(blob);low,high=layout['tile_span']
    if not 0<=low<high<=(len(blob)-info['offsets'][5])//32:raise ValueError('Invalid English credit tile span')
    targets={f['frame'] for f in layout['frames']}
    if len(targets)!=len(layout['frames']):raise ValueError('Duplicate English credit frame')
    if targets!=set(range(1700,1782)):raise ValueError('English credit layout must cover exactly the staff frames')
    owned=set();source_tiles=set()
    for f in range(info['frames']):
        first,count,_=struct.unpack_from('<IHH',blob,info['offsets'][2]+f*8)
        for k in range(first,first+count):
            c=ncd.cell(blob,info,k)
            begin=c['tile'];end=begin+c['width']*c['height']//64
            if f not in targets:
                if begin<high and end>low:raise ValueError('Credit pool is shared with another frame')
            else:
                if begin<low or end>high:raise ValueError('Credit cell lies outside reserved pool')
                owned.add(k);source_tiles.update(range(begin,end))
    if source_tiles!=set(range(low,high)):raise ValueError('Credit pool does not match source cell coverage')
    written=set();cursor=low;expected_frames=[]
    for frame in layout['frames']:
        first,count,_=struct.unpack_from('<IHH',blob,info['offsets'][2]+frame['frame']*8)
        if [c['id'] for c in frame['cells']]!=list(range(first,first+count)):
            raise ValueError('English credit changes cell count or ownership')
        path=root/'graphics/ui'/frame['path'];px,pal=gfx.read_png(str(path))
        original=root/'graphics/ui'/frame['original'];_,expected=gfx.read_png(str(original))
        if pal!=expected or gfx.png_alpha(path)!=gfx.png_alpha(original):raise ValueError('Credit palette/transparency changed')
        width=frame['width'];height=frame['height']
        if len(px)!=height or any(len(row)!=width or any(v>15 for v in row) for row in px):
            raise ValueError('Invalid English credit PNG dimensions or indices')
        coverage=set()
        for cell in frame['cells']:
            if cell['id'] in written:raise ValueError('English credit reuses a cell record')
            written.add(cell['id'])
            w=cell['width'];h=cell['height'];x=cell['x'];y=cell['y']
            shapes=[(shape,size) for shape,row in enumerate(ncd.DIMS) for size,dim in enumerate(row) if dim==(w,h)]
            if not shapes or not -256<=x<=255 or not -128<=y<=127:raise ValueError('Invalid English OAM shape or position')
            left=x-w//2-frame['x'];top=y-h//2-frame['y']
            if not 0<=left<=width-w or not 0<=top<=height-h:raise ValueError('Credit cell exceeds PNG bounds')
            area={(a,b) for b in range(top,top+h) for a in range(left,left+w)}
            if coverage&area:raise ValueError('Overlapping English credit cells')
            coverage.update(area)
            tile=cell['tile'];count_tiles=w*h//64
            if tile!=cursor or tile+count_tiles>high:raise ValueError('English credit tile allocation exceeds pool or has gaps')
            cursor+=count_tiles
            pixels=[row[left:left+w] for row in px[top:top+h]]
            data=gfx.pixels_to_tiles(pixels,4);start=info['offsets'][5]+tile*32
            blob[start:start+len(data)]=data
            pos=info['offsets'][3]+cell['id']*20
            shape,size=shapes[0]
            # Palette, frame duration and both unknown cell words stay original.
            struct.pack_into('<HH',blob,pos,(y&255)|(shape<<14),(x&511)|(size<<14))
            struct.pack_into('<I',blob,pos+8,tile)
        if coverage!={(x,y) for y in range(height) for x in range(width)}:raise ValueError('English credit layout has holes')
        expected_frames.append((frame,px))
    if written!=owned:raise ValueError('English credit cell ownership incomplete')
    references=sum(c['width']*c['height']//64 for c in (ncd.cell(blob,info,k) for k in range(info['cells'])))
    struct.pack_into('<I',blob,0x58,references)
    import scenes
    for frame,pixels in expected_frames:
        actual,_,_,meta=scenes.render(blob,info,frame['frame'])
        if actual!=pixels or any(meta[key]!=frame[key] for key in ('x','y','width','height')):
            raise ValueError('English credit OAM render differs from source PNG')
    return blob
