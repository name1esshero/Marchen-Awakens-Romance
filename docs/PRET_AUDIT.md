# PRET standards audit

Related load-bearing docs: [PRET_STANDARDS.md](PRET_STANDARDS.md) defines the
rules this audit enforces, [SCORE_METRIC_CRITERIA.md](SCORE_METRIC_CRITERIA.md)
explains how audit regressions are scored, [AGBCC_CODEGEN.md](AGBCC_CODEGEN.md)
and [COMPILER_HINT_CLEANUP.md](COMPILER_HINT_CLEANUP.md) describe clean fix
patterns, [decompilation-notes.md](decompilation-notes.md) records the evidence
behind individual recoveries and deferrals, and [AGENT_ENVIRONMENT.md](AGENT_ENVIRONMENT.md)
describes how the documentation funnel fits together.

Run `make pret-audit` to regenerate the detailed machine-readable reports at
`reports/code/pret-standards.json` and `reports/code/pret-standards.md`. Run
`python3 tools/audit_pret_standards.py --strict` when checking whether the
hard-error backlog has reached zero. GitHub Actions runs
`make ci-audits`, which compares every current hard error with the explicit
fingerprinted baseline in `tools/pret-audit-baseline.json`. A rule/file/detail
combination absent from that baseline fails CI. Line numbers are excluded so
ordinary edits above a finding do not create false regressions; duplicates are
counted independently so one fixed error cannot hide one newly introduced
error. The audit prints known, new, and resolved counts on one line.

## Verified snapshot: stale-claim audit and GameStateFindRecord35E0, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- The mechanical PRET audit is **0 errors / 0 warnings / 18 documented
  exceptions**.
- `audit_provenance.py` now reports 1825/1825 mapped ranges compiled C.

A systematic grep for "abandoned"/"unmatchable"-style phrasing across this
file and `docs/decompilation-notes.md`, prompted by `SpriteRuntimeGetFields8C4`
(previous entry) having been exactly that kind of stale claim, found three
more pure documentation-lag cases (functions already matched elsewhere with
nobody updating the note that called them unresolved) and one genuine,
still-open case that turned into a new match. See "Stale-claim audit, and a
new decompile it turned up" in `docs/decompilation-notes.md` for detail on
all four. The new match, `GameStateFindRecord35E0` (0x08055F4C), was a
block-order artifact, not the register-allocation difference two separate
notes had both (accurately, at the time) described: writing the ROM's exact
`if (found) { ...; return ...; } return failure;` shape instead of the
inverted `if (!found) return failure; ...; return ...;` every prior attempt
used puts the registers back in the ROM's own assignment as a side effect
of matching the fallthrough order.

## Verified snapshot: SpriteRuntimeGetFields8C4 and probe tool bytes mode, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- All 199 host tests pass; `make check-modern` exits clean.
- The mechanical PRET audit is **0 errors / 0 warnings / 18 documented
  exceptions** -- two warnings appeared mid-session (`missing_doxygen` on
  `SpriteRuntimeSetFlag800`, `undocumented_nonmatching` on
  `sprite_tile_allocator_release.c`), both pre-existing gaps a file-layout
  accident had been masking from the audit's line-proximity heuristics, not
  regressions from this session's edits; both fixed.
- `audit_provenance.py` now reports 1824/1824 mapped ranges compiled C.

`SpriteRuntimeGetFields8C4` (0x0808053C), previously logged as abandoned in
`docs/decompilation-notes.md`'s batch-C entry, matches: the ROM re-reads the
global `gSpriteRuntime` fresh for the third output field instead of reusing
the pointer cached for the first two, and agbcc reproduces that exact
redundant reload once the C is written with that same mismatched-identifier
shape rather than a uniformly cached local. See "Resolved:
SpriteRuntimeGetFields8C4" in `docs/decompilation-notes.md` for the full
mechanism and integration detail.

Also fixed: `tools/agbcc_probe.py` compared only agbcc's assembly *text*,
which is not proof of byte-identity in either direction (a scheduling
fence's removal can change `.code 16` directives without changing any real
instruction; a commutative operand swap like `adds r0,r0,r1` vs
`adds r0,r1,r0` looks like harmless reordering in text but is a genuinely
different encoding). A new `--bytes` flag assembles with `arm-none-eabi-as`
and diffs `objdump -d` output instead, mirroring
`tools/drop_register_hints.py`'s existing `compile_unit()`. Verified against
both failure directions before being trusted: a fence-removal pair reports
identical in both text and bytes mode, and an `adds r0,r0,r1`/
`adds r0,r1,r0` pair reports identical in text but genuinely different in
bytes mode (`0x1840` vs `0x1808`, matching the `SpriteResourceFindGroup`
case exactly).

## Verified snapshot: SpriteFixedSqrt and check-modern fixed, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- All 198 host tests pass; `make check-modern` exits clean (0) for the first
  time this session -- see "Fixed `make check-modern`" in
  `docs/decompilation-notes.md` for the three configuration gaps involved.
- The mechanical PRET audit remains at **0 errors / 0 warnings / 18
  documented exceptions**.
- `audit_provenance.py` now reports 1823/1823 mapped ranges compiled C.

`SpriteFixedSqrt` (0x0807D9F0), previously rejected as needing register-
steering compiler hints, matches with two fixes: combining the division and
its `+= previous` into one expression keeps the rounding chain in r0
throughout instead of relocating through r1, and mirroring the initial
comparison's branch polarity to match the ROM's actual `bge` (not the
logically-equivalent but differently-compiled `blt`) fixes the constant's
register at the same time, since the two were coupled rather than
independent. `CountConsumableInventoryCopies` (0x08057078) was similarly
reversed from an earlier rejection: its candidate's own fixed-point-stepping
shape matches once split into the ROM's exact three-statement order
(dereference the root, compute the constant, add the offset). Full detail
for both in `docs/decompilation-notes.md`.

## Verified snapshot: game-state root record getters, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- All 198 host tests pass.
- The mechanical PRET audit remains at **0 errors / 0 warnings / 18
  documented exceptions**.
- `audit_provenance.py` now reports 1822/1822 mapped ranges compiled C.

Five more functions matched this session, all off the same
`IwramGameStateRootLayout` root or the same game-state base: three record
getters (`GameStateGetEffectSlot`, `GameStateGetRecord3F3C`,
`GameStateGetRecord403C`), the real-named `GameStateGetHitRegion` (a third
sibling whose type and name were already established by existing callers),
and `CountConsumableInventoryCopies` -- see the "Rejected match, later
reversed" entry below, which this same pass reopened and matched. A sixth
candidate (`sub_080099E0`, extern-declared elsewhere as
`s16 *(u32 actor, u32 group)`) hit a genuine allocator tie -- its trailing
constant's different immediate-loading strategy flips which register holds
the actor accumulator versus the dereferenced base -- and was left as
assembly after six restructurings failed to reach it. Full detail for all six
in `docs/decompilation-notes.md`.

## Verified snapshot: compiler-steering audit, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- `make english` succeeds and all 198 host tests pass.
- The mechanical PRET audit reports **0 errors / 0 warnings / 18 documented
  exceptions** on one summary line. Every exception is enumerated in the
  generated report.

