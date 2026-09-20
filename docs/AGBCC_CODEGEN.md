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

When one operand is genuinely the value being updated, make that relationship
visible rather than constructing a third result pointer. The old_agbcc newlib
`_Bfree` implementation demonstrates the difference:

```c
key = node->key;
buckets = table->buckets;
key <<= 2;
key += (u32)buckets;
```

This keeps `key` as the add destination and emits `add r0, r0, r1`, matching
the ROM. Advancing `buckets` by `key` emits the commuted
`add r1, r1, r0`. This does not override the allocator; it describes which
source value continues to exist after the addition.

## Alias modeling can explain apparently missing reloads

agbcc applies C type-based alias assumptions. If two writes are expressed
through unrelated pointer types, it may forward a previously stored value or
reuse a computed address because the intervening write is not allowed to
change it. `CreateRuntimeTask69DB4` (0x08069DB4) exposed this at two child-task
fields around a write to its parent work block.

The matching reconstruction uses one partial union type with parent and child
views. The parent-state store can therefore alias the child's stored-parent
field, so the compiler reloads that field and materializes the child's
`0x11CC` and `0x11D0` offsets independently, exactly as the ROM does. This is
preferable to `volatile`, which would claim asynchronous mutation and can add
too many accesses. It is also preferable to an empty compiler barrier.

Only introduce a shared union or common structure when caller and layout
analysis supports that memory model. A fabricated alias solely to obtain an
instruction sequence is not evidence of the original program and must remain
nonmatching.

## Bitwise OR and AND: distinguish observations from allocator rules

`iorsi3` has the same commutative-operand ambiguity as `addsi3` (see
"Addition" above): for `*(u16 *)p |= CONST;` written through a bare scalar
pointer, agbcc is free to put the loaded field value or the constructed
constant in either scratch register, and source order does not decide which.
`IwramSetFlags0810` at 0x08006ADC initially exposed this problem: the scalar
pointer form put the loaded word in r1 and copied the bit from r3 to r2, while
the ROM loads the word into r2 and copies the bit from r3 to r1.

That mismatch was a missing type, not an unsteerable allocator tie. Modeling
IWRAM +0x810 as a one-field `struct IwramFlags0810` and applying the same
compound assignment through its `value` member emits the ROM sequence exactly
in all four switch arms. Member access changes the RTL lifetime enough for the
ordinary allocator to choose the original registers, without named temporary
values, a forced register, or a volatile qualifier. When a scalar access has a
stubborn commutative-operand swap, test the recovered record type before
classifying it as a compiler limitation.

The asymmetry worth remembering: `*(u16 *)p &= CONST;` written the same
bare-literal way, in the *same function*, reproduced the ROM in that trial.
This observation does not establish a general difference in operand ties:
Thumb register AND and OR both update a destination operand. Inspect the
machine-description alternative and generated code for the actual case
before attributing a mismatch to a general allocator rule.

Naming either OR operand as a local changed the allocation but also caused
undesired switch-tail merging in the earlier trials. The record-member form is
the useful solution because it preserves the original control-flow shape.

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

## Signed-halfword narrowing can look like fixed-point iteration

`CreateEncounterResetTask` (0x0806EFCC) clears flag indices zero through
three. Its ROM loop keeps `0x10000`, `0x20000`, and so on in `r4`, copies the
value to `r0`, advances `r4` by `0x10000`, and arithmetic-shifts `r0` right by
16 before the comparison. This initially looks like a fixed-point counter,
but the called routine receives the ordinary indices 0, 1, 2, and 3.

The source operation agbcc is representing is a signed-halfword narrowing on
the loop update:

```c
for (i = 0; i <= LAST_FLAG; i = (s16)(i + 1))
    RuntimeSetFlagC0(i, FALSE);
```

Writing `s16 i` is not equivalent for code generation: agbcc repeatedly
extends the low halfword and emits a different loop. Writing a plain `s32 i`
also loses the narrowing and emits an ordinary increment. With the explicit
assignment conversion, agbcc strength-reduces the recurrence into the ROM's
high-halfword representation. The odd-looking machine code is therefore a
type-width clue, not evidence of hand-written assembly or a fixed-point API.

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

## Initialize an index before loading a moving pointer

