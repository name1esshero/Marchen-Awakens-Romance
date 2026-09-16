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

## Argument narrowing depends on the surrounding layout

Earlier probes of `CreateFieldEventTask` compared in-place casts, `s16`
parameters, and copying an argument to a local before casting. They produced
different scratch registers and sign/zero-extension instructions. Those were
observations about particular candidates, not rules that `s16` parameters
always zero-extend or that copy-then-cast is the only matching form.

The recovered function at 0x08061EA8 now matches with `s16` parameters and
ordinary `s32` locals assigned from them, without redundant casts. The key
structural correction was representing the payload **after** the 32-byte
`EngineTask` header instead of treating the entire task as the payload.
The compiler then retains the same base pointers and live values as the ROM.

One remaining byte difference exposed a real layout error: the final signed
halfword is at task offset **0xB2**, not 0xB4. Its payload offset is 0x92,
and the preceding gap is 12 bytes. Correcting that gap completed the match
with both compiler snapshots; the normal agbcc configuration is retained.
See `src/map_field.c` and `include/map_events.h`.

When narrowing differs, check the actual field offsets and pointer bases as
well as the local types. A register-allocation mismatch can conceal an
incorrect reconstruction of the structure.

## Addition: which operand becomes `rn`

`addsi3` declares operand 1 commutative (the `%` in its constraint string
`"%0,0,l,*0,*0,!k,!k"`). The allocator is therefore free to swap the two
operands, and **source order does not reliably decide which value lands in
which register.**

The three-register alternative `add %0, %1, %2` is only selected when neither
operand can be made to share the destination; the common alternatives are the
two-operand `add %0, %0, %2` forms.

In the tested `SpriteResourceFindGroup` candidates, the ROM computes a scaled
index first (into r0), loads a base second (r1), then adds `add r0, r1, r0` —
base as `rn`. Writing `offset + base` gets the evaluation order right and the
add operands backwards; writing `base + offset` fixes the add and swaps the
registers. Those expressions did not reach the required combination. This
does not prove that no ordinary C reconstruction can match; local types,
lifetimes, and control flow remain possible causes. Keep unsuccessful
reconstructions nonmatching rather than treating a pin as a completed match.

## Bitwise OR and AND: distinguish observations from allocator rules

`iorsi3` has the same commutative-operand ambiguity as `addsi3` (see
"Addition" above): for `*(u16 *)p |= CONST;` written as a bare literal with
no named locals, agbcc is free to put the loaded field value or the
constructed constant in either scratch register, and source order does not
decide which. Confirmed in `sub_08006ADC` (see
`src/nonmatching/iwram_set_flags_0810.c`): the ROM builds the constant into
one register, copies it into the OR's destination register, *then* loads the
field into the other register; agbcc's natural allocation for the identical
literal expression swaps which value lands in which of those two registers.

The asymmetry worth remembering: `*(u16 *)p &= CONST;` written the same
bare-literal way, in the *same function*, reproduced the ROM in that trial.
This observation does not establish a general difference in operand ties:
Thumb register AND and OR both update a destination operand. Inspect the
machine-description alternative and generated code for the actual case
before attributing a mismatch to a general allocator rule.

Naming either OR operand as a local (to try steering the allocator) does
change which register it lands in, but for a `switch` with one case per bit
this also makes the case bodies structurally identical apart from the
constant, and agbcc merges them into one shared tail with per-case jumps --
which changes the instruction count and order regardless of whether the
registers are now right. No plain expression found gets both requirements at
once for this shape.

## Copies that the tested expressions did not retain

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

## Compare a derived index without overwriting the lookup index

`NfpGetEntrySizeByName` (0x0807AD4C) formerly pinned its directory index to
`r4`. Removing the pin alone swapped several registers throughout the
function. Keeping the index unchanged and expressing the next-entry test
directly fixed the allocation with the existing compiler flags:

```c
/* index has already been checked for a failed lookup. */
if ((u32)index + 1 >= NfpGetEntryCount(handle))
```