The audit now reviews explicit pointer-to-integer casts and pointer/integer
address unions across all `src/` C. It deliberately permits typed pointer
arithmetic, `u8 *` byte offsets, THUMB-bit function encodings, alignment and
low-bit tests, BIOS wrappers, serialized sound-table pointers, and documented
linker `.set` offsets. A `PRET_PTR_INT_OK` note is accepted only when it states
the modeled operation, the caller/ABI/ROM evidence, and why typed pointer
arithmetic is unsuitable. The report still warns that declaration order,
local lifetime, integer width, unions, and control-flow spelling can steer
code generation without an explicit pointer cast.

Fifteen findings were removed through natural types and expressions. Two
previous matches were rejected after exact comparison showed that their
pointer/integer spelling only selected registers or alias behavior:
`SpriteAffineAllocate` and `ScriptResourceSet`. Their exact implementations are restored to named
assembly and their clean candidates remain in `src/nonmatching/` with the
specific code-generation differences documented.

`GameStateAddResourceCounter` has since been recovered as exact C without the
rejected pointer/integer union. The IWRAM root is a shared 32-bit storage word;
reading the runtime address from that word gives the root and counter field the
same `u32` alias class. agbcc consequently reloads the root after the counter
store exactly as the ROM does. The cast back to the typed runtime structure is
documented and counted as a pointer/integer exception.

`ScriptNativeSetFriendArms` now uses two block-scoped slot pointers for its
two independent address calculations. The distinct lexical lifetimes make
agbcc reuse r0 naturally and remove the last forced-register and duplicate
inline-assembly findings from `src/resource_native.c` without changing the
ROM.

`CreateSoundFadeTask` no longer claims a fixed r10 callback register as C.
The exact symbolic implementation is retained in assembly and the natural,
typed reconstruction is kept in `src/nonmatching/sound_fade_create.c`. This
removes its forced-register and duplicate inline-assembly findings while
leaving the unresolved callback lifetime visible to future contributors.

`CreateSoundPlayerIdleWait` follows the same honest fallback for its three
unresolved register allocations. Its exact 216-byte switch and task creation
path are symbolic assembly, while its clean behavioral reconstruction remains
under `src/nonmatching/`. This removes six more duplicated audit findings.

`CreateSaveWriteTask` is now an exact 24-byte symbolic assembly wrapper with
its ordinary forwarding implementation retained under `src/nonmatching/`.
Both callers verify the public argument order, so the unresolved r4/r5 swap is
documented rather than hidden by two C register constraints.

`ScriptNativeCopyMapHalfwords` no longer uses fixed r2 and r1 declarations.
Its exact 72-byte implementation is named, relocatable assembly and its clean
typed loop remains in `src/nonmatching/`. The clean candidate has the same
size, calls, loop bounds, mutable-root reload, and stores, but both available
agbcc snapshots order the index shift and record-offset addition differently
and select a different left operand for the induction update. This removes the
last four hard findings from `mapping.c` without presenting compiler steering
as recovered source.

The final 43 findings came from the three affine OAM matrix writers. Their old
source forced twelve registers per routine and passed two uninitialized dummy
locals through empty inline-assembly constraints. The exact routines are now
named assembly, while a typed reconstruction in
`src/nonmatching/sprite_affine_matrix.c` records the matrix layout and
fixed-point formulas. The checked-in hard-error baseline is consequently
empty; any new hard finding fails CI rather than becoming accepted debt.

`ReadSram`, `WriteSram`, and `VerifySram` no longer force the WAITCNT pointer
and value into r2 and r0. These cartridge-bus library routines are retained as
exact symbolic assembly, while their clean volatile-register implementations
live in `src/nonmatching/sram_access.c`. The shared GBA register header now
provides `REG_WAITCNT`. This removes all eight findings from `src/sram.c` and
keeps the fixed hardware address visibly classified as platform I/O.

`SpriteAffineFind` no longer uses the shared `TARGET_REGISTER` macro or three
fixed low-register temporaries. Its exact 106-byte routine plus two-byte tail
is named assembly, and its readable wraparound search is retained under
`src/nonmatching/`. The natural candidate hoists a loop unit into r8 and moves
the engine-root address to r9, expanding the saved-register frame; the older
frontend also differs. This removes the six findings from
`src/sprite_affine_slots.c` without encoding that allocation in C.

`SpriteInterpolationInit` no longer uses four forced-register declarations, an
empty assembly scheduling fence, or an initializer that reads an uninitialized
pointer. Its exact 94-byte routine and two-byte tail are named assembly, while
the typed eight-array initializer remains in `src/nonmatching/`. Both compiler
frontends and all 120 declaration orders for the five relevant locals failed
to recover the ROM's storage/count/output register cycle. This removes all
eight findings from `src/sprite_interpolation.c`; only the two clean matching
interpolation evaluators remain in that translation unit.

`SpriteProjectPoint` now exposes the projection divisor and viewport origin as
typed `SpriteEngineState` fields. Its prior matching source forced the running
origin into r4. Removing that constraint preserves the 104-byte size but
rotates six live values; all 720 declaration orders for those values failed to
recover the ROM prologue. The exact symbolic routine is retained in assembly
and the direct typed projection remains in `src/nonmatching/`, removing one
more forced-register finding without losing the recovered behavior.

The X/Y/Z sprite-vector rotations no longer pin a shared table scratch to r3.
Their exact named assembly preserves the ROM's repeated sine-table loads, while
concise typed implementations remain in `src/nonmatching/`. Both compiler
revisions, typed indexing, an unused fourth-parameter ABI hypothesis, explicit
scratch-value flow, and 2,000 declaration orders all produced the same smaller
allocation family. Moving the exact routines honestly to assembly removes
three more hard findings without presenting the r3 choice as recovered C.

`SpritePackAffinePosition` and `SpriteBuildAffineMatrix` no longer carry eleven
fixed-register declarations and two empty assembly barriers. The recovered
record now names the 28-bit X/Y packing split across its first eight bytes, and
the clean reference uses direct fixed-point matrix formulas and named masks.
New and old agbcc emit different smaller routines, while the ROM's exact forms
remain symbolic assembly. Removing the obsolete matching translation unit
clears its final 16 findings; `sprite_affine_matrix.c` is now the sole hard
finding source.

## Verified snapshot: renderer reference fallback, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- The English ROM builds successfully and all 175 host tests pass.
- The mechanical PRET audit reports 112 errors, zero warnings, and zero
  documented exceptions.

`SpriteTileAllocatorRelease`, `SpriteResourceFindGroup`, and
`SpriteFixedSqrt` no longer use forced registers or inline assembly in linked
C. Their exact instructions are restored to assembly and their clean C
implementations are retained under `src/nonmatching/`, with each remaining
code-generation difference documented in `COMPILER_HINT_CLEANUP.md`. This
follows the required fallback in `PRET_STANDARDS.md` without claiming that the
original source used compiler hints.

The following clean-C pass added five more functions without increasing the
112-error backlog: four actor byte-field accessors at 0x0800943C..0x080094B4
and `EncodeHexDigitFromS16` at 0x080577A0. One accessor had previously been
misclassified as anonymous word data. The Japanese ROM remains byte-identical,
the English ROM builds, and all 175 host tests pass after this pass.

## Verified snapshot: object-cleanup batch, 2026-09-15

- `make compare` reproduces the Japanese ROM byte for byte.
- All 143 host tests pass, and the English build succeeds.
- The mechanical PRET audit reports 149 errors, zero warnings, and zero
  documented exceptions. The project is not yet fully PRET-compliant.

These results describe this batch. Use fresh tool output for current counts;
older counts below describe historical passes. The earlier 157-error and
13-exception snapshot is stale. The mechanical audit does not establish that
every name, type, comment, or claimed decompilation is semantically correct.

