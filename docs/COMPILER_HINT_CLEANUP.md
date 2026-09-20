# Compiler-hint cleanup: findings

Working notes for the effort to remove forced-register pins and inline-assembly
scheduling fences from `src/`, so the codebase reads as genuine matching C and
outside contributors can start on decompilation instead of on cleanup.

Tooling: `tools/drop_register_hints.py`. Authoritative backlog:
`make pret-audit` → `reports/code/pret-standards.{json,md}`.

## What counts as a hint

Two spellings, both hard errors in the audit:

- A forced register: `register s32 x asm("r1")`, or the same thing behind the
  per-file `TARGET_REGISTER(name)` macro, which expands to `asm(name)` under
  agbcc and to nothing on the host build.
- A scheduling fence: a whole statement `asm("" : "+r"(x));`, used to stop
  agbcc reordering the instructions around it.

## Shiftability: named sound-player globals

The nine fixed M4A player objects already have linker-defined symbols in
`asm/iwram_symbols.s`. Their declarations now live in `include/sound.h`, and
the sound-task, idle-wait, and scene-native code uses `gSoundPlayer0` through
`gSoundPlayer8` rather than raw `0x0300xxxx` casts. This preserves the ROM
exactly while making the shared player-selection logic readable and linkable
through one named interface.

## Method

Never assume a hint is needed. Prove it:

1. Remove the hint.
2. Recompile that single translation unit with the real toolchain
   (`cpp` → `agbcc` → `as`), about a second per iteration.
3. Keep the removal only if the **machine code** is unchanged.

`make compare` remains the final authority, but it is far too slow to drive a
search over hundreds of candidates.

## Three findings that decide whether this works

**Compare machine code, not assembly text.** A fence emits `.code 16`
directives that disappear along with it. Diffing the compiler's `.s` output
therefore reports a difference for an identical instruction stream, and every
fence looks load-bearing. Assembling and diffing `objdump -d` output instead
took `sprite_transform.c` from 9 removable hints to 26, and `sprite_math.c`
from 0 to 4. This was the single biggest correction to the method.

**Removals interact, so apply them cumulatively.** Register allocation is
global to a function. In `sprite_transform.c` 54 of 61 pins passed when tested
individually, but only 34 survived once applied together. Testing one at a time
against a fixed baseline overstates what can go.

**Fences come in sets.** Dropping one fence while its partner still pins the
schedule changes the instruction order, so each looks necessary in isolation
while the whole set is removable. The tool therefore tries all fences in a
function together before falling back to one at a time. `SpriteFixed8Multiply`
first demonstrated this interaction; a later source-lifetime reconstruction
removed its remaining register pin as well.

## Worked example: SpriteFixed8Multiply (0x0807D8F8)

The ROM keeps `product` in r0 and a separate `rounded` in r1:

    muls r0, r1
    adds r1, r0, #0
    cmp  r0, #0
    bge  .L
    adds r1, #255
    .L:
    lsls r0, r1, #8

The first clean-C attempts left `product` dead after the comparison, so agbcc
coalesced `product` and `rounded` in r0. The matching reconstruction reuses
`product` for the final narrowing after copying it into `rounded`:

    product = left * right;
    rounded = product;
    if (product < 0)
        rounded += 255;
    product = rounded << 8;
    return product >> 16;

That last assignment extends the real source variable's lifetime far enough
for agbcc to preserve the r0/r1 copy. The function now compiles byte-identically
from ordinary C in `src/sprite_math.c`, with no fences or register pinning.

## Character-code width removes a font register pin

`FontCharacterToGlyph` (0x0807AE7C) no longer needs its `r2` pin.
The input is a packed 16-bit character code, so the local is now
`u16 code = (u16)input` rather than a pinned `u32` initialized from a
shifted temporary. The separate shifted input remains for the high-byte
calculations. This expresses the encoding's actual width and produces the
same agbcc machine code. The now-unused `TARGET_REGISTER` macro was removed
from `font.c` as well.

The audit falls from 156 to 152 findings; that is one corrected function,
not four functions. Straight removal of the remaining hints, including
removing all hints within each function together, did not match in this
pass. Small C rewrites of the sound-player selection/fade helpers, SRAM
helpers, save-write adapter, map native helpers, and fixed-point square
root also did not match and were not applied.

