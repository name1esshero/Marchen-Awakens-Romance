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
  1,639 compiled-C ranges plus ten BIOS assembly-wrapper ranges.
- No unresolved Git conflict markers remain.
- Every project header has an include guard or `#pragma once`.
- `tools/audit_thumb_ptrs.py` finds no odd raw function pointer whose target
  already has a recovered name.

The source does not yet satisfy every readability and matching-method rule.
The generated report is the authoritative list. The largest hard-error groups
are forced-register declarations, inline assembly or scheduling fences, and
raw ROM addresses used by logic or tables. The largest warning groups are
missing Doxygen comments and names that are not yet PascalCase.

The current mechanical scan reports 351 hard-rule occurrences, 759 missing
function-documentation warnings, and 12 documented low-level exceptions. This
cleanup removed seven forced-register declarations whose generated object code
was unchanged, replaced all 630 raw ROM addresses with verified symbols, and
added or converted documentation for 200 manifest-backed functions. The
remaining compiler hints cannot be removed mechanically: every one must either
match as ordinary C after a structural rewrite or return to an assembly
implementation while its readable C stays under `src/nonmatching/`.

## Remediation order

1. Move functions that require forced registers or inline scheduling fences
   back behind their original assembly implementation. Keep readable C under
   `src/nonmatching/` with the exact mismatch documented.
2. Move the ten BIOS instruction wrappers from naked inline C into a small
   named assembly wrapper file, preserving their C prototypes.
3. Replace raw ROM function and string addresses with verified linker symbols.
   Confirm odd THUMB pointers before subtracting their low bit.
4. Replace proven flags, limits, strides, and structure offsets with named
   constants. Do not invent semantic names without call-site evidence.
5. Rename placeholder and non-PascalCase functions as their behavior becomes
   known, then add Doxygen comments to public functions and recovered structs.
6. Enable the audit's `--strict` mode in CI only after hard errors reach zero.

This order preserves the byte-identical build throughout the cleanup. A lower
C percentage is preferable to counting C that violates the project's matching
rules.
