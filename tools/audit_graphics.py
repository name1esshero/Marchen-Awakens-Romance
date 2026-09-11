#!/usr/bin/env python3
"""Audit active named image sources against ROM bytes and the named filesystem.

A byte round-trip proves reversibility, not that bytes are artwork. Record
those conclusions separately. NCD interiors need their own table decoding;
never promote an entire container to verified pixels on its name alone.
"""
import collections
import hashlib
import html
import json
from pathlib import Path
import struct

import gfx
import lz77
import nfp
import ncd
import mapped_images
import build_assets

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'reports/graphics'


def audit():
    rom = (ROOT / 'baserom.gba').read_bytes()
    directory = nfp.entries(rom)
    by_name = {e['name']: e for e in directory}
    containers = {}
    for e in directory:
        if e['name'].endswith('.NCD'):
            blob = rom[e['rom_offset']:e['end']]
            info = ncd.layout(blob)
            containers[e['name']] = (blob, info, [ncd.cell(blob, info, i) for i in range(info['cells'])])
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'previews').mkdir(exist_ok=True)
    rows = []
    for manifest in ('assets.json', 'assets_raw.json'):
        for entry in json.loads((ROOT / manifest).read_text()):
            png = ROOT / (entry['path'] + '.png')
            if not png.exists():
                if entry['path'].startswith('graphics/'):
                    raise FileNotFoundError('Missing active graphics source: ' + str(png))
                continue
            start = entry['rom_offset']
            end = start + entry.get('compressed_size', entry['raw_size'])
            owners = nfp.owners(directory, start, end)
            names = [e['name'] for e in owners]
            row = dict(path=entry['path'], manifest=manifest, rom_offset=start,
                       raw_size=entry['raw_size'], kind=entry['kind'], owners=names,
                       status='unverified', evidence=[], roundtrip=False)
            bpp = entry.get('bpp', 4)
            try:
                if 'compressed_size' in entry:
                    packed = build_assets.compile_entry(entry, ROOT)
                    row['compressed_roundtrip'] = packed == rom[start:end]
                    if not row['compressed_roundtrip']:
                        raise ValueError('Image-derived compressed bytes differ from original ROM')
                    decoder = lz77
                    raw, consumed = decoder.decompress(packed)
                    if consumed != len(packed):
                        raise ValueError('Compressed size does not match decoder consumption')
                else:
                    raw = rom[start:end]
                if len(raw) != entry['raw_size']:
                    raise ValueError('Decompressed length differs from manifest')
                px, _ = gfx.read_png(str(png))
                if entry['kind'] != 'mapped_image' and any(v >= 1 << bpp for line in px for v in line):
                    raise ValueError('Pixel index exceeds bit depth')
                rebuilt = mapped_images.compile_image(entry) if entry['kind']=='mapped_image' else gfx.pixels_to_tiles(px, bpp)
                row['roundtrip'] = rebuilt[:len(raw)] == raw and not any(rebuilt[len(raw):])
                row['sha256'] = hashlib.sha256(raw).hexdigest()
                if not row['roundtrip']:
                    row['evidence'].append('PNG differs from ROM payload or contains nonzero padding')
            except (ValueError, IndexError, OSError) as ex:
                row['evidence'].append(str(ex))
                rows.append(row)
                continue
            if len(owners) != 1 or not owners[0]['rom_offset'] <= start < end <= owners[0]['end']:
                row['status'] = 'mixed_or_unowned'
                row['evidence'].append('Range crosses named members or lies outside the named archive')
            else:
                owner = owners[0]
                ext = owner['name'].rsplit('.', 1)[-1]
                row['member_offset'] = start - owner['rom_offset']
                if ext == 'NCD':
                    blob, info, cells = containers[owner['name']]
                    pool = owner['rom_offset'] + info['offsets'][5]
                    if 'compressed_size' in entry:
                        row['status'] = 'false_compression_hit'
                        row['evidence'].append('NCD consumer directly addresses uncompressed tile data; scan hit is not a stored compressed graphic')
                    elif start >= pool:
                        row['status'] = 'verified_sprite_tile_fragment' if (start-pool)%32 == 0 else 'misaligned_sprite_tile_fragment'
                        row['evidence'].append('Inside the NCD 4bpp pool; an arbitrary slice, with tile alignment offset '+str((start-pool)%32))
                        row['palette_banks_used'] = sorted({c['palette'] for c in cells if owner['rom_offset']+c['start'] < end and start < owner['rom_offset']+c['end']})
                    else:
                        row['status'] = 'animation_data_as_pixels'
                        row['evidence'].append('Range begins in NCD animation/frame/cell tables')
                elif ext in ('KCG', 'TCG') and start == owner['rom_offset']:
                    row['status'] = 'named_tile_graphics'
                    row['evidence'].append('Whole named character-graphics member; valid LZ77 stream and complete tiles')
                    if len(raw) % (32 if bpp == 4 else 64):
                        row['status'] = 'unverified'
                    if entry['kind'] == 'mapped_image':
                        row['status'] = 'mapped_background_image'
                        row['evidence'].append('Editable image compiled through affine byte indices' if bpp==8 else 'Editable image compiled through KMP tile indices, flips and palette banks')
                        row['layout'] = entry['image_layout']
                        rows.append(row)
                        continue
                    palname = owner['name'][:-1] + 'L'
                    pal = by_name.get(palname)
                    if pal:
                        palette_data = rom[pal['rom_offset']:pal['end']]
                        row['palette_member'] = palname
                        row['palette_banks'] = len(palette_data) // 32
                        # Preview real BGR555 banks without changing editable source pixels.
                        for bank in range(min(row['palette_banks'], 16) if bpp == 4 else 1):
                            count = 16 if bpp == 4 else len(palette_data)//2
                            colors = gfx.read_palette(palette_data, bank * 32, count)
                            if bpp == 8:
                                colors = [(0,0,0)] * entry.get('palette_base', 0) + colors
                                row['palette_placement'] = entry.get('palette_base_evidence', 'Unspecified')
                            name = f'{start:06X}_bank{bank:02}.png'
                            gfx.write_png(str(OUT / 'previews' / name), px, colors)
                        row['preview'] = f'previews/{start:06X}_bank00.png'
                        row['preview_bank_count'] = min(row['palette_banks'],16) if bpp == 4 else 1
                elif ext in ('KMP', 'TSC'):
                    row['status'] = 'map_data_as_pixels'
                    row['evidence'].append('Named map/screen data, not a sheet of packed character pixels')
                elif ext in ('KCL', 'TCL'):
                    row['status'] = 'palette_data_as_pixels'
                    row['evidence'].append('Named palette member; a tile rendering misrepresents palette words')
                elif ext == 'SPC':
                    row['status'] = 'script_data_as_pixels'
                    row['evidence'].append('Range belongs to a script member, not character graphics')
                elif ext == 'NFT':
                    row['status'] = 'wrong_font_format'
                    row['evidence'].append('FONT.NFT is 8x8 1bpp, not the displayed 4bpp interpretation')
            rows.append(row)
    report = dict(rom_sha256=hashlib.sha256(rom).hexdigest(), archive=dict(base=nfp.BASE, end=nfp.END,
                  count=len(directory)), totals=dict(collections.Counter(r['status'] for r in rows)),
                  png_roundtrips=sum(r['roundtrip'] for r in rows),
                  compressed_roundtrips=sum(r.get('compressed_roundtrip', False) for r in rows), assets=rows)
    (OUT / 'audit.json').write_text(json.dumps(report, indent=2) + '\n')
    (OUT / 'archive.json').write_text(json.dumps(directory, indent=2) + '\n')
    render(report)
    print(json.dumps({k:v for k,v in report.items() if k != 'assets'}, indent=2))
    return report