## Keep the archive lookup index separate from the next-entry comparison

`NfpGetEntrySizeByName` (0x0807AD4C) now uses an ordinary `s32 index`.
Replacing `index++; if ((u32)index >= count)` with
`if ((u32)index + 1 >= count)` removes its `r4` pin while retaining the
generated instructions. The lookup index is no longer reassigned just to
compare the following directory entry against the entry count. The index
has already passed its negative-result check at this point.

This is a concrete example of the compiler notes' variable-lifetime rule:
equivalent expressions can give the allocator different live ranges.

## Structural patterns that can replace register pinning

The successful clean-C matches so far point to a better search strategy than
trying arbitrary declaration orders.  agbcc allocates registers from the
values that are live at each program point.  Source expressions that are
mathematically equivalent can therefore produce different allocation when
they create, reuse, or end a value at a different time.  Before concluding
that a pin is load-bearing, test the following ordinary-C shapes against the
isolated machine code.

**Represent the actual storage width immediately.**
`FontCharacterToGlyph` stopped needing an `r2` pin when its packed character
code became a `u16` local instead of a `u32` containing a narrowed value.  The
wide shifted input remains a separate value for the high-byte calculation.
This tells the compiler which value truly ends after the 16-bit operations,
rather than keeping one wide temporary live through the whole function.

**Keep an input, its normalized form, and a loop value distinct when the ROM
does.** `GameStateSetAttributeFlagRange` (0x08006858) matches without pins with
three separate stages:

```c
if (enabled != 0)
    enabled = 1;
value = enabled;
flags = ORDERED_GAME_STATE_BASE + GAME_STATE_ATTRIBUTE_FLAGS_OFFSET;
for (index = first; index <= last; index++)
    BitSet(flags, index, value);
```

Passing `enabled` directly shortens one lifetime and lets agbcc coalesce values
that the ROM keeps apart.  The separate `value` is not dummy work: it models
the stable argument reused by every iteration while `enabled` has finished
normalization.  This naturally reproduces the ROM's r5/r6/r7 allocation.

**Do not combine address components before proving the load order.** The
matching object-release helpers and the `ORDERED_GAME_STATE_BASE` form load a
named IWRAM base and a named root offset into separate locals, then add them.
Writing one folded pointer expression can shorten both lifetimes or fold
constants, changing literal-load and register order.  Separate linker symbols
also preserve shiftability; raw addresses and artificial barriers do not.

**Use a typed function-pointer local when the original makes an indirect
call.** `ApplyTileRemainderMask` (0x080023B0) assigns
`ApplyEightWordMaskArm` to a `TileMaskFunc` local and invokes that local.  Plain
C then emits the existing `_call_via_r2` trampoline exactly.  A direct call or
inline assembly would describe a different source operation.  The trampoline
register is a result of normal allocation and the function-pointer type, not a
register request.

**Separate a current index from a next-entry test.** In
`NfpGetEntrySizeByName`, writing `(u32)index + 1 >= count` instead of mutating
`index` before the comparison ended the current-index lifetime at the right
place and removed its `r4` pin.  Apply the same idea to pointer advancement:
initializing the index before loading the pointer array and advancing both in
the loop header made `ScriptFrameReleasePools` match without steering.

**Check the recovered layout before blaming the allocator.**
`CreateFieldEventTask` appeared to have unavoidable argument pressure, but the
reference C had placed a payload halfword two bytes late.  Correcting the
task/payload boundary and using signed 16-bit locals produced the original
allocation.  A wrong offset, signedness, or field width changes live ranges
and addressing modes, so register experiments based on the wrong model are
misleading.

**Model real aliasing instead of using `volatile` to defeat optimization.**
`CreateRuntimeTask69DB4` (0x08069DB4) stores a parent work pointer in a new
child task, writes the parent's state, then reloads the stored pointer before
forming another child-task field. The former reference C used unrelated
`u8 **` and `u16 *` casts. Under C's alias rules agbcc could then prove that
the state write did not change the stored pointer, so it forwarded the
original argument and combined the two child offsets.

A partial union with parent-work and child-task views expresses that the two
accesses can alias. agbcc then emits the ROM's reload and independent
`0x11CC`/`0x11D0` offsets naturally. This is ordinary, shiftable C: the task
manager and callback remain named symbols, and no raw address, forced register,
inline assembly, or `volatile` qualifier is involved. Use this pattern only
when the recovered data model really permits aliasing; adding a union solely
as an optimizer barrier would be another fakematch.

