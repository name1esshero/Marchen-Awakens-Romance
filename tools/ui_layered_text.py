#!/usr/bin/env python3
"""Check or refresh layered English title previews from their editable cells.

The New/Continue words are reused by dungeon options. Foreground cell PNGs own
these edits; the original background cells and layout tables remain unchanged.
"""
import argparse
import json
from pathlib import Path
import gfx
import ncd
import scenes
import sprite_sources

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh-frames',action='store_true',help='Explicitly replace assembled previews after cell edits')
    args=parser.parse_args();base=ROOT/'graphics/ui'
    labels=json.loads((base/'layered_labels.json').read_text())
    images={e['tile']:e for e in json.loads((base/'source/images.json').read_text())}
    tiles={t for label in labels for t in label['tiles']}
    for tile in tiles:
        source=base/images[tile]['path']
        if not source.with_stem(source.stem+'_en').exists():raise ValueError('Missing English foreground: '+str(source))
    blob=sprite_sources.compile('SYSTEM',views=False,english=True);info=ncd.layout(blob)
    manifest=json.loads((base/'manifest.json').read_text())
    frames=sorted({f for label in labels for f in label['frames']})
    for frame in frames:
        source=base/manifest['frames'][frame]['path'];dest=source.with_stem(source.stem+'_en')
        px,_,pal,_=scenes.render(blob,info,frame)
        if args.refresh_frames:gfx.write_png(str(dest),px,pal,transparent_index=0)
        else:
            current,_=gfx.read_png(str(dest))
            if current!=px:raise ValueError('Preview differs from foreground cells: '+str(dest))
    print(f'{len(tiles)} foreground cells / {len(frames)} frame previews '+('refreshed' if args.refresh_frames else 'verified'))


if __name__=='__main__':main()
