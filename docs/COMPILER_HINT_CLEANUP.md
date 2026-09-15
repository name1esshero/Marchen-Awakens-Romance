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
did come out; only the pin is load-bearing. **Moved to real assembly** (see
below) rather than kept as pinned C, per the Golden Rule.

`SpriteResourceFindGroup` (`sprite_engine_state.c`) needs the scaled index
computed *first* (into r0), the base loaded second (r1), and then
`add r0, r1, r0` -- base as the left operand. agbcc ties the two together: the
operand written first in the C addition gets both the lower register and the
`rn` slot. Writing `offset + base` gets the evaluation order right and the add
backwards; writing `base + offset` fixes the add and reverses the registers.
The natural `descriptor->level0[low].name` form is otherwise instruction-exact.
Still a candidate for the `src/nonmatching/` route rather than more search.

## SpriteFixed8Multiply moved to real assembly

A forced-register pin is exactly the "compiler hack" the Golden Rule forbids
keeping in `src/`; `tools/drop_register_hints.py --all` independently confirms
every surviving pin (this one included) is load-bearing, so the only
Golden-Rule-compliant options are a real structural rewrite (tried and failed,
above) or moving the function behind honest assembly, per PRET_STANDARDS.md
§8/§8a and the precedent already set for the BIOS SWI wrappers
(`8d7874c8`).

The real instructions were extracted directly from the previously-matching
build (`objdump` of `build/mar.elf`, cross-checked byte-for-byte against
`baserom.gba`) rather than hand-derived, since the pinned C already compiled
to the right bytes -- only its *location* (hacked C vs. honest asm) needed to
change. Now at `asm/code/code_0780C0.s`, with the readable, non-matching C
kept at `src/nonmatching/sprite_fixed8_multiply.c` for reference.

One trap along the way: the function's single internal branch
(`bge`, skipping the `+= 255` rounding step) must be a **plain local label**
(`1:` / `1f`), not `.global`. A `.global` branch target defers its offset to
a linker relocation instead of resolving it at assemble time; the relocated
result still assembled to a 2-byte instruction, but with the wrong offset
byte (`da fe` instead of the ROM's `da 00`) until final link -- a mismatch
that a same-file objdump can hide if you don't check post-link bytes.

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

## State

Recorded at the time of writing; regenerate rather than trusting these numbers.

- Hard-rule findings: 351 → 230 in a first pass that compared assembly text,
  then 230 → 162 once the comparison moved to machine code, then → 157 as the
  sound files' padding halfwords stopped being inline asm, then → 156 moving
  the BIOS SWI wrappers to real assembly (`8d7874c8`), then → 152 (two solo
  structural rewrites: `FontCharacterToGlyph`, `NfpGetEntrySizeByName`), then
  → 149 moving `SpriteFixed8Multiply` to real assembly, the current count.
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
  `sound_fade_create.c`, `sound_player_select.c`.
- `make compare` byte-exact and all host tests passing throughout.

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
