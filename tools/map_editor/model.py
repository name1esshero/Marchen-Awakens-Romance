"""Source-level map/event editor model. All writes are validated before commit."""
import copy
import hashlib
import json
from pathlib import Path
import struct
import sys
import tempfile
import os

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import gfx
import mapped_images
import build_assets
import script_events
import named_scripts

ROOT=Path(__file__).resolve().parents[2]


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


class Project:
    def __init__(self,root=ROOT):
        self.root=Path(root)
        self.members={e['name']:e for e in read(self.root/'maps/nfp/manifest.json') if e['name'].startswith('MAP') and e['name'].endswith('.KMP')}
        self.assets={e['archive_name']:e for e in read(self.root/'assets.json') if e.get('archive_name')}
        self.scripts={e['name']:e for e in read(self.root/'scripts/nfp/manifest.json')}
        self.unsupported={}

    def entry(self,name):
        if name not in self.members:raise ValueError('Unknown map')
        member=self.members[name];blob=(self.root/member['path']).read_bytes()
        tile_name=blob[0x1C:0x5C].split(b'\0')[0].decode('ascii')
        entry=self.assets.get(tile_name)
        if not entry or entry['bpp']!=4:raise ValueError('No verified editable 4bpp tile source')
        doc=map_document(blob)
        if not doc['planes']:raise ValueError('Map has no decoded planes')
        return member,blob,entry,doc

    def catalog(self):
        result=[]
        for name in sorted(self.members):
            try:
                _,_,entry,doc=self.entry(name)
                result.append(dict(name=name,width=doc['width'],height=doc['height'],tiles=entry['archive_name']))
            except (ValueError,KeyError,UnicodeError) as ex:self.unsupported[name]=str(ex)
        return dict(maps=result,unsupported=self.unsupported,scripts=sorted(self.scripts))

    def script_data(self,name):
        if name not in self.scripts:raise ValueError('Unknown script')
        e=self.scripts[name];original=(self.root/e['path']).read_bytes()
        blob=named_scripts.rebuild(original,named_scripts.edits(self.root/e['text']))
        path=self.root/script_events.patch_path(name)
        doc=read(path) if path.exists() else dict(version=1,source_sha256=sha(original),arguments={})
        result=script_events.apply(blob,original,doc)
        return dict(name=name,revision=sha(original+(self.root/e['text']).read_bytes()+json.dumps(doc,sort_keys=True).encode()),
                    document=doc,calls=script_events.calls(result),references=list(named_scripts.records(original).values()) if False else [],
                    text_path=e['text'])

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
        for plane in document['planes']:
            for word in plane['entries']:
                if word&1023>=count or word>>12>=palette_base+entry['palette_banks']:raise ValueError('Tile/palette reference outside decoded art')
        dependencies=[original,tile_raw,json.dumps(colors).encode(),json.dumps(document,sort_keys=True).encode()]
        associated=[s for s in self.scripts if s==name[:-4]+'.SPC']
        return dict(name=name,revision=sha(b''.join(dependencies)),document=document,
                    tiles=[[v for row in t for v in row] for t in mapped_images.tiles_from_bytes(tile_raw)],palette=colors,
                    palette_base=palette_base,palette_banks=entry['palette_banks'],scripts=associated,
                    source=member['path'],_base=original)

    def save(self,name,payload):
        current=self.load(name)
        if payload.get('revision')!=current['revision']:raise ValueError('Map changed on disk; reload before saving')
        doc=payload['document'];compile_map(current['_base'],doc)
        for plane in doc['planes']:
            for word in plane['entries']:
                if word&1023>=len(current['tiles']) or not current['palette_base']<=word>>12<current['palette_base']+current['palette_banks']:
                    raise ValueError('Tile/palette outside available art')
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
