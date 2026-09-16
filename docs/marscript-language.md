# marscript: a readable source language for MAR event scripts

## What this is

MAR's event scripts (the things that make an NPC walk, a door open, a
cutscene play) are stored in the ROM as `.SPC` files: compiled bytecode for
a stack machine with 128 numbered registers. That format is fine for the
game to run and unreadable for a person to write or edit -- a real
instruction from a real script looks like this:

```
31 0F 08 00 00 00   29 96 00 00 00   29 01 00 00 00   29 02 00 00 00   80 00 00 00 00   2A 00
```

marscript is a plain-text language that compiles down to exactly that
bytecode, so you can write (or read) the same script as:

```
native "SetBool"(150, 1)
```

Every existing `.SPC` script in the game can be decoded into marscript and
recompiled back into byte-for-byte identical bytecode -- this has been
verified against all 334 real scripts in the ROM, both directions. That
means you can safely open any event script as readable marscript, and safely
write brand new ones from scratch, using the same language and the same
tools (`tools/marscript.py`).

## Quick start

Here's a small, complete script -- an NPC that appears, then despawns:

```
// EV_BA03.marscript -- a small cutscene

script EV_BA03 {
    sprite dorothy = spawn(container: 2, resource: "09A00", animation: 0, at: 120, 80)

    move dorothy to 120, 72 over 30 frames blocking
    change dorothy to resource "09A01" animation 0

    end
}
```

Read it top to bottom: spawn a sprite named `dorothy`, slide her upward over
30 frames, swap her artwork to a different pose, then end the script. That's
the whole shape every marscript file follows: a `script NAME { ... }` block
containing declarations and statements, one per line.

## File format

A `.marscript` file is plain UTF-8 text (Japanese dialogue text included --
this project's scripts are almost entirely Japanese, and marscript renders
that text directly rather than escaping it). `//` starts a line comment.
Blank lines are ignored. Block boundaries are `{`/`}`; indentation is purely
cosmetic, for your own readability.

## Writing a new script

### Declaring a sprite

```
sprite <local-name> = spawn(container: <int>, resource: <string>, animation: <int>, at: <x>, <y>)
```

```
sprite dorothy = spawn(container: 2, resource: "09A00", animation: 0, at: 120, 80)
```

This brings an actor onto the screen with a starting resource, animation,
and position. `dorothy` is a name *you* pick -- it's not written into the
compiled output at all, it's just how the rest of your script refers to this
actor (`move dorothy ...`, `change dorothy ...`). Declare one `sprite` per
actor, at the top of the script, before you use it.

### Declaring a hitregion

```
hitregion <local-name> = region(id: <int>, at: <x>, <y>, size: <w>, <h>)
```

```
hitregion doorway = region(id: 3, at: 96, 64, size: 16, 24)
```

A hitregion is a rectangular trigger/collision zone -- a doorway, a pressure
plate, an invisible wall. Declare one per zone the script owns, then adjust
it (`hit doorway rect ...` / `hit doorway set ...`) or turn it off
(`free doorway`) later in the same script.

### Declaring a variable

```
var <local-name> [= <expr>]
```

```
var attempts = 0
```

A `var` is a script's own scratch value -- a counter, a flag, something an
`if`/`while` condition depends on. Up to 15 can be live in one script at
once; if you need more than that, the compiler tells you exactly where the
conflict is rather than silently reusing one.

### Moving a sprite

```
move dorothy to 120, 72 over 30 frames blocking   // absolute position
move dorothy to +0, -8 over 58 frames blocking    // relative -- "move up 8"
```

`over N frames` is how long the move takes to interpolate; `blocking` makes
the script itself wait for the move to finish before running its next line
(leave it off and the move plays out while the script moves on immediately).
The `+`/`-` form is a relative move -- exactly the "move up 1" style motion
this language was originally built to make easy to write -- and the compiler
expands it for you into an absolute move under the hood (see **How the
compiler builds this** at the bottom, if you're curious what that expansion
looks like).

### Changing a sprite's look

```
change dorothy to resource "09A01" animation 0
```