A 2026-09-14 pass closed out the remaining Doxygen backlog: 531 missing-doc
warnings across 37 files went to zero over 12 commits, verified with
`make compare` and the full 140-test host suite after every file (or small
group of files). Two of those commits needed care beyond adding comments:
`script_sprite.c`'s plain-text section banners doubled as literal slice
markers for four host tests that compile parts of the file standalone
against a fake ABI (replacing a banner with a Doxygen block broke that
slicing until the banners were restored alongside the new comments and the
tests' anchor strings updated to match), and a few cross-file claims (e.g.
`ScriptNativeSpriteEffect` and `ScriptNativeHitEffectQuery` sharing workers
with commands in `script_sprite.c`) were checked against the actual callee
signatures before being written down, not assumed from naming alone.

A later pass cut the hard-rule count from 351 to 157 by proving which compiler
hints were load-bearing rather than assuming it. See
`docs/COMPILER_HINT_CLEANUP.md` for the method and its pitfalls. `tools/drop_register_hints.py`
removes one pin at a time, recompiles that single translation unit with the real
toolchain, and keeps the removal only when the generated assembly is unchanged.
Removals are cumulative because register allocation is global, so a hint that
looks redundant alone can become necessary once its neighbours are gone: in
`src/sprite_transform.c` 54 of 61 hints passed individually but only 34 survived
cumulatively. The comparison must be on machine code rather than assembly text,
because a removed scheduling fence also removes `.code 16` directives that would
otherwise register as a difference. In total 210 hints were removed with the ROM still byte-identical, and the
fifteen alignment-padding halfwords in the sound files moved from inline asm to
the project's AT(...) Tail[2] idiom.

The 157 that remain were needed by the C shapes tested at that time. That does
not establish that the original source required them: each still needs a
structural recovery attempt in ordinary C, or an assembly implementation with
its readable C kept under `src/nonmatching/`.

Re-ran `tools/drop_register_hints.py --all` on 2026-09-14: it removed zero
additional hints (all 22 forced-register and 92 TARGET_REGISTER pins report
load-bearing). This confirms the count is stable, not that it is finished --
per the remediation order below, the next step for each surviving hint is a
structural C rewrite, not another mechanical pass. Manual attempts at the
former `src/sound_fade_create.c` r10 pin and `src/sound_idle_wait.c`'s three
pins via reordering declarations did not reproduce the original register
allocation. Both sound constructors have since moved to exact assembly with
their clean candidates retained under `src/nonmatching/`. Those probes rule
out only the tested declaration orders, not a future clean-C reconstruction.
`src/sprite_affine_matrix.c` carries the largest single concentration
(48 of the 92 TARGET_REGISTER pins) and is the highest-value structural-rewrite
target for a future pass.

A 2026-09-14 round tried declaration-order and expression-merging variants
for `SpriteTileAllocatorRelease` and `ScriptNativeSetFriendArms` without a
match. This establishes failure of those candidates, not an immutable
register-allocation limitation. Subsequent typed array indexing removed the
allocator's base-pointer pin, demonstrating why conclusions must remain
provisional. Do not add dummy live values or forced registers to manufacture
a match; reconstruct the real types, expressions, and lifetimes instead.

The removal tool tests specific transformations of the current C. It is not
an exhaustive search of equivalent C programs and cannot prove that a pin is
inherently necessary.

## Remediation order

1. Run `tools/drop_register_hints.py --all` first: it tests whether direct hint
   removals preserve the generated code. A remaining hint may still be
   removable through a different C expression or type. Then, for each surviving hint, attempt a structural rewrite that
   matches as ordinary C. Only when that fails should the function move back
   behind its original assembly implementation, keeping readable C under
   `src/nonmatching/` with the exact mismatch documented.
2. Done: the BIOS instruction wrappers (eleven, not ten -- the sound driver
   ships its own separate CpuFastSet copy) now live in real assembly
   (asm/code/bios_calls.s and one entry in asm/code/code_0780C0.s) instead of
   naked inline C, with plain extern prototypes at their call sites.
3. Replace raw ROM function and string addresses with verified linker symbols.
   Confirm odd THUMB pointers before subtracting their low bit.
4. Replace proven flags, limits, strides, and structure offsets with named
   constants. Do not invent semantic names without call-site evidence.
5. Rename placeholder and non-PascalCase functions as their behavior becomes
   known. Doxygen coverage over every manifest-backed function is complete;
   keep it that way by documenting each newly decompiled function as it
   lands, rather than letting the backlog reaccumulate.
6. Periodically re-check `src/nonmatching/` against `src/decompiled.json`. A
   file there is stale once its address appears in the manifest, meaning the
   function was matched elsewhere and nobody removed the old copy. Two of the
   eight were stale when this was first checked, and one of them additionally
   claimed its branch layout was "not something the source controls", which was
   false. Treat an unverified claim in a nonmatching header as a lead to retest,
   not as a settled result.
7. Keep CI baseline-strict while the historical hard-error backlog is reduced.
   Remove resolved fingerprints from `tools/pret-audit-baseline.json` as cleanup
   lands. Once the baseline is empty, plain `--strict` and baseline-strict mode
   are equivalent.

This order preserves the byte-identical build throughout the cleanup. A lower
C percentage is preferable to counting C that violates the project's matching
rules.

## New-decompile session, 2026-09-14/15

Reconstructed three previously-undecompiled functions (all logically verified
against the disassembly, none forced with a new register hint or inline
asm), landing two as `src/nonmatching/` and one still unresolved:

- `sub_08004DA8` -> `RuntimeGetLinkActivityState`. This was subsequently
  matched as clean C in `src/runtime_core.c`; `sub_08004DBC` was confirmed as
  a false internal split, and both unresolved `bl` encodings target
  `SioGetPlayerId`.
- `sub_08056290` -> `GameStateGetResourceCounter` now matches as clean C in
  `src/game_state_records.c`, alongside its sibling at 0x080562C8,
  `GameStateAddResourceCounter`. Both operate on a 999999-capped counter at
  the game root's +0x38BC. The getter was recovered by expressing the fixed
  IWRAM root-slot offset as a named layout constant and separating the root
  dereference from the counter-field offset.
- `sub_08078644` is now named `SoundTrackReleaseChannels`. Comparison with
  Nintendo MusicPlayer2000's preserved `TrackStop` routine in pret's
  `pokeemerald/src/m4a_1.s` showed an instruction-for-instruction match,
  including the local `bx r3` indirect-call helper. This is original library
  assembly rather than compiler output. The speculative nonmatching C was
  removed and the original assembly retained under semantic names, as required
  by PRET's rule against forcing hand-written assembly through unnatural C.

**Stale, corrected later the same session**: this paragraph described the
resource-counter getter (`GameStateGetResourceCounter`) as still needing
more than a superficial register swap, contradicting the "now matches as
clean C" note two paragraphs above for the same function. It matches; see
`src/game_state_records.c` and `src/decompiled.json`. Original text, kept
for reference: "The resource-counter getter still needs more than a
superficial register swap: the matching union alias model for the increment
function does not reproduce the getter's r4/r3 root-and-offset allocation.
Verify every instruction and relocation before promoting the remaining
candidate."