`ScriptFrameReleasePools` (0x0807EC58) shows that declaration order can be
the final difference in a nested loop. The ROM clears the element index
before loading the pointer array. This ordinary C shape reproduces that order:

    u32 index = 0;
    void **element = entry->data;
    for (; index < entry->count; index++, element++) {
        if (*element)
            HeapFree(heap, *element);
    }

Declaring and initializing `element` first reverses the two instructions and
causes a four-byte mismatch. No register pinning is needed; the source-level
initialization and moving-pointer loop are sufficient.

## Some loops really use a 16.16 induction counter

The repeated `mov rN,#128; lsl rN,#9` sequence is not always compiler noise.
`CountPmbDeckEntryCopies` (0x080569B0) keeps its eight-pass loop counter in
16.16 form, copies the current fixed value before adding `1 << 16`, advances a
separate table pointer, and compares the copied value after shifting it right.
Writing that recovered algorithm directly produces the ROM bytes without a
register declaration:

```c
fixedIndex = 1 << 16;
do {
    /* process the current table entry */
    index = fixedIndex;
    fixedIndex += 1 << 16;
    entry++;
} while ((index >> 16) <= (s32)ARRAY_COUNT(table) - 1);
```

A conventional integer `for` loop is equivalent for this fixed trip count,
but agbcc compiles it as a countdown and cannot match this ROM shape. Preserve
the fixed-point form when the instructions and neighboring functions show it;
do not introduce it solely to steer registers.

## Where a pool address is materialised

A global's address and the load through it can be scheduled apart: the ROM
often keeps the pool address in a register across other work and dereferences
it later. Two things control this, and both are source-level.

**Hold the address, not the value.** Writing `u8 **root = &gRuntimeState;`
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
    root = &gRuntimeState;     /* ldr r2, pool lands here */
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
- A branch or assignment that is unreachable for every valid input is not an
  acceptable way to inhibit code motion. During `ScriptNativePmbDeckMake`
  work, a redundant `if (index < 0)` after a completed nonnegative countdown
  happened to stop agbcc from hoisting a later array pointer and produced the
  target bytes. The condition communicated no game behavior and existed only
  to steer optimization, so it was rejected under `PRET_STANDARDS.md` even
  though the machine code matched. Treat such probes as evidence about the
  missing source lifetime or type, never as production source.
- **Fixed-size cursor copies can reveal the original source better than a
  struct assignment.** `RuntimeHistoryPush` (0x08004E88) copies a 24-byte
  record as six consecutive word assignments through advancing source and
  destination cursors. agbcc emits the ROM's one-register `ldmia`/`stmia`
  sequence, including its reuse of the original source base for word one.
  Writing the equivalent aggregate assignment emits two three-register block
  transfers instead. When the ROM has a short unrolled copy, recover the
  element type and cursor lifetime before assuming the source used `memcpy`
  or whole-struct assignment; keep the fixed count named in the record layout.
- **Distinguish an accumulator's initial value from its signed delta.** In
  `CreatePaletteInterpolationTask` (0x0800382C), the 8.24 accumulator starts
  at `+1.0`, while the step is `-1.0 / frameCount`. Reusing the positive
  initializer in the division produced plausible code but the wrong
  transition. Writing `-PALETTE_BLEND_ONE / state->remainingFrames` exposes
  both the signed behavior and the original state reload; agbcc then emits
  the ROM's `0xFF000000` numerator and exact `ldrsh`/division sequence.
- **Verify the bytes used for section alignment.** Thumb code only needs
  two-byte alignment, but the next ROM object can require a four-byte start.
  GNU `as` fills that gap in an executable section with `0x46C0` (a Thumb
  `nop`), while this ROM sometimes contains `0x0000`. In
  `CreatePaletteSequenceTask` (0x080041E0), the constructor itself matched
  exactly but the complete ROM differed at 0x08004266 until the verified
  two-byte zero tail was represented as data in the same placed section.
  Treat those bytes as an explicit object only after checking the base ROM;
  do not add tails merely to make section sizes line up.
