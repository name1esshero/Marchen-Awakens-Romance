#!/usr/bin/env python3
"""Start an isolated two-map fixture for browser tests; never edit real maps."""
import argparse
import json
from pathlib import Path
import shutil
import tempfile
from test_map_editor import fixture,ROOT
from map_editor.model import Project,SPRITE_CONTAINERS
from map_editor.server import make_server


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--session',type=Path,default=ROOT/'build/map-editor-browser.json')
    args=parser.parse_args()
    fixture_parent=ROOT/'build';fixture_parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='map-editor-browser-fixture-',dir=fixture_parent) as temp:
        work=Path(temp);fixture(work);source=Project(ROOT)
        def copy(rel):
            target=work/rel;target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(ROOT/rel,target)
        def add_map(name):
            member,_,entry,_=source.entry(name)
            paths={member['path'],entry['palette_path'],entry['path']+'.png'}
            if entry.get('image_layout'):
                layout=json.loads((ROOT/entry['image_layout']).read_text())
                paths.add(entry['image_layout']);paths.update(layer['image'] for layer in layout['layers'])
                if entry.get('unused_tiles_image'):paths.add(entry['unused_tiles_image'])
            for rel in paths:copy(rel)
            for rel,value in [('maps/nfp/manifest.json',member),('assets.json',entry)]:
                path=work/rel;entries=json.loads(path.read_text());entries.append(value);path.write_text(json.dumps(entries))
        def add_script(name):
            entry=source.scripts[name]
            for rel in (entry['path'],entry['text']):copy(rel)
            path=work/'scripts/nfp/manifest.json';entries=json.loads(path.read_text())
            if not any(item['name']==name for item in entries):entries.append(entry)
            path.write_text(json.dumps(entries))

        copy(Path('maps/script_catalog.json'))
        copy(Path('maps/tile_resolutions.json'))

        member,_,entry,_=source.entry('MAP27_A.KMP')
        paths={member['path'],entry['palette_path'],entry['path']+'.png'}
        if entry.get('image_layout'):
            layout=json.loads((ROOT/entry['image_layout']).read_text())
            paths.add(entry['image_layout']);paths.update(layer['image'] for layer in layout['layers'])
            if entry.get('unused_tiles_image'):paths.add(entry['unused_tiles_image'])
        for rel in paths:
            copy(rel)
        for rel,value in [('maps/nfp/manifest.json',member),('assets.json',entry)]:
            path=work/rel;entries=json.loads(path.read_text());entries.append(value);path.write_text(json.dumps(entries))
        # Include an actual small literal hit rectangle for semantic UI checks.
        hit_script=source.scripts['D0_BOSS.SPC']
        for rel in (hit_script['path'],hit_script['text']):
            target=work/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/rel,target)
        path=work/'scripts/nfp/manifest.json'
        entries=json.loads(path.read_text());entries.append(hit_script);path.write_text(json.dumps(entries))
        add_map('AD4_B_A.KMP')

        # The documentation recording uses real numbered fields and scripts.
        add_map('MAP01_3A.KMP')
        for name in ('MAP01_3A.SPC','SP_M01_3.SPC','CH_M01_3.SPC','BTOM01_3.SPC',
                     'EV_BA02.SPC','EV_BA03.SPC','EV_BA04.SPC'):
            add_script(name)
        add_map('MAP04_A.KMP');add_script('EV_ICE02.SPC')
        def add_sprite_groups(manifest_path,names):
            manifest=json.loads((ROOT/manifest_path).read_text());copy(manifest_path)
            frame_ids={frame for group in manifest['groups'] if group['name'] in names
                       for animation in group['animations'] for frame in animation['frames']}
            for frame_id in frame_ids:copy(manifest_path.parent/manifest['frames'][frame_id]['path'])
        sprite_resources={path:set() for path in SPRITE_CONTAINERS.values()}
        for entry in json.loads((work/'scripts/nfp/manifest.json').read_text()):
            for candidate in source.script_data(entry['name'])['sprite_candidates']:
                container=candidate.get('container');resource=candidate.get('resource')
                if container in SPRITE_CONTAINERS and isinstance(resource,str):
                    sprite_resources[SPRITE_CONTAINERS[container]].add(resource)
        for manifest_path,names in sprite_resources.items():
            add_sprite_groups(manifest_path,names)
        server,token=make_server(Project(work),0)
        args.session.parent.mkdir(parents=True,exist_ok=True)
        session_root=str(work.relative_to(ROOT))
        args.session.write_text(json.dumps(dict(url=f'http://127.0.0.1:{server.server_port}/#token={token}',root=session_root,isolated_fixture=True)))
        print('Fixture ready. Pass this session file to map_editor_browser.cjs:',args.session,flush=True)
        try:server.serve_forever()
        except KeyboardInterrupt:pass
        finally:
            server.server_close()
            if args.session.exists() and json.loads(args.session.read_text()).get('root')==session_root:args.session.unlink()


if __name__=='__main__':main()
