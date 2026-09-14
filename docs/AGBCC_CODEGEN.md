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
