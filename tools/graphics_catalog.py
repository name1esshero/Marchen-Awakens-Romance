#!/usr/bin/env python3
"""Catalog named background art and remove redundant palette viewing copies.

Only a sidecar identical to the authoritative named palette (including the
known 8bpp display offset) can be removed. Custom/different sidecars survive.
These sidecars are not build inputs; palette_path identifies the ROM source.
"""
import asset_safety
import argparse
import hashlib
import html
import json
from pathlib import Path
import gfx

ROOT=Path(__file__).resolve().parents[1]


def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--remove-sidecars',action='store_true')
    args=parser.parse_args(argv)
    folder=ROOT/'graphics/backgrounds';folder.mkdir(parents=True,exist_ok=True)
    rows=[];removed=[]
    for e in sorted(json.loads((ROOT/'assets.json').read_text()),key=lambda e:e.get('archive_name','')):
        if 'palette_path' not in e:continue
        side=ROOT/(e['path']+'.pal')
        colors=gfx.read_jasc(str(ROOT/e['palette_path']))
        expected=[(0,0,0)]*e.get('palette_base',0)+(colors[:16] if e['bpp']==4 else colors)
        if args.remove_sidecars and side.exists() and gfx.read_jasc(str(side))==expected:
            raw=side.read_bytes()
            removed.append(dict(path=str(side.relative_to(ROOT)),replacement=e['palette_path'],sha256=hashlib.sha256(raw).hexdigest(),bytes=len(raw)))
        png='../../'+e['path']+'.png';pal='../../'+e['palette_path']
        english_path = ROOT / (e['path'] + '_en.png')
        english = ''
        if english_path.is_file():
            english_png = '../../' + e['path'] + '_en.png'
            english = '<br><a href="' + english_png + '"><img loading="lazy" src="' + english_png + '" alt="English override">English override</a>'
        mapped = e['kind'] == 'mapped_image'
        description = 'Tile sheet'
        links = ''
        if mapped:
            layout = json.loads((ROOT/e['image_layout']).read_text())
            description = f"Mapped image, {layout['width_tiles']*8}×{layout['height_tiles']*8}"
            links = '<br><a href="../../'+e['image_layout']+'">Edit map layout</a>'
            for layer in layout['layers'][1:]:
                links += '<br><a href="../../'+layer['image']+'">Image layer '+str(layer['index'])+'</a>'
        rows.append('<tr data-row><td>'+html.escape(e['archive_name'])+'</td><td><a href="'+png+'"><img loading="lazy" src="'+png+'" alt="'+description+'">Japanese source</a>'+english+links+'</td><td><a href="'+pal+'">'+str(e['palette_banks'])+' palette banks</a></td><td>'+description+' · '+str(e['bpp'])+'bpp</td></tr>')

    page='''<!doctype html><meta charset="utf-8"><title>MAR named background art</title><style>body{background:#202a38;color:#eee;font:17px system-ui;margin:30px}a{color:#9cf}td{padding:12px;border-bottom:1px solid #596575}img{display:block;image-rendering:pixelated;max-width:160px;max-height:96px;object-fit:contain}input{font:inherit;padding:10px}</style><a href="../index.html">Graphics index</a><h1>Background images and tile sources</h1><p>Mapped images use KMP dimensions, flips and palette banks, or affine TSC byte-index frames. Open each image at its native size; additional layers and map layouts are linked separately. Resources awaiting map decoding remain tile sheets. Japanese sources are shown first; active English overrides appear beneath them.</p><input id="search" placeholder="Filter archive name"><table><tr><th>Resource</th><th>Editable PNG</th><th>Editable palettes</th><th>Format</th></tr>'''+''.join(rows)+'''</table><script>document.getElementById('search').oninput=function(){const q=this.value.toLowerCase();document.querySelectorAll('[data-row]').forEach(r=>r.hidden=!r.textContent.toLowerCase().includes(q))}</script>'''
    (folder/'index.html').write_text(page)
    if args.remove_sidecars:
        dest=ROOT/'reports/graphics/removed-sidecars.json'
        old=json.loads(dest.read_text()) if dest.exists() else []
        dest.write_text(json.dumps(old+removed,indent=2)+'\n')
        for e in removed:
            path=ROOT/e['path']
            if hashlib.sha256(path.read_bytes()).hexdigest()!=e['sha256']:raise ValueError('Sidecar changed during cleanup')
            asset_safety.unlink_generated(path)
    print(f'{len(rows)} named background sources; {len(removed)} redundant palette sidecars removed')


if __name__=='__main__':main()
