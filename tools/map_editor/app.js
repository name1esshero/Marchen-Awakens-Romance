'use strict';
const $=id=>document.getElementById(id);
let token=new URLSearchParams(location.hash.slice(1)).get('token')||'';
try{token=token||sessionStorage.getItem('mar-editor-token')||'';if(token)sessionStorage.setItem('mar-editor-token',token);}catch{}
if(location.protocol!=='file:'){try{history.replaceState(null,'',location.pathname);}catch{}}
let map=null,script=null,mapEventScripts=[],selectedTile=0,dirty=false,busy=false,stroke=null,eventDrag=null,visible=[],undo=[],redo=[],atlas=new Map(),catalog=null,mode='map',tool='pencil',marscriptDoc=null,scriptEditorDirty=false;
const spriteImages=new Map();
const connectionMapCache=new Map();
const previewPlacements=new Map();
let playback={active:false,running:false,frame:0,duration:0,actors:[],lastTime:0,unresolved:0,request:0};
const ZOOM_LEVELS=[0.03125,0.0625,0.125,0.25,0.5,0.75,1,1.5,2,3,4];
const clone=x=>JSON.parse(JSON.stringify(x));
function status(message,error=false){$('status').textContent=message;$('status').className=error?'error':'';}
function controls(){document.querySelectorAll('button,select,input,textarea').forEach(element=>element.disabled=busy);$('save').textContent=dirty?'Save sources *':'Save sources';$('undo').disabled=busy||!undo.length;$('redo').disabled=busy||!redo.length;$('script-save').disabled=busy||!marscriptDoc;$('script-reset').disabled=busy||!marscriptDoc?.has_override;$('script-editor').disabled=busy||!marscriptDoc;}
async function api(path,payload){const options={headers:{'X-Editor-Token':token}};if(payload){options.method='POST';options.headers['Content-Type']='application/json';options.body=JSON.stringify(payload);}const response=await fetch('/api/'+path,options);const data=await response.json();if(!response.ok)throw Error(data.error||response.statusText);return data;}
function options(select,items){select.replaceChildren();for(const [value,label] of items){const o=document.createElement('option');o.value=value;o.textContent=label;select.append(o);}}
function state(){return {document:clone(map.document),events:script?clone(script.document):null,script:script?.name||null};}
function remember(before=state()){undo.push(before);if(undo.length>60)undo.shift();redo=[];dirty=true;controls();}
function restore(s){map.document=s.document;if(script&&s.script===script.name&&s.events)script.document=s.events;playback.running=false;playback.active=false;cancelAnimationFrame(playback.request);refreshAttributeInspector();drawMap();drawArgs();dirty=true;controls();}
function undoEdit(){if(!map||busy||!undo.length)return;redo.push(state());restore(undo.pop());}
function redoEdit(){if(!map||busy||!redo.length)return;undo.push(state());restore(redo.pop());}
function bank(){return Number($('palette').value);}
function tileAtlas(palette){if(atlas.has(palette))return atlas.get(palette);const c=document.createElement('canvas');c.width=256;c.height=Math.ceil(map.tiles.length/32)*8;const ctx=c.getContext('2d');const data=ctx.createImageData(c.width,c.height);map.tiles.forEach((tile,i)=>tile.forEach((index,p)=>{const x=i%32*8+p%8,y=Math.floor(i/32)*8+Math.floor(p/8),at=(y*c.width+x)*4;const color=map.palette[palette*16+index]||[255,0,255];data.data.set([...color,index?255:0],at);}));ctx.putImageData(data,0,0);atlas.set(palette,c);return c;}
function drawTiles(){if(!map)return;const c=$('tiles'),source=tileAtlas(bank());c.width=256;c.height=Math.ceil(map.tiles.length/16)*16;const ctx=c.getContext('2d');ctx.imageSmoothingEnabled=false;map.tiles.forEach((_,i)=>ctx.drawImage(source,i%32*8,Math.floor(i/32)*8,8,8,i%16*16,Math.floor(i/16)*16,16,16));ctx.strokeStyle='#ffde70';ctx.lineWidth=1;ctx.strokeRect(selectedTile%16*16+.5,Math.floor(selectedTile/16)*16+.5,15,15);$('brush').textContent=`Tile ${selectedTile} / ${map.tiles.length-1} · bank ${bank()}`;}


