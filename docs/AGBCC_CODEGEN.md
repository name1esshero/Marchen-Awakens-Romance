# What agbcc does, and why your C didn't match

Matching a function is a search over C spellings that mean the same thing but
compile differently. Guessing is slow. This is what the compiler actually does,
read out of its own source and confirmed by experiment, so the search can start
from a prediction instead of from scratch.

The compiler is **GNU C 2.9-arm-000512 (thumb-elf)**, driven as the Makefile
drives it: `-mthumb-interwork -O2 -fhex-asm`. Two files
(`sound_m4a.c`, `sound_cgb_update.c`) use the older `old_agbcc` snapshot.

Source: <https://github.com/pret/agbcc>. The Thumb machine description is
`gcc/thumb.md`; the rules below cite the pattern they come from.

Check any claim here yourself:

    python3 tools/agbcc_probe.py --types -f myFunc snippet.c
    python3 tools/agbcc_probe.py --types --diff shapeA.c shapeB.c

## Signed byte loads: `ldrsb` or `ldrb`+`lsl`+`asr`

This one costs the most time, because it looks like evidence that a different
compiler built that part of the ROM. It isn't. Both forms come out of the same
pattern, and **which one you get depends on register allocation, not on how you
write the load.**

From `*extendqisi2_insn` in `gcc/thumb.md`, for a signed byte read through a
memory reference:

| address form | condition | emitted |
|---|---|---|
| `[reg, reg]` | — | `ldrsb %0, [%1, %2]` |
| `[reg, const]` | destination register **is** the base register | `ldrb %0,[%1,%2]` + `lsl #24` + `asr #24` |
| `[reg, const]` | destination differs from base | `mov %0, %2` + `ldrsb %0, [%1, %0]` |
| `[reg]` | destination **is** the base | `ldrb %0,[%0,#0]` + `lsl #24` + `asr #24` |