Same session, continued: landed two more functions as clean, byte-exact
matches (`RuntimeSetFlagC0` at 0x08009728, `GameStateSelectDeckPointer` at
0x0800696C -- the latter needed the two-local-variable idiom documented in
"Method notes" in `docs/AGBCC_CODEGEN.md` to avoid a `.set`-symbol folding
false match), then reconstructed a fourth as `src/nonmatching/`:

- `sub_08070EA0` -> `GeneratedMapFindFreeRuntimeRoom`
  (`src/nonmatching/generated_map_free_runtime_room.c`). A sibling of the
  already-matching `GeneratedMapFindRuntimeRoom` (0x08070EEC): mode 0
  returns the same room-array base pointer, mode -1 searches the same
  64-entry array for the first *inactive* room (a free slot to allocate
  into) instead of an active one matching an index. Logically confirmed
  correct; the per-iteration active-byte check has an extra register copy
  in the ROM (`adds r1,r5,#0` before combining with the base) that no C
  shape reproduced without also changing the addition's grouping or the
  loop's register assignment (r4/r5 swapped from the ROM's in every
  variant tried) -- see the function's own header comment for the full list
  of shapes tried. Same allocator-not-steerable family as the OR/ADD
  register ties already documented, not a logic error.
- `sub_0807EC58` is now the byte-exact `ScriptFrameReleasePools` in
  `src/script_frames.c`. The earlier claim that its inner loop could not be
  reproduced in ordinary C was false. Initializing the element index before
  loading the moving pointer, then advancing both in the `for` header,
  produces the original instruction order without register variables or
  inline assembly. The recovered `ScriptFrame` fields at 0x44, 0x84,
  0x88 and 0xA8 are now typed and the obsolete nonmatching file is gone.
- `sub_08080070` -> `ScriptRunFrameStep`
  (`src/nonmatching/script_run_frame_step.c`), called from
  `ScriptDispatchCurrentFrame()`. Dispatches one deferred callback bit from
  the frame's 8-entry callback table if one is pending, else reads one
  bytecode opcode and tail-calls its handler out of the 256-entry
  `gScriptOpcodeHandlers` table. Needed two raw `bl` fragments manually
  decoded (to already-decompiled `ScriptPushFrameAndJump` and
  `ScriptReadNextU8`) to find the function's true extent. Very close: only
  one BIC destination differs from the ROM. Modeling the VM through its
  recovered shared-storage union and reusing the real pointer/scalar scratch
  lifetimes now reproduces the context in r4, flag pointer in r2, mask in r1,
  loaded word in r3, post-store frame reload, and exact frame+0xAA address.
  The ROM copies r3 to r0 before BIC; agbcc clears r3 in place. Full detail is
  in the file's header comment and `docs/decompilation-notes.md`.

`CreateEncounterResetTask` (0x0806EFCC, 76 bytes) was also promoted from its
guarded nonmatching reference. The ROM's apparent 16.16 loop is agbcc's
strength reduction of `i = (s16)(i + 1)` on an `s32` index. Expressing that
real narrowing produces a byte-identical routine without a forced register or
compiler workaround; its assembly block is gone and its caller uses the named
prototype in `task_constructors.h`.

`IsMainMountName` (0x08002158, 48 bytes) was recovered from a region that the
old recursive disassembly emitted entirely as raw words. It now uses ordinary
early-return C and named `SYSTEM`/`MAR` mount-name symbols, with no fixed ROM
pointer, compiler hint, inline assembly, or artificial qualifier. The exact
ROM hash confirms both the function and the corrected 0x08002188 boundary of
the following raw region.

The raw-code audit also recovered two functions that the original disassembly
had classified as `.4byte` data. `RuntimeActorSetField358` (0x08009778, 28
bytes) and `RuntimeActorHasPartField37Value6` (0x08009868, 56 bytes) now build
from clean, shiftable C in `src/runtime_buffers.c`. Both use the named
`gSecondaryRuntime`/`RuntimeGetActorRecord` interfaces and exact-width types;
neither uses a register variable, inline assembly, a hardcoded address, or an
artificial volatile access. Their former 84 raw assembly bytes are removed,
and `make compare` still reproduces the original SHA1.

Six more raw/mis-split actor-part accessors now contribute 304 byte-identical
bytes from `src/runtime_objects.c`: the paired fixed-point setters at
0x0800A3B8 and 0x0800A484, plus paired raw-word setters/getters at
0x0800A550, 0x0800A580, 0x0800A604, and 0x0800A634. They use named record
layout constants and `gSecondaryRuntime`, with no absolute pointers, inline
assembly, forced registers, or fake volatile accesses. The neighboring signed
halfword readers remain assembly because agbcc naturally emits `ldrh` where
the ROM uses `ldrsh`; exact matching remains the higher priority.

The raw-word audit also identified `CreateNamedRuntimeRecordTask` at
0x080073DC..0x08007410. Its entire 52-byte body now comes from documented,
shiftable C in `src/task_constructors.c`; the raw `.4byte` sequence was removed
and the following assembly was given its true section boundary. The exact-ROM
comparison passes. This conversion adds no register variable, inline assembly,
absolute ROM address, artificial volatile access, or compiler-shaping dead
code.

`ScriptNativePmbDeckMake` was retested with a newly recovered typed argument
layout. It can be made byte-identical only by an unreachable conditional whose
sole purpose is suppressing agbcc's pointer hoist; that candidate was rejected
as a fakematch. It therefore remains honestly nonmatching under the golden-rule
requirements.

`GameStateGetResourceCounter` was subsequently resolved without the aggregate
or any hint. Holding the named numeric IWRAM root-slot offset in a local before
forming the root pointer, then dereferencing that root before assigning the
counter-field offset, produces the ROM's r4/r3/r2 lifetimes naturally. Its
obsolete nonmatching source and raw assembly body are removed.

`CreateFieldEventModeTask` at 0x08065E88 adds a second raw-word recovery in
the same pass. Its 44 bytes now compile from documented C, and the following
callback retains its true 0x08065EB4 section boundary. The source uses only
the task API, an exact-width signed narrowing, and a named C symbol; it adds
none of the constructs prohibited by the golden rule.

`RuntimeHistoryPush` at 0x08004E88 removes another function that the original
recursive disassembly emitted as 84 bytes of raw `.4byte` data. The recovered
routine advances a signed ten-entry circular-history cursor, records the
previous slot, and copies one 24-byte snapshot through typed word cursors.
Named structure fields replace offsets 0xFA and 0xFB, and the fixed entry and
word counts are named constants. It contains no absolute address, register
request, inline assembly, volatile access, or dead optimizer-shaping branch;
the complete ROM remains byte-identical.

`CreatePaletteInterpolationTask` at 0x0800382C replaces 120 bytes that were
previously emitted as anonymous words. The recovered 20-byte task state names
its frame counter, packed palette selector, 8.24 blend accumulator and signed
per-frame step, plus its two palette buffers. The constructor uses the typed
task API, named fixed-point constants, and a normal structure; it has no raw
address, inline assembly, register request, fake volatile access, or dead
compiler-shaping code. The callback also now has the meaningful assembly
symbol `PaletteInterpolationTask`, and the complete ROM remains byte-identical.

`CreatePaletteSequenceTask` at 0x080041E0 replaces a further 136-byte region
that the old disassembly split between anonymous words and the false internal
function label `sub_0800421E`. The recovered 36-byte task state owns copies of
both caller-supplied color arrays and records the target, signed color count,
and normalized frame count. Its callback is now named `PaletteSequenceTask`.
The source uses typed heap, copy, and task interfaces without fixed addresses,
register requests, inline assembly, artificial qualifiers, or dead code. An
explicit, base-ROM-verified two-byte zero tail preserves the original section
padding; otherwise GNU `as` emits a Thumb NOP. The complete ROM remains
byte-identical.