Swaps an actor's art/animation in place -- the sprite doesn't move or reset,
only its appearance changes. Handy right before a `move`, e.g. switching
from an idle pose to a walking one.

### Setting a raw sprite property

```
set <sprite> property <int> to <expr>
```

```
set dorothy property 5 to 12
```

An escape hatch for sprite properties that don't have their own named verb
yet (X and Y -- properties 0 and 1 -- are already covered by `move`/
`sprite ... at`). If you find yourself writing the same property number
over and over, that's usually a sign it deserves a real verb added to this
language instead.

### Placing or resizing a hitregion

```
hit doorway rect at 100, 70 size 20, 30
```

Activates the region and (re)places its rectangle. This doesn't perform a
hit test by itself -- it just tells the engine where the zone is, for its
own collision pass to use afterward.

### Changing one hitregion property

```
hit doorway set mode to 1
```

`property-name` must be one of `x`, `y`, `width`, `height`, or `mode` --
those are the only properties a hitregion actually has. Use this to tweak
one field (commonly `mode`) without rebuilding the whole rectangle.

### Turning off a hitregion

```
free doorway
free all hitregions
```

Deactivates one region, or all sixteen at once. The region's geometry and
mode are left in place, just inert -- use this at the end of whatever scene
set the region up.

### Loading a field (map transition)

```
load field "MAP01_A" at 320, 160
```

A scripted map/screen change. `x`/`y` position the viewport -- they are not
(yet) proven to also place the player, so don't assume that without
checking for the specific transition you're writing.

### Calling or chaining another script

```
call script "SP_M01.SPC"
chain script "CH_M01.SPC"
```

`call` and `chain` are genuinely different behaviors in the engine (not a
marscript naming choice) -- use whichever one the scene you're replacing
actually used.

### Branching and looping

```
if (counter == limit) {
    call script "A.SPC"
} else {
    chain script "B.SPC"
}

while (counter < limit) {
    set dorothy property 0 to 1
}
```

Ordinary structured control flow -- write conditions with `==`, `!=`, `<`,
`<=`, `>`, `>=`, `&&`, `||`, `!`, and ordinary arithmetic
(`+ - * / % & | ^ ~`). Use `if`/`while` instead of hand-written jumps
wherever the shape fits; they're much harder to get wrong.

### Jumping to a label

```
label loop:
    set dorothy property 0 to 1
goto loop
```

The lower-level escape hatch under `if`/`while` -- reach for it only for
control flow those structured forms genuinely can't express (like jumping
into the middle of another loop). Every decoded real script uses labels
extensively, since that's how real compiled bytecode's jumps are rendered.

### Ending a script

```
end
```

Every path through a script needs to reach an `end` somewhere -- it's the
instruction that actually hands control back to the game.

## A complete example using declarations, branching, and a loop

```
script SHOP_DEMO {
    sprite clerk = spawn(container: 0, resource: "0C100", animation: 0, at: 64, 48)
    var attempts = 0

    while (attempts < 3) {
        set clerk property 0 to 1
    }

    if (attempts == 3) {
        call script "SHOP_FULL.SPC"
    } else {
        chain script "SHOP_OPEN.SPC"
    }

    end
}
```

## Advanced / low-level constructs

Everything above covers what a hand-written script normally needs. The
constructs below exist because a *decoded* real script sometimes contains
bytecode shapes with no verified named meaning yet -- so the decoder falls
back to writing them out directly, instruction by instruction, rather than
guessing at what a native call means. You'll see these in decoded output
far more often than you'll want to type them by hand, but they're real,
supported marscript syntax -- anything the decoder writes is guaranteed to
recompile back to the exact original bytes.

- **`string NAME: "text"`** declares a piece of text (dialogue, a resource
  name, anything) at a labelable location, so it can be referenced by
  address elsewhere:
  ```
  string greeting: "何かご用でしょうか"
  ```

- **`address rX, NAME`** loads a register with the address of a previously
  declared `string` (or a label), the same way the game loads a string's
  address before passing it to a native call:
  ```
  address r0, greeting
  ```

