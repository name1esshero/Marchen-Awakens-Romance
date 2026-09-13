# Save data format

The game stores one `0x3F2C`-byte block at the start of SRAM. The matching C
definition is `struct SaveBlock` in `include/save.h`.

| Offset | Size | Meaning |
|---:|---:|---|
| `0x00` | `0x10` | NUL-terminated magic: `MARHEAVEN000000` |
| `0x10` | `4` | Format/version word (`0x3F828F5C`) |
| `0x14` | `4` | Save generation counter |
| `0x18` | `4` | Payload recovery counter; incremented after clearing a bad payload |
| `0x1C` | `4` | Field whose purpose is not yet identified |
| `0x20` | `4` | CRC-32 of header bytes `0x00..0x2B`, calculated with this field zero |
| `0x24` | `4` | CRC-32 of the `0x3F00`-byte payload, calculated with this field zero |
| `0x28` | `4` | Reserved; zero while calculating/writing checksums |
| `0x2C` | `0x3F00` | Game progress payload |

`WriteSaveBlock` installs the magic and version, increments the generation,
calculates the payload and header checksums, and writes the complete block to
`0x0E000000`. `LoadSaveBlock` reads it through the SRAM routine copied to
IWRAM, then calls `ValidateSaveBlock`.

Validation distinguishes four failures:

- `-1`: header checksum mismatch
- `-2`: magic mismatch
- `1`: incompatible format/version
- `2`: payload checksum mismatch

For a payload-level failure, the loader keeps the header and clears only the
payload. For a header or format failure, it clears the complete block. The
low-level SRAM copy and verify routines set WAITCNT for eight-cycle byte
accesses, as required by GBA SRAM.

The nonblocking path uses engine tasks. `CreateSaveBlockLoadTask` reads the
complete block, validates it, performs the same recovery policy, and publishes
`1` for a valid block or `-1` after recovery. A low-level read failure records
game-state status `-2`. `CreateSaveBlockWriteTask` prepares the magic, format,
generation, and checksums before delegating the verified SRAM transfer. Header
updates use a separate 44-byte task path. These state machines deliberately
leave scheduler phases between preparation, transfer, and completion.
