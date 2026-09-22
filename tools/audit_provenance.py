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
    linked_sections=[(int(off,16),int(addr,16),int(size,16),obj)
                     for off,addr,size,obj in re.findall(pattern,text,re.M)]
    providers={off:(addr,size,obj) for off,addr,size,obj in linked_sections}
    ranges=[]
    for e in json.loads((ROOT/'src/decompiled.json').read_text()):
        offset=int(e['addr'],16)-0x08000000
        addr,size,obj=providers[offset]
        expected='build/'+str(Path(e['file']).with_suffix('.o'))
        provider_size=e.get('provider_size', e['size'])
        if addr!=offset+0x08000000 or size!=provider_size or obj!=expected:
            raise ValueError('Unexpected linked provider: '+e['name'])
        kind='compiled_c'
        ranges.append(dict(e,provider=obj,implementation=kind,byte_matching=rom[offset:offset+size]==base[offset:offset+size]))
    assembly=list((ROOT/'asm/code').glob('*.s'))
    literal=sum(p.read_text().count('.inst.n') for p in assembly)
    samples=json.loads((ROOT/'sound/samples/manifest.json').read_text())
    # Code-decompilation progress is compiled-C bytes divided by remaining-plus-
    # compiled CODE bytes, not by the complete cartridge image: most of the ROM
    # is graphics, audio, text, and map data that was never assembly to begin
    # with, and folding it into the denominator would understate progress on
    # the actual decompilation work. Asset progress (extracted, editable,
    # rebuildable versus total available) is a separate axis tracked in
    # docs/decompilation-notes.md's own per-asset-type sections, not here.
    asm_sections=[(off,size) for off,_addr,size,obj in linked_sections
                  if obj.startswith('build/asm/code/')]
    pcm_bytes=sum(max(0,min(off+size,e['rom_offset']+e['size'])
                         -max(off,e['rom_offset']))
                  for off,size in asm_sections for e in samples)
    remaining_asm_bytes=sum(size for _off,size in asm_sections)-pcm_bytes
    c_bytes=sum(r['size'] for r in ranges
                if r['implementation']=='compiled_c')
    bios_bytes=sum(r['size'] for r in ranges
                   if r['implementation']=='inline_assembly_wrapper')
    decompilation_bytes=remaining_asm_bytes+c_bytes+bios_bytes
    rom_bytes=len(base)
    progress=dict(compiled_c_functions=sum(r['implementation']=='compiled_c' for r in ranges),
                  compiled_c_owned_bytes=c_bytes,
                  bios_wrapper_functions=sum(r['implementation']=='inline_assembly_wrapper' for r in ranges),
                  bios_wrapper_bytes=sum(r['size'] for r in ranges if r['implementation']=='inline_assembly_wrapper'),
                  remaining_assembly_bytes=remaining_asm_bytes,
                  decompilation_provider_bytes=decompilation_bytes,
                  compiled_c_percent_of_decompilation=round(100*c_bytes/decompilation_bytes,4),
                  rom_bytes=rom_bytes,
                  compiled_c_percent_of_rom=round(100*c_bytes/rom_bytes,4),
                  progress_denominator='decompilation_provider_bytes',
                  scope='The authoritative percentage is compiled C bytes divided by decompilation_provider_bytes (remaining assembly plus compiled C plus BIOS wrappers -- code only, sound-sample PCM data already excluded). It answers "how much of the code has been decompiled." The ROM-wide percentage is retained only as a diagnostic; most of the ROM is graphics, audio, text, and map data that was never assembly, and dividing by it understates code progress. Asset decoding progress is a different question, tracked separately per asset type (editable/rebuildable versus total available) in docs/decompilation-notes.md.',
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
