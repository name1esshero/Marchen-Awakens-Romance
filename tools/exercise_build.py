#!/usr/bin/env python3
"""Exercise the full build with reversible C, frame, cell, animation and font mutations.

Run from the repository root with an unedited matching baseline. This forces
an agbcc rebuild, blocks Python reads of the original ROM during build steps,
checks intentional source edits in the final ROM, and restores sources in a
finally block. Logs and proof results are written to reports/build/.
"""
import hashlib,json,os,subprocess,sys,tempfile
from pathlib import Path
root=Path.cwd();sys.path.insert(0,str(root/'tools'))
import gfx,ncd,scenes,font,wave
report={};out=root/'reports/build';out.mkdir(parents=True,exist_ok=True)
base=(root/'baserom.gba').read_bytes()
with tempfile.TemporaryDirectory(prefix='mar-build-proof-') as tmp:
 guard=Path(tmp)/'sitecustomize.py'
 guard.write_text('''import os,sys\nfrom pathlib import Path\nblocked=Path(os.environ["MAR_BLOCK_ROM"]).resolve()\ndef audit(event,args):\n if event=="open" and isinstance(args[0],(str,bytes)):\n  if Path(os.fsdecode(args[0])).resolve()==blocked:raise RuntimeError("Original ROM read blocked during build")\nsys.addaudithook(audit)\n''')
 env=dict(os.environ,MAR_BLOCK_ROM=str(root/'baserom.gba'))
 env['PYTHONPATH']=tmp+os.pathsep+env.get('PYTHONPATH','')
 def build(label,force=False):
  with (out/(label+'.log')).open('w') as log:
   subprocess.run(['make','-j4']+(['-B'] if force else [])+['all'],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
  return (root/'mar.gba').read_bytes()
 current=build('forced-rebuild',True)
 assert current==base,'Forced rebuild does not match'
 report['forced_rebuild']={'byte_matching':True,'sha1':hashlib.sha1(current).hexdigest(),'python_original_rom_reads':'blocked','command':'make -B -j4 all'}
 print('Forced agbcc rebuild matches with Python reads of baserom blocked',flush=True)
 c=root/'src/entity.c';png=root/'graphics/battle/characters/frames/0000.png';fontpng=root/'graphics/fonts/font.png'
 cell_entry=json.loads((root/'graphics/battle/characters/source/images.json').read_text())[-1]
 cellpng=root/'graphics/battle/characters'/cell_entry['path']
 frame_table=root/'graphics/battle/characters/source/frames.json'
 sample=next(e for e in json.loads((root/'sound/samples/manifest.json').read_text()) if e['status']==0x4000)
 wav=root/sample['wav']
 saved={p:p.read_bytes() for p in (c,png,fontpng,cellpng,frame_table,wav)}
 try:
  text=c.read_text();assert text.count('return entity->unk_20;')==1
  c.write_text(text.replace('return entity->unk_20;','return entity->unk_24;',1))
  blob=ncd.source('CHR');info=ncd.layout(blob)
  px,owners,pal,meta=scenes.render(blob,info,0)
  y,x=next((y,x) for y,row in enumerate(px) for x,v in enumerate(row) if v)
  addr,shift,bank=owners[y][x];px[y][x]=bank*16+(px[y][x]%16)%15+1
  gfx.write_png(str(png),px,pal,transparent_index=0)
  fp,fc=gfx.read_png(str(fontpng));fp[0][0]^=1;gfx.write_png(str(fontpng),fp,fc)
  cp,cc=gfx.read_png(str(cellpng));cp[0][0]^=1;gfx.write_png(str(cellpng),cp,cc,transparent_index=0)
  table=json.loads(frame_table.read_text());table[0]['duration']^=1;frame_table.write_text(json.dumps(table)+'\n')
  with wave.open(str(wav),'rb') as source:
   params=source.getparams();pcm=bytearray(source.readframes(source.getnframes()))
  pcm[sample['loop_start']]^=1
  with wave.open(str(wav),'wb') as dest:
   dest.setparams(params);dest.writeframes(pcm)
  modified=build('source-mutations')
  diff=[i for i,(a,b) in enumerate(zip(base,modified)) if a!=b]
  chr_entry=next(e for e in json.loads((root/'graphics/sprite_containers.json').read_text()) if e['stem']=='CHR')
  font_meta=json.loads((root/'graphics/fonts/font.json').read_text())
  expected_sprite=chr_entry['rom_offset']+addr
  assert len(modified)==len(base)
  assert any(0x32A8<=i<0x32AC for i in diff),'C edit did not reach ROM'
  assert expected_sprite in diff,'Sprite edit did not reach ROM'
  expected_font=font_meta['rom_offset']+font_meta['glyph_offset']
  expected_cell=chr_entry['rom_offset']+info['offsets'][5]+cell_entry['tile']*32
  expected_duration=chr_entry['rom_offset']+info['offsets'][2]+6
  assert set(diff)=={0x32A8,expected_sprite,expected_font,expected_cell,expected_duration,sample['rom_offset']+16+sample['loop_start'],sample['rom_offset']+16+sample['sample_count']},('Unexpected mutation differences',diff)
  result=subprocess.run([sys.executable,'tools/compare.py','baserom.gba','mar.gba'],capture_output=True,text=True)
  assert result.returncode==1,'Comparator accepted an edited ROM'
  (out/'mutation-compare.log').write_text(result.stdout)
  report['source_mutations']={'files':[str(p.relative_to(root)) for p in saved],'differing_offsets':[f'{i:06X}' for i in diff],'differing_bytes':len(diff),'compare_exit_code':result.returncode}
  print('C, frame, cell, duration, font and WAV edits reached final ROM; compare rejected all 7 changed bytes',flush=True)
 finally:
  for path,raw in saved.items():path.write_bytes(raw)
  restored=build('restored-rebuild')
  assert restored==base,'Restored sources no longer match'
  report['restored_rebuild']={'byte_matching':True,'sha1':hashlib.sha1(restored).hexdigest()}
  (out/'provenance.json').write_text(json.dumps(report,indent=2)+'\n')
  print('Restored original sources and matching ROM',flush=True)
