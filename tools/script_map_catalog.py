#!/usr/bin/env python3
"""Build a static map/sprite call-site catalog from every named SPC."""
import hashlib
import json
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import script_events


def clean(record,fields):
    out={k:record[k] for k in ('offset',)+fields}
    out['dynamic']=any(v is None for k,v in out.items() if k!='offset')
    return out


def build(root=ROOT):
    manifest=json.loads((root/'scripts/nfp/manifest.json').read_text());scripts=[];script_links={}
    event_keys=('field_loads','sprite_resources','sprite_properties','sprite_moves')
    totals={k:0 for k in event_keys};totals['script_links']=0
    for entry in manifest:
        blob=(root/entry['path']).read_bytes()
        calls=script_events.calls(blob)
        summary=script_events.semantic_summary(calls)
        record={'name':entry['name'],'source_sha256':hashlib.sha256(blob).hexdigest()}
        record['field_loads']=[clean(x,('destination','x','y')) for x in summary['field_loads']]
        links=[clean(x,('operation','script')) for x in summary['script_links']]
        if links:script_links[entry['name']]=links
        record['sprite_resources']=[clean(x,('operation','sprite','container','resource','animation','extra')) for x in summary['sprite_resources']]
        record['sprite_properties']=[clean(x,('sprite','property','value')) for x in summary['sprite_properties']]
        record['sprite_moves']=[{'offset':x['offset'],'values':x['values'],
                                'dynamic':any(v is None for v in x['values'])} for x in summary['sprite_moves']]
        origin_fields=('sprite','container','resource','animation','x','y','init_offset')
        origins=[{key:item[key] for key in origin_fields}
                 for item in script_events.initial_sprite_placements(calls)]
        if origins:
            record['sprite_origins']=origins
        for key in event_keys:totals[key]+=len(record[key])
        totals['script_links']+=len(links)
        if any(record[k] for k in event_keys):scripts.append(record)
    return {'version':2,'scope':'Static native call sites; branches are not executed',
            'totals':totals,'script_links':script_links,'scripts':scripts}


def main():
    data=build();path=ROOT/'maps/script_catalog.json'
    path.write_text(json.dumps(data,indent=2)+'\n')
    print('wrote',path.relative_to(ROOT),data['totals'])


if __name__=='__main__':main()
