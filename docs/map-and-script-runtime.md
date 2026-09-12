# Map generation and script bytecode

The procedural map generator has a separate deterministic random stream. Its
32-bit seed is stored at IWRAM `0x03004044`; each value applies the familiar
LCG step `seed * 0x41C64E6D + 0x3039` and returns bits 16 through 30. Map setup
seeds this stream independently, so generation remains repeatable without
consuming the game's general RNG state.

`MapGenerationGetState` follows the engine root pointer at IWRAM `0x03003FDC`
and returns the block at root offset `0x1304`. The recovered layout in
`include/map_generation.h` covers offsets `0x00..0x8B`: six pointers, indexed
tables at `0x38`, `0x48`, and `0x58`, signed 16-bit arrays at `0x26` and
`0x2E`, four signed bytes at `0x68`, and four 8-byte vectors at `0x6C`. These
accessors are used while inspecting KMP attributes and choosing procedural map
connections. Offset-based names are intentional until those callers establish
which values represent directions, coordinates, rooms, or connection types.

The script VM is reached through two `+0x0C` pointers from IWRAM
`0x0300611C`. Its current bytecode buffer is at VM offset `0x30`, instruction
cursor at `0x44`, and downward-growing stack cursor at `0x84`.

The bytecode helpers now make these rules explicit:

- 16-bit and 32-bit operands are little-endian byte sequences.
- Sequential reads advance the cursor by one, two, or four bytes.
- A push subtracts four from the stack cursor before writing; a pop reads and
  then adds four.
- Jumps store a bytecode offset in the instruction cursor. Calls push the
  return cursor before jumping, and returns restore it.
- The switch command reads an operand, a case count, then pairs of 32-bit case
  values and destinations. It selects the first matching destination.
- Assignment and arithmetic commands use one-byte operand descriptors. The
  descriptor resolver at `0x0807F624` remains assembly and is the next key
  piece for naming local, global, indirect, and immediate operand classes.

All routines described here are compiled by agbcc and covered by the normal
byte-for-byte Japanese ROM comparison. Their addresses and occupied sizes are
recorded in `src/decompiled.json`.
