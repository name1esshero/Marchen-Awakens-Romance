# C shiftability audit

Run `make shiftability-audit` to regenerate the detailed JSON and Markdown
reports under `reports/code/`. Those generated reports are ignored by Git; this
page records the current result and explains what the score means.

## Current result

As of 2026-09-20:

| Scope | Relocatable ROM references | No raw software addresses |
| --- | ---: | ---: |
| Matching build | 111 / 131 (84.73%) | 101 / 131 (77.10%) |
| English-only C | 3 / 3 (100.00%) | 3 / 3 (100.00%) |
| Nonmatching candidates | 8 / 9 (88.89%) | 8 / 9 (88.89%) |
| All tracked C | 122 / 143 (85.31%) | 112 / 143 (78.32%) |

The matching manifest gives a conservative lower bound of 1,336 / 1,813
ranges (73.69%) and 190,931 / 229,639 bytes (83.14%) with no detected software
address dependency. A single finding currently marks every manifest range from
the containing source file, so this deliberately understates progress in large
files.

The largest concentration is `src/game_tables.c`: 455 of the 491 ROM findings
across all tracked C are named handlers or data objects whose linker symbols
are still absolute `.set` aliases. They are more readable than raw pointer
literals, but they will not follow code or data when its address moves. This is
the main shiftability backlog. The direct-literal backlog is much smaller:
matching C contains one raw ROM address, 20 raw IWRAM addresses, and six uses
of header macros that expand to fixed IWRAM addresses.

## What is counted

A C file passes the ROM-reference check when it has no raw cartridge address
and does not refer to a symbol implemented as an absolute ROM `.set` or
`.thumb_set` alias. The stricter software-reference check also rejects raw
EWRAM/IWRAM literals and header aliases that expand to them.

The following are intentionally not failures:

- GBA hardware registers and the fixed VRAM, palette, OAM, BIOS, and save-memory
  regions. Their physical addresses are part of the platform. Named `#define`
  constants and typed volatile access macros are the normal PRET interface for
  these addresses.
- Ordinary typed pointer arithmetic and `u8 *` byte-offset arithmetic.
- `#define` aliases that resolve to linker symbols or platform constants. A
  macro that merely hides a fixed software-object address is still reported as
  transitional address debt.
- Named EWRAM/IWRAM linker symbols. Exact RAM placement belongs in the linker;
  C should refer to the object by name.
- `AT()` section annotations. The matching linker must reproduce the original
  ROM layout, so fixed placement is expected there. A future expanded or modern
  linker can relax placement without changing ordinary symbolic references in
  C.

This is a syntactic audit, not proof that a file is semantically position
independent. It cannot detect an address assembled from several nonliteral
expressions, hidden placement assumptions, incorrect types, or compiler
steering unrelated to addresses. Review and byte comparison remain required.

## How to reduce the backlog

Replace a fixed ROM alias only when the underlying object or function has a
real relocatable definition. Merely changing a raw address into a named `.set`
alias improves readability but does not make it shiftable. The safe sequence
is to recover the real C/data definition, redirect every reference to that
symbol, remove the absolute alias, and run `make compare`.

For RAM, define named objects in the correct EWRAM, IWRAM, BSS, or COMMON
section and let the matching linker assign their historical addresses. Keep
hardware access through the volatile register macros in `include/gba/`.