This pattern improved but did not complete the resource-counter pair at
0x08056290/0x080562C8. A shared root/counter view makes the add routine perform
the ROM's second root dereference, address calculation, and counter load, but
agbcc still rotates the three live values among r2/r3/r4. The tested
struct-member, moving-pointer, u16-offset, declaration-order, and delayed-
assignment forms all retain that register mismatch, so both routines remain
in `src/nonmatching/game_state_resource_counter.c`.

**Modify the value that the ROM keeps as the arithmetic destination.** The
newlib `_Bfree` source at 0x08085A24 loads a bucket index into r0 and the bucket
array into r1, then keeps the scaled index as the destination of their add.
Spelling the operation as `key += (u32)buckets` produces `add r0, r0, r1`;
advancing the bucket pointer instead produces the equivalent
`add r1, r1, r0`. This is a useful source-level choice for commutative
operations. `_Bfree` was already present and matching in
`src/libc/mprec.c`; the stale duplicate reconstruction in
`src/nonmatching/misc.c` was removed.

These are candidate-generation rules, not promises that any one spelling will
match.  Compare assembled instructions after each cumulative change.  If no
honest C shape matches, retain honest assembly and a readable nonmatching C
reference as required by `PRET_STANDARDS.md`; never encode the desired answer
with a forced register, empty scheduling fence, volatile abuse, or fake data
dependency.

## Reuse one semantic index across consecutive loops

`ScriptNativeDeckMake` (0x080127F8) formerly pinned its cached deck mode to
`r3`. The first loop copies twenty values and the second loop marks each input
as owned. The reference C had separate signed and unsigned loop variables,
which let agbcc place the short-lived copy counter in `r3` and the mode in
`r4`. The ROM instead resets and reuses `r4` for the second loop.

The matching source uses one `s32 i` for both loops. The first comparison is
signed, so agbcc retains its compact countdown form. The second compares
`(u32)i` with the unsigned argument count, preserving the ROM's unsigned
branch:

```c
for (i = 0; i < DECK_ENTRY_COUNT; i++)
    values[i] = args[i + 1];

for (i = 0; (u32)i < count; i++)
    BitSet(flags, args[i], TRUE);
```

This extends the real index lifetime across the intervening calls. agbcc then
allocates that index to `r4` and the cached mode to `r3` without any register
request. The result is byte-identical and shiftable. When two nearby loops use
the same conceptual index and the ROM resets the same physical register,
test one shared source variable before assuming the allocation needs steering.

## Alignment padding is not inline assembly

Three files emitted their trailing zero halfword as
`asm(".section .rom.ADDR,\"ax\",%progbits\n.space 2, 0\n");`, which the audit
counts as inline assembly. The project already has a plain-C idiom for exactly
this, used throughout `src/`:

    AT("00005848") const u8 CreateSoundPlayerIdleWaitTail[2] = {0};

All fifteen padding halfwords across `sound_idle_wait.c`, `sound_cgb_update.c`
and `sound_m4a.c` converted cleanly. One catch: the tail must be written
*immediately after its own function*, not collected in a block at the end of
the file. Grouping them produces `Error: changed section attributes for
.rom.ADDR`, because `as` reopens a section it had emitted as code and finds
read-only data flags instead. Adjacent placement keeps each section opened once.

## Cases that resist a clean rewrite

One function in this pass still resisted the clean shapes tested:

`SpriteResourceFindGroup` (`sprite_engine_state.c`) needs the scaled index
computed *first* (into r0), the base loaded second (r1), and then
`add r0, r1, r0` -- base as the left operand. agbcc ties the two together: the
operand written first in the C addition gets both the lower register and the
`rn` slot. Writing `offset + base` gets the evaluation order right and the add
backwards; writing `base + offset` fixes the add and reverses the registers.
The earlier trial recorded the natural `descriptor->level0[low].name` form
as otherwise instruction-exact; recheck that against the current source
before relying on it. These failed candidates do not rule out a different
matching reconstruction.

## SpriteFixed8Multiply recovered as clean C