`TryPurchaseArmOrConsumable` replaces 128 raw bytes with readable inventory
logic and a public result enum. Its costs, limits, modes, and failure sentinel
are named; both known callers use linker symbols. The C contains no fixed ROM
address, register request, inline assembly, artificial volatile access,
unreachable compiler-shaping branch, or incompatible helper prototype. The
complete ROM remains byte-identical.

`CreateTask62304` replaces 128 bytes of anonymous words with a documented task
payload structure and named constants for its allocation size, owner stage,
resource offset, selection bound, and fallback. It uses linker symbols and a
typed field reload to express the task state's ownership; it contains no fixed
ROM address, register request, inline assembly, artificial volatile access,
unreachable compiler-shaping branch, or compiler change. Its unaligned raw
caller is now a relocatable named branch, and the complete ROM remains
byte-identical.

Five additional callable regions that the old split represented as raw words
now build from clean C. `RuntimeSetBufferEEAName` (0x08007604) copies a name
into the secondary runtime's +0xEEA buffer. `CreateTaskB478`,
`CreateTask1BAD8`, `CreateTask51E84`, and `CreateTask68C0C` recover four
main-manager constructors, including their state sizes, retained owner fields,
and the final constructor's shared-runtime reset call. Their callback roles
are not yet known, so the address suffixes remain instead of speculative
names. All five use named runtime/task symbols and contain no fixed ROM
address, register request, inline assembly, fake volatile access, or dead
compiler-shaping code. Five formerly encoded call displacements are now
relocatable `bl` references. The complete ROM remains byte-identical.

`NcdReleaseSpriteArray`, `CreateTask66068`, and `CreateTask683C4` replace a
further 176 raw bytes with exact-width, documented C. Named constants cover
the array count, fixed-point shift, task-state sizes, owner flag mask, and
initial stages. Typed payload structures replace raw task-offset arithmetic,
and all known callers now branch to linker symbols. These functions contain
no absolute ROM address, register request, inline assembly, artificial
volatile access, unreachable compiler-shaping branch, or compiler change. A
base-ROM-verified two-byte zero object records `CreateTask66068`'s original
alignment bytes. The complete ROM remains byte-identical.

`CreateTask6EAFC` replaces another 72-byte anonymous raw region with clean C.
A partial `TaskAdapterRuntime` describes the verified distance between the
main task manager and game-state root pointer; the code uses that structure,
the task API, and named constants rather than either absolute IWRAM address.
The false internal `sub_0806EB0C` label is removed, the true callback boundary
is recorded, and the known caller uses a relocatable symbol. The function has
no register request, inline assembly, artificial volatile access, unreachable
compiler-shaping branch, or compiler change. The complete ROM remains
byte-identical.
- `sub_08070F20` -> `GeneratedMapChooseAttributeIndex` (`src/mapping.c`). The
  96-byte KMP attribute-grid selector now uses typed viewport/header fields,
  the named deterministic map RNG, and a relocatable symbol at both callers.
  It contains no raw address, inline assembly, register constraint, volatile
  access, or optimizer-only construct.
- `sub_08015268` -> `BattleActionUnavailable`
  (`src/battle_task_adapters.c`). The 20-byte fallback now uses an exact-width
  callback signature and ordinary status handling. Forty-four dispatch slots
  reference its relocatable C symbol, replacing the fixed `sub_08015269`
  THUMB-address alias. It contains no raw address, inline assembly, register
  constraint, volatile access, or optimizer-only construct.
- `sub_08006760` -> `GameStateSetFlagsAC` (`src/runtime_misc.c`). The
  36-byte game-state flag setter uses a typed bit-bank address and the named
  `BitSet` helper. Both known callers use its relocatable symbol. Separate
  base and offset locals reproduce agbcc's literal order without a register
  constraint, fixed address, inline assembly, volatile access, or dead code.
- `sub_08006F0C` -> `GameStateSetField425A` (`src/game_state.c`). The
  40-byte setter names its required runtime-status reset and its game-state
  byte field. Eight raw callers now use a relocatable symbol. Natural locals
  preserve the call-live value and literal order without register pinning,
  inline assembly, fixed addresses, volatile access, or dead code.
- `sub_08006DF0` -> `GameStateSetField424C50` (`src/game_state.c`). The
  48-byte paired-field setter uses partial IWRAM and game-state structures to
  model the root-slot alias that requires a reload between stores. Sixty-eight
  callers use its relocatable symbol. The exact C needs no register pin,
  inline assembly, volatile access, fixed address, or dead code.
- `sub_08009A04` -> `RuntimeUpdateFiveParts` (`src/runtime_buffers.c`). The
  32-byte wrapper is an ordinary five-iteration loop over the shared per-part
  updater, with an explicit verified two-byte zero tail. Eight callers use a
  relocatable symbol; the implementation has no register pin, inline
  assembly, volatile access, fixed address, or dead code.
- `sub_0800AFA0` -> `RuntimeSetModeE4B` (`src/table_accessors.c`). The
  44-byte setter updates secondary-runtime byte +0xE4B and clears +0xE4C when
  the mode is zero. Six callers use its relocatable symbol. The false
  `sub_0800AFC2` split inside the IWRAM-address literal is removed; the clean
  C uses no register pin, inline assembly, volatile access, fixed address, or
  dead code.
- `sub_08017E4C` -> `FieldActorUpdateForMode` (`src/map_field.c`). The
  36-byte dispatcher selects the mode-1 or general field-actor update path
  through the named game-state accessor. Both callers use a relocatable
  symbol. It contains no register pin, inline assembly, volatile access,
  fixed address, or dead code.

- `sub_08075EBC` -> `RuntimeUpdateFirstThreeGroups`
  (`src/runtime_buffers.c`). The 24-byte wrapper uses an ordinary bounded loop
  to update actor zero across groups zero through two. The following raw word
  run is confirmed hidden code and now begins in its own preserved section.
  The C uses no register pin, inline assembly, volatile access, fixed address,
  or dead code.
- `sub_08006C2C` -> `HitBoundsTranslate` (`src/hit_region.c`). The 40-byte
  helper translates four corner bounds by signed world coordinates. Its only
  caller uses the relocatable name. Explicit typed locals and a well-defined
  signed expression preserve agbcc's original operand order without a register
  pin, inline assembly, volatile access, dead code, or a fixed address.

- `sub_08005C38` -> `ScriptNativeSetCrtFade` and hidden code at 0x08005C5C
  -> `ScriptNativeGetCrtFade` (`src/scene_native.c`). The 36- and 24-byte
  handlers are identified by the native registration table and use a named
  IWRAM offset for the shared fade value. Their table pointers are symbolic.
  Neither uses a register pin, inline assembly, volatile access, dead code, or
  a fixed address.
- `sub_08009AF8` -> `RuntimeGetPartValue` (`src/runtime_buffers.c`). The
  44-byte accessor indexes the actor/group signed-halfword table through named
  layout constants and an ordinary reused offset local. All three callers use
  its relocatable symbol; no compiler hint or fixed address is present.
