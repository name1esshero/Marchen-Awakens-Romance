"""Source-level map/event editor model. All writes are validated before commit."""
import hashlib
import base64
import json
from pathlib import Path
import struct
import sys
import tempfile
import os
import re

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import gfx
import mapped_images
import build_assets
import script_events
import named_scripts

ROOT=Path(__file__).resolve().parents[2]

SPRITE_CONTAINERS = {
    0: Path('graphics/ui/manifest.json'),
    1: Path('graphics/battle/effects/manifest.json'),
    2: Path('graphics/battle/characters/manifest.json'),
}

# Character 09 is tied to Dorothy by the matching F09 portrait/dialogue calls;
# A00 is the static pose and A01 is the six-frame movement sequence.
SPRITE_RESOURCE_NAMES = {'09A00': 'Dorothy (idle)', '09A01': 'Dorothy (moving)'}


def read(path):return json.loads(path.read_text())
def sha(data):return hashlib.sha256(data).hexdigest()
def integer(value,low,high):
    if type(value)!=int or not low<=value<=high:raise ValueError('Integer outside supported range')
    return value


def map_document(blob):
    if len(blob)<0xC0:raise ValueError('Truncated KMP header')
    w,h=struct.unpack_from('<2I',blob,0x14)
    if not 0<w<=1024 or not 0<h<=1024 or w*h>262144:raise ValueError('Unsupported dimensions')
    planes=[]
    for index,offset in enumerate(struct.unpack_from('<4I',blob,0x9C)):
        if not offset:continue
        if offset<0xC0 or offset+2*w*h>len(blob):raise ValueError('Invalid KMP plane span')
        planes.append(dict(index=index,offset=offset,entries=list(struct.unpack_from('<'+'H'*(w*h),blob,offset))))
    offset=struct.unpack_from('<I',blob,0xAC)[0];word_size=2 if struct.unpack_from('<H',blob,0xBA)[0] else 1
    attributes=None
    if offset:
        if offset<0xC0 or offset+w*h*word_size>len(blob):raise ValueError('Invalid attribute span')
        attributes=dict(offset=offset,word_size=word_size,entries=list(struct.unpack_from('<'+('H' if word_size==2 else 'B')*(w*h),blob,offset)))
    return dict(version=1,source_sha256=sha(blob),width=w,height=h,planes=planes,attributes=attributes)


def compile_map(blob,doc):
    if not isinstance(doc,dict):raise ValueError('Invalid map document')
    base=map_document(blob)
    for field in ('version','source_sha256','width','height'):
        if doc.get(field)!=base[field]:raise ValueError('KMP source/header revision differs')
    if len(doc.get('planes',[]))!=len(base['planes']):raise ValueError('Plane count changed')
    out=bytearray(blob);spans=[]
    for source,plane in zip(base['planes'],doc['planes']):
        if (plane['index'],plane['offset'])!=(source['index'],source['offset']):raise ValueError('Plane layout changed')
        if len(plane['entries'])!=base['width']*base['height']:raise ValueError('Wrong plane length')
        words=[integer(v,0,65535) for v in plane['entries']]
        packed=struct.pack('<'+'H'*len(words),*words);spans.append((plane['offset'],packed))
    attrs=doc.get('attributes')
    if bool(attrs)!=bool(base['attributes']):raise ValueError('Attribute layout changed')
    if attrs:
        for field in ('offset','word_size'):
            if attrs[field]!=base['attributes'][field]:raise ValueError('Attribute layout changed')
        if len(attrs['entries'])!=base['width']*base['height']:raise ValueError('Wrong attribute length')
        values=[integer(v,0,65535 if attrs['word_size']==2 else 255) for v in attrs['entries']]
        spans.append((attrs['offset'],struct.pack('<'+('H' if attrs['word_size']==2 else 'B')*len(values),*values)))
    # Aliased planes/attributes are legal only if proposed bytes agree.
    written={}
    for offset,data in spans:
        for i,value in enumerate(data):
            at=offset+i
            if at in written and written[at]!=value:raise ValueError('Conflicting edits to overlapping KMP spans')
            written[at]=value;out[at]=value
    return bytes(out)


def override_path(name):
    if Path(name).name!=name or not name.endswith('.KMP'):raise ValueError('Invalid map name')
    return Path('maps/editable')/(name+'.json')


def apply_override(blob,name,root):
    path=Path(root)/override_path(name)
    return compile_map(blob,read(path)) if path.exists() else blob