Moving the function to honest assembly was a compliant intermediate state,
but it was not proof that natural C was impossible. The successful rewrite
preserves the algorithm while making `product` carry the final shifted value.
This changes agbcc's lifetime graph enough to reproduce the ROM's r0-to-r1
copy without naming a register or inserting an assembly fence. The assembly
body and the obsolete nonmatching reference have therefore been removed.

A second trap: `SpriteFixed8Divide` and `SpriteFixed8Tail` had never been
independently tracked in `decompiled.json` -- they rode inside
`SpriteFixed8Multiply`'s single 52-byte manifest entry purely because all
three shared one `AT()` section string and the linker happened to lay same-
named sections out contiguously. Giving `SpriteFixed8Divide` its *own*
section (so it could be tracked once Multiply left) inflated it from 22 to
24 bytes: a fresh, independently-opened section gets 4-byte end padding that
a function merely continuing inside an already-open section does not. Fix:
`SpriteFixed8Divide` and `SpriteFixed8Tail` now share one `AT("0007D914")`
section (24 bytes, one manifest entry) -- the same grouping trick as before,
just with Multiply no longer part of the group. `tools/audit_provenance.py`
also caught an unrelated, pre-existing manifest error surfaced by the same
run: `RuntimeSetFlagC0` (`2ad2ddbe`) was recorded as 48 bytes against an
actual linked size of 52; corrected in the same pass.

## Sound-player lifecycle helpers recovered from original library structure

`SoundPlayerResume` (0x080789B0), `SoundPlayerFadeOut` (0x080789CC),
`SoundPlayerFadeOutTemporary` (0x08078C18), and `SoundPlayerFadeIn`
(0x08078C38) all read the same ready signature at `SoundPlayer + 0x34`.
They now compile byte-identically from clean C in `src/sound_m4a.c`.

The missing source-level dependency was MusicPlayer2000's identity lock. The
standard library shape increments `player->ident` on entry to the guarded
body and restores `SOUND_PLAYER_READY` on exit. Because these four helpers
make no calls, old_agbcc correctly removes both stores as unobservable. Their
presence in the source still changes the compiler's lifetime analysis before
dead-store elimination, preserving the ready signature in r3 exactly as the
ROM does. Omitting the apparently dead lock operations made every direct and
local-variable reconstruction allocate that value to a different register.

This is legitimate source recovery rather than register steering: the same
lock protocol remains observable in neighboring player functions that do
call other routines. It removed four assembly implementations and the entire
nonmatching lifecycle file with no forced register, inline assembly,
`volatile`, or compiler change. Full ROM comparison remains exact.

## Dynamic sound-player selection recovered as clean C

`StartSongOnFreePlayer` (0x08005F7C) selects the first inactive player from
the six-entry dynamic priority order. It now compiles byte-identically from
ordinary C in `src/scene_native.c`.

The old reconstruction declared and initialized the table pointers before it
filled the local player-pointer array. agbcc consequently hoisted the table
loads and song-entry calculation ahead of the nine stack stores, and an empty
assembly fence had been used to prevent that schedule. The matching source
fills the array first, then assigns the table pointers, and explicitly
initializes the loop counter before those assignments. This produces the ROM's
natural order: nine stack stores, loop-counter initialization, three table
loads, then the song-entry calculation. The assembly fallback and nonmatching
reference were removed; all player and table addresses remain linker symbols.

## State

Recorded at the time of writing; regenerate rather than trusting these numbers.

- Hard-rule findings: 351 → 230 in a first pass that compared assembly text,
  then 230 → 162 once the comparison moved to machine code, then → 157 as the
  sound files' padding halfwords stopped being inline asm, then → 156 moving
  the BIOS SWI wrappers to real assembly (`8d7874c8`), then → 152 (two solo
  structural rewrites: `FontCharacterToGlyph`, `NfpGetEntrySizeByName`), then
  → 149 moving `SpriteFixed8Multiply` to real assembly, then → 141 moving
  the four sound-player lifecycle helpers, then → 140 moving dynamic
  sound-player selection to real assembly. Later structural reconstruction
  restored `SpriteFixed8Multiply` and all four lifecycle helpers as clean,
  matching C. Regenerate the current count with `make pret-audit`.
  `tools/drop_register_hints.py --all` finds 0 further mechanically-safe
  removals at 149; everything left needs either a structural rewrite (slow,
  one function at a time, as above) or the same real-assembly move.
