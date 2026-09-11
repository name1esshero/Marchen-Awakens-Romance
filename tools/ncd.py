#!/usr/bin/env python3
"""NCD sprite containers, recovered from 0807B96C, 0807BBxx, 0807BCAC.

Tables at +20/+24/+28/+2C have 16/8/8/20-byte records. +30 points
at 16-color palette banks; +34 points at uncompressed 4bpp tile data.
A cell's +8 tile index is multiplied by 32 by the ROM's graphics getter.
"""
import argparse
import html
import json
from pathlib import Path
import re
import struct
import zlib
import gfx
import nfp

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'reports/sprites'
DIMS = (((8,8),(16,16),(32,32),(64,64)),
        ((16,8),(32,8),(32,16),(64,32)),
        ((8,16),(8,32),(16,32),(32,64)))


def layout(blob):
    if not blob.startswith(b'NCD (C)2004 NOBODY'):
        raise ValueError('Not an NCD container')
    offsets = struct.unpack_from('<6I', blob, 0x20)
    counts = struct.unpack_from('<6I', blob, 0x40)
    for i, size in enumerate((16,8,8,20)):
        if offsets[i] + counts[i] * size > offsets[i + 1]:
            raise ValueError('NCD table exceeds its bounds')
    palettes = counts[5]
    if offsets[4] + palettes * 32 != offsets[5]:
        raise ValueError('NCD palette count does not reach graphics pool')
    if not offsets[5] <= len(blob) or (len(blob) - offsets[5]) % 32:
        raise ValueError('NCD tile pool is not complete 4bpp tiles')
    return dict(offsets=offsets, groups=counts[0], animations=counts[1],
                frames=counts[2], cells=counts[3], palettes=palettes)


def cell(blob, info, index):
    if not 0 <= index < info['cells']:
        raise ValueError('Cell index out of bounds')
    pos = info['offsets'][3] + index * 20
    a0,a1,pal,tile = struct.unpack_from('<HHII', blob, pos)
    shape = a0 >> 14
    if shape == 3 or a0 & 0x2000 or pal >= info['palettes']:
        raise ValueError('Unsupported NCD cell geometry/bit depth/palette')
    w,h = DIMS[shape][a1 >> 14]
    start = info['offsets'][5] + tile * 32
    if start + w*h//2 > len(blob):
        raise ValueError('NCD cell exceeds tile pool')
    return dict(index=index, x=((a1 & 511)+256)%512-256,
                y=((a0 & 255)+128)%256-128, width=w, height=h,
                palette=pal, tile=tile, start=start, end=start+w*h//2,
                hflip=bool(a1 & 0x1000), vflip=bool(a1 & 0x2000))


def write_rgb(path, rows):
    h,w = len(rows),len(rows[0])
    raw = b''.join(b'\0'+bytes(c for p in row for c in p) for row in rows)
    path.write_bytes(b'\x89PNG\r\n\x1a\n'+gfx._chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,2,0,0,0))+
                     gfx._chunk(b'IDAT',zlib.compress(raw,9))+gfx._chunk(b'IEND',b''))


def frame_preview(blob, info, frame):
    if not 0 <= frame < info['frames']:
        raise ValueError('NCD frame index out of bounds')
    start,count,duration = struct.unpack_from('<IHH',blob,info['offsets'][2]+frame*8)
    cells = [cell(blob,info,start+i) for i in range(count)]
    if not cells:
        return None
    x0=min(c['x'] for c in cells);y0=min(c['y'] for c in cells)
    w=max(c['x']+c['width'] for c in cells)-x0
    h=max(c['y']+c['height'] for c in cells)-y0
    if w>1024 or h>1024:
        raise ValueError('Unexpected frame extent')
    rows=[[(40,44,52)]*w for _ in range(h)]
    for c in reversed(cells):
        px=gfx.tiles_to_pixels(blob[c['start']:c['end']],4,c['width']//8)
        pal=gfx.read_palette(blob,info['offsets'][4]+c['palette']*32,16)
        for y in range(c['height']):
            for x in range(c['width']):
                v=px[c['height']-1-y if c['vflip'] else y][c['width']-1-x if c['hflip'] else x]
                if v:
                    rows[c['y']-y0+y][c['x']-x0+x]=pal[v]
    return rows


def source(stem):
    """Reconstruct the container baseline from readable cell/table sources."""
    import sprite_sources
    return sprite_sources.compile(stem, views=False)


def extract():
    import sprite_sources
    sprite_sources.extract()


def build():
    import sprite_sources
    sprite_sources.build()


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('command',choices=['extract','build']);a=p.parse_args()
    extract() if a.command=='extract' else build()