// Native registry: HitInit 080121C4, HitHitRect 08012244, HitSet 08012208.
// Rectangle layout and modes are confirmed by collision routine 08018C4C.
function selectedCall() {
    return script && $('calls').value !== '' ? script.calls[Number($('calls').value)] : null;
}
function callValues(call) {
    return call.arguments.map(arg => script.document.arguments[arg.offset] ?? arg.value);
}
function decodedValues(call) {
    if (call.editable) return callValues(call);
    return (call.decoded_arguments||[]).map(arg=>arg.kind==='integer'||arg.kind==='string'?arg.value:null);
}
function playbackValues(call,registers){return (call.decoded_arguments||[]).map(arg=>arg.kind==='integer'?(script.document.arguments[arg.offset]??arg.value):arg.kind==='string'?arg.value:arg.kind==='operand'?registers[arg.register]:null);}
function currentSpritePlacement(item){return {...item,x:script.document.arguments[item.x_argument_offset]??item.x,y:script.document.arguments[item.y_argument_offset]??item.y};}
function spriteFrame(resource,frame){const frames=resource?.frames||[];if(!frames.length)return resource;const total=frames.reduce((sum,item)=>sum+Math.max(1,item.duration||1),0);let cursor=((Math.floor(frame)%total)+total)%total;for(const item of frames){cursor-=Math.max(1,item.duration||1);if(cursor<0)return item;}return frames[0];}
function imageFor(frame){if(!frame)return null;let img=spriteImages.get(frame.image);if(!img){img=new Image();img.onload=drawMap;img.src=frame.image;spriteImages.set(frame.image,img);}return img.complete?img:null;}
function eventClass(call) {
    if (/^Spr(?:Init|Chg|Set|Move|Free)/.test(call.function)) return 'sprite';
    if (/^Hit(?:Init|HitRect|HitHitRect|Set|Free)/.test(call.function)) return 'hit';
    if (call.function==='FldSet') return 'field';
    return 'other';
}
function eventLabel(call) {
    const values=decodedValues(call),kind=eventClass(call),at='@'+call.offset.toString(16).toUpperCase();
    if(kind==='sprite') return `${call.function} · object ${values[0]??'?'} · ${at}`;
    if(kind==='hit') return `${call.function} · region ${values[0]??'?'} · ${at}`;
    if(kind==='field') return `Load ${values[0]??'?'} at viewport (${values[1]??'?'}, ${values[2]??'?'}) · ${at}`;
    return `${call.function} · ${at}${call.editable?'':' · read-only'}`;
}
function hitDescription(call) {
    if (call?.count === 5 && ['SprInit', 'SprChg'].includes(call.function))
        return {labels: ['Sprite index', 'NCD container', 'Resource name', 'Animation', 'Frame'],
            note: call.function === 'SprChg'
                ? 'Changes an active sprite’s named resource, animation and frame; preserves X/Y. Arguments: sprite, container, name, animation, frame.'
                : 'Creates the sprite asynchronously using its container, resource name and animation. Starts at frame zero; the fifth argument is not used as a starting frame on this path.'};
    if (!call?.editable) return null;
    if (call.function === 'SprSet' && call.count === 3) {
        const property = callValues(call)[1];
        return {labels: ['Sprite index', 'Property (0 X, 1 Y)',
            property === 0 ? 'X (pixels, signed 16-bit)' : property === 1 ? 'Y (pixels, signed 16-bit)' : 'Property value'],
            note: 'Position properties 0 and 1 reach the sprite renderer as pixel coordinates. Other properties remain numeric. The guide shows only this assignment, not a simulated sprite or its other coordinate.'};
    }
    if (['HitInit', 'HitHitRect'].includes(call.function) && call.count === 5) {
        return {labels: ['Region index (0–15)', 'X (pixels, signed 16-bit)', 'Y (pixels, signed 16-bit)',
            'Width (pixels, signed 16-bit)', 'Height (pixels, signed 16-bit)'],
            note: call.function === 'HitInit'
                ? 'Activates this rectangle and resets mode to overlap (0).'
                : 'Activates this rectangle and preserves its existing mode. This call does not test for a hit.'};
    }
    if (call.function === 'HitSet' && call.count === 3) {
        const property = callValues(call)[1];
        const fields = {54: 'X', 55: 'Y', 56: 'Width', 57: 'Height', 61: 'Mode'};
        return {labels: ['Region index (0–15)', 'Property (54 X, 55 Y, 56 width, 57 height, 61 mode)',
            fields[property] || 'Unsupported property value'],
            note: property === 61
                ? 'Mode 0: overlap; 1: strict containment. Other modes never hit. Touching edges is not a hit.'
                : 'Only active regions are updated. Rectangle values truncate to signed 16-bit integers.'};
    }
    if (call.function === 'HitFree' && call.count === 1)
        return {labels: ['Region index (0–15), or -1 for all'], note: 'Disables regions without clearing their geometry or mode.'};
    if (call.function === 'FldSet' && call.count === 3)
        return {labels: ['KMP resource name', 'Viewport X (pixels, signed 16-bit)', 'Viewport Y (pixels, signed 16-bit)'],
            note: 'Loads the named field and positions both visual planes. These coordinates are not yet proven to be the player’s arrival point.'};
    return null;
}
function drawHitPreview(ctx, zoom) {
    if (!$('hit-preview').checked) return;
    const call = selectedCall();
    if (!call?.editable || !['HitInit', 'HitHitRect'].includes(call.function) || call.count !== 5) return;
    const [id, ...rect] = callValues(call);
    if (id < 0 || id > 15) return;
    const [x, y, width, height] = rect.map(value => (value << 16) >> 16);
    ctx.save();
    ctx.strokeStyle = '#00ffff';
    ctx.fillStyle = '#00ffff30';
    ctx.lineWidth = 2 / zoom;
    ctx.fillRect(x, y, width, height);
    ctx.strokeRect(x, y, width, height);
    ctx.fillStyle = '#001820';
    ctx.fillRect(x, y, 95, 12);
    ctx.fillStyle = '#00ffff';
    ctx.font = '10px monospace';
    ctx.fillText('Hit region ' + id, x + 2, y + 10);
    ctx.restore();
}

function drawSpriteGuide(ctx, zoom) {
    if (!$('sprite-guide').checked) return;
    const call = selectedCall();
    if (!call?.editable || call.function !== 'SprSet' || call.count !== 3) return;
    const [id, property, value] = callValues(call);
    if (id < 0 || id >= 32 || (property !== 0 && property !== 1)) return;
    const coordinate = (value << 16) >> 16;
    const width = map.document.width * 8, height = map.document.height * 8;
    ctx.save();
    ctx.strokeStyle = '#ffdf00'; ctx.lineWidth = 2 / zoom;
    ctx.beginPath();
    if (property === 0) {ctx.moveTo(coordinate, 0); ctx.lineTo(coordinate, height);}
    else {ctx.moveTo(0, coordinate); ctx.lineTo(width, coordinate);}
    ctx.stroke(); ctx.restore();
}

function drawSpritePlacement(ctx,zoom,source,item,selected=false,associated=false) {
    const preview=item.preview;
    if (!preview) return;
    const img=imageFor(preview);
    if (!img) return;
    ctx.drawImage(img,item.x+preview.x,item.y+preview.y,preview.width,preview.height);
    ctx.save();
    if(selected){ctx.strokeStyle='#fff36b';ctx.lineWidth=2/zoom;ctx.strokeRect(item.x+preview.x,item.y+preview.y,preview.width,preview.height);}
    const label=`${source} · ${item.sprite}: ${item.resource}`;
    ctx.font='9px monospace';const width=ctx.measureText(label).width+4;
    ctx.fillStyle='#101820cc';ctx.strokeStyle=associated?'#7dff8a':'#ffdf00';ctx.lineWidth=1/zoom;
    ctx.fillRect(item.x-width/2,item.y+3,width,11);
    ctx.strokeRect(item.x-width/2,item.y+3,width,11);
    ctx.fillStyle=associated?'#7dff8a':'#ffdf00';ctx.fillText(label,item.x-width/2+2,item.y+12);
    ctx.restore();
}

function drawInitialSprites(ctx,zoom) {
    if (!$('show-sprites').checked || !script) return;
    if(playback.active&&playback.actors.length)drawPlaybackSprites(ctx,zoom);
    else for (const source of script.sprite_placements||[]) {
        const item=currentSpritePlacement(source),initCall=script.calls.find(call=>call.offset===item.init_offset);
        drawSpritePlacement(ctx,zoom,script.name,item,initCall===selectedCall(),false);
    }
    if(mode==='events'&&$('show-map-event-sources').checked)for(const source of mapEventScripts){
        if(!source.data||source.data.name===script.name)continue;
        for(const item of source.data.sprite_placements||[])
            drawSpritePlacement(ctx,zoom,source.data.name,item,false,true);
    }
}

function actorCoordinate(actor,axis,time){let value=actor[axis];for(const event of actor.events){if(event.axis!==axis)continue;if(event.type==='set'&&event.time<=time)value=event.value;if(event.type==='move'){if(time>=event.end)value=event.to;else if(time>=event.time)return event.from+(event.to-event.from)*(time-event.time)/(event.end-event.time);}}return value;}
function actorAppearance(actor,time){let resource=actor.resource,visible=false,born=0;for(const event of actor.events){if(event.time>time)break;if(event.type==='show'){resource=event.resource;visible=true;born=event.time;}if(event.type==='change'){resource=event.resource;born=event.time;}if(event.type==='hide')visible=false;}return {resource,visible,born};}
function drawPlaybackSprites(ctx,zoom){for(const actor of playback.actors){const appearance=actorAppearance(actor,playback.frame),x=actorCoordinate(actor,'x',playback.frame),y=actorCoordinate(actor,'y',playback.frame);if(!appearance.visible||!Number.isFinite(x)||!Number.isFinite(y))continue;const frame=spriteFrame(appearance.resource,playback.frame-appearance.born),img=imageFor(frame);if(!img)continue;ctx.drawImage(img,x+frame.x,y+frame.y,frame.width,frame.height);ctx.save();ctx.strokeStyle='#7dff8a';ctx.lineWidth=1/zoom;ctx.strokeRect(x+frame.x,y+frame.y,frame.width,frame.height);ctx.restore();}}