- **`native "NAME"(args...) -> rX`** calls a native function directly by
  name, with a mix of literal integers, string literals, and register
  values as arguments, storing its result in `rX`:
  ```
  native "SetBool"(150, 1) -> r0
  native "FceSet"(r0, 1, 56) -> r0
  ```
  This is also what every higher-level verb above (`move`, `change`,
  `set`, `hit`, `load field`, `call script`, ...) compiles down to
  internally -- they're just names for specific, verified `native(...)`
  calls.

- **`switch rX { candidate -> label, ... }`** jumps to whichever label's
  candidate value matches the register, falling through if none match:
  ```
  switch r0 {
      400 -> case_a,
      401 -> case_b,
  }
  ```

- **`goto_if_zero rX, label`** jumps only if the register is zero -- the
  primitive `if`/`while` themselves compile down to.

- **Raw register operations** (`add r0, r1`, `add_i32 r0, 5`, `sub`, `mul`,
  `div`, `mod`, `and`, `or`, `xor`, `logical_and`, `logical_or`,
  `eq_zero`, and the rest of the VM's instruction set) are available
  directly by name when an expression needs something `if`/`while`/`var`
  don't already generate for you.

- **`raw "<hex bytes>"`** carries opaque bytes through completely
  unchanged -- used by the decoder for the rare span of bytes (like
  trailing padding after a script's last real instruction) that isn't
  itself a real instruction, so even those bytes still round-trip exactly.

- **`stack_size <int>`** / **`entry <int>`** are two raw header values every
  real script has, whose exact meaning isn't fully understood yet (see
  `docs/decompiled-flags.md`). New hand-written scripts can leave these out
  entirely -- they default to sensible values. The decoder always writes
  them explicitly, purely so a decoded real script recompiles to the exact
  original bytes.

## Decoding existing scripts to marscript

`tools/marscript.py`'s `decompile_to_source()` turns any real `.SPC` file's
bytecode into a full marscript file: every instruction, every branch, every
loop, not just the call sites. Recognized native calls are rendered as a
single readable `native "NAME"(...)` line; a call the decoder can't safely
prove the boundaries of, and any bytecode shape without a higher-level verb
yet, falls back to the low-level constructs above -- so the result is always
readable where the project's own analysis is confident, and always exact
even where it isn't.

This has been verified end-to-end against all 334 real scripts in the game:
decode every script to marscript text, recompile that text, and the result
matches the original bytecode byte-for-byte (`tests/test_marscript_decompile_roundtrip.py`).
That's the same standard a decompiled `.c` file is held to elsewhere in this
project -- it isn't "close enough," it's exact.

## Build integration

Working today, for `mar_english.gba` only. **`mar.gba` never touches
marscript at all** and always uses the original untouched script bytes --
its whole purpose is matching `baserom.gba` byte for byte
(PRET_STANDARDS.md's Golden Rule), and it has no ROM expansion region to
grow into even if it wanted to. `make compare` (which verifies exactly
that byte-for-byte match) is unaffected by anything in this section.

To edit a script for the English build, drop a `<NAME>.marscript` file in
`scripts/marscript/` (matching a `scripts/nfp/manifest.json` entry's
`name`, e.g. `scripts/marscript/BTIM00.SPC.marscript`) -- write it by hand,
or start from `python3 tools/marscript.py decompile scripts/nfp/<NAME>.bin
<NAME>.marscript`. On build, `tools/marscript_rom_build.py` compiles it and
decides where the result goes:

- **Fits in the script's original archive slot** (the common case, and
  always true for an unedited decompile/recompile): placed there directly,
  padded to exactly the original size so nothing else in the ROM shifts.
- **Grew past the original slot** (or the script is brand new): the
  *original* slot is left untouched, and the new, larger content is placed
  instead in the English build's ROM expansion region -- the same
  mechanism `mar_english.gba` already uses to be a 32 MB ROM against the
  original's 16 MB. `tools/split_scripts_english.py` and
  `tools/resource_catalog.py`'s `build-english` mode handle placing it and
  re-pointing that one script's resource-catalog entry there (as a linker
  symbol, resolved at link time -- not a value anyone has to compute by
  hand). Every other script's catalog entry is untouched.

