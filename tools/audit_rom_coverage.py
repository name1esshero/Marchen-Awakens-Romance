#!/usr/bin/env python3
"""Audit linked ROM coverage without interpreting raw sections as decoded data."""
import collections,hashlib,json,re
from pathlib import Path
import nfp
ROOT=Path(__file__).resolve().parents[1]

def main():
 base=(ROOT/'baserom.gba').read_bytes();built=(ROOT/'mar.gba').read_bytes()
 if base!=built:raise ValueError('Built Japanese ROM differs from original')
 text=(ROOT/'build/mar.map').read_text()
 pattern=r'^ \.rom\.([0-9A-Fa-f]+)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(build/\S+\.o)'
 spans=[]
 for off,addr,size,obj in re.findall(pattern,text,re.M):
  a=int(off,16);n=int(size,16)
  if int(addr,16)!=a+0x08000000:raise ValueError('Wrong linked address: '+obj)
  if a+n>len(base):raise ValueError('Section beyond ROM: '+obj)
  spans.append(dict(start=a,end=a+n,size=n,provider=obj))
 spans.sort(key=lambda s:s['start']);gaps=[];overlaps=[];end=0
 for s in spans:
  if s['start']>end:gaps.append(dict(start=end,end=s['start'],size=s['start']-end,byte_values=sorted(set(base[end:s['start']]))))
  if s['start']<end and s['size']:overlaps.append(s)
  end=max(end,s['end'])
 if end<len(base):gaps.append(dict(start=end,end=len(base),size=len(base)-end,byte_values=sorted(set(base[end:]))))
 directory=nfp.entries(base);aliases=collections.defaultdict(list)
 for e in directory:aliases[e['rom_offset']].append(e['name'])
 report=dict(sha1=hashlib.sha1(base).hexdigest(),rom_size=len(base),byte_matching=True,linked_sections=len(spans),gaps=gaps,overlaps=overlaps,archive_member_count=len(directory),archive_aliases={hex(k):v for k,v in aliases.items() if len(v)>1},raw_linked_sections=[s for s in spans if (ROOT/s['provider'].replace('build/','',1)).with_suffix('.s').exists() and 'build/data/' in (ROOT/s['provider'].replace('build/','',1)).with_suffix('.s').read_text()],limitation='Link coverage and equality do not establish semantic decoding. Raw providers remain raw even when byte matching.')
 out=ROOT/'reports/rom/coverage.json';out.parent.mkdir(exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:v for k,v in report.items() if k!='raw_linked_sections'},indent=2))
 if overlaps:raise ValueError('Overlapping linked sections')
if __name__=='__main__':main()