def initial_sprite_placements(calls):
    """Project literal SprInit/SprSet setup sequences into map coordinates."""
    active={};result=[]
    for call in calls:
        args=call.get('decoded_arguments',[])
        values=[a.get('value') if a.get('kind') in ('integer','string') else None
                for a in args]
        if call.get('function')=='SprInit' and len(values)==5 and isinstance(values[0],int):
            active[values[0]]=dict(sprite=values[0],container=values[1],resource=values[2],
                                   animation=values[3],x=None,y=None,init_offset=call['offset'],emitted=False)
        elif call.get('function')=='SprChg' and len(values)==5 and isinstance(values[0],int) and values[0] in active:
            active[values[0]].update(container=values[1],resource=values[2],animation=values[3])
        elif call.get('function')=='SprSet' and len(values)==3 and isinstance(values[0],int):
            item=active.get(values[0]);prop=values[1];value=values[2]
            if item and prop in (0,1) and isinstance(value,int):
                item['xy'[prop]]=((value+0x8000)&0xFFFF)-0x8000
                if len(args)>2 and isinstance(args[2].get('offset'),int):
                    item['xy'[prop]+'_argument_offset']=args[2]['offset']
                if item['x'] is not None and item['y'] is not None and not item['emitted']:
                    item['emitted']=True
                    result.append({k:v for k,v in item.items() if k!='emitted'})
    return result