This replaces `index++;` followed by the comparison. No later statement
needs the incremented index. The generated instructions and the linked ROM
are unchanged, while the local now retains its original meaning. When a
pin seems necessary, check whether one local is being reused for a derived
value that could instead be expressed at its use site.

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

## Where a pool address is materialised

A global's address and the load through it can be scheduled apart: the ROM
often keeps the pool address in a register across other work and dereferences
it later. Two things control this, and both are source-level.

**Hold the address, not the value.** Writing `u8 **root = (u8 **)0x0300401C;`
and dereferencing at the point of use gives `ldr r2, pool` early and
`ldr r0, [r2]` late. Reading the pointer straight into a local
(`u8 *base = RUNTIME_ROOT;`) emits both loads together, and referencing the
global inline emits both at the point of use.

**Assign the root after the first statement that uses the index.** The pool
load lands where the assignment sits relative to the surrounding arithmetic.
`RuntimeGetRecord17C` (0x08004FF0) needs it after the parameter's narrowing:

    s32 narrowed = index;      /* lsl #16 / asr #16 */
    u8 **root;
    u32 offset;
    root = (u8 **)0x0300401C;  /* ldr r2, pool lands here */
    offset = narrowed * 24 + 380;
    return *root + offset;

Declaring and initialising `root` first hoists the pool load above the
narrowing; computing `offset` before assigning `root` sinks it below the
shifts. Only the spelling above puts it between them, as the ROM has.

This does **not** resolve the record-getter family below, whose remaining
difference is register pressure rather than scheduling.

## Method notes

- **A single-file probe doesn't know the project's `.set` symbols.**
  `gIwramBase`, `gMapGenerationRootOffset`, and the rest of
  `asm/iwram_symbols.s` are absolute `.set` constants, not real addresses in
  a section. When both operands of a sum are that kind of symbol, `as` can
  fold `gIwramBase + gMapGenerationRootOffset` into one relocatable literal
  pool word at assemble time -- collapsing what the ROM writes as two
  separate loads and an `adds` into a single `ldr`. A probe or single-.o test
  that leaves these `extern` and undefined won't see that folding (the
  symbols simply stay unresolved, so `as` can't fold them), and will report a
  false match for C that turns out to change the real build's byte count.
  Confirmed on `GameStateSelectDeckPointer` (0x0800696C): a direct
  `gIwramBase + (u32)gMapGenerationRootOffset` inline matched in isolation
  but changed the linked size in the real build; splitting the sum across
  two locals first (`u8 *iwram = gIwramBase; u32 offset = (u32)...; return
  *(u8 **)(iwram + offset) + ...;`, the same shape `GAME_STATE_BASE` in
  runtime_accessors.c already uses) reproduced the ROM's unfolded form in
  both places. When a function touches one of these symbols, confirm with a
  real `make && make compare` before trusting an isolated probe.
- Iterate against a single translation unit, not the ROM. `cpp` + `agbcc` +
  `as` on one file takes about a second; a full `make && make compare` takes a
  minute and a half. See `docs/PRET_STANDARDS.md` §8a.
- Compare **machine code**, not the compiler's assembly text. Inline asm emits
  `.code 16` directives that vanish when you remove it, so identical
  instructions can read as a difference. `tools/drop_register_hints.py`
  assembles and diffs `objdump` output for this reason.
- `make compare` stays the final authority. Everything above is a way to spend
  fewer of those minute-and-a-half runs.
- **After adding a function, run the host tests, not just `make compare`.** The
  tests compile individual `.c` files on their own, so a new reference to a
  symbol the ROM link resolves -- `gSecondaryRuntime`, `gIwramBase`, anything
  `.set` in `asm/game_table_handlers.s` -- fails at link with "undefined
  reference" even though the ROM is byte-exact. Fix it by defining the symbol in
  that test's scaffold, never by changing the source back. This has now bitten
  five tests: test_item, test_map_field, test_map_native, test_script_native and
  test_input_random.
