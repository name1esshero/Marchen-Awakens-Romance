#!/usr/bin/env python3
"""Read-only scan of linked section tails and the region after the NFP archive.

Signatures are candidates, not proof of file type. No extraction/build inputs
are altered. Bounded decompression avoids trusting arbitrary length fields.
"""
import collections,hashlib,json,struct
from pathlib import Path
import lz77,nfp
ROOT=Path(__file__).resolve().parents[1]

def main():
 rom=(ROOT/'baserom.gba').read_bytes();members=nfp.entries(rom)
 manifest=json.loads((ROOT/'data/rom_data_sections.json').read_text())
 spans=[(int(e['start'],16),int(e['end'],16),e.get('source',e['group']))
        for e in manifest['sections']]
 windows=[(max(a,b-256),b) for a,b,_ in spans]+[(nfp.END,len(rom))]
 signatures=[]
 for magic in (b'NFP2.0',b'SCRP',b'RIFF',b'KMP',b'NC',b'KCG'):
  pos=0
  while (pos:=rom.find(magic,pos))>=0:
   if any(a<=pos<b for a,b in windows):signatures.append(dict(offset=hex(pos),signature=magic.decode(),status='unverified_signature'))
   pos+=1
 candidates=[]
 for a,b in windows:
  for off in range((a+3)&~3,b-4,4):
   size=int.from_bytes(rom[off+1:off+4],'little')
   if rom[off]!=16 or not 32<=size<=262144:continue
   try:raw,used=lz77.decompress(rom[off:min(b,off+524288)])
   except (ValueError,IndexError):continue
   if used>=size:continue
   candidates.append(dict(archive_owners=[e['name'] for e in nfp.owners(members,off,off+1)],offset=hex(off),raw_size=size,compressed_size=used,status='valid_lz_candidate_not_classified',sha256=hashlib.sha256(raw).hexdigest()))
 tails=[]
 for a,b,p in sorted(spans):
  end=b;value=rom[b-1]
  while end>a and rom[end-1]==value:end-=1
  tails.append(dict(start=hex(a),end=hex(b),provider=p,trailing_run=dict(start=hex(end),size=b-end,value=value),archive_owners=[e['name'] for e in nfp.owners(members,max(a,b-256),b)]))
 tail_bytes=rom[0xFE3420:0xFFFF00]
 tail_words=collections.Counter(struct.unpack('<%dI'%(len(tail_bytes)//4),tail_bytes))
 report=dict(final_raw_section_word_counts={hex(k):v for k,v in tail_words.items()},scope='Last 256 bytes of every ROM-data manifest span, plus all bytes after NFP END',after_archive_start=hex(nfp.END),section_tails=tails,signature_candidates=signatures,lz_candidates=list({c['offset']:c for c in candidates}.values()),limitation='Candidate scan does not prove absence of other assets or identify formats from signatures alone.')
 out=ROOT/'reports/rom/tails.json';out.parent.mkdir(exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n')
 print(f'{len(tails)} section tails; {len(signatures)} signatures; {len(report["lz_candidates"])} compressed candidates')
if __name__=='__main__':main()
