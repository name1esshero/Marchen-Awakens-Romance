# Sprite placement and rendering evidence

This is a partial runtime trace, not a complete scene interpreter. Addresses
refer to the original Japanese ROM. The editable NCD groups, animations,
frames, cells and palettes live under `graphics/battle` and `graphics/ui`.

## Script positions reach the renderer

`SprSet` (08011F40) forwards `(id, property, 0, 0, value, 1, 0)` to
08010E44. Properties 0–5 enter the interpolation task path at 08010C58;
08010CCC reads and writes the corresponding signed halfword. Its switch also
handles property 6. The sprite lookup at 080106C8 returns
`engineState + 0xB90 + id * 40`. The update loop at 08010730 visits 32 records.

| Record offset | Evidence / interpretation |
| --- | --- |
| 0, 1 | Packed flags; not completely named |
| 2 | NCD container selector |
| 4 | Group index |
| 6 | Animation index within group |
| 8 | Frame index within animation, updated after animation advancement |
| A, C | X and Y pixels; `SprSet` properties 0 and 1 |
| E, 10 | Properties 2 and 3; copied to runtime instance offsets 1C, 1E |
| 12 | Property 5; copied to runtime transform offset 2A |
| 14, 16 | Properties 6 and 4 respectively; copied to runtime scale offsets 2C, 2E |
| 1A | Property task occupancy bits |
| 24 | Pointer to the higher-level render object |

08010730 copies X/Y to render-object offsets 3E/40. The visibility check at
0800E518 subtracts the signed integer halves of camera coordinates at
03003BD4/03003BD8 and accepts `-64 < x <= 303`, `-64 < y <= 223`.
The object traversal at 08008AFC subtracts that camera again to produce the
NCD instance's screen X/Y at offsets 18/1A, unless object flag 3C bit 0
requests camera-independent positioning. Therefore script X/Y are pixel
positions, but not every object must be interpreted as world-relative.

The editor labels these two properties and can project the selected assignment
as a yellow axis guide. It deliberately does not pair arbitrary nearby X/Y
calls: CODE order does not establish executed branches, time, or active map.

## Animation selection and drawing

The instance begins eight bytes into the higher-level render object.
08010730 passes its container/group/animation/frame to 0807BC7C, which follows
registered NCD tables: group.firstAnimation + animation, then
animation.firstFrame + frame, then frame.firstCell. It loads frame duration
and cell count, allocates per-cell handles and requests cell tile uploads
through 0807D444. Cell byte 13 hex (offset 19 decimal) is passed to allocation
and upload; its complete encoding is still under investigation.

0807BFD0 decrements the 16-bit duration counter and advances when its signed
value becomes negative. It either loops to frame zero or marks completion,
depending on instance flags. This is not simply a frame advance every tick.

**0807BDAC was incorrectly named `GetObjectPaletteSlot`.** It is now named
`NcdQueueSprite`: it appends the instance to a linked list through 0807A878
and increments the queued-object count. Instance offset 26 bits 2–3 select
one of four queue arrays; the second argument selects a 12-byte bucket.
No palette is returned or looked up by this function.

0807BDEC traverses those queues. It dispatches to 0807C188 for ordinary
instances, or 0807C2DC / 0807C758 for transformed instances according to
flags at 27. The transformed path rejects scales <= 4. These drawing
routines still require full C recovery and verification of OAM allocation,
affine transforms, clipping, ordering and transfer timing.

## Recovered C and remaining work

`src/ncd_sprite.c` recovers the complete 80-byte initializer at 0807BC2C.
`include/ncd.h` describes its 52-byte instance. Link/address fields use explicit
32-bit GBA addresses. Initialization clears the record, selects no container,
sets animation/frame to -1, identity scales to 256, and default flag bits.
Unknown flag names retain their offsets instead of guessed meanings.

Map tiles, raw attributes and literal hit-region calls are editable today.
Complete event control flow, spawn/resource association, world versus screen
flags, sprite dragging with correct script ownership, collision-attribute
semantics, and the full hardware rendering pipeline remain unfinished.

## Cell centers (corrected sprite assembly)

