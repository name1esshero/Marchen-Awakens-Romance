# MAR map editor

The planned Porymap-style event, collision, and connections workflow is documented in [`docs/map-editor-roadmap.md`](../../docs/map-editor-roadmap.md).

Run `make map-editor`, or `py tools/map_editor/server.py` on Windows.
The server opens its full editor URL in your default browser automatically.
If browser launch is unavailable, open the printed localhost URL manually.
Use `--no-browser` to disable automatic launch.
The URL carries a random per-session token. The server binds only to 127.0.0.1;
it serves the editor UI and validated project APIs, not arbitrary files.
Stop it with Ctrl+C. A different port is available with
`python3 tools/map_editor/server.py --port 8766`.

## Editing and building

Select a map and plane, choose a tile and palette bank, and paint on the map.
Scroll the mouse wheel over the map viewport to zoom from 3.125% to 4× around
the pointer. Use **Fit whole map** to select the largest scale that displays
every tile; Ctrl+0 invokes the same action. Scrollbars pan the map; wheel
scrolling elsewhere behaves normally.
Horizontal/vertical flips, layer visibility, zoom, a grid, right-click picking,
and stroke-level undo/redo are available. Select **Raw attributes** to paint
numeric u8/u16 attributes. Their collision/event meanings are not fully decoded.

Map and Collision mode share a Porymap-style tool palette above the tile
selector: **Pencil** (N) paints the current selection; **Bucket Fill** (B)
flood-fills a contiguous region (matching Porymap's Bucket Fill Tool: whole
connected region only, no fill-all-matching-tiles variant); **Eyedropper**
(E) samples a tile with a left click, the same as right-clicking with any
other tool active; **Pointer** (P) just inspects the hovered cell without
painting. Bare N/B/E/P switch tools while focus isn't in a text field;
right-click always samples regardless of the active tool.

The mode bar separates map painting, collision attributes, decoded event call
sites, field-load connections, and script text. Event mode groups objects, hit
regions, field loads, and other calls; clickable hit/field markers select their
source call. Connections mode shows incoming and outgoing `FldSet` dependencies
and can open a decoded destination map. `FldSet` coordinates position the
viewport; they are not currently presented as proven player warp coordinates.
Every connection card renders the destination's real visual planes in a
240×160 GBA viewport beginning at those coordinates. Incoming cards preview the
same section of the current destination map. Double-clicking an outgoing card
loads its destination map; incoming cards open their source script because an
incoming `FldSet` call does not by itself identify that script's owning map.
The **Map event sources** list combines the map's direct same-name, spawn,
character, history, and incoming-load scripts. Literal sprite placements from
all of those sources are drawn together with their real NCD artwork and source
script names. Click an associated actor to switch to its source call, or click
a source to inspect and play that script. Verified chained
scripts containing map or sprite activity also appear in the source list and
load on demand, so scene actors such as Dorothy are reachable without following
several links by hand. They are not all drawn at once because many are
conditional alternatives selected by dispatcher logic.
Numbered fields also include matching `BTOM...` battle-event scripts. On
`MAP01_3A`, open `EV_BA02.SPC` from that chain to see Dorothy's decoded
`09A00` sprite at her literal `(530, 308)` creation position; `EV_BA03.SPC`
and `EV_BA04.SPC` contain the later movement and removal commands for object 2.
Literal sprite placements and hit rectangles can be dragged in Event mode.
Their verified SPC integer operands update live, optionally snap to the 8-pixel
grid, and participate in the same undo/redo and atomic save path as form edits.
The Event transport previews decoded sprite calls and animates the real NCD
frames using their stored durations. `SprMove` properties 0 and 1 interpolate X
and Y to the command's verified target over its verified duration; its blocking
flag advances the preview clock. Calls with register-derived IDs or targets are
counted as unresolved and skipped. The sprite-resource tray still displays
their decoded artwork—including Dorothy's `09A00` idle and `09A01` movement
resources—so dynamic enemy and ally actors are no longer silently absent.
An unplaced literal actor can be staged at the center of the map for a
non-saving visual preview. This is deliberately session-only because its real
position may come from another script, a register, or engine-owned battle state.
The preview also follows the verified `SprGet` X/Y result into register 0 when
that actor has a known, inherited, or staged position. A map-level origin is
inherited only when all reachable literal placements for that object ID agree;
conflicts stay unresolved. Dorothy's object 2 therefore inherits the unique
`EV_BA02` position `(530, 308)`. Her real `09A01` frames run for the later
`EV_BA03`/`EV_BA04` command intervals, and the former switches her back to
`09A00` without requiring manual staging.
The default **Game BG order** preview draws the second field plane first and
plane 0 last, matching the loader's plane-0/BG0 and plane-1/BG1 assignment and
the GBA's lower-BG-number ordering when priorities tie. Disable it to inspect
planes in their raw file order. Runtime priority changes, animated tiles,
palette effects, and additional resources sharing VRAM are not yet simulated.
The editor draws the first frame of a script-created sprite when one literal
`SprInit` is followed by literal X and Y `SprSet` calls for the same ID. It uses
the decoded NCD cell layout and the frame's engine-relative origin.

