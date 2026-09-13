'use strict';
const $=id=>document.getElementById(id);
let token=new URLSearchParams(location.hash.slice(1)).get('token')||'';
try{token=token||sessionStorage.getItem('mar-editor-token')||'';if(token)sessionStorage.setItem('mar-editor-token',token);}catch{}
if(location.protocol!=='file:'){try{history.replaceState(null,'',location.pathname);}catch{}}
let map=null,script=null,selectedTile=0,dirty=false,busy=false,stroke=null,visible=[],undo=[],redo=[],atlas=new Map(),catalog=null;
const clone=x=>JSON.parse(JSON.stringify(x));
function status(message,error=false){$('status').textContent=message;$('status').className=error?'error':'';}
function controls(){document.querySelectorAll('button,select,input').forEach(element=>element.disabled=busy);$('save').textContent=dirty?'Save sources *':'Save sources';$('undo').disabled=busy||!undo.length;$('redo').disabled=busy||!redo.length;}
async function api(path,payload){const options={headers:{'X-Editor-Token':token}};if(payload){options.method='POST';options.headers['Content-Type']='application/json';options.body=JSON.stringify(payload);}const response=await fetch('/api/'+path,options);const data=await response.json();if(!response.ok)throw Error(data.error||response.statusText);return data;}
function options(select,items){select.replaceChildren();for(const [value,label] of items){const o=document.createElement('option');o.value=value;o.textContent=label;select.append(o);}}
function state(){return {document:clone(map.document),events:script?clone(script.document):null,script:script?.name||null};}
function remember(before=state()){undo.push(before);if(undo.length>60)undo.shift();redo=[];dirty=true;controls();}
function restore(s){map.document=s.document;if(script&&s.script===script.name&&s.events)script.document=s.events;drawMap();drawArgs();dirty=true;controls();}
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