function buildPlayback(){cancelAnimationFrame(playback.request);const candidates=new Map((script?.sprite_candidates||[]).map(item=>[item.offset,item])),placements=new Map((script?.sprite_placements||[]).map(item=>[item.init_offset,currentSpritePlacement(item)])),actors=new Map(),pending=new Map(),registers={};let time=0,unresolved=0;
    const actorFor=id=>{if(!actors.has(id))actors.set(id,{id,x:undefined,y:undefined,resource:null,events:[]});return actors.get(id);};
    for(const call of script?.calls||[]){const values=playbackValues(call,registers),id=values[0];if((call.function==='SprInit'||call.function==='SprChg')&&Number.isInteger(id)){const actor=actorFor(id),candidate=candidates.get(call.offset),placement=placements.get(call.offset)||previewPlacements.get(call.offset);if(candidate?.preview){actor.resource=candidate.preview;actor.events.push({type:call.function==='SprInit'||previewPlacements.has(call.offset)?'show':'change',time,resource:candidate.preview});}else unresolved++;if(placement){actor.x=placement.x;actor.y=placement.y;actor.events.push({type:'set',axis:'x',time,value:placement.x},{type:'set',axis:'y',time,value:placement.y});}}
        else if(call.function==='SprSet'&&Number.isInteger(id)&&[0,1].includes(values[1])&&Number.isInteger(values[2])){const actor=actorFor(id),axis='xy'[values[1]];actor[axis]=values[2];actor.events.push({type:'set',axis,time,value:values[2]});}
        else if(call.function==='SprGet'&&Number.isInteger(id)&&[0,1].includes(values[1])){const actor=actorFor(id),value=actor['xy'[values[1]]];if(Number.isFinite(value))registers[0]=value;else unresolved++;}
        else if(call.function==='SprMove'&&Number.isInteger(id)&&[0,1].includes(values[1])){const actor=actorFor(id),axis='xy'[values[1]],duration=values[2],target=values[4];if(Number.isInteger(duration)&&duration>0&&Number.isInteger(target)&&Number.isFinite(actor[axis])){const end=time+duration;actor.events.push({type:'move',axis,time,end,from:actor[axis],to:target});actor[axis]=target;pending.set(id,Math.max(pending.get(id)||0,end));if(values[5])time=end;}else unresolved++;}
        else if(call.function==='SprSync'){unresolved++;}
        else if(call.function==='SprFree'&&Number.isInteger(id)){if(id===-1)for(const actor of actors.values())actor.events.push({type:'hide',time});else actorFor(id).events.push({type:'hide',time});}
    }
    playback={active:false,running:false,frame:0,duration:Math.max(1,time,...pending.values()),actors:[...actors.values()],lastTime:0,unresolved,request:0};$('preview-time').max=String(playback.duration);$('preview-time').value='0';updatePlaybackClock();drawMap();renderSpriteCandidates();}
function updatePlaybackClock(){$('preview-time').value=String(Math.round(playback.frame));$('preview-clock').value=`${Math.round(playback.frame)} / ${playback.duration} frames${playback.unresolved?' · '+playback.unresolved+' unresolved steps skipped':''}`;}
function playbackTick(now){if(!playback.running)return;const elapsed=playback.lastTime?now-playback.lastTime:0;playback.lastTime=now;playback.frame+=elapsed*60/1000*Number($('preview-speed').value);if(playback.frame>=playback.duration){playback.frame=playback.duration;playback.running=false;}updatePlaybackClock();drawMap();if(playback.running)playback.request=requestAnimationFrame(playbackTick);}
function renderSpriteCandidates(){const div=$('sprite-candidates');div.replaceChildren();for(const candidate of script?.sprite_candidates||[]){const staged=previewPlacements.has(candidate.offset),card=document.createElement('div');card.className='sprite-candidate'+(candidate.placed?'':' unplaced');if(candidate.preview){const img=document.createElement('img');img.src=candidate.preview.image;img.alt='';card.append(img);}const text=document.createElement('div'),strong=document.createElement('strong'),span=document.createElement('span');strong.textContent=`${candidate.display_name||candidate.resource||'Dynamic resource'} · object ${candidate.sprite??'dynamic'}`;span.textContent=`${candidate.resource||'dynamic'} · ${candidate.operation} @${candidate.offset.toString(16).toUpperCase()} · ${candidate.placed?'literal map position':staged?'staged at map center':'position supplied elsewhere or dynamically'}`;text.append(strong,span);card.append(text);if(!candidate.placed&&candidate.preview&&Number.isInteger(candidate.sprite)){const button=document.createElement('button');button.textContent=staged?'Remove preview':'Stage preview';button.onclick=event=>{event.stopPropagation();if(staged)previewPlacements.delete(candidate.offset);else previewPlacements.set(candidate.offset,{x:Math.round(map.document.width*4),y:Math.round(map.document.height*4)});buildPlayback();playback.active=previewPlacements.size>0;drawMap();};card.append(button);}card.onclick=()=>{const index=script.calls.findIndex(call=>call.offset===candidate.offset);if(index>=0)selectCall(index);};div.append(card);}if(!div.children.length){const p=document.createElement('p');p.className='empty';p.textContent='No sprite resources decoded in this script.';div.append(p);}}
function renderEventSources(){const div=$('event-sources');div.replaceChildren();for(const source of mapEventScripts){const {meta,data}=source,button=document.createElement('button'),counts=meta.event_counts||{},placed=(data?.sprite_placements||[]).length,total=data?(data.sprite_candidates||[]).length:(counts.sprite_resources||0);button.className='event-source'+(meta.script===script?.name?' active':'')+(meta.confidence==='inferred'?' inferred':'')+(meta.depth?' chained':'');button.textContent=data?`${meta.script} · ${placed} placed / ${total} resources`:`${meta.script} · ${total} sprite resources · open`;button.title=`${meta.relation.replaceAll('_',' ')} · ${meta.confidence}: ${meta.evidence}${meta.parent?' · called by '+meta.parent:''}`;button.onclick=()=>{$('scripts').value=meta.script;$('scripts').dispatchEvent(new Event('change'));};div.append(button);}if(!div.children.length){const p=document.createElement('p');p.className='empty';p.textContent='No associated event scripts decoded.';div.append(p);}}
async function loadMapEventSources(){mapEventScripts=[];renderEventSources();const sources=(map?.event_sources||[]).filter(item=>item.depth===0||Object.values(item.event_counts||{}).some(Boolean));mapEventScripts=sources.map(meta=>({meta,data:null}));renderEventSources();const direct=mapEventScripts.filter(source=>source.meta.depth===0);const loaded=await Promise.allSettled(direct.map(async source=>({...source,data:source.meta.script===script?.name?script:await api('script/'+encodeURIComponent(source.meta.script))})));for(const item of loaded)if(item.status==='fulfilled'){const index=mapEventScripts.findIndex(source=>source.meta.script===item.value.meta.script);if(index>=0)mapEventScripts[index]=item.value;}renderEventSources();drawMap();}
function scriptButton(name,title,label=name){const button=document.createElement('button');button.textContent=label;button.title=title;button.onclick=()=>{$('scripts').value=name;$('scripts').dispatchEvent(new Event('change'));};return button;}
function renderScriptLinks(){const div=$('script-links');div.replaceChildren();const associations=map?.script_associations||[],links=script?.analysis?.script_links||[];if(associations.length){const row=document.createElement('div'),label=document.createElement('span');label.textContent='Map scripts: ';row.append(label);for(const item of associations)row.append(scriptButton(item.script,`${item.relation.replaceAll('_',' ')} · ${item.confidence}: ${item.evidence}`,item.script+(item.confidence==='inferred'?' · inferred':'')));div.append(row);}if(links.length){const row=document.createElement('div'),label=document.createElement('span');label.textContent='Chained scripts: ';row.append(label);for(const link of links)row.append(scriptButton(link.script,`${link.operation} at CODE +0x${link.offset.toString(16).toUpperCase()}`));div.append(row);}}