class Project:
    def __init__(self,root=ROOT):
        self.root=Path(root)
        context_path=self.root/'maps/runtime_scenes.json'
        scene_maps={layer['map'] for scene in read(context_path)['scenes'] for layer in scene['layers']} if context_path.exists() else set()
        self.members={e['name']:e for e in read(self.root/'maps/nfp/manifest.json') if (e['name'].startswith('MAP') or e['name'] in scene_maps) and e['name'].endswith('.KMP')}
        self.assets={e['archive_name']:e for e in read(self.root/'assets.json') if e.get('archive_name')}
        self.scripts={e['name']:e for e in read(self.root/'scripts/nfp/manifest.json')}
        resolution_path=self.root/'maps/tile_resolutions.json'
        self.tile_resolution_data=read(resolution_path) if resolution_path.exists() else {'version':1,'maps':{}}
        if self.tile_resolution_data.get('version')!=1:raise ValueError('Unsupported tile-resolution revision')
        catalog_path=self.root/'maps/script_catalog.json'
        self.script_catalog=read(catalog_path) if catalog_path.exists() else {'scripts':[],'totals':{}}
        self.incoming={}
        for script in self.script_catalog['scripts']:
            for link in script['field_loads']:
                destination=link.get('destination')
                if isinstance(destination,str):
                    self.incoming.setdefault(destination+'.KMP',[]).append(dict(link,script=script['name']))
        self.unsupported={}
        self.sprite_manifests={}

    def map_script_associations(self,name):
        """Expose proven and filename-family scripts associated with a field."""
        result=[]
        def add(script,relation,confidence,evidence):
            if script in self.scripts and not any(item['script']==script for item in result):
                result.append(dict(script=script,relation=relation,
                                   confidence=confidence,evidence=evidence))
        add(name[:-4]+'.SPC','same_resource_name','verified',
            'KMP and SPC share a basename')
        match=re.fullmatch(r'MAP(\d+)(?:_(\d+))?A\.KMP',name)
        if match:
            family='M'+match.group(1)+(('_'+match.group(2)) if match.group(2) else '')
            for prefix,relation in (('SP_','spawn'),('CH_','character_event'),('HI_','history_event')):
                add(prefix+family+'.SPC',relation,'inferred',
                    'script and map share the recovered area-number naming family')
        for link in self.incoming.get(name,[]):
            add(link['script'],'loads_field','verified',
                'decoded FldSet call names this KMP resource')
        return result

    def sprite_preview(self,container,resource,animation):
        if container not in SPRITE_CONTAINERS or not isinstance(resource,str) or not isinstance(animation,int):
            return None
        if container not in self.sprite_manifests:
            path=SPRITE_CONTAINERS[container];full=self.root/path
            if not full.exists():return None
            manifest=read(full)
            self.sprite_manifests[container]=(path,manifest,
                {group['name']:group for group in manifest['groups']})
        manifest_path,manifest,groups=self.sprite_manifests[container]
        group=groups.get(resource)
        if not group:return None
        sequence=next((a for a in group['animations'] if a['index']==animation),None)
        if not sequence or not sequence['frames']:return None
        frames=[]
        for frame_id in sequence['frames']:
            frame=manifest['frames'][frame_id]
            raw=(self.root/manifest_path.parent/frame['path']).read_bytes()
            frames.append(dict(x=frame['x'],y=frame['y'],width=frame['width'],height=frame['height'],
                duration=frame.get('duration',1),frame=frame['id'],
                image='data:image/png;base64,'+base64.b64encode(raw).decode('ascii')))
        return dict(frames[0],frames=frames)

    def entry(self,name):
        if name not in self.members:raise ValueError('Unknown map')
        member=self.members[name];blob=(self.root/member['path']).read_bytes()
        tile_name=blob[0x1C:0x5C].split(b'\0')[0].decode('ascii')
        entry=self.assets.get(tile_name)
        if not entry or entry['bpp']!=4:raise ValueError('No verified editable 4bpp tile source')
        doc=map_document(blob)
        if not doc['planes']:raise ValueError('Map has no decoded planes')
        return member,blob,entry,doc

    def runtime_contexts(self,name):
        """Read-only traced loading context; never fills unknown VRAM tiles."""
        path=self.root/'maps/runtime_scenes.json'
        if not path.exists():return []
        return [dict(scene=scene['id'],evidence=scene['evidence'],layer=layer)
                for scene in read(path)['scenes'] for layer in scene['layers']
                if layer['map']==name]

    def tile_resolutions(self,name):
        """Return explicit editor renderings for KCG-external tile values."""
        result={}
        for group in self.tile_resolution_data.get('maps',{}).get(name,[]):
            if group.get('kind')!='transparent':raise ValueError('Unsupported tile resolution')
            for tile in group.get('tiles',[]):
                integer(tile,0,1023)
                if str(tile) in result:raise ValueError('Duplicate tile resolution')
                result[str(tile)]={key:group[key] for key in ('kind','confidence','evidence')}
        return result

    def catalog(self):
        result=[]
        for name in sorted(self.members):
            try:
                _,_,entry,doc=self.entry(name)
                result.append(dict(name=name,width=doc['width'],height=doc['height'],tiles=entry['archive_name'],runtime_contexts=self.runtime_contexts(name),incoming_field_loads=self.incoming.get(name,[])))
            except (ValueError,KeyError,UnicodeError) as ex:self.unsupported[name]=str(ex)
        return dict(maps=result,unsupported=self.unsupported,scripts=sorted(self.scripts),script_totals=self.script_catalog['totals'])

    def script_data(self,name):
        if name not in self.scripts:raise ValueError('Unknown script')
        e=self.scripts[name];original=(self.root/e['path']).read_bytes()
        blob=named_scripts.rebuild(original,named_scripts.edits(self.root/e['text']))
        path=self.root/script_events.patch_path(name)
        doc=read(path) if path.exists() else dict(version=1,source_sha256=sha(original),arguments={})
        result=script_events.apply(blob,original,doc)
        calls=script_events.calls(result)
        placements=initial_sprite_placements(calls)
        for item in placements:
            item['preview']=self.sprite_preview(item['container'],item['resource'],item['animation'])
        placed_offsets={item['init_offset'] for item in placements}
        placed_sprites={item['sprite'] for item in placements}
        candidates=[]
        for item in script_events.semantic_summary(calls)['sprite_resources']:
            candidate={key:item.get(key) for key in ('offset','operation','sprite','container','resource','animation','extra')}
            candidate['display_name']=SPRITE_RESOURCE_NAMES.get(item.get('resource'),item.get('resource'))
            candidate['placed']=item['offset'] in placed_offsets
            candidate['placement_status']=('literal_initial' if candidate['placed'] else
                'same_script_actor' if isinstance(item.get('sprite'),int) and item['sprite'] in placed_sprites else
                'external_or_dynamic')
            candidate['preview']=self.sprite_preview(item.get('container'),item.get('resource'),item.get('animation'))
            candidate['dynamic']=not isinstance(item.get('sprite'),int) or not isinstance(item.get('resource'),str)
            candidates.append(candidate)
        return dict(name=name,revision=sha(original+(self.root/e['text']).read_bytes()+json.dumps(doc,sort_keys=True).encode()),
                    document=doc,calls=calls,analysis=script_events.semantic_summary(calls),sprite_placements=placements,
                    sprite_candidates=candidates,text_path=e['text'])

    def load(self,name):
        member,original,entry,base=self.entry(name)
        # Existing assembled-image layouts remain the source of tile artwork.
        if entry['kind']=='mapped_image':
            tile_raw=mapped_images.compile_image(entry,self.root)
            layout=read(self.root/entry['image_layout'])
            if layout.get('map_path')==member['path']:original=mapped_images.build_map(original,layout)
        else:
            pixels,_=gfx.read_png(str(self.root/(entry['path']+'.png')))
            tile_raw=build_assets.pack_pixels(entry,pixels)
        base=map_document(original)
        path=self.root/override_path(name)
        document=read(path) if path.exists() else base
        compile_map(original,document)
        count=len(tile_raw)//32
        colors=gfx.read_jasc(str(self.root/entry['palette_path']))
        palette_base=entry.get('palette_bank_base',0)
        colors=[(0,0,0)]*(palette_base*16)+colors
        resolved_tiles=self.tile_resolutions(name)
        unresolved=[]
        resolved_cells=[]
        for plane in document['planes']:
            for cell,word in enumerate(plane['entries']):
                tile=word&1023
                if tile>=count and str(tile) in resolved_tiles and palette_base<=word>>12<palette_base+entry['palette_banks']:
                    resolved_cells.append(dict(plane=plane['index'],cell=cell,word=word,
                                               resolution=resolved_tiles[str(tile)]))
                elif tile>=count or not palette_base<=word>>12<palette_base+entry['palette_banks']:
                    unresolved.append(dict(plane=plane['index'],cell=cell,word=word))
        dependencies=[original,tile_raw,json.dumps(colors).encode(),json.dumps(document,sort_keys=True).encode()]
        script_associations=self.map_script_associations(name)
        associated=[item['script'] for item in script_associations]
        return dict(name=name,revision=sha(b''.join(dependencies)),document=document,
                    tiles=[[v for row in t for v in row] for t in mapped_images.tiles_from_bytes(tile_raw)],palette=colors,
                    palette_base=palette_base,palette_banks=entry['palette_banks'],scripts=associated,
                    script_associations=script_associations,
                    source=member['path'],unresolved=unresolved,resolved_tiles=resolved_tiles,resolved_cells=resolved_cells,
                    runtime_contexts=self.runtime_contexts(name),
                    incoming_field_loads=self.incoming.get(name,[]),_base=original)

    def save(self,name,payload):
        if not isinstance(payload,dict):raise ValueError('Invalid save request')
        current=self.load(name)
        if payload.get('revision')!=current['revision']:raise ValueError('Map changed on disk; reload before saving')
        doc=payload['document'];compile_map(current['_base'],doc)
        for previous,plane in zip(current['document']['planes'],doc['planes']):
            for cell,word in enumerate(plane['entries']):
                if word&1023>=len(current['tiles']) or not current['palette_base']<=word>>12<current['palette_base']+current['palette_banks']:
                    if word!=previous['entries'][cell]:
                        raise ValueError('Cannot introduce an unresolved tile/palette reference')
        writes={self.root/override_path(name):(json.dumps(doc,indent=2)+'\n').encode()}
        script=payload.get('script')
        if script:
            previous=self.script_data(script['name'])
            if previous['revision']!=script['revision']:raise ValueError('Script changed on disk; reload before saving')
            e=self.scripts[script['name']];original=(self.root/e['path']).read_bytes()
            blob=named_scripts.rebuild(original,named_scripts.edits(self.root/e['text']))
            script_events.apply(blob,original,script['document']) # size/opcode validation before writing
            writes[self.root/script_events.patch_path(script['name'])]=(json.dumps(script['document'],indent=2)+'\n').encode()
        before={p:p.read_bytes() if p.exists() else None for p in writes}
        staged=[]
        try:
            for path,content in writes.items():
                path.parent.mkdir(parents=True,exist_ok=True)
                fd,temp=tempfile.mkstemp(prefix='.editor-',dir=path.parent)
                with os.fdopen(fd,'wb') as stream:stream.write(content);stream.flush();os.fsync(stream.fileno())
                staged.append((path,Path(temp)))
            for path,temp in staged:os.replace(temp,path)
        except Exception:
            for path,content in before.items():
                if content is None:path.unlink(missing_ok=True)
                else:path.write_bytes(content)
            raise
        finally:
            for _,temp in staged:temp.unlink(missing_ok=True)
        return self.load(name)
