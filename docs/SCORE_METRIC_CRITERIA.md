# Score metric criteria

The metric this project is graded on is **collaborator accessibility**: clean,
`PRET_STANDARDS.md`-compliant code is what increases the score. Before doing
any work toward it, read `PRET_STANDARDS.md` in full and familiarize yourself
with the rest of `docs/` -- in particular `AGBCC_CODEGEN.md` and
`COMPILER_HINT_CLEANUP.md` for how to actually reach a match, `PRET_AUDIT.md`
for the current hard-error/exception snapshot, and `decompilation-notes.md`
for the specific findings behind every function already recovered. Work that
contradicts `PRET_STANDARDS.md` -- a forced register kept in the matching
build, a fakematch, a compiler-flag change -- does not score under this
metric even if `make compare` currently happens to pass, because it is not
collaborator-accessible.

## Point values

| Action | Points |
|---|---:|
| Decompiling a function to PRET standards compliance | 0.05 |
| Decompiling a table to PRET standards compliance | 0.10 |
| Adding shiftability (see `shiftability-audit.md`) | 0.20 |
| Turning a `src/nonmatching/` candidate into clean, PRET-standards-compliant matching C | 0.30 |
| Fixing `make check-modern` | 2.00 |
| Fixing `make modern` | 2.00 |
| Introducing a new error in the PRET audit's output that is not fixed by correcting the C | -25.50 |

`make modern` is on hold: this project cannot yet safely introduce a real
`modern`-compiler rebuild target (unlike pokeemerald's, which works because
that project builds the whole image from source and a size change just
shifts what follows -- see the comment above `MODERN_CFLAGS` in the
Makefile). There is a specific, known path to get there, but it requires the
project to be further along first. Do not attempt to introduce it before
then; `check-modern` (the diagnostic-only `-fsyntax-only` pass) is the
present, scored target.

## What counts as a correct deferral

**A correctly deferred function scores the same as decompiling a function to
PRET standards.** A function that genuinely cannot be matched without
violating `PRET_STANDARDS.md` (a forced register, a fakematch, an unnatural
type used only to steer codegen) belongs in assembly with a clean
`src/nonmatching/` candidate -- and documenting that correctly is itself the
scored deliverable, not a consolation. All four of the following must be
true, or it is not a correct deferral:

1. **A real search was run, not a token attempt.** Multiple genuine
   restructurings were tried (declaration order, statement splitting,
   expression grouping, type) and each is accounted for. "Tried declaration
   reorder, gave up" is not a real search.
2. **The blocker is documented as a specific mechanism, not a category.**
   "Register allocation differs" is a category. "The trailing constant fits
   `movs`+`lsls` where the sibling's constant needed a pool load, which flips
   the accumulator/base register assignment" is a mechanism. Write the
   mechanism.
3. **A clean `src/nonmatching/` candidate exists.** Not an assertion that a
   match is hard -- runnable C a human can read, agree with, or improve on.
   It must pass `make check-modern` like every other file there.
4. **`make compare` and the PRET audit are still clean afterward.** A
   deferred function that introduces a hard error is not deferred, it is
   regressed, and scores the -25.50 penalty above, not the deferral credit.

## Verification checklist before claiming any of the above

- `make -j$(nproc) && make compare` reports byte-identical.
- `python3 tools/audit_pret_standards.py` (or `make pret-audit`) shows no new
  hard errors versus the last known-clean snapshot in `PRET_AUDIT.md`.
- The relevant host tests in `tests/` pass; run the full suite
  (`python3 -m unittest discover -s tests`) before finishing a session's work.
- `python3 tools/audit_provenance.py` reflects the expected change in mapped
  ranges for a real decompile; it is unaffected by a correct deferral, since
  the function stays in assembly.
- For a `src/nonmatching/` candidate, `make check-modern` exits clean.