The normal ARM OAM callback is 0807D718, installed into engine offset 628
by initialization at 0807B14x. It sign-extends cell X from nine bits and Y
from eight bits, adds the instance offsets, then subtracts half-width and
half-height using tables at 0807D834 / 0807D844 before writing OAM. Thus
`source/cells.json` X/Y are **centers**, not top-left corners. Raw cell records
remain unchanged; `ncd.cell` additionally exposes `left` and `top` for display.
Both frame composition and pixel-to-source ownership use these bounds.

CHR frame 61 demonstrates the error: its 32×32 body is centered at (0,-28),
32×16 legs at (0,-4), and 16×8 hair at (0,-48). Their top-lefts are respectively
(-16,-44), (-16,-12), and (-8,-52). The hair meets the body at Y=-44.
The corrected frame bounds are (-16,-52), 32×56, rather than (0,-48), 32×60.
This fixes ordinary cell assembly; runtime affine and blending effects still
require further decoding. Gallery regeneration also refreshes the historical
named first-frame previews from current frame PNGs.

## Script resource selection

The native registry at 081AFF30/081AFF38 identifies `SprInit` (08011ECC)
and `SprChg` (08011EF4). Both take sprite ID, NCD container selector,
resource-name string, animation index, and a fifth integer. `SprChg` forwards
that fifth integer as the frame index to `ScriptSpriteSelect` (08010C0C),
now recovered as matching C. It only changes active records, uppercases the
name in a 16-byte local buffer, finds the group through `FindResourceByName`,
and stores signed 16-bit selectors. Failed group lookup remains -1, as in
the original. Position and unrelated flag bits are preserved; draw-order bits
are reset to 3.

`SprInit` instead schedules 08010AEC / 08010B6C. That task waits for sprite
cleanup, allocates a 72-byte render object, initializes it, resolves the group,
and starts frame zero. The inspected path does not use the native call's fifth
integer as a starting frame. This distinction must be preserved by any scene
preview or script rewrite.

MAP01_A contains `SprInit` calls whose resource literals are MAP01_O and
PS_WK02. Their bytecode interleaves a string expression and temporary-variable
operations among integer pushes, so the existing integer-only editor correctly
leaves these calls read-only. Recognizing an embedded name is not yet proof of
the complete operand stack or executed control flow; automatic scene placement
must wait for that decoder rather than pairing arbitrary nearby calls.

### Sprite creation worker (08010B6C)

The 160-byte worker now compiles from C to the original bytes. State 0 calls the preparation task at 0801097C with the sprite ID, payload +40, and a completion-word pointer, then transitions to state 16. State 16 waits until payload +44 is nonzero. It then allocates and clears a 72-byte auxiliary block, runs 08008A70 on it, activates the sprite, resolves the named NCD group, copies animation/frame, and updates the original flag bits. Finally it decrements the pending-script-task count, writes -1 through the task result pointer if present, and finishes the task.

The worker itself does not assign X/Y. The preparation task still needs tracing before concluding what an entire SprInit operation does to old position/state. Auxiliary-block and unnamed flag semantics also remain unresolved. Host tests cover waiting, completion, unsupported states, and preservation of coordinates in this worker; these are not an emulator playthrough.

### Deferred sprite reset (080109C4 / 08010A70)

The single-slot (104 bytes) and all-slot (124 bytes) workers are now matching C. An active slot waits while its u16 at +0x1A is nonzero; once ready, its auxiliary block at +0x24 is torn down through 08008BD8 and freed. Reset clears all 40 bytes, then sets the two signed fields at +0x14/+0x16 to 256 and draw-order bits to 3. Completion decrements the pending-operation counter, writes -1 to an optional task result pointer, and finishes the task.

Single reset clears inactive records too. Reset-all scans exactly 32 slots, skips inactive records, and stays pending while any active slot is still busy. Host tests cover these differences, teardown order, waiting, default fields, and optional result pointers.

Consequently the SprInit preparation phase clears X/Y before creation proceeds. A future sprite-placement compiler must account for this asynchronous reset rather than assigning coordinates before it. Trigger dispatch, statement insertion/relocation, and new-map archive registration remain unfinished.
