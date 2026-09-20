# PRET standards audit

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

## Verified snapshot: pointer/integer authenticity audit, 2026-09-20

- `make compare` reproduces the Japanese ROM byte for byte.
- `make english` succeeds and all 197 host tests pass.
- The mechanical PRET audit reports **99 errors / 0 warnings / 17 documented
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

Fifteen findings were removed through natural types and expressions. Three
previous matches were rejected after exact comparison showed that their
pointer/integer spelling only selected registers or alias behavior:
`GameStateAddResourceCounter`, `SpriteAffineAllocate`, and
`ScriptResourceSet`. Their exact implementations are restored to named
assembly and their clean candidates remain in `src/nonmatching/` with the
specific code-generation differences documented.

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
allocation. `CreateSoundFadeTask` has since moved to exact assembly with its
clean candidate retained under `src/nonmatching/`; the sound-idle routines
remain matching C with unresolved pins. Those probes rule out only the tested
declaration orders, not a clean-C reconstruction.
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

The resource-counter getter still needs more than a superficial register swap:
the matching union alias model for the increment function does not reproduce
the getter's r4/r3 root-and-offset allocation. Verify every instruction and
relocation before promoting the remaining candidate.

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

## Rejected match: consumable inventory scan (2026-09-20)

The apparent C match for `CountConsumableInventoryCopies` at 0x08057078 used
a pointer/`u32` union solely to reproduce the ROM's low-register allocation.
The disassembly instead shows normal pointer formation and pointer increments,
so the union was rejected as compiler steering. The natural C candidate is
documented in `src/nonmatching/consumable_inventory.c`, and the exact body
remains in assembly under the PRET fallback rule. Its assembly callers still
use the descriptive relocatable symbol.

The associated trace remains valid: game-state offsets `0x31D0` and `0x33D0`
are the 256-slot consumable inventory and its snapshot, so four older
offset-based or map-named helpers retain their inventory-specific public names.
The native command registered as `ItemInit` is now named
`ScriptNativeClearConsumableInventory`, replacing its earlier speculative map
name without changing its still-audited implementation.