The script selector itself lives in the **Scripts** tab (see below), not in
Event mode's own sidebar -- Event mode shows which script is currently
selected and an **Open in Scripts tab** button, and filters/inspects that
script's decoded calls. Filter native calls by name and edit supported signed
integer arguments. Save writes `maps/editable/NAME.KMP.json` and, if selected,
`maps/events/NAME.SPC.json`. These are tracked source inputs. The original KMP
and SPC framing files remain the baseline; unknown fields are preserved.
Read-only calls now display compiler-generated embedded string arguments. This
makes `FldSet("MAP...", x, y)` and sprite resource setup visible while their
variable-length strings remain protected from fixed-allocation edits. If the
selected script has a marscript override (see below), this per-argument editor
shows its decoded values read-only instead: the override is the sole source of
truth for that script's content, and an argument edit here would silently have
no effect once it does.

For numbered fields, **Map scripts** also exposes matching `SP_M...`,
`CH_M...`, and `HI_M...` filename families when those resources exist. The UI
labels this relationship as inferred from the recovered naming convention;
decoded `FldSet` references remain separately marked as verified. MAP01_3A,
for example, links to CH_M01_3, whose decoded script links reach EV_BA03 and
EV_BA04 and their Dorothy movement previews.

## Scripts tab: editing script text with marscript

The **Scripts** tab is a full-text editor for [marscript](../../docs/marscript-language.md),
the readable scripting language that compiles to the game's native SCRP/CODE
bytecode. Choose a script from the dropdown; the editor shows either a saved
override (`scripts/marscript/NAME.SPC.marscript`, if one exists) or the
current script decompiled fresh, so what you first see always matches what
Event mode already shows. Edit the text and click **Save script** to write
the override; **Reset to original** deletes it and reverts to the
decompiled view. Both are validated -- a save that doesn't compile is
rejected before anything is written, matching every other save path in this
editor.