function drawEventMarkers(ctx,zoom) {
    if(!script||mode!=='events')return;
    if($('show-hit-regions').checked)for(const call of script.calls){
        if(!['HitInit','HitHitRect'].includes(call.function)||call.count!==5)continue;
        const [id,x,y,w,h]=decodedValues(call);if(![id,x,y,w,h].every(Number.isInteger)||id<0||id>15)continue;
        ctx.save();ctx.fillStyle='#00ddeb22';ctx.strokeStyle='#00eaff';ctx.lineWidth=(call===selectedCall()?3:1)/zoom;
        ctx.fillRect(x,y,w,h);ctx.strokeRect(x,y,w,h);ctx.fillStyle='#001820';ctx.fillRect(x,y,18,11);ctx.fillStyle='#00ffff';ctx.font='9px monospace';ctx.fillText('H'+id,x+2,y+9);ctx.restore();
    }
    for(const call of script.calls){
        if(call.function!=='FldSet'||call.count!==3)continue;const [destination,x,y]=decodedValues(call);
        if(!Number.isInteger(x)||!Number.isInteger(y))continue;ctx.save();ctx.strokeStyle='#ff72c7';ctx.fillStyle='#4a1238dd';ctx.lineWidth=(call===selectedCall()?3:1)/zoom;
        ctx.beginPath();ctx.arc(x,y,7,0,Math.PI*2);ctx.fill();ctx.stroke();ctx.fillStyle='#fff';ctx.font='9px monospace';ctx.fillText(String(destination||'?'),x+10,y+3);ctx.restore();
    }
}

