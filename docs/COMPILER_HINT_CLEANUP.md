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
is the worked example: both of its fences are removable together, and only its
`register s32 rounded asm("r1")` pin is genuinely load-bearing.

## Worked example: SpriteFixed8Multiply (0x0807D8F8)

The ROM keeps `product` in r0 and a separate `rounded` in r1:

    muls r0, r1
    adds r1, r0, #0
    cmp  r0, #0
    bge  .L
    adds r1, #255
    .L:
    lsls r0, r1, #8

Plain C coalesces the two into r0, because `product` is dead after the compare.
Every shape tried produced the coalesced form: separate locals in either
declaration order, `if`/`else` assignment, a ternary, reusing the `left`
parameter, and splitting the final shifts. The one pin is currently the only
way to reproduce the copy, so it stays; both fences went.

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

Two functions were tried properly and kept their pin, because agbcc will not
produce the ROM's register choice from any ordinary C shape:

`SpriteFixed8Multiply` (0x0807D8F8) needs `product` in r0 and a *copy* in r1,
but `product` is dead after the compare, so agbcc coalesces them. Tried:
separate locals in both declaration orders, `if`/`else` assignment, a ternary,
reusing the `left` parameter, and splitting the final shifts. Its two fences
did come out; only the pin is load-bearing.

`SpriteResourceFindGroup` (`sprite_engine_state.c`) needs the scaled index
computed *first* (into r0), the base loaded second (r1), and then
`add r0, r1, r0` -- base as the left operand. agbcc ties the two together: the
operand written first in the C addition gets both the lower register and the
`rn` slot. Writing `offset + base` gets the evaluation order right and the add
backwards; writing `base + offset` fixes the add and reverses the registers.
The natural `descriptor->level0[low].name` form is otherwise instruction-exact.

Both are candidates for the `src/nonmatching/` route rather than more search.

## State

Recorded at the time of writing; regenerate rather than trusting these numbers.

- Hard-rule findings: 351 → 230 in a first pass that compared assembly text,
  then 230 → 169 once the comparison moved to machine code. 182 hints removed
  in total; 169 survive and are, by construction, load-bearing for the current C.
- Remaining concentrations: `sprite_affine_matrix.c` (42), `sprite_transform.c`
  (17), `sprite_interpolation.c` (5), `sram.c` (4), `mapping.c` (4),
  `nfp.c` (3), `sound_m4a.c` (4), `sound_idle_wait.c` (3).
- `make compare` byte-exact and all 140 host tests passing throughout.

## Watch out for

Replacing a raw ROM address with a linker symbol (audit remediation step 3)
breaks any host test that compiles a single `.c` in isolation, because the
symbol is resolved by `asm/game_table_handlers.s` in the ROM link only. Four
tests broke this way. Fix the test by defining the aliased literal, not by
reverting the source. Tests that string-substituted the old raw address silently
become no-ops and must be updated too.

## Next

For each surviving hint, attempt a structural rewrite that matches as ordinary
C. Only when that fails should the function move back behind its assembly, with
readable C kept under `src/nonmatching/` and the mismatch documented.
