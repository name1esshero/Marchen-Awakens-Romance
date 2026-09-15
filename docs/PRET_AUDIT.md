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
2. Move the ten BIOS instruction wrappers from naked inline C into a small
   named assembly wrapper file, preserving their C prototypes.
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