function layer(){return $('layer').value==='attributes'?map.document.attributes:map.document.planes[Number($('layer').value)];}
function drawMap(){if(!map)return;const {width,height,planes,attributes}=map.document,zoom=Number($('zoom').value),c=$('map');c.width=Math.max(1,Math.round(width*8*zoom));c.height=Math.max(1,Math.round(height*8*zoom));const ctx=c.getContext('2d');ctx.imageSmoothingEnabled=false;ctx.scale(zoom,zoom);const drawPlanes=planes.map((plane,j)=>({plane,j}));if($('game-order').checked)drawPlanes.reverse();drawPlanes.forEach(({plane,j})=>{if(!visible[j])return;plane.entries.forEach((word,i)=>{const tile=word&1023,x=i%width*8,y=Math.floor(i/width)*8;if(tile>=map.tiles.length&&map.resolved_tiles?.[tile]?.kind==='transparent')return;if(tile>=map.tiles.length||word>>>12<map.palette_base||word>>>12>=map.palette_base+map.palette_banks){ctx.fillStyle='#bd326e';ctx.fillRect(x,y,8,8);ctx.strokeStyle='#fff';ctx.beginPath();ctx.moveTo(x,y);ctx.lineTo(x+8,y+8);ctx.stroke();return;}ctx.save();ctx.translate(x+(word&1024?8:0),y+(word&2048?8:0));ctx.scale(word&1024?-1:1,word&2048?-1:1);ctx.drawImage(tileAtlas(word>>>12),tile%32*8,Math.floor(tile/32)*8,8,8,0,0,8,8);ctx.restore();});});if((mode==='collision'||$('layer').value==='attributes')&&attributes){attributes.entries.forEach((value,i)=>{if(!value)return;ctx.fillStyle=isConnectionAttribute(value)?'#39db7180':`hsla(${value*47%360},90%,60%,.45)`;ctx.fillRect(i%width*8,Math.floor(i/width)*8,8,8);});}if($('grid').checked){ctx.strokeStyle='#ffffff40';ctx.lineWidth=1/zoom;ctx.beginPath();for(let x=0;x<=width;x++){ctx.moveTo(x*8,0);ctx.lineTo(x*8,height*8);}for(let y=0;y<=height;y++){ctx.moveTo(0,y*8);ctx.lineTo(width*8,y*8);}ctx.stroke();}drawInitialSprites(ctx,zoom);drawEventMarkers(ctx,zoom);drawHitPreview(ctx,zoom);drawSpriteGuide(ctx,zoom);}
function point(event){const rect=$('map').getBoundingClientRect(),zoom=Number($('zoom').value);const x=Math.floor((event.clientX-rect.left)/(8*zoom)),y=Math.floor((event.clientY-rect.top)/(8*zoom));return x>=0&&y>=0&&x<map.document.width&&y<map.document.height?{x,y,index:y*map.document.width+x}:null;}
function brushValue(target){if(mode==='collision'){const value=Number($('attribute').value);const max=target.word_size===1?255:65535;if(!Number.isInteger(value)||value<0||value>max){status(`Attribute must be an integer from 0 to ${max}.`,true);return null;}return value;}return selectedTile|(bank()<<12)|($('hflip').checked?1024:0)|($('vflip').checked?2048:0);}
function paint(event,pick=false){if(!map||busy||!['map','collision'].includes(mode))return;const p=point(event);if(!p)return;const target=layer(),word=target.entries[p.index];$('position').textContent=`Tile (${p.x}, ${p.y}) · pixel (${p.x*8}, ${p.y*8}) · value ${word} / 0x${word.toString(16).padStart(4,'0')}`;if(pick){if(mode==='collision'){$('attribute').value=word;describeAttribute();}else{if((word&1023)>=map.tiles.length){const resolution=map.resolved_tiles?.[word&1023];status(resolution?`Tile ${word&1023} is a ${resolution.confidence} transparent runtime tile; choose stored artwork to replace it.`:'This tile reference is unresolved. Choose a decoded tile to replace it.',!resolution);return;}selectedTile=word&1023;$('palette').value=word>>>12;$('hflip').checked=!!(word&1024);$('vflip').checked=!!(word&2048);drawTiles();}return;}if(!stroke)return;const value=brushValue(target);if(value===null)return;if(word!==value){target.entries[p.index]=value;stroke.changed=true;dirty=true;drawMap();controls();}}
// Bucket Fill: classic 4-connected flood fill over the active layer's flat
// entries array, matching Porymap's Bucket Fill Tool (contiguous region only
// -- no Ctrl-for-all-matching-tiles variant, since that's a rarer need here).
function floodFill(event){const p=point(event);if(!p)return false;const target=layer(),startValue=target.entries[p.index],value=brushValue(target);if(value===null||startValue===value)return false;const{width,height}=map.document,stack=[p.index],seen=new Set([p.index]);while(stack.length){const i=stack.pop();target.entries[i]=value;const x=i%width,y=(i-x)/width;for(const[dx,dy] of[[1,0],[-1,0],[0,1],[0,-1]]){const nx=x+dx,ny=y+dy;if(nx<0||ny<0||nx>=width||ny>=height)continue;const ni=ny*width+nx;if(seen.has(ni)||target.entries[ni]!==startValue)continue;seen.add(ni);stack.push(ni);}}dirty=true;drawMap();controls();return true;}
// Wheel zoom is local to the viewport. Keep the same map pixel beneath the
// pointer, except where browser scroll limits require clamping at an edge.
$('map-scroll').addEventListener('wheel', event => {
    event.preventDefault();
    if (!map || busy || !event.deltaY) return;
    endStroke();
    const viewport = $('map-scroll');
    const oldZoom = Number($('zoom').value);
    const candidates = event.deltaY < 0 ? ZOOM_LEVELS.filter(value => value > oldZoom + 1e-8) : ZOOM_LEVELS.filter(value => value < oldZoom - 1e-8).reverse();
    const newZoom = candidates[0] ?? oldZoom;
    if (newZoom === oldZoom) return;
    const bounds = viewport.getBoundingClientRect();
    const x = event.clientX - bounds.left - viewport.clientLeft;
    const y = event.clientY - bounds.top - viewport.clientTop;
    const mapX = (viewport.scrollLeft + x) / oldZoom;
    const mapY = (viewport.scrollTop + y) / oldZoom;
    $('zoom').value = newZoom;
    drawMap();
    viewport.scrollLeft = mapX * newZoom - x;
    viewport.scrollTop = mapY * newZoom - y;
}, {passive: false});
$('fit-map').onclick=()=>{
    if(!map||busy)return;
    endStroke();
    const viewport=$('map-scroll'),width=map.document.width*8,height=map.document.height*8;
    const limit=Math.min(4,(viewport.clientWidth-2)/width,(viewport.clientHeight-2)/height);
    const newZoom=ZOOM_LEVELS.filter(value=>value<=limit+1e-8).pop()||ZOOM_LEVELS[0];
    $('zoom').value=String(newZoom);drawMap();viewport.scrollLeft=0;viewport.scrollTop=0;
};
$('map').addEventListener('contextmenu',e=>{e.preventDefault();paint(e,true);});
$('map').addEventListener('pointerdown',e=>{if(e.button!==0||!map||busy)return;if(mode==='events'){beginEventDrag(e);return;}if(!['map','collision'].includes(mode))return;if(tool==='eyedropper'){paint(e,true);return;}if(tool==='pointer'){paint(e);return;}if(tool==='bucket'){const before=state();if(floodFill(e)){remember(before);if(mode==='collision')refreshAttributeInspector();}return;}stroke={before:state(),changed:false};$('map').setPointerCapture(e.pointerId);paint(e);});
$('map').addEventListener('pointermove',e=>eventDrag?moveEvent(e):paint(e));
function endStroke(){if(stroke?.changed){remember(stroke.before);if(mode==='collision')refreshAttributeInspector();}stroke=null;}
$('map').addEventListener('pointerup',()=>{endStroke();endEventDrag();});$('map').addEventListener('pointercancel',()=>{endStroke();endEventDrag();});
$('tiles').onclick=e=>{if(!map||busy)return;const r=e.currentTarget.getBoundingClientRect(),i=Math.floor((e.clientY-r.top)/16)*16+Math.floor((e.clientX-r.left)/16);if(i<map.tiles.length){selectedTile=i;drawTiles();}};
function drawCalls(){const value=$('calls').value,q=$('call-filter').value.toLowerCase(),kind=$('event-class').value;const items=(script?.calls||[]).map((call,index)=>({call,index,label:eventLabel(call)})).filter(item=>(kind==='all'||eventClass(item.call)===kind)&&item.label.toLowerCase().includes(q)).map(item=>[item.index,item.label]);options($('calls'),items);if([...$('calls').options].some(o=>o.value===value))$('calls').value=value;drawArgs();}
function selectCall(index){if(![...$('calls').options].some(option=>Number(option.value)===index)){$('event-class').value='all';$('call-filter').value='';drawCalls();}$('calls').value=String(index);drawArgs();$('calls').focus();}
function eventPoint(event){const rect=$('map').getBoundingClientRect(),zoom=Number($('zoom').value);return {x:(event.clientX-rect.left)/zoom,y:(event.clientY-rect.top)/zoom};}
function eventAt(event){if(!script)return null;const {x,y}=eventPoint(event);let best=null;
    for(const source of script.sprite_placements||[]){const item=currentSpritePlacement(source),preview=item.preview,rx=item.x+(preview?.x||-8),ry=item.y+(preview?.y||-8),w=preview?.width||16,h=preview?.height||16;if(x>=rx&&x<=rx+w&&y>=ry&&y<=ry+h){const index=script.calls.findIndex(call=>call.offset===item.init_offset);if(index>=0)best={index,distance:0,type:'sprite',item};}}
    script.calls.forEach((call,index)=>{const values=decodedValues(call);if(['HitInit','HitHitRect'].includes(call.function)&&values.slice(0,5).every(Number.isInteger)){const [,rx,ry,w,h]=values;if(x>=Math.min(rx,rx+w)&&x<=Math.max(rx,rx+w)&&y>=Math.min(ry,ry+h)&&y<=Math.max(ry,ry+h))best={index,distance:0,type:'hit',call};}if(call.function==='FldSet'&&Number.isInteger(values[1])&&Number.isInteger(values[2])){const distance=Math.hypot(x-values[1],y-values[2]);if(distance<=12&&(!best||distance<best.distance))best={index,distance,type:'field',call};}});return best;
}
function beginEventDrag(event){const target=eventAt(event);if(!target)return;playback.running=false;playback.active=false;cancelAnimationFrame(playback.request);selectCall(target.index);const pointer=eventPoint(event);let offsets,start;
    if(target.type==='sprite'&&Number.isInteger(target.item.x_argument_offset)&&Number.isInteger(target.item.y_argument_offset)){offsets=[target.item.x_argument_offset,target.item.y_argument_offset];start=[target.item.x,target.item.y];}
    if(target.type==='hit'&&target.call.editable){offsets=[target.call.arguments[1].offset,target.call.arguments[2].offset];const values=callValues(target.call);start=[values[1],values[2]];}
    if(offsets){eventDrag={before:state(),offsets,start,pointer,changed:false};$('map').setPointerCapture(event.pointerId);}
}
function moveEvent(event){if(!eventDrag)return;const point=eventPoint(event),snap=$('snap-events').checked?8:1;let x=Math.round((eventDrag.start[0]+point.x-eventDrag.pointer.x)/snap)*snap,y=Math.round((eventDrag.start[1]+point.y-eventDrag.pointer.y)/snap)*snap;x=Math.max(-32768,Math.min(32767,x));y=Math.max(-32768,Math.min(32767,y));if((script.document.arguments[eventDrag.offsets[0]]??eventDrag.start[0])!==x||(script.document.arguments[eventDrag.offsets[1]]??eventDrag.start[1])!==y){script.document.arguments[eventDrag.offsets[0]]=x;script.document.arguments[eventDrag.offsets[1]]=y;eventDrag.changed=true;dirty=true;drawMap();drawArgs();$('position').textContent=`Event position (${x}, ${y}) pixels`;controls();}}
function endEventDrag(){if(eventDrag?.changed)remember(eventDrag.before);eventDrag=null;}
function previewAtlas(data,palette){const canvas=document.createElement('canvas');canvas.width=256;canvas.height=Math.ceil(data.tiles.length/32)*8;const ctx=canvas.getContext('2d'),pixels=ctx.createImageData(canvas.width,canvas.height);data.tiles.forEach((tile,index)=>tile.forEach((colorIndex,p)=>{const x=index%32*8+p%8,y=Math.floor(index/32)*8+Math.floor(p/8),at=(y*canvas.width+x)*4,color=data.palette[palette*16+colorIndex]||[255,0,255];pixels.data.set([...color,colorIndex?255:0],at);}));ctx.putImageData(pixels,0,0);return canvas;}
function renderConnectionMap(data,link,canvas){const scale=.75,viewWidth=240,viewHeight=160,originX=Number.isInteger(link.x)?link.x:0,originY=Number.isInteger(link.y)?link.y:0,atlases=new Map(),ctx=canvas.getContext('2d');canvas.width=viewWidth*scale;canvas.height=viewHeight*scale;ctx.imageSmoothingEnabled=false;ctx.scale(scale,scale);ctx.fillStyle='#111923';ctx.fillRect(0,0,viewWidth,viewHeight);const getAtlas=bank=>{if(!atlases.has(bank))atlases.set(bank,previewAtlas(data,bank));return atlases.get(bank);};[...data.document.planes].reverse().forEach(plane=>plane.entries.forEach((word,index)=>{const tile=word&1023,x=index%data.document.width*8-originX,y=Math.floor(index/data.document.width)*8-originY;if(x<=-8||y<=-8||x>=viewWidth||y>=viewHeight)return;if(tile>=data.tiles.length&&data.resolved_tiles?.[tile]?.kind==='transparent')return;if(tile>=data.tiles.length||word>>>12<data.palette_base||word>>>12>=data.palette_base+data.palette_banks){ctx.fillStyle='#bd326e';ctx.fillRect(x,y,8,8);return;}ctx.save();ctx.translate(x+(word&1024?8:0),y+(word&2048?8:0));ctx.scale(word&1024?-1:1,word&2048?-1:1);ctx.drawImage(getAtlas(word>>>12),tile%32*8,Math.floor(tile/32)*8,8,8,0,0,8,8);ctx.restore();}));ctx.strokeStyle='#ff72c7';ctx.lineWidth=2/scale;ctx.strokeRect(.5,.5,viewWidth-1,viewHeight-1);}
async function loadConnectionPreview(name,link,canvas){try{let data=connectionMapCache.get(name);if(!data){data=await api('map/'+encodeURIComponent(name));connectionMapCache.set(name,data);}if(canvas.isConnected)renderConnectionMap(data,link,canvas);}catch(error){if(canvas.isConnected){const context=canvas.getContext('2d');context.fillStyle='#492a2d';context.fillRect(0,0,canvas.width,canvas.height);context.fillStyle='#ffd0c8';context.fillText('Preview unavailable',8,18);}}}
function connectionCard(link,incoming=false){const div=document.createElement('div');div.className='connection';const strong=document.createElement('strong'),detail=document.createElement('span'),button=document.createElement('button'),canvas=document.createElement('canvas');canvas.className='connection-preview';const destination=incoming?map.name.replace(/\.KMP$/,''):link.destination,mapName=String(destination).toUpperCase()+'.KMP',available=incoming||catalog.maps.some(item=>item.name===mapName);strong.textContent=incoming?`From ${link.script}`:`To ${destination}`;detail.textContent=`(${link.x??'?'}, ${link.y??'?'}) · CODE +0x${Number(link.offset).toString(16).toUpperCase()}${available?'':' · map unavailable'}`;button.textContent=incoming?'Open script':'Open map';button.disabled=!available;button.onclick=()=>{if(incoming){$('scripts').value=link.script;$('scripts').dispatchEvent(new Event('change'));}else openDestination(destination);};div.ondblclick=event=>{if(button.disabled||event.target===button)return;if(incoming){$('scripts').value=link.script;$('scripts').dispatchEvent(new Event('change'));}else openDestination(destination);};if(!incoming)div.dataset.destination=mapName;if(!available)div.classList.add('warning');div.append(strong,detail,button,canvas);if(available)loadConnectionPreview(mapName,link,canvas);return div;}
function renderConnections(){for(const [id,items,incoming] of [['outgoing-links',script?.analysis?.field_loads||[],false],['incoming-links',map?.incoming_field_loads||[],true]]){const div=$(id);div.replaceChildren();if(!items.length){const p=document.createElement('p');p.className='empty';p.textContent='None decoded.';div.append(p);}else items.forEach(item=>div.append(connectionCard(item,incoming)));}}
function openDestination(name){const full=String(name||'').toUpperCase()+'.KMP';if(!catalog.maps.some(item=>item.name===full)){status(`Destination ${name} is not a decoded editable map.`,true);return;}if(dirty&&!confirm('Discard unsaved map and event edits?'))return;loadMap(full);}
function setTool(next){tool=next;document.body.dataset.tool=tool;document.querySelectorAll('.tool-palette .tool').forEach(button=>button.classList.toggle('active',button.dataset.tool===tool));}
function setMode(next){if(next==='collision'&&map&&!map.document.attributes){status('This map has no decoded attribute plane.',true);return;}mode=next;document.body.dataset.mode=mode;document.querySelectorAll('#modes button').forEach(button=>button.classList.toggle('active',button.dataset.mode===mode));$('inspector-title').textContent={map:'Map tools',collision:'Collision',events:'Decoded events',connections:'Field connections',scripts:'Scripts'}[mode];if(mode==='collision'){$('layer').value='attributes';refreshAttributeInspector();}else if(mode==='map'&&$('layer').value==='attributes')$('layer').value='0';if(mode==='scripts')loadMarscriptTab();drawMap();renderConnections();}
function drawArgs(){drawMap();const div=$('arguments');div.replaceChildren();if(!script||$('calls').value==='')return;const call=script.calls[Number($('calls').value)];if(!call)return;const title=document.createElement('p');title.textContent=`${call.function} (${call.count} arguments)`;div.append(title);const description=hitDescription(call);if(description){const note=document.createElement('p');note.textContent=description.note;div.append(note);}if(script.has_marscript_override){const values=call.decoded_arguments||[];values.forEach((arg,i)=>{const p=document.createElement('p');p.textContent=`Argument ${i}: ${arg.kind==='string'?'"'+arg.value+'"':arg.kind==='integer'?arg.value:'register '+arg.register+' (dynamic)'}`;div.append(p);});const note=document.createElement('p');note.textContent='This script has a marscript override — edit it in the Scripts tab instead; argument edits here would have no effect.';div.append(note);return;}if(!call.editable){const values=call.decoded_arguments||[];values.forEach((arg,i)=>{const p=document.createElement('p');p.textContent=`Argument ${i}: ${arg.kind==='string'?'"'+arg.value+'"':arg.kind==='integer'?arg.value:'register '+arg.register+' (dynamic)'}`;div.append(p);});const p=document.createElement('p');p.textContent=values.length?'This call contains a string or dynamic expression; editing is disabled.':'Dynamic or unsupported argument sequence; editing is disabled.';div.append(p);return;}call.arguments.forEach((arg,i)=>{const label=document.createElement('label');label.textContent=`${description?.labels[i]||'Argument '+i} · CODE +0x${arg.offset.toString(16)}`;const input=document.createElement('input');input.type='number';input.min=-2147483648;input.max=2147483647;input.step=1;input.value=script.document.arguments[arg.offset]??arg.value;input.onchange=()=>{const value=Number(input.value);if(!Number.isInteger(value)||value<-2147483648||value>2147483647){status('Argument must be a signed 32-bit integer.',true);drawArgs();return;}remember();script.document.arguments[arg.offset]=value;drawArgs();status('Event argument changed. Save sources to write its JSON override.');};label.append(input);div.append(label);});}
async function loadScript(name,refreshMarscriptTab=true){script=null;previewPlacements.clear();buildPlayback();renderScriptLinks();drawCalls();renderConnections();if(!name){$('script-info').textContent='No script selected.';renderEventSources();if(refreshMarscriptTab&&mode==='scripts')await loadMarscriptTab();return;}try{script=await api('script/'+encodeURIComponent(name));const a=script.analysis||{},counts={all:script.calls.length,sprite:0,hit:0,field:0,other:0};script.calls.forEach(call=>counts[eventClass(call)]++);const labels={all:'All decoded events',sprite:'Objects / sprites',hit:'Hit regions',field:'Field loads',other:'Other calls'};for(const option of $('event-class').options)option.textContent=`${labels[option.value]} (${counts[option.value]})`;$('script-info').textContent=`${name}${script.has_marscript_override?' · marscript override active':''} · ${script.calls.filter(c=>c.editable).length}/${script.calls.length} literal calls editable · ${(a.field_loads||[]).length} field loads · ${(a.sprite_resources||[]).length} sprite resources · ${(a.sprite_properties||[]).length} sprite properties`;const existing=mapEventScripts.find(item=>item.meta.script===name);if(existing)existing.data=script;buildPlayback();renderScriptLinks();renderEventSources();drawCalls();renderConnections();}catch(e){$('script-info').textContent='Read-only: '+e.message;status('Map is editable; this script could not be decoded: '+e.message,true);}if(refreshMarscriptTab&&mode==='scripts')await loadMarscriptTab();}

