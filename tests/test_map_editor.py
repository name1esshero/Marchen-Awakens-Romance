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
from map_editor.model import Project,compile_map,map_document,override_path,initial_sprite_placements
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
    def test_script_created_sprite_gets_its_first_literal_position(self):
        def call(offset,name,*values):
            args=[{'kind':'string' if isinstance(v,str) else 'integer','value':v}
                  for v in values]
            return {'offset':offset,'function':name,'decoded_arguments':args}
        calls=[call(10,'SprInit',7,0,'PS_WK02',0,0),
               call(20,'SprSet',7,0,200),call(30,'SprSet',7,1,412),
               call(40,'SprSet',7,0,999)]
        self.assertEqual(initial_sprite_placements(calls),[{
            'sprite':7,'container':0,'resource':'PS_WK02','animation':0,
            'x':200,'y':412,'init_offset':10}])

    def test_map_script_exposes_renderable_initial_sprites(self):
        data=Project(ROOT).script_data('MAP01_A.SPC')
        sprite=next(item for item in data['sprite_placements'] if item['resource']=='PS_WK02')
        self.assertEqual((sprite['x'],sprite['y']),(200,412))
        self.assertIsInstance(sprite['x_argument_offset'],int)
        self.assertIsInstance(sprite['y_argument_offset'],int)
        self.assertTrue(sprite['preview']['image'].startswith('data:image/png;base64,'))
        self.assertTrue(sprite['preview']['frames'])
        self.assertTrue(all(frame['duration']>0 for frame in sprite['preview']['frames']))
        candidate=next(item for item in data['sprite_candidates'] if item['resource']=='PS_WK02')
        self.assertTrue(candidate['placed'])
        self.assertTrue(candidate['preview']['image'].startswith('data:image/png;base64,'))

    def test_all_maps_and_traced_scenes_round_trip(self):
        project=Project(ROOT);catalog=project.catalog()
        # Every extracted KMP is attempted. EFCSTART.KMP is a known, genuine
        # exception: its header names a "TEST_M00.KCG" tile source that was
        # never extracted as an asset, so it stays unsupported rather than
        # being force-decoded against the wrong tiles.
        self.assertGreaterEqual(len(catalog['maps']),192)
        self.assertEqual(catalog['unsupported'],{'EFCSTART.KMP':'No verified editable 4bpp tile source'})
        for name in ('MAP27_A.KMP','PW_BG01.KMP','PW_BOX.KMP'):
            data=project.load(name)
            self.assertFalse(data['unresolved'])
            self.assertTrue(data['resolved_cells'])
            self.assertTrue(all(cell['resolution']['kind']=='transparent' for cell in data['resolved_cells']))
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

    def test_script_analysis_exposes_field_and_sprite_calls(self):
        project=Project(ROOT)
        field=None;sprite=None
        for name in project.scripts:
            analysis=project.script_data(name)['analysis']
            field=field or next((x for x in analysis['field_loads'] if isinstance(x['destination'],str)),None)
            sprite=sprite or next((x for x in analysis['sprite_resources'] if isinstance(x['resource'],str)),None)
            if field and sprite:break
        self.assertTrue(field['destination'].startswith('MAP'))
        self.assertIn(sprite['operation'],('SprInit','SprChg'))
        self.assertTrue(sprite['resource'])
        catalog=project.catalog()
        self.assertEqual(catalog['script_totals']['field_loads'],113)
        self.assertTrue(any(m['incoming_field_loads'] for m in catalog['maps']))

    def test_script_chain_exposes_dorothy_event_resources(self):
        project=Project(ROOT)
        parent=project.script_data('CH_M01_3.SPC')
        links={link['script'] for link in parent['analysis']['script_links']}
        self.assertIn('EV_BA03.SPC',links)
        child=project.script_data('EV_BA03.SPC')
        names={item['display_name'] for item in child['sprite_candidates']}
        self.assertIn('Dorothy (moving)',names)

    def test_map_event_sources_follow_verified_script_chains(self):
        project=Project(ROOT)
        data=project.load('MAP01_3A.KMP')
        sources={item['script']:item for item in data['event_sources']}
        self.assertIn('MAP01_3A.SPC',sources)
        self.assertIn('CH_M01_3.SPC',sources)
        self.assertIn('EV_BA03.SPC',sources)
        self.assertIn('EV_BA04.SPC',sources)
        self.assertEqual(sources['EV_BA03.SPC']['relation'],'script_chain')
        self.assertEqual(sources['EV_BA03.SPC']['confidence'],'verified')
        self.assertGreater(sources['EV_BA03.SPC']['depth'],0)
        self.assertEqual(sources['EV_BA03.SPC']['event_counts']['sprite_resources'],2)
        self.assertEqual(sources['EV_BA03.SPC']['event_counts']['sprite_moves'],1)

    def test_event_model_has_stable_source_ids(self):
        data=Project(ROOT).script_data('MAP01_A.SPC')
        model=data['event_model']
        self.assertEqual(model['version'],1)
        self.assertEqual(model['script'],'MAP01_A.SPC')
        self.assertEqual(len(model['items']),len(data['calls']))
        self.assertEqual(len({item['id'] for item in model['items']}),len(model['items']))
        for item,call in zip(model['items'],data['calls']):
            self.assertEqual(item['id'],call['event_id'])
            self.assertEqual(item['source_offset'],call['offset'])

    def test_numbered_map_exposes_companion_spawn_and_character_scripts(self):
        data=Project(ROOT).load('MAP01_3A.KMP')
        relations={item['script']:item for item in data['script_associations']}
        self.assertEqual(relations['MAP01_3A.SPC']['confidence'],'verified')
        self.assertEqual(relations['SP_M01_3.SPC']['relation'],'spawn')
        self.assertEqual(relations['CH_M01_3.SPC']['relation'],'character_event')
        self.assertEqual(relations['CH_M01_3.SPC']['confidence'],'inferred')

    def test_loaded_map_exposes_incoming_field_dependencies(self):
        data=Project(ROOT).load('MAP00_A.KMP')
        self.assertTrue(data['incoming_field_loads'])
        self.assertTrue(all(link['destination']=='MAP00_A' for link in data['incoming_field_loads']))
        self.assertIn('OPEN.SPC',data['scripts'])

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