function layer(){return $('layer').value==='attributes'?map.document.attributes:map.document.planes[Number($('layer').value)];}
function drawMap(){if(!map)return;const {width,height,planes,attributes}=map.document,zoom=Number($('zoom').value),c=$('map');c.width=width*8*zoom;c.height=height*8*zoom;const ctx=c.getContext('2d');ctx.imageSmoothingEnabled=false;ctx.scale(zoom,zoom);const drawPlanes=planes.map((plane,j)=>({plane,j}));if($('game-order').checked)drawPlanes.reverse();drawPlanes.forEach(({plane,j})=>{if(!visible[j])return;plane.entries.forEach((word,i)=>{const tile=word&1023,x=i%width*8,y=Math.floor(i/width)*8;if(tile>=map.tiles.length||word>>>12<map.palette_base||word>>>12>=map.palette_base+map.palette_banks){ctx.fillStyle='#bd326e';ctx.fillRect(x,y,8,8);ctx.strokeStyle='#fff';ctx.beginPath();ctx.moveTo(x,y);ctx.lineTo(x+8,y+8);ctx.stroke();return;}ctx.save();ctx.translate(x+(word&1024?8:0),y+(word&2048?8:0));ctx.scale(word&1024?-1:1,word&2048?-1:1);ctx.drawImage(tileAtlas(word>>>12),tile%32*8,Math.floor(tile/32)*8,8,8,0,0,8,8);ctx.restore();});});if($('layer').value==='attributes'&&attributes){attributes.entries.forEach((value,i)=>{if(!value)return;ctx.fillStyle=`hsla(${value*47%360},90%,60%,.45)`;ctx.fillRect(i%width*8,Math.floor(i/width)*8,8,8);});}if($('grid').checked){ctx.strokeStyle='#ffffff40';ctx.lineWidth=1/zoom;ctx.beginPath();for(let x=0;x<=width;x++){ctx.moveTo(x*8,0);ctx.lineTo(x*8,height*8);}for(let y=0;y<=height;y++){ctx.moveTo(0,y*8);ctx.lineTo(width*8,y*8);}ctx.stroke();}drawHitPreview(ctx,zoom);drawSpriteGuide(ctx,zoom);}
function point(event){const rect=$('map').getBoundingClientRect(),zoom=Number($('zoom').value);const x=Math.floor((event.clientX-rect.left)/(8*zoom)),y=Math.floor((event.clientY-rect.top)/(8*zoom));return x>=0&&y>=0&&x<map.document.width&&y<map.document.height?{x,y,index:y*map.document.width+x}:null;}
function paint(event,pick=false){if(!map||busy)return;const p=point(event);if(!p)return;const target=layer(),word=target.entries[p.index];$('position').textContent=`Tile (${p.x}, ${p.y}) · pixel (${p.x*8}, ${p.y*8}) · value ${word} / 0x${word.toString(16).padStart(4,'0')}`;if(pick){if($('layer').value==='attributes')$('attribute').value=word;else{if((word&1023)>=map.tiles.length){status('This tile reference is unresolved. Choose a decoded tile to replace it.',true);return;}selectedTile=word&1023;$('palette').value=word>>>12;$('hflip').checked=!!(word&1024);$('vflip').checked=!!(word&2048);drawTiles();}return;}if(!stroke)return;let value;if($('layer').value==='attributes'){value=Number($('attribute').value);const max=target.word_size===1?255:65535;if(!Number.isInteger(value)||value<0||value>max){status(`Attribute must be an integer from 0 to ${max}.`,true);return;}}else value=selectedTile|(bank()<<12)|($('hflip').checked?1024:0)|($('vflip').checked?2048:0);if(word!==value){target.entries[p.index]=value;stroke.changed=true;dirty=true;drawMap();controls();}}
// Wheel zoom is local to the viewport. Keep the same map pixel beneath the
// pointer, except where browser scroll limits require clamping at an edge.
$('map-scroll').addEventListener('wheel', event => {
    event.preventDefault();
    if (!map || busy || !event.deltaY) return;
    endStroke();
    const viewport = $('map-scroll');
    const oldZoom = Number($('zoom').value);
    const newZoom = Math.max(1, Math.min(4, oldZoom + (event.deltaY < 0 ? 1 : -1)));
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
$('map').addEventListener('contextmenu',e=>{e.preventDefault();paint(e,true);});
$('map').addEventListener('pointerdown',e=>{if(e.button!==0||!map||busy)return;stroke={before:state(),changed:false};$('map').setPointerCapture(e.pointerId);paint(e);});
$('map').addEventListener('pointermove',e=>paint(e));
function endStroke(){if(stroke?.changed)remember(stroke.before);stroke=null;}
$('map').addEventListener('pointerup',endStroke);$('map').addEventListener('pointercancel',endStroke);
$('tiles').onclick=e=>{if(!map||busy)return;const r=e.currentTarget.getBoundingClientRect(),i=Math.floor((e.clientY-r.top)/16)*16+Math.floor((e.clientX-r.left)/16);if(i<map.tiles.length){selectedTile=i;drawTiles();}};
function drawCalls(){const value=$('calls').value,q=$('call-filter').value.toLowerCase();options($('calls'),(script?.calls||[]).map((c,i)=>[i,`${c.function} · @${c.offset.toString(16)}${c.editable?'':' · read-only'}`]).filter((_,i)=>script.calls[i].function.toLowerCase().includes(q)));if([...$('calls').options].some(o=>o.value===value))$('calls').value=value;drawArgs();}
function drawArgs(){drawMap();const div=$('arguments');div.replaceChildren();if(!script||$('calls').value==='')return;const call=script.calls[Number($('calls').value)];if(!call)return;const title=document.createElement('p');title.textContent=`${call.function} (${call.count} arguments)`;div.append(title);const description=hitDescription(call);if(description){const note=document.createElement('p');note.textContent=description.note;div.append(note);}if(!call.editable){const p=document.createElement('p');p.textContent='Dynamic or unsupported argument sequence; editing is disabled.';div.append(p);return;}call.arguments.forEach((arg,i)=>{const label=document.createElement('label');label.textContent=`${description?.labels[i]||'Argument '+i} · CODE +0x${arg.offset.toString(16)}`;const input=document.createElement('input');input.type='number';input.min=-2147483648;input.max=2147483647;input.step=1;input.value=script.document.arguments[arg.offset]??arg.value;input.onchange=()=>{const value=Number(input.value);if(!Number.isInteger(value)||value<-2147483648||value>2147483647){status('Argument must be a signed 32-bit integer.',true);drawArgs();return;}remember();script.document.arguments[arg.offset]=value;drawArgs();status('Event argument changed. Save sources to write its JSON override.');};label.append(input);div.append(label);});}
async function loadScript(name){script=null;drawCalls();if(!name){$('script-info').textContent='No script selected.';return;}try{script=await api('script/'+encodeURIComponent(name));$('script-info').textContent=`${name} · ${script.calls.filter(c=>c.editable).length}/${script.calls.length} literal calls editable`;drawCalls();}catch(e){$('script-info').textContent='Read-only: '+e.message;status('Map is editable; this script could not be decoded: '+e.message,true);}}
async function loadMap(name){busy=true;controls();try{map=await api('map/'+encodeURIComponent(name));atlas=new Map();selectedTile=0;visible=map.document.planes.map(()=>true);$('maps').value=name;const d=map.document;$('dimensions').textContent=`${d.width} × ${d.height} tiles · ${d.width*8} × ${d.height*8} pixels`;options($('layer'),d.planes.map((p,i)=>[i,'Plane '+p.index]).concat(d.attributes?[['attributes','Raw attributes']]:[]));options($('palette'),Array.from({length:map.palette_banks},(_,i)=>[map.palette_base+i,String(map.palette_base+i)]));const field=$('visibility');field.replaceChildren();const legend=document.createElement('legend');legend.textContent='Visible layers';field.append(legend);d.planes.forEach((p,i)=>{const label=document.createElement('label'),input=document.createElement('input');input.type='checkbox';input.checked=true;input.onchange=()=>{visible[i]=input.checked;drawMap();};label.append(input,' Plane '+p.index);field.append(label);});dirty=false;undo=[];redo=[];drawTiles();drawMap();status(`Loaded ${name}. ${map.unresolved.length?map.unresolved.length+' unresolved cells are marked pink and preserved.':'Sources are unchanged until you save.'}`);const associated=map.scripts[0]||'';$('scripts').value=associated;await loadScript(associated);}catch(e){status(e.message,true);}finally{busy=false;controls();}}
$('maps').onchange=()=>{const name=$('maps').value;if(dirty&&!confirm('Discard unsaved map and event edits?')){$('maps').value=map.name;return;}loadMap(name);};
$('scripts').onchange=async()=>{const name=$('scripts').value;if(dirty&&!confirm('Switching scripts clears undo history. Save first to keep unsaved event edits. Continue?')){$('scripts').value=script?.name||'';return;}busy=true;controls();await loadScript(name);undo=[];redo=[];busy=false;controls();};
$('reload').onclick=()=>{if(map&&(!dirty||confirm('Discard unsaved edits and reload?')))loadMap(map.name);};
$('save').onclick=async()=>{if(!map||busy)return;endStroke();busy=true;controls();try{const data=await api('map/'+encodeURIComponent(map.name),{revision:map.revision,document:map.document,script:script?{name:script.name,revision:script.revision,document:script.document}:null});map=data;if(data.saved_script)script=data.saved_script;dirty=false;undo=[];redo=[];drawCalls();status('Saved editable JSON sources. Run make or make english to build your changes.');}catch(e){status('Not saved: '+e.message,true);}finally{busy=false;controls();}};
$('undo').onclick=undoEdit;$('redo').onclick=redoEdit;$('calls').onchange=drawArgs;$('call-filter').oninput=drawCalls;$('palette').onchange=drawTiles;for(const id of ['zoom','grid','layer','game-order','hit-preview','sprite-guide'])$(id).onchange=drawMap;
window.addEventListener('beforeunload',e=>{if(dirty){e.preventDefault();e.returnValue='';}});
window.addEventListener('keydown',e=>{if(!(e.ctrlKey||e.metaKey))return;if(e.key.toLowerCase()==='s'){e.preventDefault();$('save').click();}if(e.target.matches('input,textarea'))return;if(e.key.toLowerCase()==='z'){e.preventDefault();e.shiftKey?redoEdit():undoEdit();}if(e.key.toLowerCase()==='y'){e.preventDefault();redoEdit();}});
(async()=>{if(location.protocol==='file:'){status('This file cannot load maps directly. Run make map-editor (or py tools/map_editor/server.py on Windows), then open the full localhost URL printed in that terminal.',true);return;}try{status('Loading map catalog from the editor server…');catalog=await api('catalog');options($('maps'),catalog.maps.map(m=>[m.name,m.name]));options($('scripts'),[['','No script'],...catalog.scripts.map(s=>[s,s])]);if(catalog.maps.length)await loadMap(catalog.maps[0].name);else status('No supported maps found.',true);}catch(e){status(e.message,true);}})();
