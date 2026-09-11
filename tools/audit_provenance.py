#!/usr/bin/env python3
"""Verify linked providers for declared C ranges; report preserved assembly.

Run after an ordinary matching build. This is read-only: it never repairs the
ROM, substitutes object bytes, changes manifests, or modifies build inputs.
"""
import hashlib
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]


def main():
    base=(ROOT/'baserom.gba').read_bytes();rom=(ROOT/'mar.gba').read_bytes()
    if rom!=base:raise ValueError('ROM differs; refusing to report a matching build')
    text=(ROOT/'build/mar.map').read_text()
    pattern=r'^ \.rom\.([0-9A-Fa-f]+)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(build/\S+\.o)'
    providers={int(off,16):(int(addr,16),int(size,16),obj) for off,addr,size,obj in re.findall(pattern,text,re.M)}
    ranges=[]
    for e in json.loads((ROOT/'src/decompiled.json').read_text()):
        offset=int(e['addr'],16)-0x08000000
        addr,size,obj=providers[offset]
        expected='build/'+str(Path(e['file']).with_suffix('.o'))
        if addr!=offset+0x08000000 or size!=e['size'] or obj!=expected:
            raise ValueError('Unexpected linked provider: '+e['name'])
        kind='inline_assembly_wrapper' if e['file']=='src/bios_calls.c' else 'compiled_c'
        ranges.append(dict(e,provider=obj,implementation=kind,byte_matching=rom[offset:offset+size]==base[offset:offset+size]))
    assembly=list((ROOT/'asm/code').glob('*.s'))
    literal=sum(p.read_text().count('.inst.n') for p in assembly)
    samples=json.loads((ROOT/'sound/samples/manifest.json').read_text())
    region_start,region_end=0xC0,0x1B0000
    pcm_bytes=sum(max(0,min(region_end,e['rom_offset']+e['size'])-max(region_start,e['rom_offset'])) for e in samples)
    region_bytes=region_end-region_start-pcm_bytes
    c_bytes=sum(r['size'] for r in ranges if r['implementation']=='compiled_c')
    progress=dict(compiled_c_functions=sum(r['implementation']=='compiled_c' for r in ranges),
                  compiled_c_owned_bytes=c_bytes,
                  bios_wrapper_functions=sum(r['implementation']=='inline_assembly_wrapper' for r in ranges),
                  bios_wrapper_bytes=sum(r['size'] for r in ranges if r['implementation']=='inline_assembly_wrapper'),
                  code_and_data_region_without_known_pcm_bytes=region_bytes,
                  compiled_c_percent_of_that_region=round(100*c_bytes/region_bytes,4),
                  scope='ROM 0000C0..1B0000 minus known PCM header/payload spans. Includes literals, tables and undecoded data; NOT a pure instruction-byte denominator.',
                  total_function_count=None,
                  limitation='Historical function starts include false positives and internal labels. Do not report a completion percentage from that count. C-owned sizes include alignment and literal pools.')
    record=dict(sha1=hashlib.sha1(rom).hexdigest(),byte_matching=True,
                progress=progress,mapped_ranges=ranges,compiled_c_ranges=sum(r['implementation']=='compiled_c' for r in ranges),
                inline_assembly_ranges=sum(r['implementation']=='inline_assembly_wrapper' for r in ranges),
                preserved_thumb_literal_directives=literal,
                compression_policy='All 104 graphics streams are encoded from PNG/layout sources with matching VRAM-safe LZ77; no graphics LZ templates. Named scripts still preserve compressed originals when unchanged.',
                limitation='Assembly literals, extracted blobs, BIOS inline assembly, and original compressed script streams are not newly compiler-generated C.')
    out=ROOT/'reports/build';out.mkdir(parents=True,exist_ok=True)
    (out/'linked-providers.json').write_text(json.dumps(record,indent=2)+'\n')
    (out/'decompilation-progress.json').write_text(json.dumps(progress,indent=2)+'\n')
    print(f"Verified {len(ranges)} mapped ranges: {record['compiled_c_ranges']} compiled C, {record['inline_assembly_ranges']} inline assembly; {literal} preserved Thumb literal directives elsewhere")


if __name__=='__main__':main()
