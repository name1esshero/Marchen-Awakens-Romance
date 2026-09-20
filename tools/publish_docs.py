#!/usr/bin/env python3
"""Stage galleries and report snapshots for gh-pages, and Markdown for the wiki.

Copies an explicit set of documentation/asset directories, never the ROM,
credentials, compiler, backups, or Git metadata. Does not publish or delete.
"""
import argparse
import hashlib
import html
from html.parser import HTMLParser
import json
import os
from pathlib import Path
import posixpath
import re
import shutil
from urllib.parse import unquote, urlsplit, quote

ROOT = Path(__file__).resolve().parents[1]
SITE = 'https://name1esshero.github.io/Marchen-Awakens-Romance/'
WIKI = 'https://github.com/name1esshero/Marchen-Awakens-Romance/wiki/'
DOCS = {
    'Build-verification': 'reports/build/README.md',
    'Setup-audit-history': 'reports/audit/README.md',
    'Graphics': 'graphics/README.md',
    'Map-editor': 'tools/map_editor/README.md',
    'Sprite-rendering': 'docs/sprite-rendering.md',
    'Background-images': 'graphics/backgrounds/README.md',
    'Sound': 'sound/README.md',
    'Translation': 'text/translation/README.md',
    'Documentation-workflow': 'tools/site/README.md',
}
ALLOWED = {'.html', '.css', '.js', '.png', '.gif', '.pal', '.json', '.txt', '.md',
           '.tsv', '.wav', '.bin', '.nft', '.log'}


def files(folder):
    for directory, children, names in os.walk(folder):
        children[:] = sorted(n for n in children if not n.startswith('.'))
        for name in sorted(names):
            path = Path(directory) / name
            if not path.is_symlink() and path.suffix.lower() in ALLOWED:
                yield path


