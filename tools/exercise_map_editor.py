#!/usr/bin/env python3
"""Prove map-editor saves reach the ROM, then remove them and restore matching.

Run alone; refuses to overwrite existing MAP01_A editor overrides. Tests a
map flip, an attribute, and a literal SprSet argument. Build subprocesses are
blocked from reading baserom.gba. No extraction or post-link patching occurs.
"""
import copy
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from map_editor.model import Project,compile_map,override_path
import named_scripts
import script_events

ROOT=Path(__file__).resolve().parents[1]


def main():
    project=Project(ROOT);name='MAP01_A.KMP';script_name='MAP01_A.SPC'
    paths=[ROOT/override_path(name),ROOT/script_events.patch_path(script_name)]
    if any(p.exists() for p in paths):raise SystemExit('Existing editor overrides; preserve them before running this proof')
    base=(ROOT/'baserom.gba').read_bytes();report={}
    out=ROOT/'reports/build';out.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='mar-map-editor-proof-') as temp:
        Path(temp,'sitecustomize.py').write_text('import os,sys\nfrom pathlib import Path\nblocked=Path(os.environ["MAR_BLOCK_ROM"]).resolve()\ndef audit(event,args):\n if event=="open" and isinstance(args[0],(str,bytes)) and Path(os.fsdecode(args[0])).resolve()==blocked:raise RuntimeError("Original ROM read blocked during build")\nsys.addaudithook(audit)\n')
        env=dict(os.environ,MAR_BLOCK_ROM=str(ROOT/'baserom.gba'),PYTHONPATH=temp+os.pathsep+os.environ.get('PYTHONPATH',''))
        def build(label,english=False):
            with (out/(label+'.log')).open('w') as log:
                subprocess.run(['make','-j4','all']+(['english'] if english else []),cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
            return (ROOT/'mar.gba').read_bytes()
        assert build('editor-baseline',True)==base
        print('Baseline Japanese matches; English build completed',flush=True)
        data=project.load(name);document=copy.deepcopy(data['document'])
        document['planes'][0]['entries'][0]^=0x400
        document['attributes']['entries'][10]^=1
        event=project.script_data(script_name)
        call=next(c for c in event['calls'] if c['function']=='SprSet' and c['editable'] and [a['value'] for a in c['arguments']]==[1,0,256])
        offset=call['arguments'][2]['offset'];event['document']['arguments'][str(offset)]=0
        payload=dict(revision=data['revision'],document=document,script={k:event[k] for k in ('name','revision','document')})
        member=project.members[name];entry=project.scripts[script_name]
        original=(ROOT/entry['path']).read_bytes()
        expected_map=compile_map(data['_base'],document)
        expected_script=script_events.apply(named_scripts.rebuild(original,named_scripts.edits(ROOT/entry['text'])),original,event['document'])
        try:
            project.save(name,payload)
            modified=build('editor-mutated')
            assert modified[member['rom_offset']:member['rom_offset']+member['size']]==expected_map
            assert modified[entry['rom_offset']:entry['rom_offset']+len(original)]==expected_script
            differences=[i for i,(a,b) in enumerate(zip(base,modified)) if a!=b]
            assert len(modified)==len(base) and differences
            assert all(member['rom_offset']<=i<member['rom_offset']+member['size'] or entry['rom_offset']<=i<entry['rom_offset']+len(original) for i in differences)
            report['mutation']=dict(map=name,script=script_name,native_call='SprSet(1, 0, 256) -> SprSet(1, 0, 0)',
                                    differing_bytes=len(differences),differing_offsets=[f'{i:06X}' for i in differences],
                                    map_and_script_spans_match_source_compilation=True)
            print('Saved tile flip, raw attribute, and event argument reached only their ROM spans',flush=True)
        finally:
            for path in paths:path.unlink(missing_ok=True)
            restored=build('editor-restored',True)
            assert restored==base,'Removing overrides did not restore the matching ROM'
            report['restored']=dict(byte_matching=True,sha1=hashlib.sha1(restored).hexdigest(),
                                    override_deletion_detected_by_make=True,python_original_rom_reads='blocked')
            (out/'map-editor-proof.json').write_text(json.dumps(report,indent=2)+'\n')
            print('Overrides removed; directory dependencies rebuilt the byte-matching ROM',flush=True)


if __name__=='__main__':main()
