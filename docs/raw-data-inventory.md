# Remaining raw ROM data

The four remaining `data_*.bin` files contain 23,956 bytes. They are retained
because each contains initialized data whose complete record layout is still
being recovered. Long erased-ROM spans surrounding the high-address payloads
are emitted with linker fill directives and are no longer stored in binaries.

| ROM range | Bytes | Current identification | Next source form |
|---|---:|---|---|
| `081B02A4..081B096C` | 1,736 | UI geometry, handler-pointer lists, animation offsets, and named title/menu resources including `T_TTL01` through `T_TTL04`, `C_CSL`, and `T_CMD` | Typed UI layout structures after callers establish each boundary |
| `081BEC3C..081BF408` | 1,996 | Item/ARM menu resource descriptors, cursor positions, navigation tables, and handler-pointer lists immediately before the decoded battle-arena tables | Typed menu descriptors and navigation arrays |
| `08FE0000..08FE4000` | 16,384 | A self-contained mixed ARM/Thumb code-and-data image. Internal pointers use the cartridge's `09FE0000` mirrored address range | Separate auxiliary-firmware disassembly, then C where individual routines match |
| `08FFF000..08FFFF00` | 3,840 | 2,048 zero bytes followed by 448 copies of the mirror pointer `09FFC000` | A named high-ROM table once its consumer is identified |

The former `data_F28410.bin` was decoded into OAM dimension/mask tables and
8.8/2.14 fixed-point sine tables. These now compile through `src/math_tables.c`
from editable values in `data/math_tables.json`. The former 64-byte
`data_1BE7EC.bin` is the one-based consumable ID-zero text sentinel and now
compiles as `gConsumableNoneText`.

The former `08F2A860..08F2ACFF` raw range now compiles as
`gScriptOpcodeHandlers`, `gScriptBuiltinFunctions`, and their 19 source-level
names. This exposes all 256 bytecode opcode slots and preserves the original
misspelled `resurn` lookup name required by existing scripts.

The former `08F2AD00..08F2AFEF` range was the bundled newlib implementation's
global `struct _reent` initialization followed by `_impure_ptr`. It now compiles
from `src/libc/reent_data.c`, including symbolic stdin/stdout/stderr pointers and
the original shared `C` locale-string address.

Names in this inventory remain conservative. In particular, the purpose of
the high-ROM code image is not inferred solely from instruction-looking bytes.
