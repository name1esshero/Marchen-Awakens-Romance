#!/usr/bin/env python3
"""Regenerate asset gallery HTML from editable manifests, without extraction."""
import html
import json
from pathlib import Path
import subprocess
import sys
import scenes
import gfx
import ncd
import text_gallery

ROOT = Path(__file__).resolve().parents[1]
STYLE = '<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><style>body{font:18px system-ui;background:#19222e;color:#eef3fa;margin:2rem}a{color:#a6d0ff}img{image-rendering:pixelated}article{display:inline-block;margin:1rem;vertical-align:top}audio{display:block}input{font:inherit}</style>'


def main():
    (ROOT/'graphics/index.html').write_text((ROOT/'tools/site/graphics.html').read_text())
    subprocess.run([sys.executable, str(ROOT/'tools/graphics_catalog.py')], cwd=ROOT, check=True)
    for category in scenes.CATEGORIES.values():
        folder = ROOT/'graphics'/category
        data = json.loads((folder/'manifest.json').read_text())
        page = scenes.TEMPLATE.replace('GRAPHICS_INDEX', '../'*len(category.split('/'))+'index.html')
        page = page.replace('DATA_JSON', json.dumps(data, separators=(',', ':')).replace('</', '<\\/'))
        (folder/'index.html').write_text(page)
    # Keep the historical first-frame gallery synchronized with editable views.
    preview_manifest = ROOT/'reports/sprites/previews.json'
    if preview_manifest.exists():
        manifests = {stem: json.loads((ROOT/'graphics'/category/'manifest.json').read_text())
                     for stem, category in scenes.CATEGORIES.items()}
        for entry in json.loads(preview_manifest.read_text()):
            stem = entry['container'].removesuffix('.NCD')
            frame = manifests[stem]['frames'][entry['frame']]
            pixels, palette = gfx.read_png(str(ROOT/'graphics'/scenes.CATEGORIES[stem]/frame['path']))
            rows = [[palette[value] if value else (40,44,52) for value in row] for row in pixels]
            target = preview_manifest.parent/entry['path']
            target.parent.mkdir(parents=True, exist_ok=True)
            ncd.write_rgb(target, rows)
    portraits = json.loads((ROOT/'graphics/portraits/manifest.json').read_text())
    page = STYLE + f'<title>MAR portraits</title><h1>{len(portraits)} dialogue portraits</h1><a href="../index.html">Graphics</a><p>Original resource labels and palette banks. Edit local PNG and PAL sources to change the ROM.</p><input id="q" placeholder="Filter resource name"><main>'
    for entry in portraits:
        name = html.escape(entry['name'])
        page += f'<article><a href="{name}.png"><img width="128" src="{name}.png" loading="lazy" alt="{name}"></a><p>{name}</p><a href="../ui/palettes/{entry["palette"]:03}.pal">Palette {entry["palette"]}</a></article>'
    page += '</main><script>document.getElementById("q").oninput=function(){document.querySelectorAll("article").forEach(a=>a.hidden=!a.textContent.toLowerCase().includes(this.value.toLowerCase()))}</script>'
    (ROOT/'graphics/portraits/index.html').write_text(page)
    icons = json.loads((ROOT/'graphics/icons/manifest.json').read_text())
    page = STYLE + f'<title>MAR icons</title><h1>{len(icons)} icon frames</h1><a href="../index.html">Graphics</a><p>Rows are animations; columns are frames. Individual indexed PNGs and palette files remain editable build inputs.</p>'
    for group in sorted({entry['group'] for entry in icons}):
        group = html.escape(group)
        page += f'<h2>{group}</h2><img width="560" src="{group}_contact.png" alt="{group} animations">'
    page += '<p><a href="manifest.json">Frame and palette metadata</a></p>'
    (ROOT/'graphics/icons/index.html').write_text(page)
    samples = json.loads((ROOT/'sound/samples/manifest.json').read_text())
    page = STYLE + f'<title>MAR sound samples</title><h1>{len(samples)} driver-referenced sound samples</h1><p>Instrument and effect PCM samples, not rendered songs. Sequencing and PSG voices are not rendered.</p><a href="samples/manifest.json">Sample metadata</a> · <a href="songs.json">Song table</a> · <a href="voice_references.json">Voice references</a><ul>'
    for entry in samples:
        name = f'{entry["rom_offset"]:06X}.wav'
        page += f'<li><a href="samples/{name}">{name}</a> — {entry["sample_count"]} samples, {entry["frequency"]//1024} Hz, loop start {entry["loop_start"]}<audio controls preload="none" src="samples/{name}"></audio></li>'
    (ROOT/'sound/index.html').write_text(page + '</ul>')
    text_gallery.main()
    print('Generated gallery HTML; editable assets unchanged')


if __name__ == '__main__':
    main()