- `sub_08011414` -> `ScriptSpriteSetHitBounds` (`src/script_sprite.c`). The
  36-byte SprHitRect worker now uses the verified `HitBounds` member and enable
  bit in `ScriptSprite`; its native adapter was renamed accordingly. It uses no
  register pin, inline assembly, volatile access, dead code, or fixed address.

- `sub_08008A44` -> `RuntimeResetListSlot` (`src/runtime_objects.c`). The
  44-byte helper initializes one secondary-runtime intrusive list and clears
  its parallel owner pointer. All nine callers use the relocatable name. The
  clean C has no register pin, inline assembly, volatile access, dead code, or
  fixed address.
- `sub_08008BE4` -> `RuntimeReleaseListSpriteAllocations`
  (`src/runtime_objects.c`). The 48-byte helper walks a selected secondary
  runtime list and releases the NCD allocation embedded after each node's two
  links. All five callers use the relocatable name. Typed list and sprite
  records plus an ordinary offset local reproduce the ROM without a register
  pin, inline assembly, volatile access, dead code, or fixed address.
- `sub_080020EC` -> `CalculatePointDistance` and `sub_08002118` ->
  `CalculatePointAngle` (`src/geometry.c`). The 44- and 48-byte helpers use
  typed signed coordinates and the named BIOS `Sqrt`/`ArcTan2` calls. All raw
  callers use relocatable symbols. Neither helper contains a register pin,
  inline assembly, volatile access, dead code, or fixed address.


## Battle-party helper batch (2026-09-20)

Five routines at 0x08055EC8..0x08055FE8 now build from documented, shiftable C,
removing 228 bytes of assembly. All call sites use linker symbols, the shared
party table uses `BATTLE_PARTY_DEFAULT_COUNT`, and the runtime root is accessed
through `gMapGenerationRoot`. The batch adds no raw ROM address, forced
register, inline assembly, artificial volatile access, or unreachable
compiler-shaping code. The hidden getter at 0x08055F38 was recovered from raw
word directives. Full ROM comparison remains byte-identical.

The resolver at 0x08055F4C was deliberately retained in assembly: its clean C
candidate is behaviorally and structurally correct but swaps two low registers.
This follows `PRET_STANDARDS.md` rather than forcing a cosmetic match.

## Rejected match, later reversed: consumable inventory scan (2026-09-20)

The apparent C match for `CountConsumableInventoryCopies` at 0x08057078 used
a pointer/`u32` union solely to reproduce the ROM's low-register allocation.
The disassembly instead shows normal pointer formation and pointer increments,
so the union was rejected as compiler steering. The natural C candidate was
initially documented in `src/nonmatching/consumable_inventory.c` with the
exact body left in assembly under the PRET fallback rule.

The associated trace remains valid: game-state offsets `0x31D0` and `0x33D0`
are the 256-slot consumable inventory and its snapshot, so four older
offset-based or map-named helpers retain their inventory-specific public names.
The native command registered as `ItemInit` is now named
`ScriptNativeClearConsumableInventory`, replacing its earlier speculative map
name without changing its still-audited implementation.

**Later reversed**, same day: the union was never necessary. The candidate's
own natural fixed-point-stepping shape reproduces the ROM byte-for-byte once
its statements are ordered to match the ROM's actual computation sequence
(dereference the root pointer, compute the `1<<16` constant, then add the
table offset -- three separate steps, not one or two). See
`docs/decompilation-notes.md`'s "Rejected match reversed" entry for the exact
technique. `src/nonmatching/consumable_inventory.c` is deleted; the real
function is now `CountConsumableInventoryCopies` in `src/runtime_accessors.c`.

## Verified snapshot: `_close_r` decompiled, 2026-09-21