- Remaining concentrations (150 findings' worth before this session; regenerate
  with `make pret-audit` rather than trusting a per-file count here):
  `sprite_affine_matrix.c` (43, largely fixed-point affine/trig -- the single
  biggest remaining chunk), `sprite_transform.c` (20), `sprite_affine_slots.c`,
  `sram.c`, `sprite_interpolation.c`, `sound_m4a.c`, `resource_native.c`,
  `mapping.c`, `sprite_math.c`, `sound_idle_wait.c`, `ncd_sprite.c`, `nfp.c`,
  `sprite_tile_allocator.c`, `sprite_engine_state.c`, `save.c`,
  `sound_fade_create.c`.
- `make compare` byte-exact and all host tests passing throughout.

## Watch out for

Replacing a raw ROM address with a linker symbol (audit remediation step 3)
breaks any host test that compiles a single `.c` in isolation, because the
symbol is resolved by `asm/game_table_handlers.s` in the ROM link only. Four
tests broke this way. Fix the test by defining the aliased literal, not by
reverting the source. Tests that string-substituted the old raw address silently
become no-ops and must be updated too.

## Matching object cleanup: linker symbols and signed bounds

`ObjectFreeNcdResources` (0x080281E0, 108 bytes) and
`ObjectFreeAuxiliaryResources` (0x0802824C, 104 bytes) moved from
`src/nonmatching/object_free.c` into matching `src/object.c`.

Two corrections were necessary:

- The original loads the IWRAM base and heap-root offset separately before
  adding them. Numeric constants fold together. The linker symbols
  `gIwramBase` and `gObjectHeapRootOffset`, assigned to separate locals,
  reproduce the two loads without a scheduling barrier or register pin.
- `Object.unk_18` is stored as `u32`, but the release loop uses signed
  comparisons. The loop explicitly casts it to `s32`. The previous reference
  C used an unsigned comparison, which could walk a huge array for a negative
  count. Zero and negative counts must skip per-record releases while still
  freeing and clearing the active object's record array.

Both agbcc and old_agbcc produced the same matching 212-byte candidate after
these corrections. The existing compiler selection was retained. This is
evidence that the older notes' claimed compiler limitation was incomplete,
not evidence that the entire subsystem needs a different compiler.

Validation: the linked Japanese ROM matches its original SHA-1, the English
build succeeds, and all 143 host tests pass. The new object-release test checks
both release callbacks, 72-byte record strides, the eight-byte embedded-sprite
offset, inactive objects, null records, and zero/negative counts. The replaced
assembly and obsolete nonmatching source were removed.

## Next

For each surviving hint, attempt a structural rewrite that matches as ordinary
C. Only when that fails should the function move back behind its assembly, with
readable C kept under `src/nonmatching/` and the mismatch documented.

## Field-event constructor: a layout error behind a supposed compiler mismatch

`CreateFieldEventTask` (0x08061EA8) now matches all 120 bytes as plain C in
`src/map_field.c`. Its old reference combined the task header and payload,
and placed the final halfword two bytes too late. A payload based at task
+32, with the last halfword at payload +0x92 (task +0xB2), fixes the layout.
Signed 16-bit parameters copied into ordinary signed locals then produce the
original register allocation without any hints. Both compiler snapshots
matched the isolated candidate; no build flags or compiler selection changed.

The linked Japanese ROM matches exactly. The replaced assembly and obsolete
`src/nonmatching/map_events.c` were removed. The field-loader host test now
also checks constructor arguments, signed coordinate extremes, object binding,
and that constructor writes leave the task header and unknown fields alone.
This replaces the earlier claim that argument narrowing or high-register
pressure alone prevented a match.

Validation on the updated checkout: all 168 host tests pass, the English
build succeeds, and the PRET audit remains at 149 errors and zero warnings.
## Script frame pool cleanup

`ScriptFrameReleasePools` at 0x0807EC58 now matches in clean C. The previous
notes that described its inner loop as impossible to steer were based on an
incomplete search. Initializing the index before loading the pointer array and
advancing both values in the loop header produces the ROM's exact order. This
conversion removed 280 bytes of assembly and the corresponding
`src/nonmatching` file without register variables or inline assembly.

## Shiftability: recovered engine-root globals

