# PRET standards audit

Run `make pret-audit` to regenerate the detailed machine-readable reports at
`reports/code/pret-standards.json` and `reports/code/pret-standards.md`. Run
`python3 tools/audit_pret_standards.py --strict` when checking whether the
hard-error backlog has reached zero.

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
structural C rewrite, not another mechanical pass. Manual attempts at two of
the six forced-register files (`src/sound_fade_create.c`'s single r10 pin,
`src/sound_idle_wait.c`'s three pins) via reordering declarations did not
reproduce the original register allocation. Those probes rule out only the
tested declaration orders, not a clean-C reconstruction. `src/sprite_affine_matrix.c` carries the largest single concentration
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
7. Enable the audit's `--strict` mode in CI only after hard errors reach zero.

This order preserves the byte-identical build throughout the cleanup. A lower
C percentage is preferable to counting C that violates the project's matching
rules.

## New-decompile session, 2026-09-14/15

Reconstructed three previously-undecompiled functions (all logically verified
against the disassembly, none forced with a new register hint or inline
asm), landing two as `src/nonmatching/` and one still unresolved:

- `sub_08004DA8` -> `RuntimeGetLinkActivityState`
  (`src/nonmatching/runtime_link_status.c`). Also documents that the
  disassembly's `sub_08004DBC` is a false split with no real callers, and
  manually decodes the function's two unresolved `bl` targets (both
  `SioGetPlayerId`) from their raw ARMv4T BL encoding.
- `sub_08056290`/`sub_080562C8` -> `GameStateGetResourceCounter`/
  `GameStateAddResourceCounter` (`src/nonmatching/game_state_resource_counter.c`).
  A 999999-capped counter at the game root's +0x38BC.
- `sub_08078644` -> `SoundTrackReleaseChannels`
  (`src/nonmatching/sound_track_release_channels.c`). The previous claim of
  a single-instruction mismatch is incorrect for the current reference C.
  A fresh `--old` probe also changes the initial load order, channel-type
  calculation, indirect-call register, and final list-head store. The ROM
  calls its local `bx r3` trampoline at 0x08078634; the reference emits
  `_call_via_r1`. Resolving the first `tst` alone would not complete this match.

The resource-counter reference also needs more than a register swap: the
current increment function's compiled C caches the counter pointer/value
across the store, whereas the ROM reloads through the root. Verify every
instruction and relocation before calling either candidate a near-match.

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
  two spots differ from the ROM -- clearing a callback bit compiles
  in-place (`bic r1,r1,r3`) since the loaded value is dead after, where the
  ROM copies it to a fresh register first (the same destination-register
  tie documented elsewhere), and re-fetching the callback table's base a
  second time gets CSE'd against the already-loaded frame pointer no
  matter how the source repeats the expression, where the ROM redoes the
  two loads from scratch. Full detail in the file's header comment.