class MarscriptTabTests(unittest.TestCase):
    """The Scripts tab's model-layer contract: marscript_data()/save_marscript(),
    and script_data()/save()'s awareness of a marscript override once one
    exists. tools/marscript_rom_build.py's own tests cover the build-time
    placement decision this feeds into; these cover the editor's view of it.
    """
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);self.project=fixture(self.root)

    def test_no_override_matches_script_data_exactly(self):
        marscript_view=self.project.marscript_data('MAP01_A.SPC')
        self.assertFalse(marscript_view['has_override'])
        self.assertIn('script MAP01_A_SPC',marscript_view['source'])
        self.assertFalse(self.project.script_data('MAP01_A.SPC')['has_marscript_override'])

    def test_save_persists_and_updates_events_view(self):
        before=self.project.marscript_data('MAP01_A.SPC')
        saved=self.project.save_marscript('MAP01_A.SPC',{'revision':before['revision'],'source':before['source']})
        self.assertTrue(saved['has_override'])
        self.assertTrue((self.root/'scripts/marscript/MAP01_A.SPC.marscript').exists())
        self.assertEqual(saved['compiled_size'],saved['original_slot_size'],
                         'an unedited decompile/recompile must round-trip to the exact original size')
        events_view=self.project.script_data('MAP01_A.SPC')
        self.assertTrue(events_view['has_marscript_override'])
        self.assertGreater(len(events_view['calls']),0)

    def test_stale_revision_rejected(self):
        self.project.save_marscript('MAP01_A.SPC',
            {'revision':self.project.marscript_data('MAP01_A.SPC')['revision'],
             'source':self.project.marscript_data('MAP01_A.SPC')['source']})
        with self.assertRaisesRegex(ValueError,'changed on disk'):
            self.project.save_marscript('MAP01_A.SPC',{'revision':'stale','source':'script X { end }'})

    def test_invalid_source_rejected_before_writing(self):
        before=self.project.marscript_data('MAP01_A.SPC')
        with self.assertRaises(ValueError):
            self.project.save_marscript('MAP01_A.SPC',{'revision':before['revision'],'source':'not marscript'})
        self.assertFalse((self.root/'scripts/marscript/MAP01_A.SPC.marscript').exists())

    def test_reset_removes_override(self):
        before=self.project.marscript_data('MAP01_A.SPC')
        saved=self.project.save_marscript('MAP01_A.SPC',{'revision':before['revision'],'source':before['source']})
        result=self.project.save_marscript('MAP01_A.SPC',{'revision':saved['revision'],'reset':True})
        self.assertFalse(result['has_override'])
        self.assertFalse((self.root/'scripts/marscript/MAP01_A.SPC.marscript').exists())
        self.assertFalse(self.project.script_data('MAP01_A.SPC')['has_marscript_override'])

    def test_growth_past_original_slot_is_allowed(self):
        before=self.project.marscript_data('MAP01_A.SPC')
        grown=before['source'].rstrip().removesuffix('}')+'    add r0, r0\n'*400+'}\n'
        saved=self.project.save_marscript('MAP01_A.SPC',{'revision':before['revision'],'source':grown})
        self.assertTrue(saved['has_override'])
        self.assertGreater(saved['compiled_size'],saved['original_slot_size'])

    def test_map_save_skips_empty_event_argument_override_once_marscript_exists(self):
        before=self.project.marscript_data('MAP01_A.SPC')
        self.project.save_marscript('MAP01_A.SPC',{'revision':before['revision'],'source':before['source']})
        map_data=self.project.load('MAP01_A.KMP')
        script_view=self.project.script_data('MAP01_A.SPC')
        payload={'revision':map_data['revision'],'document':map_data['document'],
                 'script':{'name':'MAP01_A.SPC','revision':script_view['revision'],'document':script_view['document']}}
        self.project.save('MAP01_A.KMP',payload)
        self.assertFalse((self.root/script_events.patch_path('MAP01_A.SPC')).exists(),
                         'saving the map must not create a pointless maps/events override once a '
                         'marscript override is the real source of truth for this script')


if __name__=='__main__':unittest.main()