The fixed IWRAM root slots are now expressed as typed linker symbols rather
than casts of numeric addresses in recovered C. `gScriptContext` and
`gScriptBytecodeRoot` are compatible typed views of the same root slot: the
script-task runtime uses the outer context, while bytecode resource code uses
the nested bytecode view. `gSpriteEngineState` names the renderer-state root,
and `gSpriteRuntime` names the cached base written by `SpriteRuntimeInit()`
for its separate 0x8CC-byte block. `gMapGenerationRoot` names the documented
main-runtime root that map-generation code originally exposed through an
IWRAM-base-plus-offset expression.

These names are defined by `.set` aliases in `asm/iwram_symbols.s`, so they
retain the original addresses without placing hard-coded addresses in C. The
conversion covered the script task/resource paths, renderer accessors, NCD
sprite helpers, affine-slot code, map-native paths, and scene/runtime helpers.
Isolated host
tests define the named globals they use, while the ROM link resolves them
through the linker aliases.

Validation after the conversion: focused script and sprite host tests pass,
and `make compare` reports a byte-identical ROM. The names document observed
layout and ownership only; opaque field offsets remain opaque until their
semantics are recovered.

## Current structural probes: SRAM and save wrapper

The SRAM copy/verify routines were re-tested without register pins. A real
`remaining` loop variable correctly recreates the ROM's initial `size` copy
into r3. The remaining mismatch is narrower: agbcc puts the volatile WAITCNT
load in r1 and the `0xFFFC` literal in r0, while the ROM uses r0 and r1 in the
opposite roles. The direct volatile read-modify-write spelling has the same
swap. The matching pins were restored; this is evidence about the candidates
tested, not evidence that the original C needed pins.

`CreateSaveWriteTask()` was also tested with ordinary locals and with its
local declarations and assignments reversed, and with the standard C
`register` storage class but no asm constraint. All candidates preserve its
shape and size, but assign the saved input pointer to r4 and size to r5; the
ROM assigns them to r5 and r4. Its matching declarations were likewise
restored. Future attempts should look for a real lifetime or type difference,
not add an artificial dependency merely to exchange registers.

The two direct callers at 0x0806E32A and 0x0806E408 also rule out a swapped
source signature as the explanation. Both prepare the save pointer in r0, the
byte count in r1, and the completion pointer in r2 before calling
`CreateSaveWriteTask()`. The current public parameter order is therefore
verified from call sites even though an ordinary forwarding wrapper assigns
its two long-lived arguments to r4/r5 in the opposite order from the ROM.

`ScriptNativeQueryModeResource()` was re-tested one constraint at a time. If
only the r0 constraint on `base` is removed, the function remains the same
size and differs solely in the commutative address addition: agbcc emits
`add r0, r2, r0` where the ROM has `add r0, r0, r2`. Writing that expression
as subtraction of a negated offset happens to recover the ROM instruction,
but it obscures a plain pointer addition solely to steer the optimizer. It is
a fakematch under `PRET_STANDARDS.md` and was rejected. Direct pointer
addition, indexed-pointer spelling, integer-address temporaries, and both
operand orders all reproduce the swapped ordinary-C instruction. The pin
remains pending until a genuine type or lifetime reconstruction explains the
allocation.

## Reuse an initialized search value before its loop role

`NfpFindEntryIndex()` (0x0807ACC4) no longer needs its `high` and `zero`
register constraints. The ROM clears r5 before the two archive calls, uses
that zero to terminate the 12-byte directory name copied to the stack, and
later assigns every binary-search midpoint to the same register. Those uses
belong to one real source variable: `middle` starts at zero, supplies the
terminator, and is reassigned by `(low + high) / 2` inside the loop.

The former reconstruction invented an unrelated `zero` local pinned to r5
and pinned `high` to r4 to compensate. Initializing `middle` before writing
the terminator makes agbcc choose r5 for the midpoint and r4 for the upper
bound naturally. This removes both forced registers and the file's
`TARGET_REGISTER` macro while producing identical instructions and an exact
ROM SHA-1. When a ROM register is initialized well before its apparent main
use, check whether the value begins another later-used variable's lifetime
instead of adding a dedicated constant local.

## NCD flag lifetime and typed allocation count

