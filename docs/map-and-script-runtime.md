# Map generation and script bytecode

The procedural map generator has a separate deterministic random stream. Its
32-bit seed is stored at IWRAM `0x03004044`; each value applies the familiar
LCG step `seed * 0x41C64E6D + 0x3039` and returns bits 16 through 30. Map setup
seeds this stream independently, so generation remains repeatable without
consuming the game's general RNG state.

`MapGenerationGetState` follows the engine root pointer at IWRAM `0x03003FDC`
and returns the block at root offset `0x1304`. The recovered layout in
`include/map_generation.h` covers offsets `0x00..0x8B`: six pointers, indexed
tables at `0x38`, `0x48`, and `0x58`, signed 16-bit arrays at `0x26` and
`0x2E`, four signed bytes at `0x68`, and four 8-byte vectors at `0x6C`. These
accessors are used while inspecting KMP attributes and choosing procedural map
connections. Offset-based names are intentional until those callers establish
which values represent directions, coordinates, rooms, or connection types.

KMP visual layers and attributes are separate arrays. Up to four nonzero u32
offsets at header +`0x9C` select row-major u16 visual planes. Offset +`0xAC`
selects the row-major attribute plane, and +`0xBA` chooses byte or word
attributes. The regular renderer preserves tile index, flip, and palette bits;
the affine renderer writes only each entry's low byte. The helper at 08072130
accepts neighboring attribute values 400–499 and 5400–5499 while constructing a
procedural connection mask. These ranges are confirmed for that generator path,
not yet a complete collision dictionary for normal field movement.

### KMP viewport and camera window

`KmpRenderViewport` at `0x08002630` dispatches to the regular renderer when the
signed mode byte is zero and to the affine renderer otherwise. The regular path
at `0x08002650` rebuilds a 32×32 `u16` screen buffer (0x800 bytes): it clears
the buffer, takes the selected plane offset from header +`0x9C` using the
viewport's signed plane byte, then copies the map area under the requested
scroll position. The fixed-point pixel coordinates become tile coordinates by
arithmetic shift 19, equivalent to dividing 16.16 pixels by the 8-pixel tile
size. Source rows and columns are clipped to the KMP dimensions; destination
coordinates wrap modulo 32. Thus the screen buffer is a circular tile window,
not a cropped bitmap of the whole map. Each copied tile word gets the combined
palette-bank and tile-index offset `(paletteBankOffset << 12) |
tileIndexOffset`, preserving the other source entry bits through 16-bit
addition.

The regular renderer writes its input X/Y pair to viewport offsets +`0x10` and
+`0x14`, and duplicates it at +`0x18` and +`0x1C`. `KmpInitViewport` initially
puts `map width << 19` and `map height << 19` in the latter pair, so those words
are phase-dependent storage: initial extents before the first render and the
last rendered coordinates afterward. `struct KmpViewport` now represents that
overlap as a union instead of claiming those fields remain dimensions. The
meaning of the duplicate coordinate pair is still unknown. The affine renderer
and final display submission path remain assembly; do not infer affine camera
or layer behavior from this regular-path evidence.

The script-native registry labels command 45 `CameraMode`. Its handler
(`ScriptNativeSetRuntimePair`, `0x0801234C`) forwards argument 0 and argument 1
to setters for signed-byte fields at secondary-runtime offsets +`0xE48` and
+`0xE4A`; the getters sign-extend those bytes. This proves scripts configure a
two-value camera-mode state, but the code traced so far does not identify the
values' meanings or show that they are the world-camera coordinates at
`0x03003BD4`/`0x03003BD8`. Those coordinates remain a separate renderer input
until a consumer connects the two systems.

The readable model in `src/nonmatching/kmp_regular_viewport.c` is deliberately
not linked. It records the recovered loop and coordinate transformations for
tooling work while leaving the matching implementation in assembly. It must be
compiled and compared before it can replace the ROM routine.

The script VM is reached through two `+0x0C` pointers from IWRAM
`0x0300611C`. Its current bytecode buffer is at VM offset `0x30`, instruction
cursor at `0x44`, and downward-growing stack cursor at `0x84`.

The bytecode helpers now make these rules explicit:

- 16-bit and 32-bit operands are little-endian byte sequences.
- Sequential reads advance the cursor by one, two, or four bytes.
- A push subtracts four from the stack cursor before writing; a pop reads and
  then adds four.
- Jumps store a bytecode offset in the instruction cursor. Calls push the
  return cursor before jumping, and returns restore it.
- The switch command reads an operand, a case count, then pairs of 32-bit case
  values and destinations. It selects the first matching destination.
- Assignment and arithmetic commands use one-byte operand descriptors. The
  descriptor resolver at `0x0807F624` remains assembly and is the next key
  piece for naming local, global, indirect, and immediate operand classes.

All routines described here are compiled by agbcc and covered by the normal
byte-for-byte Japanese ROM comparison. Their addresses and occupied sizes are
recorded in `src/decompiled.json`.

The adjacent native-call layer now has readable implementations for random
range generation and seeding, integer parsing, normalized string comparison,
string length, and bounded right/substring copies. Native calls return `1` on
success; the allocating substring calls return `-1` if the VM heap cannot
supply their result buffer.

Two additional state views document data used by callers around the map and
script systems. `src/game_state.c` follows the main engine root and exposes 36
typed fields at offsets `0x4240..0x42C4`. `src/runtime_buffers.c` exposes 27
operations on the secondary allocation published at IWRAM `0x03004020`,
including three buffer addresses and an indexed record whose stride is 1,672
bytes. Names remain offset-based where the underlying gameplay role is not yet
proven.