async function loadMarscriptTab(){const editor=$('script-editor');marscriptDoc=null;scriptEditorDirty=false;if(!script){editor.value='';editor.disabled=true;editor.placeholder='Choose a script above.';$('script-status').textContent='';$('script-save').disabled=true;$('script-reset').disabled=true;return;}editor.disabled=true;editor.placeholder='Loading…';$('script-status').textContent='Loading…';try{marscriptDoc=await api('marscript/'+encodeURIComponent(script.name));editor.value=marscriptDoc.source;editor.disabled=false;$('script-status').textContent=marscriptDoc.has_override?`Editing scripts/marscript/${script.name}.marscript (a saved override — this replaces the original script for the English build).`:'Showing the current decompiled script. No override saved yet; save to create one.';$('script-save').disabled=false;$('script-reset').disabled=!marscriptDoc.has_override;}catch(e){editor.value='';editor.disabled=true;$('script-status').textContent='Could not load: '+e.message;$('script-save').disabled=true;$('script-reset').disabled=true;}}
async function loadMap(name){busy=true;controls();try{map=await api('map/'+encodeURIComponent(name));mapEventScripts=[];atlas=new Map();selectedTile=0;visible=map.document.planes.map(()=>true);$('maps').value=name;const d=map.document;$('dimensions').textContent=`${d.width} × ${d.height} tiles · ${d.width*8} × ${d.height*8} pixels`;options($('layer'),d.planes.map((p,i)=>[i,'Plane '+p.index]).concat(d.attributes?[['attributes','Raw attributes']]:[]));options($('palette'),Array.from({length:map.palette_banks},(_,i)=>[map.palette_base+i,String(map.palette_base+i)]));const field=$('visibility');field.replaceChildren();const legend=document.createElement('legend');legend.textContent='Visible layers';field.append(legend);d.planes.forEach((p,i)=>{const label=document.createElement('label'),input=document.createElement('input');input.type='checkbox';input.checked=true;input.onchange=()=>{visible[i]=input.checked;drawMap();};label.append(input,' Plane '+p.index);field.append(label);});dirty=false;undo=[];redo=[];refreshAttributeInspector();drawTiles();drawMap();status(`Loaded ${name}. ${map.unresolved.length?map.unresolved.length+' unresolved cells are marked pink and preserved.':map.resolved_cells.length?map.resolved_cells.length+' external blank cells were resolved from documented runtime context.':'Sources are unchanged until you save.'}`);const associated=map.scripts[0]||'';$('scripts').value=associated;await loadScript(associated);await loadMapEventSources();}catch(e){status(e.message,true);}finally{busy=false;controls();}}
$('maps').onchange=()=>{const name=$('maps').value;if(dirty&&!confirm('Discard unsaved map and event edits?')){$('maps').value=map.name;return;}loadMap(name);};
$('scripts').onchange=async()=>{const name=$('scripts').value;if(dirty&&!confirm('Switching scripts clears undo history. Save first to keep unsaved event edits. Continue?')){$('scripts').value=script?.name||'';return;}if(scriptEditorDirty&&!confirm('Switching scripts discards your unsaved script text. Save it first to keep it. Continue?')){$('scripts').value=script?.name||'';return;}busy=true;controls();await loadScript(name);undo=[];redo=[];busy=false;controls();};
$('open-script-tab').onclick=()=>{if(!script)return;setMode('scripts');};
$('script-editor').addEventListener('input',()=>{scriptEditorDirty=true;});
$('script-save').onclick=async()=>{if(!script||!marscriptDoc||busy)return;busy=true;controls();try{const data=await api('marscript/'+encodeURIComponent(script.name),{revision:marscriptDoc.revision,source:$('script-editor').value});marscriptDoc=data;scriptEditorDirty=false;$('script-status').textContent=`Saved · compiled to ${data.compiled_size} bytes (original archive slot was ${data.original_slot_size} bytes)`+(data.compiled_size>data.original_slot_size?' · will use the ROM expansion region on build.':'.');$('script-reset').disabled=!data.has_override;status('Saved scripts/marscript override. Run make english to build your changes.');await loadScript(script.name,false);}catch(e){status('Not saved: '+e.message,true);$('script-status').textContent='Error: '+e.message;}finally{busy=false;controls();}};
$('script-reset').onclick=async()=>{if(!script||!marscriptDoc?.has_override||busy)return;if(!confirm('Delete the saved override and go back to the current script bytes?'))return;busy=true;controls();try{const data=await api('marscript/'+encodeURIComponent(script.name),{revision:marscriptDoc.revision,reset:true});marscriptDoc=data;$('script-editor').value=data.source;scriptEditorDirty=false;$('script-status').textContent='Reset to the current script bytes. No override saved.';$('script-reset').disabled=true;status('Removed the marscript override.');await loadScript(script.name,false);}catch(e){status('Could not reset: '+e.message,true);}finally{busy=false;controls();}};
document.querySelectorAll('.tool-palette .tool').forEach(button=>button.onclick=()=>setTool(button.dataset.tool));
$('reload').onclick=()=>{if(map&&(!dirty||confirm('Discard unsaved edits and reload?')))loadMap(map.name);};
$('save').onclick=async()=>{if(!map||busy)return;endStroke();endEventDrag();busy=true;controls();try{const data=await api('map/'+encodeURIComponent(map.name),{revision:map.revision,document:map.document,script:script?{name:script.name,revision:script.revision,document:script.document}:null});map=data;if(data.saved_script)script=data.saved_script;dirty=false;undo=[];redo=[];drawCalls();status('Saved editable JSON sources. Run make or make english to build your changes.');}catch(e){status('Not saved: '+e.message,true);}finally{busy=false;controls();}};
$('undo').onclick=undoEdit;$('redo').onclick=redoEdit;$('calls').onchange=drawArgs;$('calls').ondblclick=()=>{const call=selectedCall();if(call?.function==='FldSet')openDestination(decodedValues(call)[0]);};$('call-filter').oninput=drawCalls;$('event-class').onchange=drawCalls;$('palette').onchange=drawTiles;for(const id of ['zoom','grid','layer','game-order','show-sprites','show-hit-regions','hit-preview','sprite-guide','show-map-event-sources'])$(id).onchange=drawMap;document.querySelectorAll('#modes button').forEach(button=>button.onclick=()=>setMode(button.dataset.mode));
$('preview-play').onclick=()=>{if(!script)return;if(!playback.active)buildPlayback();if(playback.frame>=playback.duration)playback.frame=0;playback.active=true;playback.running=true;playback.lastTime=0;cancelAnimationFrame(playback.request);playback.request=requestAnimationFrame(playbackTick);};
$('preview-pause').onclick=()=>{playback.running=false;cancelAnimationFrame(playback.request);};
$('preview-reset').onclick=()=>{playback.running=false;playback.active=false;playback.frame=0;cancelAnimationFrame(playback.request);updatePlaybackClock();drawMap();};
$('preview-time').oninput=()=>{playback.running=false;playback.active=true;playback.frame=Number($('preview-time').value);cancelAnimationFrame(playback.request);updatePlaybackClock();drawMap();};
window.addEventListener('beforeunload',e=>{if(dirty||scriptEditorDirty){e.preventDefault();e.returnValue='';}});
window.addEventListener('keydown',e=>{if(!(e.ctrlKey||e.metaKey))return;if(e.key.toLowerCase()==='s'){e.preventDefault();$('save').click();}if(e.target.matches('input,textarea'))return;if(e.key==='0'){e.preventDefault();$('fit-map').click();}if(e.key.toLowerCase()==='z'){e.preventDefault();e.shiftKey?redoEdit():undoEdit();}if(e.key.toLowerCase()==='y'){e.preventDefault();redoEdit();}});
// Porymap's tool shortcuts (N/B/E/P), bare keys -- only while a map/collision
// mode canvas tool is actually relevant, and never while typing anywhere.
window.addEventListener('keydown',e=>{if(e.ctrlKey||e.metaKey||e.altKey)return;if(e.target.matches('input,textarea,select'))return;if(!['map','collision'].includes(mode))return;const tools={n:'pencil',b:'bucket',e:'eyedropper',p:'pointer'};const next=tools[e.key.toLowerCase()];if(next){e.preventDefault();setTool(next);}});
(async()=>{if(location.protocol==='file:'){status('This file cannot load maps directly. Run make map-editor (or py tools/map_editor/server.py on Windows), then open the full localhost URL printed in that terminal.',true);return;}try{status('Loading map catalog from the editor server…');catalog=await api('catalog');options($('maps'),catalog.maps.map(m=>[m.name,m.name]));options($('scripts'),[['','No script'],...catalog.scripts.map(s=>[s,s])]);if(catalog.maps.length)await loadMap(catalog.maps[0].name);else status('No supported maps found.',true);}catch(e){status(e.message,true);}})();