class Links(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []

    def handle_starttag(self, tag, attrs):
        self.links.extend(v for k, v in attrs if k in ('href', 'src') and v)


def check_links(site):
    missing = []
    checked={}
    site_root=site.resolve()
    def present(path,file_only=False):
        key=(str(path),file_only)
        if key not in checked:
            resolved=path.resolve()
            checked[key]=resolved.is_relative_to(site_root) and (resolved.is_file() if file_only else resolved.exists())
        return checked[key]
    pages = list(site.rglob('*.html'))
    for page in pages:
        parser = Links()
        parser.feed(page.read_text())
        for link in parser.links:
            parts = urlsplit(link)
            if parts.scheme or parts.netloc or not parts.path:
                continue
            path = page.parent / unquote(parts.path)
            if not present(path):
                missing.append({'page': str(page.relative_to(site)), 'link': link})
    # Animation image URLs are assembled in JavaScript, not HTML attributes.
    frames = 0
    for category in ('battle/characters', 'battle/effects', 'ui'):
        folder = site / 'graphics' / category
        manifest = folder / 'manifest.json'
        if not manifest.exists():
            continue
        frame_data=json.loads(manifest.read_text())
        viewer=folder/'index.html'
        if viewer.exists() and 'const data=' in viewer.read_text():
            frame_data=json.JSONDecoder().raw_decode(viewer.read_text().split('const data=',1)[1])[0]
        for frame in frame_data['frames']:
            paths = [frame['path'], *frame.get('cell_images', [])]
            if 'english' in frame:paths.append(frame['english']['path'])
            paths += [f'palettes/{bank:03}.pal' for bank in frame['banks']]
            for link in paths:
                path = folder / link
                if not present(path,True):
                    missing.append({'page': str(manifest.relative_to(site)), 'link': link})
            frames += 1
    return {'html_pages': len(pages), 'animation_frames': frames, 'missing': missing}


def markdown_links(text, source):
    def replace(match):
        target = match.group(1)
        if urlsplit(target).scheme or target.startswith(('#', '//')):
            return match.group(0)
        path = posixpath.normpath(posixpath.join(posixpath.dirname(source), target))
        return '](' + SITE + quote(path, safe='/#') + ')'
    return re.sub(r'\]\(([^\s)]+)\)', replace, text)


def stage_wiki(wiki,reports):
    wiki.mkdir(parents=True, exist_ok=True)
    for title, source in DOCS.items():
        text = (ROOT / source).read_text()
        prefix = ('> Historical audit narrative; its counts describe earlier work. Consult the published reports for later snapshots.\n\n' if title == 'Setup-audit-history' else '')
        (wiki / (title + '.md')).write_text(prefix + markdown_links(text, source))
    (wiki / 'Reports.md').write_text('# Audit and build reports\n\nThese are preserved snapshots, not a claim that every audit describes the latest state.\n\n' + '\n'.join('- [' + p[8:] + '](' + SITE + quote(p) + ')' for p in reports) + '\n')
    home = wiki / 'Home.md'
    old = home.read_text() if home.exists() else '# MAR decompilation\n'
    old = old.split('<!-- MAR documentation navigation -->')[0].rstrip()
    navigation = '\n\n<!-- MAR documentation navigation -->\n\n[Interactive graphics, sound, and script galleries](' + SITE + ')\n\n'
    navigation += '\n'.join('- [' + title.replace('-', ' ') + '](' + title + ')' for title in (*DOCS, 'Reports')) + '\n'
    home.write_text(old + navigation)
    (wiki / '_Sidebar.md').write_text('[Home](Home)\n\n' + navigation.split('<!-- MAR documentation navigation -->\n\n')[-1])


def stage(site, wiki):
    if site.resolve() == ROOT or ROOT.is_relative_to(site.resolve()):
        raise ValueError('Output must not contain the source repository')
    for name in ('graphics', 'sound', 'text', 'reports', 'tools', '.git'):
        if site.resolve().is_relative_to((ROOT / name).resolve()):
            raise ValueError('Output must not overwrite source directories')
    absent = [source for source in DOCS.values() if not (ROOT / source).is_file()]
    if absent:
        raise SystemExit('Missing documentation snapshots; run make docs-fetch first: ' + ', '.join(absent))
    site.mkdir(parents=True, exist_ok=True)
    copied = {}
    for name in ('graphics', 'sound', 'text', 'reports'):
        count = 0
        for source in files(ROOT / name):
            rel = source.relative_to(ROOT)
            target = site / rel
            target.parent.mkdir(parents=True, exist_ok=True)
            raw = source.read_bytes()
            target.write_bytes(raw)
            copied[str(rel)] = {'bytes': len(raw), 'sha256': hashlib.sha256(raw).hexdigest()}
            count += 1
        print(f'Staged {count} {name} files', flush=True)
    for source in files(ROOT / 'docs' / 'media'):
        rel=source.relative_to(ROOT);target=site/rel
        target.parent.mkdir(parents=True,exist_ok=True)
        raw=source.read_bytes();target.write_bytes(raw)
        copied[str(rel)]={'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest()}
    # Wiki references must also resolve on Pages, including map-editor notes.
    for relative in DOCS.values():
        source=ROOT/relative;target=site/relative
        target.parent.mkdir(parents=True,exist_ok=True)
        raw=source.read_bytes();target.write_bytes(raw)
        copied[relative]={'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest()}
    reports = [p for p in copied if p.startswith('reports/') and Path(p).suffix in ('.json', '.md', '.log')]
    report_links = '\n'.join('<li><a href="' + html.escape(p[8:]) + '">' + html.escape(p[8:]) + '</a></li>' for p in reports)
    (site / 'reports/index.html').write_text('<!doctype html><meta charset="utf-8"><title>MAR reports</title><h1>MAR report snapshots</h1><p>Historical reports describe the state when each audit ran. They are not all current progress claims.</p><a href="../index.html">Documentation home</a><ul>' + report_links + '</ul>')
    (site / 'index.html').write_text('''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MAR decompilation documentation</title><style>body{font:18px system-ui;background:#19222e;color:#eef3fa;max-width:900px;margin:4rem auto;padding:1rem}a{color:#a6d0ff}li{margin:1.2rem 0}img{display:block;max-width:100%;height:auto;border:1px solid #617086}</style><h1>MAR decompilation documentation</h1><p>Recovered graphics, sound samples, translated script reviews, and build evidence.</p><ul><li><a href="''' + WIKI + '''">Project wiki and format documentation</a></li><li><a href="''' + WIKI + '''Map-editor">Map editor guide</a></li><li><a href="graphics/index.html">Graphics and animation galleries</a></li><li><a href="sound/index.html">Sound sample browser</a></li><li><a href="reports/text/index.html">Japanese / English script review</a></li><li><a href="reports/index.html">Audit and build report snapshots</a></li></ul><h2>Map editor</h2><p>The recording below uses the running editor and its decoded game assets.</p><a href="''' + WIKI + '''Map-editor"><img src="docs/media/map-editor.gif" alt="Map editor loading maps, changing modes, and previewing sprite events"></a><p>Published assets are viewing and download copies. Edit the local decomp sources and rebuild to change the game. Snapshots do not establish full decompilation or emulator verification.</p></html>''')
    (site / '.nojekyll').write_text('')
    audit = check_links(site)
    manifest = {'version': 1, 'files': copied, 'validation': audit}
    (site / 'publication.json').write_text(json.dumps(manifest, indent=2) + '\n')
    stage_wiki(wiki,reports)
    print(json.dumps(audit, indent=2), flush=True)
    if audit['missing']:
        raise SystemExit('Broken local links: publication is not ready')


def restore(snapshot):
    """Restore missing generated outputs only; never replace editable sources."""
    count = 0
    for name in ('reports', 'graphics', 'sound'):
        for source in files(snapshot / name):
            if name != 'reports' and source.name != 'index.html':
                continue
            target = ROOT / source.relative_to(snapshot)
            if target.exists():
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
            count += 1
    print(f'Restored {count} missing documentation outputs; existing files retained')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--site', type=Path, default=ROOT / 'build/site')
    parser.add_argument('--wiki', type=Path, default=ROOT / 'build/wiki')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--restore', type=Path, help='Restore missing outputs from a gh-pages checkout')
    args = parser.parse_args()
    if args.restore:
        restore(args.restore)
        raise SystemExit(0)
    if args.check:
        result = check_links(args.site)
        print(json.dumps(result, indent=2))
        raise SystemExit(bool(result['missing']))
    stage(args.site, args.wiki)