This has been proven against the real build, not just in isolation:
`tests/test_marscript_rom_build.py`'s end-to-end test deliberately grows a
real script via a marscript override, runs the actual build, and verifies
the grown content lands correctly in the expansion region, the original
slot is untouched, and the resource catalog resolves to the right address
-- with a real `arm-none-eabi-*`/agbcc link, not a simulation.

Not yet built: the map editor's own UI for creating/editing these override
files (currently, `scripts/marscript/` files are written by hand or by
running the CLI directly).

## How the compiler builds this (internals)

This section is for anyone modifying `tools/marscript.py` itself, not for
writing scripts -- everything above is the complete, self-sufficient
reference for that.

**The one hard rule**: every marscript construct must compile to bytecode
whose semantics are already verified -- either a native call the project
has proven (see `tools/map_editor/README.md`), or one of the general VM
primitives already implemented in `tools/script_assembler.py`'s `OPS`
table (arithmetic, comparisons, jumps, calls). marscript is not permitted
to invent behavior the engine doesn't actually have. Where a convenience
verb expands to more than one instruction, the language reference above
says so explicitly -- never silently.

**Movement macros**: a relative `move sprite to +dx, +dy over ...` is a
compiler macro, not a native call. It expands to:

```
var _tmp_x = sprite.x    // SprGet(id, 0) -- see README: "propagate verified
var _tmp_y = sprite.y    //   X/Y SprGet results through register 0"
move sprite to _tmp_x + dx, _tmp_y + dy over n frames
```

This is only valid where the sprite's current position is actually knowable
-- a literal initial placement, or a prior `move`/`set` in the same script
whose target the compiler can still track. If the compiler cannot prove the
current position (a dynamic/external actor), it is a compile error, not a
guess.

**Expressions** compile directly to the corresponding opcode
(arithmetic/bitwise/logical operators, and comparisons as the appropriate
`*_zero` test after a subtraction). Only what's already in
`script_assembler.py`'s `OPS` table is permitted.

**Text encoding**: script strings are Shift-JIS with embedded single-byte
engine control codes, not ASCII (see `tools/text_codec.py`). marscript
renders ordinary text directly and any control/private-use byte as a
`<XX>` hex escape; this is exactly invertible, and is the same codec
`tools/extract_scrp_text.py` already uses for the same byte layout
elsewhere in this project.

**Native-call collapsing**: the decoder only renders a `native "NAME"(...)`
line where it can prove, against its own already-verified instruction
decode (not a fragile byte-pattern guess), that the exact expected
`add_i32 r15,N` / pushes / `push_i32 argc` / `native_call` / `pop`
sequence is really there with nothing foreign interleaved, and that no
other branch in the script targets a point in the middle of it. Real
scripts do occasionally put unrelated instructions (like a `jump`) right
where a call's setup would otherwise be, or have some other branch target
land mid-sequence -- collapsing across either of those would silently
delete or mis-render real code, so the decoder detects both cases and
falls back to rendering the raw instructions individually instead.

**Header fields**: every script's `SCRP`/`CODE` payload starts with two raw
values (`stack_size`, `entry`) that this project can decode and preserve
byte-exactly but doesn't yet have a fully confirmed meaning for -- see
`docs/decompiled-flags.md` for the full evidence trail.

## Open questions

1. `wait <n> frames` has no verified bytecode pattern yet -- needs a real
   script exhibiting a timing/yield idiom before it's added, not a guess.
2. Register allocation across deeply nested `if`/`while` blocks needs a
   concrete spilling rule once real scripts are compiled and register
   pressure is observed in practice.
3. Whether every native in the registry deserves a named verb, or the
   generic `native "name"(args...)` escape hatch is enough for the long
   tail, is still open -- leaning toward the escape hatch for anything
   without map-editor-README-level verified semantics, so marscript never
   states a native's behavior with more confidence than the project
   actually has.
