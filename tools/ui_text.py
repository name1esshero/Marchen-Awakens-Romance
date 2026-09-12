#!/usr/bin/env python3
"""Author small English UI labels with the extracted game's own Latin glyphs.

Explicit authoring only: ordinary builds consume the resulting editable PNGs.
Use whole glyph pixels and integer scales. Never change palettes or silently
overwrite authored variants.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import font
import gfx

ROOT=Path(__file__).resolve().parents[1]


def raster(text, atlas, mapping, tracking=1):
    if tracking not in (0,1):raise ValueError('Unsupported glyph spacing')
    columns=len(atlas[0])//8
    out=[[] for _ in range(8)]
    for char in text:
        if char==' ':
            for row in out:row.extend([0]*3)
            continue
        code=int(mapping[char],16);slot=font.glyph_index(code)
        if slot==0xffff:raise ValueError('Unsupported glyph: '+char)
        block=[r[slot%columns*8:slot%columns*8+8] for r in atlas[slot//columns*8:slot//columns*8+8]]
        if len(block)!=8:raise ValueError('Missing label glyph: '+char)
        xs=[x for row in block for x,v in enumerate(row) if v]
        if not xs:raise ValueError('Empty label glyph: '+char)
        for y in range(8):
            if out[y]:out[y].extend([0]*tracking)
            out[y].extend(block[y][min(xs):max(xs)+1])
    return out


def compose(original, glyphs, kind, recipe=None):
    if kind=='location':return compose_location(original,glyphs,recipe or {})
    if kind in ('button','prompt','banner'):return compose_prompt(original,glyphs,kind,recipe or {})
    px=[row[:] for row in original];height=len(px);width=len(px[0])
    if width!=48 or height!=(8 if kind=='selected_name' else 16):raise ValueError('Unexpected label geometry')
    ink=(recipe or {}).get('ink',14) if kind=='shop' else 9
    if kind=='selected_name':
        # The gradient is flat behind the text. Preserve the shaped left edge;
        # only remove original white lettering and its black shadow.
        for row in px:
            background=Counter(v for v in row[8:] if v not in (0,1,ink)).most_common(1)
            if not background:raise ValueError('No clear gradient sample')
            for x,v in enumerate(row):
                if v in (1,ink):row[x]=background[0][0]
        y0=0;x0=46-len(glyphs[0])
    else:
        background=3 if kind=='shop' else 1
        for row in px:
            for x,v in enumerate(row):
                if v==ink:row[x]=background
        y0=4;x0=0 if kind=='shop' else 46-len(glyphs[0])
    if x0<5 and kind!='shop' or x0<0 or x0+len(glyphs[0])>((recipe or {}).get('right',32) if kind=='shop' else 46):
        raise ValueError('Label exceeds its own tile area')
    if kind=='selected_name':
        # Keep the original one-pixel opaque black text shadow, clipped to the
        # eight-pixel tab. Never use transparent index zero as the shadow.
        for y,row in enumerate(glyphs):
            for x,v in enumerate(row):
                sx=x0+x+1;sy=y0+y+1
                if v and sy<height and sx<width and px[sy][sx]!=0:px[sy][sx]=1
    for y,row in enumerate(glyphs):
        for x,v in enumerate(row):
            if v:
                if px[y0+y][x0+x]==0:raise ValueError('Glyph would enter transparent margin')
                px[y0+y][x0+x]=ink
    return px



def compose_prompt(original,glyphs,kind,recipe):
    px=[row[:] for row in original];h=len(px);w=len(px[0]);scale=recipe.get('scale',1)
    mask=[[v for v in row for _ in range(scale)] for row in glyphs for _ in range(scale)]
    mw=len(mask[0]);mh=len(mask)
    if kind=='button':
        left=recipe.get('left',8);right=w;y0=0;x0=left
        if h!=8:raise ValueError('Unexpected button height')
        for row in px:row[left:]=[0]*(w-left)
    elif kind=='prompt':
        left=32;right=160;y0=3;x0=(w-mw)//2
        if (w,h)!=(208,16):raise ValueError('Unexpected prompt geometry')
        for y in range(3,12):px[y][left:right]=[10]*(right-left)
    else:
        left=0;right=w;y0=0;x0=0;px=[[0]*w for _ in range(h)]
    if x0<left or x0+mw>right or y0+mh>h:raise ValueError('English label exceeds drawable text area')
    if kind!='prompt':
        for y,row in enumerate(mask):
            for x,v in enumerate(row):
                if v and y0+y+1<h and x0+x+1<right:px[y0+y+1][x0+x+1]=10
    for y,row in enumerate(mask):
        for x,v in enumerate(row):
            if v:px[y0+y][x0+x]=15
    return px



def compose_location(original,glyphs,recipe):
    # Preserve complete glyph columns when a long place name shares a narrow
    # sprite with a separate floor marker. Integer vertical scale only.
    px=[r[:] for r in original];right=recipe['right'];h=len(px)
    mask=[row[:] for row in glyphs for _ in range(2)]
    mw=len(mask[0]);mh=len(mask);x0=(right-mw)//2;y0=(h-mh)//2
    if x0<1 or x0+mw>=right or y0<1 or y0+mh>=h:raise ValueError('Location title cannot fit with outline')
    for row in px:row[:right]=[0]*right
    for y,row in enumerate(mask):
        for x,v in enumerate(row):
            if v:
                for dy,dx in ((-1,0),(1,0),(0,-1),(0,1)):
                    px[y0+y+dy][x0+x+dx]=15
    for y,row in enumerate(mask):
        for x,v in enumerate(row):
            if v:px[y0+y][x0+x]=6 if y<4 else (7 if y<10 else 8)
    return px

def compose_message(original,recipe,atlas,mapping):
    px=[r[:] for r in original];left,top,right,bottom=recipe['text_rect']
    if not (0<=left<right<=len(px[0]) and 0<=top<bottom<=len(px)):raise ValueError('Invalid message rectangle')
    for y in range(top,bottom):
        for x in range(left,right):
            if px[y][x] not in (10,15):raise ValueError('Message rectangle overlaps frame decoration')
            px[y][x]=10
    line_step=recipe.get('line_step',8)
    if line_step<8 or top+(len(recipe['lines'])-1)*line_step+8>bottom:raise ValueError('Too many message lines')
    for line,text in enumerate(recipe['lines']):
        mask=raster(text,atlas,mapping)
        if len(mask[0])>right-left:raise ValueError('Message line overflows')
        x0=recipe.get('text_left',left+(right-left-len(mask[0]))//2)
        if x0<left or x0+len(mask[0])>right:raise ValueError('Message line overflows')
        for y,row in enumerate(mask):
            for x,v in enumerate(row):
                if v:px[top+line*line_step+y][x0+x]=15
    return px

def compose_segments(original,recipe,atlas,mapping):
    """Compile native-font labels inside explicit, shared-tile-safe regions."""
    px=[row[:] for row in original]
    for part in recipe['segments']:
        left,top,right,bottom=part['rect']
        if not (0<=left<right<=len(px[0]) and 0<=top<bottom<=len(px)):
            raise ValueError('Invalid label region')
        background=part.get('background',0)
        ink=part.get('ink',15);outline=part.get('outline')
        if ink==0 or outline==0:raise ValueError('Text ink must be opaque')
        mask=raster(part['text'],atlas,mapping,part.get('tracking',1))
        sx,sy=part.get('scale',[1,1])
        mask=[[v for v in row for _ in range(sx)] for row in mask for _ in range(sy)]
        margin=1 if outline is not None and not part.get('clip_outline') else 0
        x0=part.get('x',left+(right-left-len(mask[0]))//2)
        y0=top+(bottom-top-len(mask))//2
        if x0-margin<left or x0+len(mask[0])+margin>right or y0-margin<top or y0+len(mask)+margin>bottom:
            raise ValueError('Label does not fit: '+part['text'])
        if background is not None:
            for y in range(top,bottom):px[y][left:right]=[background]*(right-left)
        if outline is not None:
            for y,row in enumerate(mask):
                for x,v in enumerate(row):
                    if v:
                        for dx,dy in ((-1,0),(1,0),(0,-1),(0,1)):
                            ox=x0+x+dx;oy=y0+y+dy
                            if left<=ox<right and top<=oy<bottom:px[oy][ox]=outline
        gradient=part.get('gradient',[ink]*len(mask))
        if len(gradient)!=len(mask) or 0 in gradient:raise ValueError('Invalid opaque text gradient')
        for y,row in enumerate(mask):
            for x,v in enumerate(row):
                if v:px[y0+y][x0+x]=gradient[y]
    return px

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--batch',type=int,help='Only author recipes for this batch')
    parser.add_argument('--write',action='store_true',help='Save new variants; default only validates')
    parser.add_argument('--replace',action='store_true',help='Explicitly overwrite existing variants')
    args=parser.parse_args()
    recipes=json.loads((ROOT/'graphics/ui/text_labels.json').read_text())
    atlas,_=gfx.read_png(str(ROOT/'graphics/fonts/font.png'))
    mapping=json.loads((ROOT/'text/translation/english_font.json').read_text())
    prepared=[]
    for recipe in recipes:
        if args.batch is not None and recipe.get('batch')!=args.batch:continue
        path=ROOT/'graphics/ui'/recipe['source'];dest=path.with_stem(path.stem+'_en')
        original,palette=gfx.read_png(str(path))
        if 'canvas' in recipe:
            width,height=recipe['canvas']
            if not 0<width<=240 or not 0<height<=16:raise ValueError('Invalid credit canvas')
            original=[[0]*width for _ in range(height)]
        if 'base' in recipe:
            base=ROOT/'graphics/ui'/recipe['base']
            pixels,base_palette=gfx.read_png(str(base))
            if base_palette!=palette or gfx.png_alpha(base)!=gfx.png_alpha(path):
                raise ValueError('Authoring background changes palette or transparency')
            if len(pixels)!=len(original) or any(len(a)!=len(b) for a,b in zip(pixels,original)):
                raise ValueError('Authoring background changes geometry')
            original=pixels
        if recipe['kind']=='segments':pixels=compose_segments(original,recipe,atlas,mapping)
        elif recipe['kind']=='message':pixels=compose_message(original,recipe,atlas,mapping)
        else:pixels=compose(original,raster(recipe['english'],atlas,mapping,recipe.get('tracking',1)),recipe['kind'],recipe)
        if args.write and dest.exists() and not args.replace:raise ValueError('Variant exists; use --replace only intentionally: '+str(dest))
        prepared.append((dest,pixels,palette))
    if args.write:
        for dest,pixels,palette in prepared:gfx.write_png(str(dest),pixels,palette,transparent_index=0)
    print(f'{len(prepared)} label sources validated'+(' and written' if args.write else ''))


if __name__=='__main__':main()
