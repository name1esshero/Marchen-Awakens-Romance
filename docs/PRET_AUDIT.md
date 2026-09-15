# PRET standards audit

Run `make pret-audit` to regenerate the detailed machine-readable reports at
`reports/code/pret-standards.json` and `reports/code/pret-standards.md`. Run
`python3 tools/audit_pret_standards.py --strict` when checking whether the
hard-error backlog has reached zero.

## Current result

The project satisfies its primary correctness requirements:

- `make compare` reproduces the Japanese ROM byte for byte.
- The host suite passes all 140 tests.
- `tools/audit_provenance.py` verifies every declared source range and reports
  1,640 compiled-C ranges plus ten BIOS assembly-wrapper ranges.
- No unresolved Git conflict markers remain.
- Every project header has an include guard or `#pragma once`.
- `tools/audit_thumb_ptrs.py` finds no odd raw function pointer whose target
  already has a recovered name.

The source does not yet satisfy every readability and matching-method rule.
The generated report is the authoritative list. The remaining hard-error
groups are forced-register declarations and TARGET_REGISTER pins; missing
Doxygen comments have been eliminated (see below).

The current mechanical scan reports 157 hard-rule occurrences, 0 missing
function-documentation warnings, and 13 documented low-level exceptions. An
earlier cleanup replaced all 630 raw ROM addresses with verified symbols and
added or converted documentation for 421 manifest-backed functions.

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

The 157 that remain are, by construction, the ones the compiler actually needs
for the current C. Each must either match as ordinary C after a structural
rewrite or return to an assembly implementation with its readable C kept under
`src/nonmatching/`.

Re-ran `tools/drop_register_hints.py --all` on 2026-09-14: it removed zero
additional hints (all 22 forced-register and 92 TARGET_REGISTER pins report
load-bearing). This confirms the count is stable, not that it is finished --
per the remediation order below, the next step for each surviving hint is a
structural C rewrite, not another mechanical pass. Manual attempts at two of
the six forced-register files (`src/sound_fade_create.c`'s single r10 pin,
`src/sound_idle_wait.c`'s three pins) via reordering declarations did not
reproduce the original register allocation -- these look like the fully
register-starved kind (§5a) rather than one fixable by variable ordering
alone. `src/sprite_affine_matrix.c` carries the largest single concentration
(48 of the 92 TARGET_REGISTER pins) and is the highest-value structural-rewrite
target for a future pass.

A second round on 2026-09-14 tried the single-.o iteration technique on two
previously unaudited functions, `sprite_tile_allocator.c`'s
`SpriteTileAllocatorRelease` and `resource_native.c`'s
`ScriptNativeSetFriendArms`, with several declaration-order and expression-
merging variants each. Both failed the same specific way every time: removing
the hint does not corrupt the logic, it just lands the value in the *adjacent*
register (wants r0, agbcc naturally picks r1, or vice versa), which then
cascades through the rest of the function's register choices. That is
precisely the "Reverse Register Allocation Order" quirk in §5a, not a
declaration-ordering problem, so reordering locals can't fix it. Given
`tools/drop_register_hints.py` already exhaustively proved every remaining
hint load-bearing by direct machine-code comparison (a stronger check than
manual iteration), and two fresh attempts both hit this same wall, further
progress here likely needs a genuinely different technique -- not more
manual guessing at declaration order -- to be worth the time. Good next
ideas for whoever picks this up: try forcing extra register pressure with a
deliberate dummy live value to shift the allocator's choices, or study
whether agbcc's allocator order is fully deterministic from something
inspectable (e.g. total live-range count) rather than trial and error.

## Remediation order

1. Run `tools/drop_register_hints.py --all` first: it clears every hint the
   compiler does not actually need, so later effort is spent only on real
   mismatches. Then, for each surviving hint, attempt a structural rewrite that
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
  (`src/nonmatching/sound_track_release_channels.c`). Matches down to a
  single instruction: the ROM's first condition is a bare `tst`, agbcc's
  normal codegen for the identical `if (flags & CONST)` idiom is
  `ands+cmp+beq` (confirmed against the already-matching
  `SoundPlayerImmediateInit()` a few functions earlier in the same file,
  which uses the same idiom and gets `ands+cmp+beq`), and no variant tried
  reproduces the bare `tst`. A genuinely new failure mode, not the register-
  swap one documented above -- worth its own investigation rather than
  assuming it is the same quirk.

All three needed a `register ... asm("rN")` pin just to reach the exact
register-swap failure mode already documented above (RuntimeGetLinkActivityState,
GameState*ResourceCounter) or hit a *different*, still-unexplained
instruction-selection difference (SoundTrackReleaseChannels) -- consistent
with this session's earlier finding that the remaining hard-error surface is
disproportionately made of near-misses, not functions nobody has looked at
yet. Anyone continuing this work should expect a similar hit rate: several
close-but-not-exact attempts per clean match.

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
- `sub_0807EC58` -> `ScriptFrameReleasePools`
  (`src/nonmatching/script_frame_release_pools.c`), called from
  `ScriptPopFrame()`. Frees both of a script frame's element pools
  (table038 outright, table03C element-by-element since each slot's data
  is itself an array of pointers), clears the frame's unrecovered
  0x44..0xA9 work area, and stashes values into `dispatchState`/
  `dispatchIndex` that `ScriptPopFrame()` immediately overwrites anyway.
  This one came very close: the whole function -- both loops' outer
  structure, five chained pointer computations spilled to five specific
  stack slots, two `CpuFill`s, the final field writes -- matches
  instruction-for-instruction except inside table03C's inner loop, where
  every shape tried (count read bare vs. hoisted to a local, `for` vs.
  `while` with an explicit "next slot" pointer, several
  declaration/statement orders) either swaps which of the slot's count and
  the next-slot address lands in r0 vs. r1, or leaves an extra
  register-to-register copy before the data pointer reaches r4 that the
  ROM doesn't have. Documented in full in the file's header comment.