`sub_080868BC` -> `_close_r` (`src/libc/closer.c`). The 44-byte newlib
reentrant wrapper around `_close` had a `.thumb_set` placeholder alias in
`asm/iwram_symbols.s` (a previous session had already identified it, per
`src/libc/stdio.c`'s existing call site, but left it as an assembly stub)
now replaced with a real matching definition. `errno` is a new named
`.set errno, 0x03006124` IWRAM alias (previously an unlabelled gap between
`gSpriteRuntime` and `end`). `audit_pret_standards.py` reports 0
errors/0 warnings/18 exceptions; `audit_provenance.py` reports 1826/1826;
`make compare` confirms the byte-identical ROM. See
`docs/decompilation-notes.md`'s "New decompile: `_close_r`" entry for the
full identification and the `agbcc_probe.py` flag pitfall it turned up.

## Verified snapshot: `_fstat_r` decompiled, 2026-09-21

`sub_080868F4` -> `_fstat_r` (`src/libc/fstatr.c`), immediate neighbor of
`_close_r` above and the same reentrant-wrapper shape with one more
parameter. `audit_pret_standards.py` reports 0 errors/0 warnings/18
exceptions; `audit_provenance.py` reports 1827/1827; `make compare`
confirms the byte-identical ROM. This one also surfaced a real mechanical
pitfall worth flagging for future cuts: removing this function's raw-asm
block also removed the only `.section` directive covering the still-raw
`abort` implementation right after it, which had been silently inheriting
that section rather than declaring its own -- the build still linked
(no missing-symbol error) but `make compare` failed with ~45 small
scattered diffs from re-encoded relative branches. See
`docs/decompilation-notes.md`'s "New decompile: `_fstat_r`" entry for the
fix and the general signature to watch for.

## Verified snapshot: `_write_r`/`_lseek_r`/`_read_r` decompiled, 2026-09-21

Three more newlib reentrant wrappers in the same cluster as `_close_r`/
`_fstat_r`, same shape, each with a pre-existing `.thumb_set` alias:
`_write_r` (`src/libc/writer.c`), `_lseek_r` (`src/libc/lseekr.c`),
`_read_r` (`src/libc/readr.c`). `audit_pret_standards.py` reports 0
errors/0 warnings/18 exceptions; `audit_provenance.py` reports 1830/1830;
`make compare` confirms the byte-identical ROM (passed on the first
attempt -- the `.section` lesson from the previous `_fstat_r` cut was
applied up front this time). `isatty`, in the same neighborhood, was
investigated but not decompiled: its real body is 4 bytes matching every
`return 1;` probe tried, but the ROM has 4 unexplained extra bytes (a dead
second `bx lr` plus a zero halfword) right after it with no confirmed
source shape reproducing them. Left as-is rather than guessed. See
`docs/decompilation-notes.md`'s "Three more newlib reentrant wrappers"
entry.

## Verified snapshot: `_sbrk_r` decompiled, cluster complete, 2026-09-21

`sub_080862E0` -> `_sbrk_r` (`src/libc/sbrkr.c`), same reentrant-wrapper
shape as the rest of this cluster, one forwarded argument, `bl` target
`_sbrk` corroborated independently by `mallocr.c`'s own pre-existing
`MORECORE(size)` macro. `audit_pret_standards.py` reports 0 errors/0
warnings/18 exceptions; `audit_provenance.py` reports 1831/1831;
`make compare` confirms the byte-identical ROM. This closes out every
`.thumb_set`-aliased-but-undecompiled newlib reentrant wrapper found this
session (`_close_r`, `_fstat_r`, `_write_r`, `_lseek_r`, `_read_r`,
`_sbrk_r` -- six decompiles from one method: grep `asm/iwram_symbols.s` for
placeholder aliases whose address has no `decompiled.json` entry yet).
`isatty` and `abort` remain, both investigated and left as assembly for
documented reasons. See `docs/decompilation-notes.md`'s "`_sbrk_r`, the
last reentrant wrapper in this cluster" entry.

## Verified snapshot: `__malloc_lock`/`__malloc_unlock` decompiled, 2026-09-21

`sub_080859C4`/`sub_080859C8` -> `__malloc_lock`/`__malloc_unlock`
(`src/libc/malloclock.c`), two empty-body no-op hooks (`bx lr`) already
declared and used by `mallocr.c`/`freer.c`/`callocr.c`'s `MALLOC_LOCK`/
`MALLOC_UNLOCK` macros. `audit_pret_standards.py` reports 0 errors/0
warnings/18 exceptions; `audit_provenance.py` reports 1832/1832;
`make compare` confirms the byte-identical ROM. Surfaced a real manifest
convention worth remembering for any future multi-function `src/libc/*.c`
file: `audit_provenance.py` expects one `decompiled.json` entry per linked
*section* (keyed by the section's start offset), not one per function --
two functions compiled into the same file share one section and need one
combined entry, the same way the pre-existing `stdio.c`/`__sread` entry
already covers four functions under one name and size. See
`docs/decompilation-notes.md`'s "`__malloc_lock`/`__malloc_unlock`: two
no-op hooks" entry.

## Verified snapshot: `ActorPartInitTask` decompiled, 2026-09-21

`sub_0800E64C` -> `ActorPartInitTask` (`src/runtime_buffers.c`), the task
callback `CreateIndexedPendingTask()` (`src/task_adapters.c`) already named
by address. First game-logic (not library-wrapper) decompile of this
session, found by scanning `asm/code/*.s` for small standalone raw-asm
functions rather than aliased newlib stubs. `audit_pret_standards.py`
reports 0 errors/0 warnings/18 exceptions; `audit_provenance.py` reports
1833/1833; `make compare` confirms the byte-identical ROM. Two real
shape lessons recorded in `docs/decompilation-notes.md`: a dual-condition
guard must be one short-circuiting `&&`, not nested ifs, to avoid an
extra ROM-absent load; and a repeated constant assigned to two locations
needs a named local assigned once, or agbcc materializes it into a scratch
register and copies it for the second use. Also needed the established
`AT(<same address>) const u8 ...Tail[2] = {0, 0};` companion-declaration
convention (`include/rom_section.h`) to fix 2 bytes of trailing alignment
padding that came out as a `nop` instead of the ROM's literal zero bytes --
the first time this specific session needed that technique. See
`docs/decompilation-notes.md`'s "First real game-logic decompile this
session" entry.

## Verified snapshot: `RuntimeActorInitFields338To344` decompiled, 2026-09-21

`sub_080094B4` -> `RuntimeActorInitFields338To344` (`src/runtime_buffers.c`),
the last of this session's small standalone-function candidates.
`audit_pret_standards.py` reports 0 errors/0 warnings/18 exceptions;
`audit_provenance.py` reports 1834/1834; `make compare` confirms the
byte-identical, correctly-16MB ROM (an intermediate attempt using a plain
C string literal for the `"A_WIN_G"` resource-group key produced a
compiler-emitted duplicate of those 8 bytes and an 8-byte-oversized ROM,
caught by `make compare`'s size-mismatch report; fixed by naming the
*existing* ROM bytes at `0x08086C84`, the same way `gResourceSpBa04`/
`gResourceTestE02` already do, instead of emitting new ones). Also
surfaced a real disassembly-reading pitfall: a `bl` instruction immediately
followed by another instruction can get merged into one `.4byte` grouping
in the generated `.s` file in a way that hides the second instruction
entirely from a naive read; `arm-none-eabi-objdump -D -b binary
--disassembler-options=force-thumb` against the raw ROM bytes decodes it
correctly where the stale symbol-grouped ELF disassembly does not. See
`docs/decompilation-notes.md`'s "`RuntimeActorInitFields338To344`" entry
for both lessons in full, including the CSE-vs-rematerialization shape fix
needed to match the ROM's actual register allocation.

## Verified snapshot: `SpriteAffineWriteDispatch` decompiled, session's scan complete, 2026-09-21

`sub_0807CDE0` -> `SpriteAffineWriteDispatch` (new file
`src/sprite_affine_matrix.c`), a `switch`-based dispatcher over the three
still-assembly `SpriteAffineWrite{Normal,Mirrored,AlternateAxis}` functions
(their own byte match remains a documented, correctly-deferred blocker;
this caller matches independently by only needing their names and
signatures). `audit_pret_standards.py` reports 0 errors/0 warnings/18
exceptions; `audit_provenance.py` reports 1835/1835; `make compare`
confirms the byte-identical ROM. Matching both the register allocation and
instruction order simultaneously required fresh, separately-named locals
for all three narrowed parameters (not reusing parameter names, not
narrowing the stack-passed fourth argument either first or via a deferred
second statement) narrowed in the ROM's own order; see
`docs/decompilation-notes.md`'s "`SpriteAffineWriteDispatch`" entry for
the full iteration log. This closes out every small standalone-function
candidate this session's `asm/code/*.s` scan turned up
(`_close_r`/`_fstat_r`/`_write_r`/`_lseek_r`/`_read_r`/`_sbrk_r`/
`__malloc_lock`/`__malloc_unlock`/`ActorPartInitTask`/
`RuntimeActorInitFields338To344`/`SpriteAffineWriteDispatch`, eleven
decompiles from eight commits this session).

## Verified snapshot: `GeneratedMapResize` decompiled, dungeon generation started, 2026-09-21

`sub_0807017C` -> `GeneratedMapResize` (`src/mapping.c`), the real logic
behind the `DungGenResize` script native (`ScriptNativeMapConfigure2`).
`audit_pret_standards.py` reports 0 errors/0 warnings/18 exceptions;
`audit_provenance.py` reports 1836/1836; `make compare` confirms the
byte-identical ROM. User-directed scope change to dungeon generation
specifically; the approach that worked for the rest of this session (scan
`asm/code/*.s` for small clean-boundary raw functions) doesn't apply to
this area (the relevant `game_table_handlers.s`-referenced functions are
large, deeply-interleaved blocks, one over 20KB), so the entry point was
instead the already-decompiled `DungGen*`/`Dung*` native-command wrappers
in `src/game_tables.c`/`src/mapping.c`, traced down to the still-raw
generator internals they call. This gives a sized work queue for
continuing (`sub_08070DA8` 472 bytes, `sub_08070F80` 316 bytes,
`sub_08070620` 296 bytes, up through much larger ones).

One struct-accuracy finding, not yet fixed: `struct GeneratedFieldMap`'s
`unknown14[0x640]` (`include/map_generation.h`) is 4 bytes too long -- it
swallows a real, distinct field at `+0x650` this function reveals (a
per-cell 4-bytes-wide array, parallel to the existing `cellRoomIndices` at
`+0x654`). Flagged for whoever next touches that struct. See
`docs/decompilation-notes.md`'s "Starting on dungeon generation" entry for
the full identification, the register-lifetime shape lesson (named pointer
locals reused across five intervening calls, not recomputed at each site),
and the harmless unavoidable `memset` built-in-prototype warning.

## Dungeon-generation dependency map and a resolved correct deferral, 2026-09-21

No new decompile this entry -- `sub_08070DA8` (`DungGenStart`'s real
logic) was investigated but not completed: its structure and two of its
three call targets are fully understood (see
`docs/decompilation-notes.md`'s "Dungeon-generation dependency map" entry
for the full trace, including a `struct GeneratedMapRoomRecord` field
finding parallel to the `unknown14` one above), but `sub_080714CC` remains
unresolved and is a real sub-investigation of its own (a five-plus-
register function that reads two argument registers holding leftover
caller state, not explicit arguments).

One confirmed result from this pass: `sub_0801097C` is
`CreateSpriteResetTask`, an existing `#ifdef NONMATCHING`-guarded
candidate in `src/task_constructors.c`. Re-verified directly against a
force-thumb disassembly of the raw ROM bytes -- every field write, call,
and the conditional immediate-run step are correct. The remaining gap
(the ROM caches the callback address in `r7` across an intervening stack
setup that clobbers `r1`; six tried C shapes never reproduce that specific
register choice) is now documented in place as a confirmed correct
deferral, the same class as `SpriteAffineWriteDispatch`'s register swap
and this session's newlib reentrant-wrapper gaps -- not a logic error.
`audit_pret_standards.py` still reports 0 errors/0 warnings/18 exceptions
and `make compare` still confirms the byte-identical ROM (this file change
is a comment only, inside the existing `#ifdef NONMATCHING` guard, with no
effect on the default build).

**[SUPERSEDED]** later the same session: the "confirmed correct deferral"
above was wrong. `sub_08080BDC` isn't an opaque callee -- it's agbcc's own
`bx r7` indirect-call veneer, meaning the "gap" was really an unrecognized
indirect call through the *same* callback pointer, not a register-
allocation limit. See "Sprite-reset task constructor recovered" in
`docs/decompilation-notes.md`: `CreateSpriteResetTask` is now a full,
byte-exact match.

The dungeon-generation work queue established this session
(`sub_0807017C` done, `sub_08070DA8`/`sub_08070F80`/`sub_08070620` sized
and partly surveyed, `sub_08070238` and several much larger functions
identified beyond that) is recorded in `docs/decompilation-notes.md` so a
future pass can pick up without re-deriving it.

## Verified snapshot: `CreateMapGenerationTask` decompiled, 2026-09-22

`sub_080714CC` -> `CreateMapGenerationTask` (`src/task_constructors.c`),
resolving the second (and last) prerequisite `sub_08070DA8` needed. Same
`_call_via_r9`-veneer shape as `CreateSpriteResetTask`; `sub_08080BE4`'s
`bx r9` is now also aliased in `asm/code/code_0800C0.s`.
`audit_pret_standards.py` reports 0 errors/0 warnings/18 exceptions;
`audit_provenance.py` reports 1839/1839; `make compare` confirms the
byte-identical ROM. Caught a new section-boundary bug distinct from every
earlier one this session: the target function was embedded in the *middle*
of an existing raw section (owning no `.section` of its own), and cutting
it without giving the following code (`sub_08071538`) a fresh,
address-named section let `SORT_BY_NAME` misplace the whole thing --
~94 scattered diffs across a huge address range, confirmed real (not
environmental) by stashing and rebuilding clean. See
`docs/decompilation-notes.md`'s "Map-generation task constructor
recovered" entry for the full mechanism and the fix.

`sub_08070DA8` itself (`DungGenStart`'s real logic) is now fully understood
logically -- both prerequisites resolved, every field access and branch
traced against direct force-thumb disassembly -- but resists an exact
register match on one swap (the incoming state pointer and a computed
field pointer land in `r4`/`r5` opposite from the ROM across every C shape
tried). This also corrects an error in this file's own earlier session
entry: the loop base is `state->`(`+0x644`), the same field toggled just
before it, not `+0x64C` as previously written here -- see
`docs/decompilation-notes.md` for the corrected trace and the
correctly-deferred candidate.

## Sound-player idle-wait constructor: three of four registers recovered, 2026-09-22

`CreateSoundPlayerIdleWait` (`src/nonmatching/sound_idle_wait.c`,
`sub_08005830`) had the same misidentified-veneer problem as the
`CreateSpriteResetTask`/`CreateSoundFadeTask`/`CreateMapGenerationTask`
entries above: it called `sub_08080BD4` as an opaque function instead of
recognizing it as agbcc's own `bx r5` indirect-call veneer for an
already-computed `callback` local. Declaring the `callback` local and
calling it directly fixed three of four register mismatches at once
(`index` in r6, the reused task pointer in r4, `callback` surviving in r5).
`sub_08080BD4` is now also aliased as `_call_via_r5` in
`asm/code/code_0800C0.s`, matching the `_call_via_r7`/`_call_via_sl`/
`_call_via_r9` precedent. One register-choice gap remains and was not
resolved -- the ROM overwrites the `player` pointer's own register with the
loaded `status` field (`ldr r0, [r0, #4]`) where every C shape tried here
loads into a fresh register (`ldr r1, [r0, #4]`) -- documented in place as
a correct deferral of the same class as `sub_08070DA8` above and
`SpriteAffineWriteDispatch`'s register swap, not pursued further per
PRET_STANDARDS.md's prohibition on register-allocator steering.
`audit_pret_standards.py` reports 0 errors/0 warnings/19 exceptions;
`audit_provenance.py` reports 1839/1839; `make compare` confirms the
byte-identical ROM (the candidate lives in `src/nonmatching/`, excluded
from the default build); `make check-modern` compiles it cleanly. See
`docs/decompilation-notes.md`'s "Sound-player idle-wait constructor" entry
for the full mechanism.

## Two more task constructors decompiled by auditing the veneer block, 2026-09-22

Rather than hunting one candidate at a time, checked every entry in
`asm/code/code_0800C0.s`'s `sub_08080BC0`..`sub_08080BE8` `_call_via_rN`
veneer block for live callers by raw `sub_` name. Six were already aliased
from earlier fixes this session; two of the rest (`sub_08080BD4`/r5,
`sub_08080BD8`/r6) had live callers -- both already-existing
`#ifdef NONMATCHING` candidates in `src/task_constructors.c`:
`CreateFieldCommandTask` (0x0800ECF8) and `CreateSpriteWaitTask`
(0x08010A2C). Same misidentified-veneer shape as every prior fix. Both are
now full, byte-exact matches; `#ifdef NONMATCHING` is gone from both,
`sub_08080BD8` is now also aliased as `_call_via_r6`, and both are declared
in `include/task_constructors.h`. Five external callers under the old
`sub_` names (across `asm/code/code_0080C0.s`, `asm/code/code_0100C0.s`,
`src/mapping.c`, `src/script_effect_native.c`) were updated to the
descriptive names.

One new lesson, distinct from the section-boundary bugs earlier this
session: the initial `callback`-local fix for each got 3 of 4 register
choices matching but not all four, because `callback = <name>;` was
assigned *before* the manager/owner pointer expression was computed --
agbcc pools PC-relative literals in source order, so the callback's address
literal landed in the ROM's manager-pointer pool slot and vice versa,
cascading into a different owner-value register and (for
`CreateFieldCommandTask`) a call to the wrong veneer entirely. Computing
`manager` first, then `callback`, matching the idiom already used by
`CreateSpriteResetTask`/`CreateMapGenerationTask`, fixed both completely.
`audit_pret_standards.py` reports 0 errors/0 warnings/19 exceptions;
`audit_provenance.py` reports 1839/1839; `make compare` confirms the
byte-identical ROM. See `docs/decompilation-notes.md`'s "Two more
`_call_via_rN`-shaped task constructors recovered" entry for the full
section-boundary and literal-pool-ordering mechanism.
