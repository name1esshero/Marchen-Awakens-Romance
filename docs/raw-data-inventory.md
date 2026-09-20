# Remaining raw ROM data

The three remaining opaque binary inputs contain 16,164 bytes. They are
retained because each contains initialized data whose complete record layout
is still being recovered. Long uniform spans surrounding the high-address
payloads are emitted with linker fill directives and are no longer stored in
binaries.

| ROM range | Bytes | Current identification | Next source form |
|---|---:|---|---|
| `081B02A4..081B096C` | 1,736 | UI geometry, handler-pointer lists, animation offsets, and named title/menu resources including `T_TTL01` through `T_TTL04`, `C_CSL`, and `T_CMD` | Typed UI layout structures after callers establish each boundary |
| `081BEC3C..081BF408` | 1,996 | Item/ARM menu resource descriptors, cursor positions, navigation tables, and handler-pointer lists immediately before the decoded battle-arena tables | Typed menu descriptors and navigation arrays |
| `08FE0000..08FE3090` | 12,432 | `agb_debug_monitor.bin`: a dormant SDK debug monitor containing ARM exception vectors, mixed ARM/Thumb code, hardware-register accesses, and internal pointers in the `09FE0000` mirror | Separate monitor disassembly, then clean C where individual routines match |

The monitor's `08FE3090..08FE4000` tail is entirely zero-filled and now builds
as a fill section. The retail cartridge header does not enable the GBA debug
ROM path, so none of this region is evidence for game event flags or saved
state. The actual packed game-state flag banks and their verified accessors are
documented in [`include/flags.h`](../include/flags.h) and
[`src/flags.c`](../src/flags.c). The identification is also consistent with the
[documented GBA BIOS debug destinations](https://gbadev.net/forum-archive/thread/8/2265.html)
at `09FE0000`, `09FE2000`, and `09FFC000`.

The former `data_FFF000.bin` is likewise fully structural:
`08FFF000..08FFF800` is zero-filled and `08FFF800..08FFFF00` contains 448
copies of the debug-vector address `09FFC000`. The placement manifest now
expresses both runs directly, so extraction and rebuilding no longer preserves
an opaque binary for this region.

The two UI/menu blobs contain small fields that may eventually prove to be
state selectors or flag IDs. They are not the event-flag bit array itself:
their interior addresses are referenced as layout records, resource names,
cursor/navigation data, and callback lists. Field names will remain
conservative until each caller establishes its meaning.

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

Names in this inventory remain conservative. The debug-monitor classification
rests on its exception-vector code, exact BIOS debug addresses, hardware I/O,
and the matching high-ROM vector table rather than on instruction-looking
bytes alone.
