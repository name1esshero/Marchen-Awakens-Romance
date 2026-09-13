#!/usr/bin/env python3
"""Inventory sprite resources whose initial coordinates are not local literals."""
import json
from pathlib import Path

from map_editor.model import Project


ROOT=Path(__file__).resolve().parents[1]


def build(root=ROOT):
    project=Project(root); scripts=[]; totals=dict(resource_calls=0,literal_initial=0,
        same_script_actor=0,external_or_dynamic=0,dynamic_arguments=0,preview_available=0)
    for name in sorted(project.scripts):
        candidates=project.script_data(name)['sprite_candidates']
        if not candidates:continue
        entries=[]
        for item in candidates:
            status=item['placement_status'];totals['resource_calls']+=1;totals[status]+=1
            totals['dynamic_arguments']+=bool(item['dynamic'])
            totals['preview_available']+=bool(item['preview'])
            entries.append({key:item.get(key) for key in ('offset','operation','sprite','container',
                'resource','display_name','animation','placement_status','dynamic')})
        if any(item['placement_status']=='external_or_dynamic' for item in entries):
            scripts.append(dict(name=name,resources=entries))
    return dict(version=1,
        scope='Static SPC call sites. external_or_dynamic means coordinates must come from another script, a runtime register, or an untraced native path.',
        totals=totals,scripts=scripts)


def main():
    data=build();path=ROOT/'maps/sprite_placement_audit.json'
    path.write_text(json.dumps(data,indent=2)+'\n')
    print('wrote',path.relative_to(ROOT),data['totals'])


if __name__=='__main__':main()
