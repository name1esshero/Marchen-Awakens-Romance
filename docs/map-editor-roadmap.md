# Map editor UI and event roadmap

The editor UI will follow the interaction model documented by
[Porymap's event editor](https://github.com/huderlem/porymap/blob/master/docsrc/manual/editing-map-events.rst),
while using MAR's recovered KMP, SPC, sprite, and field-transition formats.
Porymap is a design reference; Pokémon-specific structures are not assumed to
exist in MAR.

## Workspace structure

The target window has persistent project and map navigation, a large central
map viewport, and a property inspector on the right. Modes above the viewport
separate **Map**, **Collision**, **Events**, **Connections**, and **Scripts**.
Changing mode changes the palette and inspector without opening another page.
Selection, zoom, layer visibility, and map position remain stable between modes.

The current editor already supplies the central viewport, visual tile palette,
a Porymap-style tool palette (Pencil/Bucket Fill/Eyedropper/Pointer, with the
usual N/B/E/P shortcuts), layer visibility, raw attributes, script calls,
decoded sprite previews, hit rectangle previews, and atomic source saves.
Wheel zoom now reaches 3.125%, and **Fit whole map** selects the largest scale
that displays every tile.

The workspace now has persistent **Map**, **Collision**, **Events**,
**Connections**, and **Scripts** modes. Events can be filtered by decoded
class and selected from the map when a hit rectangle or field-load marker is
clicked. Connections lists both incoming and outgoing `FldSet` dependencies
and navigates to decoded maps. Scripts is a full marscript text editor (see
`docs/marscript-language.md` and the "Safe source editing milestones" section
below) -- the script picker itself lives there now rather than duplicated in
every mode's sidebar.
The selected call's source offset and editable literal arguments remain visible
in the inspector.

Event mode now builds a map-level source list instead of showing only the
same-basename script. Direct field, spawn, character-event, history-event, and
incoming field-load associations are loaded together. Every literal sprite
placement in those direct sources is drawn with its real decoded NCD frame and
a source-script label, so objects contributed by companion scripts no longer
disappear merely because another script is selected. Clicking one of those
actors switches to its owning script and selects its stable source event, so it
can be inspected or dragged without manually finding the script first. Decoded `chain` and
`exec` dependencies with decoded map, sprite, or movement events are listed in
the same source browser and loaded on demand. This puts Dorothy's `EV_BA03` and
`EV_BA04` scenes one click from `MAP01_3A`, including their real artwork and
movement preview. Chained scripts are not all overlaid automatically:
dispatcher scripts contain mutually exclusive branches for many unrelated
scenes, and drawing every branch would misrepresent runtime state.

Event mode now includes a playback transport. It previews literal sprite
creation, resource/animation changes, X/Y assignments, blocking X/Y `SprMove`
tweens, and frees in CODE order. NCD frame durations animate the actual
decoded frames. A sprite-resource tray also shows actors whose placement comes
from registers or external engine state; these remain visibly unplaced rather
than disappearing. Full control-flow execution and runtime register resolution
remain required for an emulator-equivalent cutscene preview.

For actors inherited from engine state, the resource tray offers a session-only
center-map staging position. The preview can then propagate verified X/Y
`SprGet` results through register 0 into a following `SprMove`. This exposes the
Dorothy `EV_BA03`/`EV_BA04` movement animation without saving a guessed spawn
position into source data.

Numbered maps now list their existing `SP_M...`, `CH_M...`, and `HI_M...`
companion scripts beside verified same-basename and incoming `FldSet` scripts.
The filename-family relationship is visibly labeled as inferred. This closes
the navigation gap that hid Dorothy's EV_BA03/EV_BA04 chain behind CH_M01_3;
it does not invent a KMP event table or an unproved spawn coordinate.

## Event mode

Event mode should show all decoded events on the map and select them by clicking
their marker or choosing them in a list. The right inspector will be divided by
MAR event class:

| MAR event class | Confirmed source | Planned inspector fields |
|---|---|---|
| Script-created object | `SprInit`, `SprChg`, and `SprSet` calls | Local editor ID, sprite resource, animation/frame, initial X/Y, source script, and source offsets |
| Hit region | `HitInit`, `HitHitRect`, `HitSet`, and `HitFree` | Region ID, X/Y, width/height, mode, source script, and source offsets |
| Field load | decoded `FldSet` calls | Loaded map, viewport coordinates, source script, and a link to open that map |
| Trigger/script attachment | pending command and map-table decoding | Tile or rectangle, activation condition, script, and arguments |
| Dynamic object | pending object lifetime/control-flow decoding | Spawn condition, initial placement, movement behavior, visibility state, and script |

The interaction sequence follows Porymap where MAR supports it: add by event
type, duplicate, delete, drag to move, coordinate inputs, multi-selection, and
double-click navigation for linked field transitions. Unsupported or dynamic
expressions remain visibly read-only instead of being guessed.

## Collision and connections

Collision mode will replace the numeric attribute input with named behavior,
elevation/layer, and raw-value fields once the KMP attribute consumers are
fully decoded. The raw word stays editable so unknown bits survive. This follows
the principle in Porymap's
[tileset editor](https://github.com/huderlem/porymap/blob/master/docsrc/manual/tileset-editor.rst)
of showing layer and behavior properties together, but MAR's bit meanings must
come from MAR code.

Connections mode lists incoming and outgoing `FldSet` dependencies and opens a
loaded map on double-click. Each link renders a 240×160 section of the actual
connected map at the decoded viewport coordinates. Its coordinates are not yet
labeled as player warps. Creating a field-load call will emit the decoded MAR
script form only after variable-length script rebuilding is supported.

## Safe source editing milestones

1. **Done:** instruction semantics and branch-target labeling for the SPC
   assembler -- `tools/marscript.py`'s flow-tracing disassembler/reassembler,
   proven byte-exact against every real script (see
   `docs/marscript-language.md`).
2. **Implemented for decoded call sites:** every script response contains a
   versioned event model and stable `SCRIPT.SPC:CODE_OFFSET` IDs. These remain
   stable across literal argument edits and byte-exact decompile/recompile
   cycles. A future structural insertion can deliberately issue new IDs when
   code offsets move.
3. **Implemented for existing literal sprites and hit regions:** drag editing,
   optional 8-pixel snapping, signed-coordinate clamping, and undo/redo. Field
   loads containing embedded strings remain read-only under the fixed-size SPC
   patcher -- **superseded for any script with a marscript override** (see
   below), which can freely rewrite string arguments and everything else.
4. **Done, for `mar_english.gba`:** the Scripts tab writes a full marscript
   override (`scripts/marscript/NAME.SPC.marscript`), no longer limited to
   same-size edits -- `tools/marscript_rom_build.py` places a grown script in
   the English build's ROM expansion region automatically, updating that
   script's resource-catalog entry via a linker symbol rather than a
   precomputed offset. Proven against a real build, not just in isolation
   (`tests/test_marscript_rom_build.py`'s end-to-end test actually grows a
   script and links the ROM). `mar.gba` is untouched by design -- it has no
   expansion region and must always match `baserom.gba` exactly, so this
   milestone deliberately doesn't apply to it. Event/object *insertion* (a
   new sprite or hit region that didn't exist before, as opposed to rewriting
   an existing script's logic) still isn't a first-class editor concept the
   way Porymap's event list is, since MAR has no separate structured event
   table to insert into -- events live entirely in script bytecode.
5. Add new-map registration after the archive directory, map lookup tables,
   initial player placement, and field-load resource lifetime are all writable.
6. Validate generated Japanese and English ROMs, reject stale sources, and show
   warnings before saving unresolved references.

## Current sprite-placement audit

`make map-audit` scans all named SPC resources. The current corpus contains 201
sprite resource calls: 135 have a literal initial position, three reuse an actor
positioned earlier in the same script, and 63 require state from another script
or a runtime expression. Fifty-two of those 63 calls contain dynamic arguments.
Artwork previews resolve for 197 calls. Dorothy is therefore one visible example
of a broader state-flow problem that also covers enemies, allies, treasure,
effects, and scene objects. The machine-readable call inventory is
`maps/sprite_placement_audit.json`.

Useful Porymap conventions to retain include named local IDs, destination-warp
navigation, event-type add menus, a selected-event index control, warnings for
invalid placement, keyboard duplication/deletion, and an optional ruler. Its
[current shortcuts](https://github.com/huderlem/porymap/blob/master/docsrc/manual/shortcuts.rst)
are a good default vocabulary for the eventual desktop-style UI.

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

## Recovered attribute search and entry overlap

`MapCollectAttributePositions` (08072924) is now matching C. It scans a
half-open rectangle in tile coordinates, row by row, for an exact attribute
value and writes matching X/Y pairs into separate arrays. It stops at the
caller's capacity and returns a boolean, not the number of matches. Attribute
-1 can select out-of-map probes through `KmpReadAttribute`. The engine's caller
must supply a positive capacity; the original writes before testing the limit.

`MapGenerationFindOverlappingEntry` (08072B48) is also matching C. It tests
active current-field entries against actor bounds, mirroring horizontal corner
offsets when the supplied facing value is 3. Entry vectors contain X/Y plus
width/height, and touching edges count as overlap. It returns a field ID, not an
entry index. This recovered geometry can inform procedural-event previews;
it does not establish a universal tile-trigger or NPC spawn format.

### Fixed a catalog filter that hid 143 of 193 extracted maps

`Project.__init__` limited the editor's map catalog to KMP entries whose name
started with `MAP`, plus a small allowlist proven by `maps/runtime_scenes.json`
evidence. That heuristic dates back to the editor's first commit and was never
about whether a map actually decodes — it just reflected which maps had been
checked by hand at the time. Every excluded map (the `AD0`/`AD1`/`AD3`/`AD4`
area screens, `NET`, `OP`, `SP`, `ST`, `T`, and others, 143 files total) decodes
correctly through the exact same `entry()`/`map_document()` path already
trusted for the visible 49, so the filter is gone: the catalog now offers every
`.KMP` manifest entry and lets the existing per-map `try`/`except` in
`catalog()` sort real failures into `unsupported`. Only one map, `EFCSTART.KMP`,
lands there: its header names a `TEST_M00.KCG` tile source that was never
extracted as an asset, so it is left unsupported rather than force-decoded
against the wrong tiles. `tests/test_map_editor.py` now asserts on this
specific, documented exception instead of requiring zero unsupported maps.