def render(report):
    cards = []
    for row in report['assets']:
        path = '../../' + row['path'] + '.png'
        preview = row.get('preview', path)
        title = ' / '.join(row['owners']) or 'Outside named archive'
        options = ''
        if 'preview' in row:
            options = '<label>Palette bank <select class="bank">' + ''.join(f'<option value="previews/{row["rom_offset"]:06X}_bank{i:02}.png">{i}</option>' for i in range(row['preview_bank_count'])) + '</select></label>'
        cards.append(f'''<article data-status="{row['status']}" data-search="{html.escape(title+' '+row['path'])}">
<h2>{html.escape(title)}</h2><p><code>{row['rom_offset']:06X}</code> · {row['status']} · byte round-trip: {'PASS' if row['roundtrip'] else 'FAIL'}</p>
{options}<a href="{path}"><img loading="lazy" src="{preview}" alt="{html.escape(title)}"></a>
<p>{html.escape('; '.join(row['evidence']))}</p><a href="{path}">Open editable source PNG</a></article>''')
    page = '''<!doctype html><meta charset="utf-8"><title>MAR graphics audit</title>
<style>body{font:16px system-ui;background:#171a20;color:#eee;margin:24px}a{color:#97caff}header{position:sticky;top:0;background:#171a20;padding:12px;z-index:1}input,select{font:inherit;margin:8px}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:16px}article{background:#262c36;padding:16px;border-radius:8px}h2{font-size:17px}img{image-rendering:pixelated;max-width:100%;max-height:400px;object-fit:contain;object-position:top}article[hidden]{display:none}p{overflow-wrap:anywhere}</style>
<header><h1>MAR graphics audit</h1><p>Byte preservation and image identity are separate checks. Unverified entries remain unverified.</p>
<p><a href="../../graphics/fonts/font.png">Full editable font sheet</a> · <a href="../../graphics/fonts/preview.png">Font preview</a> · <a href="../../graphics/fonts/glyphs.tsv">Glyph index</a> · <a href="audit.json">Evidence JSON</a></p>
<input id="query" placeholder="Filename or ROM offset"><select id="status"><option value="">All classifications</option>'''
    page += ''.join(f'<option>{s}</option>' for s in report['totals'])
    page += '</select><span id="count"></span></header><main>' + ''.join(cards) + '</main>'
    page += '''<script>const cards=[...document.querySelectorAll('article')];function filter(){let n=0;for(const c of cards){c.hidden=!(c.textContent.toLowerCase().includes(query.value.toLowerCase())&&(!statusSelect.value||c.dataset.status===statusSelect.value));if(!c.hidden)n++}document.getElementById('count').textContent=n+' assets'}const query=document.getElementById('query'),statusSelect=document.getElementById('status');query.oninput=filter;statusSelect.onchange=filter;document.querySelectorAll('.bank').forEach(s=>s.onchange=()=>s.closest('article').querySelector('img').src=s.value);filter();</script>'''
    (OUT / 'index.html').write_text(page)


if __name__ == '__main__':
    audit()