- **A hidden BL can straddle two raw words.** When an unlabelled caller was
  emitted as `.4byte` data, a Thumb call beginning at an address congruent to
  two modulo four occupies the upper halfword of one directive and the lower
  halfword of the next. To make that caller shiftable, preserve the ordinary
  halfword on each side and replace the middle pair with `bl Symbol`; do not
  leave the old encoded displacement in place. The callers of
  `RuntimeSetBufferEEAName`, `CreateTask51E84`, and `CreateTaskB478` all had
  this shape. A full ROM comparison verifies both the split and relocation.
- An absolute linker symbol and a named numeric layout offset are not always
  interchangeable to agbcc. `GameStateGetResourceCounter` needs the IWRAM base
  and the fixed root-slot offset `0x3FDC` loaded as two values before they are
  added. Writing the offset as the address of the linker symbol
  `gMapGenerationRootOffset` gave the right literals but assigned the root and
  counter offsets to r3/r4; holding the named numeric offset in a local gives
  the ROM's r4/r3 allocation. Writing `gIwramBase + 0x3FDC` directly is also
  wrong because the compiler folds it to a single relocated pool value. Keep
  fixed RAM-layout offsets centralized and named, and preserve the separate
  locals when the ROM separately materializes the base and offset.
- **After adding a function, run the host tests, not just `make compare`.** The
  tests compile individual `.c` files on their own, so a new reference to a
  symbol the ROM link resolves -- `gSecondaryRuntime`, `gIwramBase`, anything
  `.set` in `asm/game_table_handlers.s` -- fails at link with "undefined
  reference" even though the ROM is byte-exact. Fix it by defining the symbol in
  that test's scaffold, never by changing the source back. This has now bitten
  five tests: test_item, test_map_field, test_map_native, test_script_native and
  test_input_random.
- **Preserve the old value of a strength-reduced loop counter explicitly.**
  `NcdReleaseSpriteArray` (0x08053FD4) uses a 16.16 induction value even
  though the counter only controls eight iterations. A normal local holding
  `previousIndex` before the increment makes agbcc emit the ROM's `mov`, add,
  arithmetic shift, and comparison sequence. Rewriting it as a conventional
  integer `for` loop changes both the counter representation and address
  calculation. This is a source-lifetime clue, not a register hint.
- **Describe payload fields relative to the task header.** In
  `CreateTask683C4` (0x080683C4), spelling both the allocation size and the
  owner-field address as raw `0x238` constants makes agbcc retain the size in
  a saved register and changes the prologue. Casting `task + 1` to the actual
  payload structure and assigning `state->owner` expresses the distinct
  source operations; agbcc then recomputes the address exactly as the ROM
  does. Recovering structure layout can therefore resolve an apparent
  register-allocation mismatch without pinning any register.
- **Model related IWRAM symbols as one partial aggregate when the code walks
  between them.** `CreateTask6EAFC` (0x0806EAFC) starts from
  `gMainTaskManager`, then adds the verified 0xD18 distance to reach the game
  state root slot at 0x03003FDC. A partial `TaskAdapterRuntime` with the task
  manager first, explicit unknown padding, and a `gameState` pointer expresses
  that relationship without embedding either RAM address in C. agbcc then
  retains the task-manager base across `CreateTask` and emits the ROM's second
  offset addition naturally. Treat this technique as a verified partial
  layout: keep unknown bytes named as padding and do not invent field meaning.
- **A field reload may express ownership better than parameter reuse.** In
  `CreateTask62304` (0x08062304), the constructor stores its owner in the task
  payload and immediately derives a resource pointer from that stored field.
  Reusing the `owner` parameter is logically equivalent, but agbcc retains it
  in a saved register and produces different instructions. Writing
  `state->resource = (u8 *)state->owner + ...` emits the ROM's reload from the
  payload and exact register lifetimes. This is meaningful C—the initialized
  task state becomes the source of the derived resource—not a dummy reload or
  register request.
- **Keep the callee prototype honest and narrow at the use site.**
  `TryPurchaseArmOrConsumable` (0x0805427C) sign-extends the result of
  `GameStateGetEntry2768Total` before comparing it with the copy limit. A
  locally narrowed return prototype emits the same instructions but conflicts
  with the function's real `s32` definition. Keeping the `s32` prototype and
  writing `(s16)GameStateGetEntry2768Total(...)` preserves both type
  consistency and the ROM's explicit `lsl`/`asr` pair. A matching instruction
  sequence is not sufficient if it relies on incompatible declarations across
  translation units.