A saved override fully replaces that script's content for `mar_english.gba`
only. `mar.gba` never reads `scripts/marscript/` and is completely
unaffected by anything saved here, by design: the Japanese ROM's whole
purpose is matching `baserom.gba` byte for byte, and it has no ROM expansion
region for grown content to live in even if it wanted to. If your edit no
longer fits the script's original archive slot, saving still succeeds --
`tools/marscript_rom_build.py` places the grown content in the English
build's ROM expansion region automatically at build time, and the status
line says so. Run `make english` (or `make map-editor`'s own build step)
afterward to actually build your changes.

Run `make` or `make english` after saving. An intentionally edited ROM should
differ from baserom. Without overrides, the default Japanese build must still
match every byte. Removing an override restores its original values on the next
build; directory dependencies detect additions and deletions as well as edits.
The English build includes the same map/event edits and its existing dialogue
translation hook; this editor does not implement a separate English map format.

Changes to the loaded map, tile art, palette, text, or event source revision
cause a stale save to fail. Invalid references and scripts that no longer fit
their compressed allocation fail before either file is written. Each file is
replaced atomically; ordinary write failures roll both files back. A machine
crash between two file replacements is not a multi-file transaction guarantee.

## Decoded formats and limits

KMP header evidence comes from the recovered viewport/attribute code at
08002604, 08003104, 08003160 and 08003178; see `include/kmp.h` and `src/kmp.c`.
Dimensions at +14/+18 count 8×8 tiles. Four plane offsets at +9C select row-major
u16 words: bits 0–9 tile index, bit 10 horizontal flip, bit 11 vertical flip,
bits 12–15 palette bank. Attribute offset +AC uses u8 or u16 entries according
to +BA. Tile and palette resource names are at +1C/+5C; palette destination
and count are at +B0/+B4. The editor uses the PNG/layout compiler for artwork.
It preserves map dimensions, plane offsets, the header, and all unknown bytes.

All 47 MAP*.KMP members load. The previously external references in MAP27_A,
PW_BG01, and PW_BOX are described in `maps/tile_resolutions.json` and render as
transparent without changing their source words. The PW resolutions are proved
by the scene's VRAM clear and the transparent PNG tail; MAP27_A's lone 0x03FF
border marker is explicitly marked as an inference. Any future reference with
no documented resolution remains pink, and the editor refuses to introduce
additional external references. Layer order, blending, scrolling, and affine
effects are not emulator-verified.

All 334 named SPC FUNC tables decode (33,064 native-call references).
FUNC records contain a NUL-terminated name and u32 CODE references ending in
zero. Each reference points to 29/u32 argument count followed by 80/u32 native
call. The editor allows the 13,643 calls whose integer pushes are contiguous
and follow the observed 31/0F-u32 statement prefix. Other expressions remain
read-only; these counts are from unedited source scripts, not runtime execution.
The NVAR chunk observed in G_EV023 uses a length including its eight-byte
header; FUNC and CODE use payload lengths. NVAR content is preserved read-only.

The native registry links SprSet to 08011F40 (delegating to 08010E44), HitSet to
08012208, and FldSet to 080122F8. Most sprite properties remain undecoded.
The hit-region helpers and collision test now compile as byte-matching C in
`src/hit_region.c`; their layouts are documented in `include/hit_region.h`.

### Hit regions

The engine has sixteen 16-byte records at engine state +0x1090. Each stores
an active word, signed 16-bit x/y/width/height, and a mode word. The actor's
input rectangle instead stores left/top/right/bottom corner offsets. The
collision test at 08018C4C translates those corners by the actor position,
then visits regions in index order and returns the first matching index + 1.
Zero means no hit. Mode 0 tests overlap, mode 1 strict containment; touching
edges does not count. Other mode values never hit. Negative widths and heights
are preserved by the original implementation, not normalized.

| Native call | Verified behavior |
| --- | --- |
| `HitInit(id, x, y, width, height)` | Activates the region, stores the rectangle, and resets mode to 0. Handler 080121C4 calls 08011530. |
| `HitHitRect(id, x, y, width, height)` | Activates and replaces the rectangle, preserving mode. Handler 08012244 calls 08011654. It does not perform a hit test. |
| `HitFree(id)` | Disables one region; -1 disables all sixteen. Geometry and mode remain stored. Handler 080121E4. |
| `HitSet(id, property, value)` | Updates active regions only: 54=x, 55=y, 56=width, 57=height, 61=mode. Other property values are unsupported. Handler 08012208 calls 08011554. |

The editor labels these arguments and offers **Preview selected hit rectangle**
for literal HitInit/HitHitRect calls. It projects that individual call onto the
current map, including signed 16-bit truncation, and follows edits and undo.
It does not execute the script, combine branches, or claim that a region is
currently active. Selecting another script can project unrelated geometry.

The registry's `HitGet` handler at 08012230 calls the sprite-property getter
at 08011174, which looks up a sprite through 080106C8. It does not call the
separate hit-region getter at 080115CC. The editor therefore does not label
HitGet as reading these region fields merely because of its name; that original
routing needs further investigation.

There is no event insertion/deletion, control-flow editing, or graphical warp
placement yet. Script call order and same-name associations do not establish
which calls execute in a particular scene. This remains a partial source
editor, with no emulator-verified event simulation.

## Verification

`python3 -m unittest discover -s tests -p test_map_editor.py` checks every map,
every script table, preservation of nonedited bytes, conflict detection,
compression overflow, two-file rollback, and authenticated HTTP save/reload.
HTTP tests require localhost socket access.
`python3 -m unittest discover -s tests -p test_hit_region.py` checks the C
helpers, edge-touch behavior, containment, first-hit ordering, and 500
deterministic randomized comparisons against a rectangle oracle.

`python3 tools/exercise_map_editor.py` performs a reversible real-ROM proof:
a map flip, raw attribute and SprSet immediate are saved through the editor
model and rebuilt. It checks the exact resulting map/script spans, removes the
overrides, and checks that Make detects the removal and restores byte equality.
Build subprocesses cannot read baserom. The proof refuses to overwrite existing
MAP01_A overrides and must run without concurrent source editing/building.
Results go to ignored `reports/build/map-editor-proof.json`.

For an optional real-browser interaction check, start
`python3 tests/map_editor_browser_server.py`, then run
`node tests/map_editor_browser.cjs build/map-editor-browser.json` in another
terminal with Node.js, `playwright-core`, and its Chromium browser installed.
`MAR_PLAYWRIGHT_MODULE` can point to a separately installed module. The server
uses temporary copies of two maps and a separate hit-region script, and the browser driver refuses a session
not marked as that isolated fixture. The browser also checks hit-rectangle
labels, rendered pixels, live edits, and undo. Ctrl+C removes the fixture. Screenshots
and results go to ignored `reports/maps/`. No browser package is required to
run the editor itself.

## Recording the README demonstration

The tracked `docs/media/map-editor.gif` consists of actual browser screenshots
of the editor loading a field, changing between its four pages, and running the
literal movement in EV_ICE02 with the Play button. No replacement UI or
generated artwork is used. Initial browser network latency is set to 800 ms so
the real loading state is visible, then removed before interaction.
To regenerate it, start the isolated browser fixture
described above, then run:

```sh
node tools/map_editor/record_demo.cjs build/map-editor-browser.json
python3 tools/map_editor/make_demo_gif.py
```

Recording requires Playwright with Chromium; GIF encoding requires Pillow.
These are optional documentation tools, not editor or ROM build dependencies.
Raw PNG frames and their capture manifest stay in ignored
`build/map-editor-demo/`; only the resulting GIF belongs in documentation.

The viewport dispatch at 08002630 is also recovered as byte-matching
`KmpRenderViewport` in `src/kmp.c`. It chooses 08002650 for mode zero and
08002798 otherwise, forwarding the original fixed-point coordinates. The
rendering paths themselves remain in assembly.

### Field loading

`FldSet` at 080122F8 delegates to the matching C `KmpLoadField` at 080032B8
in `src/map_field.c`. Its arguments are a resource basename and two signed
16-bit pixel coordinates. It saves the basename through 08006A1C, appends
`.KMP`, uppercases the result, and loads plane 0 into background/viewport 0
and plane 1 into background/viewport 1. Both share tile VRAM at 06000000.
The first load uses flags 3 (palette and tiles); the second uses flags 0,
reusing those resources. The coordinates are multiplied by 65536 and passed
to both viewport render calls. This is viewport positioning; actor spawning
and event transitions still need separate tracing.

The original filename buffer is sixteen bytes and uses unchecked copies;
source edits must keep the basename plus `.KMP` and NUL within that limit.
The slot stride is 0xFC bytes, with slots 0 and 1 at 03003BC4 and 03003CC0.
`KmpSetClip` (08003280) writes the four existing clip fields, and `KmpResetClip`
(08003294) restores zero origins and the KMP's tile dimensions. These also
compile to the original bytes. `tests/test_map_field.py` checks the loader's
call order, names, resource flags, slot stride, signed coordinate extremes,
and clip reset behavior using the actual C with mocked hardware calls.

The script adapters for `FldSet` (080122F8), `HitInit` (080121C4),
`HitHitRect` (08012244), and `HitFree` (080121E4) are now matching C in
`src/map_native.c`. They read the VM argument array without checking its
count and leave the result pointer untouched. FldSet truncates its coordinate
arguments to signed 16-bit pixels. HitFree selects the all-regions helper
only for index -1. The field and free adapters return 0x7FFF; the init and
rectangle adapters return 1. These values are preserved without yet assigning
complete VM scheduling semantics to them. `tests/test_map_native.py` checks
argument forwarding, truncation, dispatch, return values, and result retention.

### Sprite coordinate evidence

`SprSet(id, 0, value)` sets X and property 1 sets Y, in signed 16-bit pixels.
Enable **Preview selected sprite X/Y guide** to draw that one assignment as a
yellow vertical or horizontal guide; edits and undo update it immediately.
The editor does not infer a second coordinate or execute branches. Other sprite
properties remain numeric. See [runtime trace](../../docs/sprite-rendering.md)
for the connection to NCD animation selection and camera-relative rendering.

`SprChg` is now traced to matching C: it replaces an active sprite's named NCD
resource, animation and frame while preserving its position. `SprInit` creates
an object asynchronously and starts at frame zero. The symbolic operand decoder
recovers embedded resources from most of these expression sequences while
keeping dynamic IDs explicit. The editor API reports static `field_loads`,
`sprite_resources`, `sprite_properties`, and `sprite_moves`;
`maps/script_catalog.json` records the same data across every named script.
Its version-2 records also include decoded `chain`/`exec` dependencies, which
the editor uses to expose the event-source graph without embedding ROM
addresses or precomputed file offsets.
`maps/sprite_placement_audit.json` separates literal initial positions from
same-script actor reuse and resources whose position comes from another script
or a runtime expression. Regenerate both with `make map-audit`.
The viewport's **Show script-created sprites at initial positions** option
joins literal IDs across those setup calls and draws the source PNG for the
selected animation's first frame. These remain possible call sites rather than
executed branch state; register-derived IDs or coordinates remain omitted.

### Tilesets versus map layers

The normal field loader (`KmpLoadField`, 080032B8) loads one KMP twice: plane 0 with flags 3 (copy palettes and tiles), then plane 1 with flags 0 (reuse that data). Both use VRAM 06000000 and zero tile/palette offsets. These are two map planes sharing one KCG/KCL pair, not a primary/secondary tileset pair.

The lower-level loader at 08003178 accepts a VRAM destination, viewport slot, plane, palette-bank offset, tile-index offset, and copy flags. It stores the offsets at viewport +0C/+0E. The regular renderer at 08002700/0800274A adds their packed value to each u16 screen entry. Special-scene resource sharing remains to be traced; tile 1023 is not automatically a blank sentinel in this loop.

The two renderer bodies now establish what the viewport mode changes. Mode zero
copies a clipped 32×32 window into a 2,048-byte regular-BG screen buffer. Each
u16 entry retains tile bits 0–9, horizontal/vertical flip bits 10–11, and
palette-bank bits 12–15 after the viewport offsets are added. Nonzero mode
copies the low byte of each source entry into a 1,024-byte 32×32 affine-BG map;
the high tile, flip, and palette bits are discarded by that path. Both paths
select the source with `planeOffsets[viewport->plane]`. BG register assignment,
priority, and blending are still controlled outside these copy loops.

### Tile attributes and collision evidence

KMP offset +`0xAC` points to one row-major attribute value per 8×8 map tile.
Header field +`0xBA` selects u8 or u16 values. `KmpReadAttribute` converts pixel
coordinates to tile coordinates, checks the map bounds, and reads this array;
it returns -1 outside the map. The attribute plane is independent of the visual
planes at +`0x9C`, so changing artwork does not itself change movement data.

The procedural connection helper at 08072130 samples the four neighboring
attributes. Values 400–499 and 5400–5499 are accepted for its connection mask.
That proves those ranges have traversal/connectivity meaning in generated-map
logic, but it does not yet prove a universal “walkable” rule for every field.
The remaining work is to trace ordinary player movement, exits, and event
dispatch and then label the full attribute dictionary. The editor therefore
keeps attributes editable as exact numeric values rather than guessing names.

### PW scene loading context

`maps/runtime_scenes.json` records the two loader calls at 08066870 and 0806688E. PW_BG01 plane 0 loads at 06000000 in viewport 0; PW_BOX plane 0 loads at 06004000 in viewport 1 with palette offset 1. Both copy tiles and palettes (flags 3) and use tile-index offset 0. The box viewport is rendered at (-60, -20) pixels. Scene setup at 08066780 first clears 64 KiB beginning at 06000000. This proves that PW_BG01 tile 1023 and PW_BOX tiles 426–430 read transparent zero-filled VRAM; the editor resolves those cells through `maps/tile_resolutions.json`.

### Toward sprite and event authoring

SprInit's native adapter (08011ECC) is now readable, byte-matching agbcc C. It forwards five script arguments to the creation task at 08010AEC with fixed arguments 1 and 0, returns 1, and leaves the result slot untouched. This does not yet make insertion of new script statements safe.

`tools/script_assembler.py` now emits the verified bytecode subset and rebuilds
FUNC relocations from readable JSON; `make script-sources` compiles all source
files under `scripts/source/`. Remaining prerequisites are lossless rewriting
of arbitrary existing branch graphs, trigger dispatch rather than guessed tile
attributes, and archive/scene registration for new KMP/KCG/KCL/SPC resources.
The editor currently edits existing verified literal arguments; it cannot yet
create/register a new map or attach an arbitrary script to a tile or sprite.

### Sprite creation worker (08010B6C)

The 160-byte worker now compiles from C to the original bytes. State 0 calls the preparation task at 0801097C with the sprite ID, payload +40, and a completion-word pointer, then transitions to state 16. State 16 waits until payload +44 is nonzero. It then allocates and clears a 72-byte auxiliary block, runs 08008A70 on it, activates the sprite, resolves the named NCD group, copies animation/frame, and updates the original flag bits. Finally it decrements the pending-script-task count, writes -1 through the task result pointer if present, and finishes the task.

The worker itself does not assign X/Y. The preparation task still needs tracing before concluding what an entire SprInit operation does to old position/state. Auxiliary-block and unnamed flag semantics also remain unresolved. Host tests cover waiting, completion, unsupported states, and preservation of coordinates in this worker; these are not an emulator playthrough.

### Deferred sprite reset (080109C4 / 08010A70)

The single-slot (104 bytes) and all-slot (124 bytes) workers are now matching C. An active slot waits while its u16 at +0x1A is nonzero; once ready, its auxiliary block at +0x24 is torn down through 08008BD8 and freed. Reset clears all 40 bytes, then sets the two signed fields at +0x14/+0x16 to 256 and draw-order bits to 3. Completion decrements the pending-operation counter, writes -1 to an optional task result pointer, and finishes the task.

Single reset clears inactive records too. Reset-all scans exactly 32 slots, skips inactive records, and stays pending while any active slot is still busy. Host tests cover these differences, teardown order, waiting, default fields, and optional result pointers.

Consequently the SprInit preparation phase clears X/Y before creation proceeds. A future sprite-placement compiler must account for this asynchronous reset rather than assigning coordinates before it. Trigger dispatch, statement insertion/relocation, and new-map archive registration remain unfinished.

### Attribute inspection and generation-entry cleanup

Collision mode now lists the exact attribute values present in the map with
their tile counts. Choose a value to use it as the brush, or right-click a tile
to sample it. The inspector explains the verified procedural connection classes
400–499 and 5400–5499; those cells have a green overlay. Other colors only
distinguish raw values. They do not declare a tile walkable or blocked. Unknown
values remain editable and survive saves unchanged. Counts refresh after a
paint stroke and undo/redo.

`MapGenerationClearCurrentFieldEntries` (080729F4, 68 bytes) now replaces its
assembly body with matching C. It checks the low byte of generation state +08,
then clears nonzero +26 entries whose signed +2E field matches the active field
identifier. This is generation-state cleanup, not evidence of a tile-trigger
format. Trigger attachment, arbitrary script insertion and new-map registration
remain unfinished.

The adjacent lookup at 08072A38 now compiles to byte-matching C after preserving
its original shared-found-block control flow. This lookup proves
that generation arrays +38/+48 contain integer tile X/Y values, not pointers.
Their accessors and declarations now use coordinate names and signed integers.
The first active matching entry yields `(tile + 1) * 8` pixel coordinates; this
is not enough evidence to assign arbitrary NPC spawn positions.

The following 112-byte helper at 08072A98 is also matching C. For active
entries belonging to the current field, it decrements a signed countdown. A
countdown reaching zero clears the entry state; other values set the entry's
update-pending byte. The state fields at +26, +2E, +58, and +68 are consequently
named `entryState`, `fieldId`, `countdown`, and `updatePending`. These records
belong to procedural field generation and are not general scripted NPC events.

For the collision inspector's JavaScript regression checks, run
`node tests/map_attribute_inspector.cjs` from the project root. Source save and
validation checks run with `python3 -m unittest discover -s tests -p 'test_map_editor.py'`.