So the three-instruction shift form appears **exactly when the compiler reuses
the base register as the destination**. Confirmed:

    s32 f(u8 *p, s32 i) { return *(s8 *)(p + i); }
    -> add r0,r0,r1 / ldrb r0,[r0,#0] / lsl r0,r0,#24 / asr r0,r0,#24

`p` is in r0 and the return value is in r0, so base and destination coincide.
Keep the base alive past the load and the same source line changes form:

    s32 f(u8 *p, s32 i) { s32 v = *(s8 *)(p + i); use(v, p); return v; }
    -> mov r4,#0 / ldrsb r4,[r1,r4]

**What to do:** if the ROM has `ldrb`+`lsl #24`+`asr #24` and you are getting
`ldrsb`, arrange for the destination to be the base register — typically by
letting the pointer die at the load. If the ROM has `ldrsb` and you are getting
the shifts, keep the base register live afterwards, or route the result through
a different variable. Do not reach for a forced register, and do not conclude
the function needs a different compiler.

If the value is already in a register rather than in memory, `extendqisi2`
always expands to `lsl #24` / `asr #24`; there is no register-to-register
`ldrsb` to reach.

## Signed halfword loads

`extendhisi2` is shaped the same way, with shifts of 16 instead of 24, and the
same "already in a register means shifts" rule. In practice `ldrsh` appears more
readily than `ldrsb`:

    s32 f(u8 *p, s32 i) { return *(s16 *)(p + i); }
    -> add r0,r0,r1 / mov r1,#0 / ldrsh r0,[r0,r1]

Note the `mov rN,#0` that materialises a zero index: Thumb's signed loads only
have a register-offset form, so a zero offset still costs a register.

## Narrowing a 16-bit argument: three shapes, not one

A function whose parameters are logically 16-bit narrows them in the prologue,
and the three ways of writing that produce three different prologues. Picking
the wrong one costs a scratch register or flips sign-extension to zero-extension.

| source | emitted |
|---|---|
| `s32` parameter cast in place: `first = (s16)first;` | `lsl r0, r6, #16` / `asr r6, r0, #16` — correct sign, but via a scratch register |
| `s16` parameter | `lsl r5, r5, #16` / `lsr r5, r5, #16` — in place, but **zero**-extends |
| `s32` parameter copied to a local, then cast: `a = first; a = (s16)a;` | `lsl r5, r5, #16` / `asr r5, r5, #16` — in place and sign-correct |

The `lsr` in the middle row is not a bug: where every use of the value stores
into a 16-bit field, the upper bits are dead and agbcc may pick either shift.
If the ROM shows `asr`, the value's upper bits mattered somewhere, or the source
used the third shape.

Matching `CreateFieldEventTask` (0x08061EA8) came down to exactly this: the
copy-then-cast shape reproduces its prologue instruction for instruction, where
both other spellings do not. See `src/nonmatching/map_events.c`.

## Addition: which operand becomes `rn`

`addsi3` declares operand 1 commutative (the `%` in its constraint string
`"%0,0,l,*0,*0,!k,!k"`). The allocator is therefore free to swap the two
operands, and **source order does not reliably decide which value lands in
which register.**

The three-register alternative `add %0, %1, %2` is only selected when neither
operand can be made to share the destination; the common alternatives are the
two-operand `add %0, %0, %2` forms.

This is a real limit. In `SpriteResourceFindGroup` the ROM computes a scaled
index first (into r0), loads a base second (r1), then adds `add r0, r1, r0` —
base as `rn`. Writing `offset + base` gets the evaluation order right and the
add operands backwards; writing `base + offset` fixes the add and swaps the
registers. Both halves are coupled through the allocator and no plain
expression reaches that combination. Some functions genuinely need the pin, or
belong in `src/nonmatching/`.

## Copies that the allocator will not make

Where the ROM keeps a value in one register and a modified copy in another,
agbcc will coalesce the two if the original is dead after the branch. In
`SpriteFixed8Multiply` the ROM has:

    muls r0, r1
    adds r1, r0, #0     @ the copy
    cmp  r0, #0
    bge  .L
    adds r1, #255

`product` is not read after the compare, so every spelling tried — two locals
in either declaration order, `if`/`else` assignment, a ternary, reusing a
parameter, splitting the final shifts — put both in r0. A live range that
genuinely extends past the modification is what forces the copy; if the
original source had one, it is not visible in the result.

## Alignment padding is plain C, not inline assembly

A trailing zero halfword belongs in the project's existing idiom:

    AT("00005848") const u8 CreateSoundPlayerIdleWaitTail[2] = {0};

not in `asm(".section .rom.ADDR,\"ax\",%progbits\n.space 2, 0\n");`. Without it
the assembler pads a code section with `0x46C0` (a Thumb `nop`) and the two
bytes mismatch.

Write each tail **immediately after its own function**. Collecting them in a
block at the end of the file produces `Error: changed section attributes for
.rom.ADDR`, because the assembler reopens a section it had emitted as code and
finds read-only data flags instead.

## Open: where a pool address is materialised

A global's address and the load through it can be scheduled apart, and source
order does not appear to control the split. `RuntimeActorGetField234`
(0x0800943C) keeps the pool address in r2 across the multiply and dereferences
it only afterwards:

    ldr  r2, [pc, #24]      @ address of the gSecondaryRuntime slot
    movs r1, #209 / lsls r1, r1, #3 / muls r0, r1
    ldr  r1, [r2, #0]       @ the load, after the multiply

Shapes tried all give either both loads early or both late: reading the pointer
into a local before the multiply, a single combined expression, pointer rather
than integer accumulation, and dereferencing the pointer expression directly.
Everything else in that function now matches, including the `ldrb`+shift tail,
which is the part previously believed impossible. If you find the shape that
splits the two loads, record it here.

## The game-state record getters: a family worth cracking

`sub_0800D648`, `sub_080106E8`, `sub_08011464`, `sub_0800F994` and others all
have one shape: take the map-generation root pointer, add a constant, add a
scaled index, return it. Solving one solves a macro family, so it is worth more
effort than a lone function.

Two thirds of it is already understood:

- **Derive the constant, do not load it.** The ROM writes
  `subs r2, #160` / `adds r2, #96` against the already-loaded root offset rather
  than loading a third pool word. Write it that way and agbcc does the same:
  `GAME_STATE_BASE + ((u32)gMapGenerationRootOffset - 160) + index * 8`
  emits `sub r0, r0, #0xa0`. This also matters because a third live pool
  constant is what forces the register pressure below.
- **The matched sibling is the template.** `GameStateGetRecord610` (0x0800D628)
  is the same family and matches from the plain one-line expression
  `return GAME_STATE_BASE + 0x610 + index * 44;`. Its constant is buildable with
  `movs`+`lsls`, so it never needs the derivation trick.

What is left is a register swap, at identical instruction count and size. The
ROM keeps the index in r0 for the whole function and copies the result out of r1
at the end:

    lsls r0, r0, #3 ... adds r1, r1, r0 / adds r0, r1, #0

agbcc instead copies the parameter out of r0 first and builds the result in r0:

    add r1, r0, #0 ... ldr r0, [r2] / add r0, r0, r1

The return-value pseudo takes r0, so the parameter is evicted. Shapes tried:
one-line expression, index accumulated first, accumulating into the base
pointer, reusing one variable for both the root address and the loaded value,
`s32` and `u32` parameters, `<<` versus `*`. All produce the leading copy.

## Method notes

- Iterate against a single translation unit, not the ROM. `cpp` + `agbcc` +
  `as` on one file takes about a second; a full `make && make compare` takes a
  minute and a half. See `docs/PRET_STANDARDS.md` §8a.
- Compare **machine code**, not the compiler's assembly text. Inline asm emits
  `.code 16` directives that vanish when you remove it, so identical
  instructions can read as a difference. `tools/drop_register_hints.py`
  assembles and diffs `objdump` output for this reason.
- `make compare` stays the final authority. Everything above is a way to spend
  fewer of those minute-and-a-half runs.