Two more NCD routines now reproduce the ROM without compiler hints.
`NcdQueueSprite()` first reads the typed `NcdSprite::flags26` member, then
initializes the queue-group mask, and finally applies the mask with `&=`.
Keeping the raw byte and the evolving masked value simultaneously live makes
agbcc assign the raw flags to r2 and the group to r0 naturally. The former
reconstruction computed `rawFlags & 12` in one expression and compensated for
the shortened lifetime by pinning `rawFlags` to r2.

`NcdRuntimeSpriteReleaseAllocation()` now compares its loop index directly
with the typed `NcdRuntimeAllocation::partCount` member. That member access
produces the ROM's pointer calculation and r0-to-r6 preservation without the
old pinned byte pointer or empty inline-assembly barrier. With these two
repairs, `src/ncd_sprite.c` contains no register constraints, scheduling
fences, or private hint macros, and the Japanese ROM remains byte-identical.

`SpriteTileAllocatorRelease()` was rechecked in the same pass. Removing its
remaining r0 constraint changes only the three-instruction previous-span merge:
the ROM copies the size mask from `ip` to r0 and masks it with the flags in r1,
whereas agbcc's clean form copies the mask to r2 and consumes r1 in place.
Direct and commuted expressions, reuse of each later local, signed and unsigned
16/32-bit temporaries, a bitfield view, and an equivalent flat-goto control
flow were tested. None matched without changing other instructions. The
constraint therefore remains pending a better source model; the failed probes
are evidence about those spellings only, not proof that original C required a
fixed register.

## Reload a mutable root through its real volatile indirection

`ScriptNativeClearMapHalfwords()` (0x08012D64) originally reloads the generated
map root pointer on every loop iteration. Declaring the root as an ordinary
`u8 **` let agbcc hoist the dereference and forced the reconstruction to pin
that pointer to `r6`. The neighboring copy routine already exposed the actual
type: `u8 * volatile *`. Applying that type to the clear routine expresses the
mutable IWRAM pointer slot, naturally retains the reload, and reproduces the
original allocation without a forced register. The 256-entry count, 16.16
step, and record offsets are now named constants as well.

The matching `ScriptNativeCopyMapHalfwords()` reconstruction still contains
two register constraints and must not be mistaken for likely original source.
An ordinary loop using `i = (s16)(i + 1)` makes agbcc generate the ROM's
apparently fixed-point `0x10000` induction variable by itself. With a volatile
root slot and a typed `u16 *` destination, the clean candidate has the same
72-byte size, stack frame, literal pool, argument cursor, destination and value
registers, and loop branches. It differs only in two code-generation details:
agbcc adds the record offset before shifting the index (the ROM shifts first),
and it uses the copied old index rather than the induction register as the left
operand of the following addition. Both compiler snapshots produce the same
candidate. This is strong evidence that the current explicit 16.16 locals are
decompilation scaffolding. Keep the byte-matching implementation until the
actual source lifetime or expression shape explains those last instructions;
do not describe either constraint as something the original developers used.

## Preserve clean references when ordinary C does not yet match

Three renderer routines were moved out of the matching build after their
remaining compiler hints resisted structural cleanup. Their readable,
shiftable implementations now live in `src/nonmatching/`, while the exact ROM
instructions are restored in `asm/code/code_0780C0.s`:

- `SpriteTileAllocatorRelease()` differs only in the register selected for the
  previous free span's size mask. Direct and commuted expressions, local
  reuse, signed and unsigned temporaries, a bitfield view, and flat control
  flow all failed to recover the ROM allocation naturally.
- `SpriteResourceFindGroup()` differs only in the operand encoding of one
  commutative address addition. The ROM emits `r1 + r0`; clean agbcc output
  emits `r0 + r1` for the otherwise identical operation.
- `SpriteFixedSqrt()` assigns the Newton estimate and the 0x1000 fixed-point
  unit to the opposite registers. More than 120 natural declaration,
  assignment, conditional, and branch variants were checked, along with both
  compiler frontends available in this project.

This is the PRET-compliant fallback described by `PRET_STANDARDS.md`: the
matching build contains honest original assembly instead of C with register
constraints or inline-assembly scheduling hints, and the clean C remains
available for review, tests, and future source-shape work. Shared allocator
types were moved to `include/sprite_tile_allocator.h` so the matching routines,
reference implementation, and host test use one verified layout. The change
reduces the mechanical PRET audit from 125 to 112 hard errors while preserving
the exact Japanese ROM SHA-1.
