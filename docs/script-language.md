# MAR script source language

`tools/script_assembler.py` converts a version-1 JSON source file into the
game's `SCRP`/`SPC` container. It only emits bytecode whose handler and operand
width have been verified from the 256-entry dispatch table at `08F2A860`.

The separate 128-entry native registry occupies `081AFEA4..081B02A4`. Each
entry is an ASCII-name pointer followed by a handler pointer. The word at
`081AFEA0` belongs to the preceding table. A `FUNC` chunk maps an ASCII
native name to one or more CODE offsets. At load time those relocation records
resolve opcode `80` call sites. A FUNC reference points to the preceding
`29 <argument-count>` instruction and includes the six-byte CODE header.

`081ACCB0` is an unrelated battle-action pointer table. Earlier project notes
mistook it for the native registry; the header and documentation now use the
name/handler pairs confirmed by `SprSet`, `FldSet`, and `HitInit` references.

## Container and execution model

The container is `SCRP`, total payload size, `CODE`, CODE size, a six-byte CODE
header, bytecode, optional `NVAR`/`FUNC` chunks, and `TERM`. The CODE header is
a 32-bit VM stack size and a 16-bit entry offset. Bytecode addresses are
relative to the first instruction after that header.

Opcode `10` is `jump_relative u16`. The original compiler uses it to jump over
inline NUL-terminated text or resource names. Opcode `22` then loads the
embedded bytecode address into a VM register. This is why a byte sequence that
looks like `10, length, text` is not itself a print instruction.

Operands `0..14` select the VM's fifteen general registers. Register 15 is the
stack cursor. The high-bit indirect operand form exists in the runtime, but the
source assembler currently accepts direct registers so newly authored scripts
remain easy to validate.

## Source example

```json
{
  "version": 1,
  "stack_size": 1024,
  "entry": "start",
  "instructions": [
    {"label": "start"},
    {"op": "native", "name": "FldSet",
     "args": [{"string": "MAP01_1A", "register": 0}, 0, 0], "result": 0},
    {"op": "return"}
  ]
}
```

Build it with:

```sh
python3 tools/script_assembler.py scripts/source/example.json build/example.SPC --compress
```

`make script-sources` builds every `scripts/source/*.json` file under
`build/scripts/custom/`. `make script-catalog` scans all 334 original named
scripts and refreshes `maps/script_catalog.json`, which records static
`FldSet`, `SprInit`/`SprChg`, `SprSet`, and `SprMove` call sites for the map
editor. Static means a call exists in the bytecode; it does not imply that its
branch executes during a particular visit.

The `native` form emits the verified compiler sequence: adjust the stack by
four bytes per argument, push arguments in source order, push the argument
count, call through opcode `80`, then pop the result into the requested VM
register. String arguments are emitted as skipped inline data and passed with
`push` rather than being treated as integer literals.

Available low-level operations are listed by `OPS` in the assembler. They
cover jumps, calls, register copies and immediates, arithmetic, bitwise and
logical operations, callbacks, context/table reads, string concatenation and
freeing, and frame restoration. Absolute jump targets may be numeric bytecode
offsets or labels.

## Current integration boundary

The assembler creates valid standalone SPC members and FUNC relocations. The
ROM build still replaces existing fixed-size NFP members. Adding a new named
map or script requires repacking the NFP directory: its 830 directory records
currently end exactly where payload data begins, so there is no unused slot to
rename safely. That archive-repack step and runtime map-to-script association
must be completed before the editor enables **New map** or **Add event**.
