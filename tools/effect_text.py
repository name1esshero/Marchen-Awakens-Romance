#!/usr/bin/env python3
"""Explicitly author English battle labels using the game's native Latin font."""
import json
from pathlib import Path
import gfx
from ui_text import raster, ROOT

def main():
    atlas,_=gfx.read_png(str(ROOT/'graphics/fonts/font.png'))
    mapping=json.loads((ROOT/'text/translation/english_font.json').read_text())
    records=[]
    for n in [386,387,388,389,402,403,404,405,406]:
        path=ROOT/f'graphics/battle/effects/frames/{n:04}.png'
        px,pal=gfx.read_png(str(path)); original=[r[:] for r in px];h=len(px);w=len(px[0])
        sealed=n<400;text='Sealed' if sealed else 'Stats'
        glyph=raster(text,atlas,mapping,0)
        x0=26 if sealed else 8;y0=8 if sealed else 4
        if sealed:
            for row in px:row[24:]=[0]*(w-24)
        else:
            for row in px:
                for x,v in enumerate(row):
                    if v in (7,15):row[x]=5
        points={(x0+x,y0+y) for y,row in enumerate(glyph) for x,v in enumerate(row) if v}
        for radius,color in ([(2,12),(1,14),(0,7)] if sealed else [(1,15),(0,7)]):
            for x,y in points:
                for dy in range(-radius,radius+1):
                    for dx in range(-radius,radius+1):
                        xx,yy=x+dx,y+dy
                        if 0<=xx<w and 0<=yy<h and (sealed or original[yy][xx]!=0):px[yy][xx]=color
        gfx.write_png(str(path.with_stem(path.stem+'_en')),px,pal,transparent_index=0)
        records.append(dict(frame=n,japanese='封印中' if sealed else 'パラメータ',english=text,source=path.name,variant=path.stem+'_en.png'))
    (ROOT/'graphics/battle/effects/english.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
if __name__=='__main__':main()
