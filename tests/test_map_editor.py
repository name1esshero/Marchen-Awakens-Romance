"""Exercise real KMP/SPC sources, reversible saves, and the local HTTP boundary."""
import copy
import hashlib
import http.client
import json
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import threading
import unittest
from unittest.mock import patch

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from map_editor.model import Project,compile_map,map_document,override_path
from map_editor.server import make_server
import script_events

ROOT=Path(__file__).resolve().parents[1]


def fixture(root):
    source=Project(ROOT)
    member,_,entry,_=source.entry('MAP01_A.KMP')
    script=source.scripts['MAP01_A.SPC']
    layout=json.loads((ROOT/entry['image_layout']).read_text())
    paths={member['path'],script['path'],script['text'],entry['image_layout'],entry['palette_path']}
    paths.update(layer['image'] for layer in layout['layers'])
    if entry.get('unused_tiles_image'):paths.add(entry['unused_tiles_image'])
    for rel in paths:
        dest=root/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/rel,dest)
    for rel,value in [('assets.json',[entry]),('maps/nfp/manifest.json',[member]),('scripts/nfp/manifest.json',[script])]:
        (root/rel).write_text(json.dumps(value))
    return Project(root)


class MapCodecTests(unittest.TestCase):
    def test_all_47_maps_round_trip(self):
        project=Project(ROOT);catalog=project.catalog()
        self.assertEqual(len(catalog['maps']),47);self.assertFalse(catalog['unsupported'])
        for item in catalog['maps']:
            with self.subTest(map=item['name']):
                data=project.load(item['name'])
                self.assertEqual(compile_map(data['_base'],data['document']),data['_base'])

    def test_all_script_func_tables_and_noop_patches(self):
        project=Project(ROOT)
        for entry in project.scripts.values():
            with self.subTest(script=entry['name']):
                original=(ROOT/entry['path']).read_bytes()
                script_events.calls(original)
                document=dict(version=1,source_sha256=hashlib.sha256(original).hexdigest(),arguments={})
                self.assertEqual(script_events.apply(original,original,document),original)

    def test_tiles_attributes_preserve_every_other_byte(self):
        blob=(ROOT/'maps/nfp/MAP01_A.KMP.bin').read_bytes();doc=map_document(blob)
        doc['planes'][0]['entries'][0]^=0x400
        doc['attributes']['entries'][10]^=1
        out=compile_map(blob,doc)
        changed={i for i,(a,b) in enumerate(zip(blob,out)) if a!=b}
        self.assertEqual(changed,{doc['planes'][0]['offset']+1,doc['attributes']['offset']+20})
        doc['source_sha256']='stale'
        with self.assertRaises(ValueError):compile_map(blob,doc)

    def test_overlapping_planes_reject_conflicting_bytes(self):
        blob=bytearray((ROOT/'maps/nfp/MAP01_A.KMP.bin').read_bytes())
        struct.pack_into('<I',blob,0xA0,struct.unpack_from('<I',blob,0x9C)[0])
        doc=map_document(blob);doc['planes'][1]['entries'][0]^=1
        with self.assertRaisesRegex(ValueError,'overlapping'):compile_map(blob,doc)


class SaveTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);self.project=fixture(self.root)

    def edit(self):
        data=self.project.load('MAP01_A.KMP')
        data['document']['planes'][0]['entries'][0]^=0x400
        return {key:data[key] for key in ('revision','document')}

    def event(self):
        data=self.project.script_data('MAP01_A.SPC')
        call=next(c for c in data['calls'] if c['function']=='SprSet' and c['editable'])
        arg=call['arguments'][-1]
        data['document']['arguments'][str(arg['offset'])]=0
        return {key:data[key] for key in ('name','revision','document')}

    def test_save_reload_and_stale_revision(self):
        data=self.edit();saved=self.project.save('MAP01_A.KMP',data)
        self.assertEqual(saved['document'],data['document'])
        self.assertNotEqual(saved['revision'],data['revision'])
        with self.assertRaisesRegex(ValueError,'changed on disk'):self.project.save('MAP01_A.KMP',data)
        self.assertEqual(saved['document'],self.project.load('MAP01_A.KMP')['document'])

    def test_event_patch_only_changes_verified_immediate(self):
        event=self.event();original=(self.root/self.project.scripts[event['name']]['path']).read_bytes()
        modified=script_events.apply(original,original,event['document'])
        before=script_events.unpack(original);after=script_events.unpack(modified)
        self.assertEqual(len(modified),len(original));self.assertEqual(len(before),len(after))
        offset=int(next(iter(event['document']['arguments'])))
        self.assertEqual(before[:16+offset],after[:16+offset])
        self.assertEqual(before[20+offset:],after[20+offset:])
        self.assertNotEqual(before,after)
        empty=dict(event['document'],arguments={})
        self.assertEqual(script_events.apply(original,original,empty),original)
        empty['arguments']={'0':123}
        with self.assertRaises(ValueError):script_events.apply(original,original,empty)

    def test_overflowing_event_prevents_both_saves(self):
        data=self.edit();event=self.event()
        offset=next(iter(event['document']['arguments']))
        event['document']['arguments'][offset]=257
        data['script']=event
        with self.assertRaisesRegex(ValueError,'allocation'):self.project.save('MAP01_A.KMP',data)
        self.assertFalse((self.root/override_path('MAP01_A.KMP')).exists())
        self.assertFalse((self.root/script_events.patch_path('MAP01_A.SPC')).exists())

    def test_invalid_event_prevents_map_save(self):
        data=self.edit();data['script']=self.event();data['script']['document']['arguments']['0']=123
        with self.assertRaises(ValueError):self.project.save('MAP01_A.KMP',data)
        self.assertFalse((self.root/override_path('MAP01_A.KMP')).exists())

    def test_second_file_failure_rolls_back_first(self):
        data=self.edit();data['script']=self.event()
        import os
        replace=os.replace;count=0
        def fail_second(src,dest):
            nonlocal count
            count+=1
            if count==2:raise OSError('Simulated write failure')
            return replace(src,dest)
        with patch('map_editor.model.os.replace',side_effect=fail_second):
            with self.assertRaises(OSError):self.project.save('MAP01_A.KMP',data)
        self.assertFalse((self.root/override_path('MAP01_A.KMP')).exists())
        self.assertFalse((self.root/script_events.patch_path('MAP01_A.SPC')).exists())
        self.assertFalse(list(self.root.rglob('.editor-*')))

    def test_http_token_host_and_saved_reload(self):
        server,token=make_server(self.project,0)
        thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
        try:
            def request(method,path,body=None,headers=None):
                con=http.client.HTTPConnection('127.0.0.1',server.server_port,timeout=10)
                con.request(method,path,body,headers or {})
                response=con.getresponse();status=response.status;data=response.read();con.close()
                return status,data
            self.assertEqual(request('GET','/api/catalog')[0],403)
            self.assertEqual(request('GET','/api/catalog',headers={'Host':'foreign.example','X-Editor-Token':token})[0],403)
            headers={'X-Editor-Token':token,'Content-Type':'application/json'}
            self.assertEqual(request('GET','/api/catalog',headers=headers)[0],200)
            self.assertEqual(request('GET','/api/map/../../baserom.gba',headers=headers)[0],400)
            data=self.edit()
            self.assertEqual(request('POST','/api/map/MAP01_A.KMP',json.dumps(data),headers)[0],200)
            code,body=request('GET','/api/map/MAP01_A.KMP',headers=headers)
            self.assertEqual(code,200);self.assertNotIn('_base',json.loads(body))
            self.assertEqual(json.loads(body)['document'],data['document'])
            self.assertEqual(request('POST','/api/map/MAP01_A.KMP',json.dumps(data),headers)[0],409)
        finally:server.shutdown();server.server_close();thread.join()


if __name__=='__main__':unittest.main()
