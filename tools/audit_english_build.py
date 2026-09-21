#!/usr/bin/env python3
"""Verify the optional English ROM bridge, expansion symbols and change scope.

This is a static link audit. C lookup tests cover behavior; neither replaces
an emulator playthrough of translated scenes and other text printer paths.
"""
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def main():
    base=(ROOT/'baserom.gba').read_bytes()
    localized=(ROOT/'mar_english.gba').read_bytes()
    if len(localized)!=0x2000000:raise ValueError('English ROM must be 32 MB')
    differences=[i for i,(a,b) in enumerate(zip(base,localized)) if a!=b]
    import sys
    sys.path.insert(0,str(ROOT/'tools'))
    import sprite_sources, ncd, english_credits
    japanese=sprite_sources.compile('SYSTEM')
    english=sprite_sources.compile('SYSTEM',english=True)
    start=0xDD69E0;end=0xF12390
    if japanese!=base[start:end]:raise ValueError('Japanese SYSTEM source does not match original')
    if english!=localized[start:end]:raise ValueError('Linked English SYSTEM differs from compiled PNG sources')
    import scenes
    manifest=json.loads((ROOT/'graphics/ui/manifest.json').read_text())
    info=ncd.layout(japanese);changed_frames=[]
    for frame in manifest['frames']:
        first,count,_=struct.unpack_from('<IHH',japanese,info['offsets'][2]+frame['id']*8)
        cells=[ncd.cell(japanese,info,first+k) for k in range(count)]
        metadata_start=info['offsets'][3]+first*20
        metadata_end=metadata_start+count*20
        if japanese[metadata_start:metadata_end]==english[metadata_start:metadata_end] and not any(japanese[c['start']:c['end']]!=english[c['start']:c['end']] for c in cells):continue
        before=scenes.render(japanese,info,frame['id'])[0]
        after=scenes.render(english,info,frame['id'])[0]
        if before==after:continue
        source=ROOT/'graphics/ui'/frame['path']
        if not source.with_stem(source.stem+'_en').exists():
            raise ValueError('English tiles unexpectedly alter unlocalized frame '+str(frame['id']))
        changed_frames.append(frame['id'])
    tile_start=ncd.layout(japanese)['offsets'][5]
    credit_metadata=english_credits.metadata_offsets(info)
    # Explicit credit OAM positions/shapes/tile pointers and the derived tile
    # reference count may change; every palette and all other metadata must match.
    if any(a!=b and i not in credit_metadata for i,(a,b) in enumerate(zip(japanese[:tile_start],english[:tile_start]))):
        raise ValueError('English UI changed metadata outside credit layouts or changed palettes')
    effect_jp=sprite_sources.compile('EFFECT')
    effect_en=sprite_sources.compile('EFFECT',english=True)
    effect_start=0x5AEAF0;effect_end=effect_start+len(effect_jp)
    effect_tiles=ncd.layout(effect_jp)['offsets'][5]
    if effect_jp!=base[effect_start:effect_end]:raise ValueError('Japanese EFFECT differs from original')
    if effect_en!=localized[effect_start:effect_end]:raise ValueError('English EFFECT differs from PNG sources')
    if effect_jp[:effect_tiles]!=effect_en[:effect_tiles]:raise ValueError('English effects changed metadata or palettes')
    effect_info=ncd.layout(effect_jp);effect_changed=[]
    for frame in json.loads((ROOT/'graphics/battle/effects/manifest.json').read_text())['frames']:
        before=scenes.render(effect_jp,effect_info,frame['id'])[0]
        after=scenes.render(effect_en,effect_info,frame['id'])[0]
        if before==after:continue
        source=ROOT/'graphics/battle/effects'/frame['path']
        variant=source.with_stem(source.stem+'_en')
        import gfx
        if not variant.exists() or after!=gfx.read_png(str(variant))[0]:
            raise ValueError('English effects alter an unlocalized or incorrectly rendered frame: '+str(frame['id']))
        effect_changed.append(frame['id'])
    import english_backgrounds
    assets = {e.get('archive_name'): e for e in json.loads((ROOT/'assets.json').read_text())}
    maps = {e['name']: e for e in json.loads((ROOT/'maps/nfp/manifest.json').read_text())}
    background_spans = []
    background_sources = []
    for name in english_backgrounds.NAMES:
        entry = assets[name]
        pixels, map_data, overrides = english_backgrounds.compile_entry(entry)
        map_entry = maps[name.replace('.KCG', '.KMP')]
        for offset, data in ((entry['rom_offset'], pixels), (map_entry['rom_offset'], map_data)):
            if localized[offset:offset+len(data)] != data:
                raise ValueError('Linked English background differs from PNG/map source: ' + name)
            background_spans.append((offset, offset+len(data)))
        background_sources.append(dict(name=name, overrides=overrides))
    bridge_spans = (
        (0x11790, 0x11870),
        (0x56474, 0x56494),
        (0x57108, 0x57138),
        (0x5715C, 0x57174),
    )
    if not differences or any(not (any(a<=i<b for a,b in background_spans) or any(a<=i<b for a,b in bridge_spans) or 0x88460<=i<0x885B0 or start+tile_start<=i<end or i-start in credit_metadata or effect_start+effect_tiles<=i<effect_end) for i in differences):
        raise ValueError('English build changed bytes outside verified text bridges and UI tiles')
    output=subprocess.check_output(['arm-none-eabi-nm','-n','build/english/mar_english.elf'],cwd=ROOT,text=True)
    symbols={fields[2]:int(fields[0],16) for line in output.splitlines() if len(fields:=line.split())==3}
    names=('EnglishDialogueStart','DialogueStartOriginal','EnglishTranslateRows',
           'EnglishTranslateSingle','EnglishItemGetName','EnglishItemGetDescription',
           'EnglishConsumableGetName','EnglishConsumableGetDescription',
           'EnglishConsumableGetResourceName','EnglishPageTask',
           'EnglishClearPage','gEnglishRows','gEnglishRowCount')
    for name in names:
        if not 0x09000000<=symbols[name]<0x0A000000:raise ValueError(name+' not in English extension')
    # The extension linker script supplies ROM only. Mutable pagination state
    # must stay in engine task allocations, not silently become orphan RAM data.
    mutable_sections=[]
    for obj in sorted((ROOT/'build/english').glob('*.o')):
        sections=subprocess.check_output(['arm-none-eabi-size','-A',str(obj)],text=True)
        for line in sections.splitlines():
            fields=line.split()
            if len(fields)==3 and fields[0].startswith(('.data','.bss','.sdata','.sbss')) and int(fields[1]):
                mutable_sections.append(dict(object=obj.name,section=fields[0],size=int(fields[1])))
    if mutable_sections:raise ValueError('Unallocated mutable English sections: '+str(mutable_sections))
    bridge=localized[0x11790:0x11870]
    target=symbols['EnglishDialogueStart']|1
    if struct.pack('<I',target) not in bridge:raise ValueError('Bridge lacks Thumb entry pointer')
    item_bridge=localized[0x56474:0x56484]
    item_target=symbols['EnglishItemGetName']|1
    if struct.pack('<I',item_target) not in item_bridge:raise ValueError('Item-name bridge lacks Thumb entry pointer')
    description_bridge=localized[0x56484:0x56494]
    description_target=symbols['EnglishItemGetDescription']|1
    if struct.pack('<I',description_target) not in description_bridge:raise ValueError('Item-description bridge lacks Thumb entry pointer')
    consumable_name_bridge=localized[0x57108:0x57120]
    consumable_name_target=symbols['EnglishConsumableGetName']|1
    if struct.pack('<I',consumable_name_target) not in consumable_name_bridge:raise ValueError('Consumable-name bridge lacks Thumb entry pointer')
    consumable_description_bridge=localized[0x57120:0x57138]
    consumable_description_target=symbols['EnglishConsumableGetDescription']|1
    if struct.pack('<I',consumable_description_target) not in consumable_description_bridge:raise ValueError('Consumable-description bridge lacks Thumb entry pointer')
    consumable_resource_name_bridge=localized[0x5715C:0x57174]
    consumable_resource_name_target=symbols['EnglishConsumableGetResourceName']|1
    if struct.pack('<I',consumable_resource_name_target) not in consumable_resource_name_bridge:raise ValueError('Consumable-resource-name bridge lacks Thumb entry pointer')
    mappings=json.loads((ROOT/'reports/text/english-runtime.json').read_text())
    report=dict(sha1=hashlib.sha1(localized).hexdigest(),size=len(localized),
                original_area_differing_bytes=len(differences),
                change_scope='Dialogue constructor, item and consumable name/description bridges, English menu labels, independently rebuilt EFFECT and SYSTEM UI tiles and explicit English credit layouts, rebuilt startup/shop background tiles and maps; C and strings in ROM expansion',
                english_backgrounds=background_sources,
                english_effect_changed_frames=effect_changed,
                english_effect_variants=[str(p.relative_to(ROOT)) for p in sorted((ROOT/'graphics/battle/effects/frames').glob('*_en.png'))],
                english_credit_layout_frames=sorted(english_credits.frame_ids()),
                english_ui_variants=[str(p.relative_to(ROOT)) for p in sorted((ROOT/'graphics/ui').rglob('*_en.png'))],
                english_ui_differing_bytes=sum(a!=b for a,b in zip(japanese,english)),
                english_ui_changed_frames=changed_frames,
                entry_points={name:f'{symbols[name]:08X}' for name in names},
                exact_row_mappings=mappings['exact_row_mappings'],
                unallocated_mutable_sections=mutable_sections,
                emulator_playthrough_verified=False,
                limitation='Static link validation plus separate host C tests; does not establish complete scene or printer coverage.')
    (ROOT/'reports/build/english-link-audit.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
