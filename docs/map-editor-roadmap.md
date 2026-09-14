# Map editor UI and event roadmap

The editor UI will follow the interaction model documented by
[Porymap's event editor](https://github.com/huderlem/porymap/blob/master/docsrc/manual/editing-map-events.rst),
while using MAR's recovered KMP, SPC, sprite, and field-transition formats.
Porymap is a design reference; Pokémon-specific structures are not assumed to
exist in MAR.

## Workspace structure

The target window has persistent project and map navigation, a large central
map viewport, and a property inspector on the right. Modes above the viewport
separate **Map**, **Collision**, **Events**, and **Connections**. Changing mode
changes the palette and inspector without opening another page. Selection,
zoom, layer visibility, and map position remain stable between modes.

The current editor already supplies the central viewport, visual tile palette,
layer visibility, raw attributes, script calls, decoded sprite previews, hit
rectangle previews, and atomic source saves. Wheel zoom now reaches 3.125%, and
**Fit whole map** selects the largest scale that displays every tile.

The workspace now has persistent **Map**, **Collision**, **Events**, and
**Connections** modes. Events can be filtered by decoded class and selected from
the map when a hit rectangle or field-load marker is clicked. Connections lists
both incoming and outgoing `FldSet` dependencies and navigates to decoded maps.
The selected call's source offset and editable literal arguments remain visible
in the inspector.

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

1. Finish instruction semantics and label branch targets in the SPC assembler.
2. Represent decoded events in a versioned JSON model with stable local IDs.
3. **Implemented for existing literal sprites and hit regions:** drag editing,
   optional 8-pixel snapping, signed-coordinate clamping, and undo/redo. Field
   loads containing embedded strings remain read-only under the fixed-size SPC
   patcher.
4. Add event creation/deletion once the assembler can resize CODE and update all
   affected offsets safely.
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