### Cast unsigned layout dimensions at the comparison site

`GeneratedMapChooseAttributeIndex` multiplies the unsigned KMP width and
height fields, but the ROM compares both the initial and advancing flat index
with signed branches.  Keeping the shared header fields unsigned and casting
each operand to `s32` at this use site preserves both facts.  A cast applied
only after the multiplication changes agbcc's multiplication destination; a
signed temporary changes it in the opposite direction.  Casting the operands
in the repeated expression produces the ROM's `muls r0, r1` and signed
`bge`/`blt` sequence naturally.

The function's success path precedes its `-1` failure block in the ROM.  An
explicit result local with `no_match` and `done` labels retains that layout;
early-return forms cause agbcc to invert the condition and move the success
block.  The labels represent the observed source control flow and do not add
dead code or compiler-only behavior.

### Load a global pointer slot before scaling an in-place index

The actor byte accessors at 0x0800943C..0x080094B4 load the address of
`gSecondaryRuntime` before multiplying the actor index by the 1672-byte record
stride. Writing `actor += (u32)gSecondaryRuntime` directly lets agbcc defer the
global load until after the multiplication and changes the instruction order.
Initializing a real pointer-to-pointer local first, then scaling `actor`, then
dereferencing that slot reproduces the ROM naturally.

Keeping the running address in the `actor` parameter is also significant for
signed-byte getters. It makes the load destination and base both r0, so agbcc
uses `ldrb` followed by explicit sign-extension shifts. A separate base local
usually keeps the address live in another register and permits `ldrsb` instead.
Both sequences implement the same C type; the difference comes from register
lifetime, not from a different compiler or a required register constraint.

### Keep branch results distinct until their shared narrowing

`EncodeHexDigitFromS16` loads its digit into a signed 32-bit local, copies that
local to a separate result in each branch, applies the branch-specific ASCII
offset, and narrows the shared result to `s8` at return. Collapsing the result
into the input lets agbcc merge the branches into `+ '0'` followed by a
conditional `+ 7`. The explicit result expresses the observed two-result
source flow and reproduces the ROM without dead code or compiler hints.


### Preserve a stored halfword as a distinct value before a signed comparison

`GameStateRecordSetField2` and `GameStateRecordAddField2` write a value through
a `u16` field and then compare that stored-width value with a signed `s16`
limit. Keeping a separate `u16 storedValue` makes the truncation part of the C
model. agbcc consequently emits the ROM's explicit zero-extension followed by
sign-extension and retains the record pointer in r1 and the unsigned limit in
r2. Reusing only the `s32` arithmetic value removes those conversions and, in
the add routine, swaps the record and limit registers.

This is useful beyond this record family: when the ROM stores a narrow value
and immediately compares it at that width, model the stored representation as
a separate exact-width local. Do not reproduce the register choice with a
forced-register declaration.

### Split assembly sections after removing an interior range

Replacing 08055EC8..08055F4C with C initially moved the following assembly
resolver from 08055F4C down to 08055EC8. The local function bytes were exact,
but every relocated caller changed because the surrounding assembly was still
one continuous section. The correct fix is a new `.section .rom.00055F4C`
boundary before the surviving resolver. Whenever C replaces bytes from the
middle of an assembly section, anchor the first surviving byte in its own
addressed section before trusting a function-level object comparison.

### Preserve a pointer load as an integer until the first offset addition

`CountConsumableInventoryCopies` at 0x08057078 scans 256 signed-halfword
inventory slots. A direct `u8 *` initialization makes agbcc load the runtime
base into r1, while the ROM loads it into r0 and writes the first computed
entry address to r1. Keeping the loaded RAM address in the integer member of
a pointer/address union until adding the named inventory offset expresses
those two distinct values and reproduces the original allocation. The pointer
member is selected before any dereference. This avoids host pointer-size
warnings as well as fixed ROM addresses and compiler hints.

The two independent fixed-point counters are also significant. One advances
through all 256 slots; the other advances only on a match and therefore
becomes the copy count. Replacing either with a conventional integer loop is
behaviorally correct but does not reproduce this compiler's instruction
sequence.