// Verified by MapAttributeGetConnectionMask (08072130). These are numeric
// classes, not bit fields or a general-purpose walkability flag.
function isConnectionAttribute(value) {
    return (value >= 400 && value <= 499) || (value >= 5400 && value <= 5499);
}

function describeAttribute() {
    const value = Number($('attribute').value);
    const maximum = map?.document.attributes?.word_size === 1 ? 255 : 65535;
    $('attribute').max = maximum;
    $('attribute-meaning').textContent = !Number.isInteger(value) || value < 0 || value > maximum
        ? `Enter an integer from 0 to ${maximum}.`
        : isConnectionAttribute(value)
            ? `${value}: procedural connection class. Neighbor scans accept this value; ordinary field collision is not yet decoded.`
            : `${value}: meaning not yet decoded. The exact value is preserved when saving.`;
}

function refreshAttributeInspector() {
    const counts = new Map();
    for (const value of map?.document.attributes?.entries || [])
        counts.set(value, (counts.get(value) || 0) + 1);
    options($('attribute-values'), [['', 'Choose an existing value'],
        ...[...counts].sort((a,b) => a[0]-b[0]).map(([value,count]) =>
            [value, `${value} · ${count} tiles${isConnectionAttribute(value) ? ' · procedural connection' : ''}`])]);
    describeAttribute();
}

$('attribute').addEventListener('input', describeAttribute);
$('attribute-values').addEventListener('change', () => {
    if ($('attribute-values').value === '') return;
    $('attribute').value = $('attribute-values').value;
    describeAttribute();
});
