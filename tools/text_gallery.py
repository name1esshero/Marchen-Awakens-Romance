#!/usr/bin/env python3
"""Build a local script review gallery with exact-name portrait references.

Rows follow CODE offsets, not inferred execution order. Resource matches show
that the named asset exists; they do not prove when a portrait is displayed.
Japanese source and translation comments are never modified by this tool.
"""
import html
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'reports/text'
STYLE = '''<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<style>body{background:#19222e;color:#eef3fa;font:17px system-ui;max-width:1100px;margin:30px auto;padding:16px}a{color:#a6d0ff}article{border-top:1px solid #526477;padding:15px 0;display:flow-root}code{color:#b9c5d3}p{white-space:pre-wrap}img{float:right;width:128px;height:128px;image-rendering:pixelated;background:#303c4b}input{font:inherit;padding:10px;width:100%;box-sizing:border-box}.pending{color:#ffd18b}li{margin:12px 0}[hidden]{display:none}</style>'''
FILTER = '''<script>const search=document.getElementById('search'),pending=document.getElementById('pending');function filter(){const q=search.value.toLowerCase();document.querySelectorAll('[data-row]').forEach(r=>r.hidden=!r.textContent.toLowerCase().includes(q)||(pending&&pending.checked&&Number(r.dataset.pending)===0))}search.oninput=filter;if(pending)pending.onchange=filter;</script>'''


def records(path):
    rows=[]
    for line in path.read_text(encoding='utf-8').splitlines():
        if not line.startswith('@'):continue
        body,_,comment=line.partition('  //')
        offset,_,text=body.partition(' ')
        rows.append((int(offset[1:],16),text,comment.strip()))
    return sorted(rows)


def main():
    escape=html.escape
    portraits={p['name']:p for p in json.loads((ROOT/'graphics/portraits/manifest.json').read_text())}
    (OUT/'scripts').mkdir(parents=True,exist_ok=True)
    links=[];matched=0
    for source in sorted((ROOT/'text/nfp').glob('*.txt')):
        rows=records(source);english=sum(c.startswith('EN:') for _,_,c in rows)
        pending=sum(c.startswith('TODO:') for _,_,c in rows)
        literals=sum(c.startswith('LITERAL:') for _,_,c in rows)
        formatting=sum(c.startswith('FORMAT:') for _,_,c in rows)
        page=[STYLE,'<title>'+escape(source.name)+'</title>',
              '<a href="../index.html">All scripts</a> · <a href="../../../text/nfp/'+escape(source.name)+'">Editable source</a>',
              '<h1>'+escape(source.name)+'</h1>',
              '<p>CODE offset order; branches are not reconstructed. Portrait thumbnails are exact resource-name matches, not inferred screen state. Formatting tokens remain visible.</p>',
              '<input id="search" placeholder="Search Japanese, English, resource name, or offset">']
        for offset,text,comment in rows:
            page.append('<article data-row id="o%06X"><code>@%06X</code>'%(offset,offset))
            portrait=portraits.get(text.strip())
            if portrait:
                matched+=1
                link='../../../'+portrait['path']
                page.append('<a href="'+escape(link)+'"><img alt="'+escape(portrait['name'])+'" src="'+escape(link)+'" loading="lazy"></a>')
                page.append('<p>Portrait resource: '+escape(portrait['name'])+' · palette '+str(portrait['palette'])+'</p>')
            page.append('<p lang="ja">'+escape(text)+'</p>')
            page.append('<p class="'+('pending' if comment.startswith('TODO:') else '')+'">'+escape(comment)+'</p></article>')
        page.append(FILTER)
        (OUT/'scripts'/(source.stem+'.html')).write_text('\n'.join(page),encoding='utf-8')
        links.append('<li data-row data-pending="'+str(pending)+'"><a href="scripts/'+escape(source.stem)+'.html">'+escape(source.stem)+'</a> — '+str(english)+' English · '+str(pending)+' pending · '+str(literals)+' literal labels · '+str(formatting)+' formatting records</li>')
    (OUT/'index.html').write_text(STYLE+'<title>MAR script review</title><h1>MAR script review</h1><p>Original Japanese, English annotations, and matching portrait assets. ASCII resource labels are separate from translated dialogue.</p><a href="../../graphics/index.html">Graphics galleries</a><p><input id="search" placeholder="Filter script name"></p><p><label><input type="checkbox" id="pending" style="width:auto"> Only scripts with pending translations</label></p><ul>'+''.join(links)+'</ul>'+FILTER,encoding='utf-8')
    print(f'{len(links)} script pages; {matched} exact-name portrait references')


if __name__=='__main__':main()