## Field actor input and direction

The large field actor update routine beginning at `0x080130A0` probes
`MapAttributeProbeDirection` using the actor's position and hit bounds. Its
direction-selection helper at `0x08017D48` reads the held GBA direction keys
through `KeyInputAnyHeld`. The readable candidate in
`src/nonmatching/actor_direction_update.c` records its confirmed behavior:
up/down take precedence over left/right, combined keys produce eight-way move
codes 1 through 8, facing is a separate four-way value, and some paths also
write an auxiliary byte with 3 or 7. No-key input clears the movement code and
returns zero. The meaning of that auxiliary byte and the map attribute classes
tested by the parent movement routine still need confirmation from more of
that caller's control flow.

The actor update routine also tests the sampled KMP attribute against
`500..599` and `5400..5499` as distinct branches. The latter range is already
known from the procedural connection-mask helper, while the actor routine
proves both ranges reach special movement handling. This is evidence for
separate engine behavior classes, not enough to call either range walkable,
blocked, or a warp. The remaining state machine around `0x080130A0` is the
next source of evidence for those meanings.

The following dispatch points are now pinned down from the force-thumb flow
in `asm/code/code_0100C0.s`. These are control-flow facts, not friendly
attribute names:

| Attribute condition | Observed result in the actor update |
| --- | --- |
| `100..199` | At `0x08013B92`, writes task mode `0x1000` at task offset `+0x14` and redispatches the actor state machine. |
| `300..350` | At `0x08013BA4`, calls `sub_08071538` with the attribute and two actor/runtime coordinates; a nonzero return selects task mode `0x2000`. |
| `400..499` or `5400..5499` | At `0x08013AC2` and `0x08013C10`, stores the current directional hit-bound probe through `MapGenerationSetProbeDirection`. |
| `500..599` | At `0x0801394A`, when the preceding actor-mode byte allows this path, selects task mode `0x7000` and clears task payload byte `+3`. A later check at `0x08013C48` uses the same range under additional actor-state conditions before calling two unresolved map-generation routines and selecting mode `0x3000`. |

The task-mode values above are copied from the halfword writes and call
arguments. They identify branches for future tracing; they do not by
themselves prove concepts such as doors, stairs, blockers, or map exits. The
`300..350` helper is only partially understood: its exact-value dispatch
includes attributes `301..304`, but its neighboring generated-grid inputs
and the resulting map-state updates still need to be decoded together.

### Procedural map carving grid

`sub_08070238` builds a temporary row-major byte grid whose dimensions are
16-bit width and height fields. It clears each cell to zero, seeds a starting
cell, then chooses cardinal directions and advances two cells at a time. The
probe helpers `GeneratedMapCanCarveTwoCellStep` (`0x080718A0`) and
`GeneratedMapHasCarveDirection` (`0x08071848`) are matching C in
`src/mapping.c`.

The directional probe checks exactly two cells away. East and south require
the current column or row to be strictly less than the corresponding
dimension minus three; west and north require the coordinate to be greater
than two. In each case, the destination byte must still be zero. The
four-direction helper tests east, south, west, then north and returns as soon
as one direction qualifies. Its caller retries a selected direction until a
qualifying destination is found, then marks the intervening/destination cells
and continues from that destination. This is direct evidence of a
two-cell-step procedural corridor-carving process with a protected outer
margin. It does not establish the visual tile IDs or every later byte-grid
state; a subsequent pass converts this work grid into packed per-cell
generation records and selects map attributes.

The next helper, `GeneratedMapGetPathNeighborShape` (`0x08071934`), inspects the four adjacent byte states and
sets bits west=`0x0001`, east=`0x0010`, north=`0x0100`, and south=`0x1000`.
When called with a nonzero mode it returns that raw mask. In the generator's
shape mode, the exact two-neighbor masks become codes: north+south `3`,
west+east `4`, east+north `5`, west+north `6`, west+south `7`, and
east+south `8`; any other mask becomes `9`. `sub_08071A20` calls this shape
mode while converting the byte grid into 32-bit per-cell records. This
recovers the corridor topology encoding, but those shape codes are not
themselves visual tile IDs; later code still combines them with generated
room data and map attributes. The direction and neighbor values are named
in `include/map_generation.h`.

All three helpers now compile byte-for-byte from `src/mapping.c`. The bounds
assume the generator's valid, nonzero dimensions, as enforced by its callers;
the probes themselves do not guard division by zero or malformed grid sizes.

### Moving-entity overlap query

The field actor update path calls `sub_08018A9C` before committing a
four-pixel cardinal step. The readable candidate in
`src/nonmatching/field_actor_collision_query.c` records the confirmed query:
it translates the actor's signed `HitBounds` by the proposed step, then scans
two 32-entry collections. One pairs 44-byte game-state records at `+0x610`
with eight-byte rectangles at `+0x3F3C`; the other pairs script-sprite records
at `+0x0B90` with eight-byte rectangles at `+0x403C`. Both require the observed
enable bits, use strict rectangle overlap (touching edges do not count), and
select the smallest Euclidean distance from the stepped actor position to the
candidate position. Equal distances replace the prior result, so later entries
and then the script-sprite collection win ties. On success the function writes
the collection selector and entry index; on failure it leaves both outputs
untouched. The relationship between the two rectangle tables and the embedded
script-sprite `HitBounds` field still needs investigation. This query detects
entity overlap; it does not decode the separate KMP tile-attribute behavior.
