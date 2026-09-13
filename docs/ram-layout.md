# RAM layout

`ram_layout.json` records the boot-time ownership of all 256 KiB of EWRAM and
all 32 KiB of IWRAM. `python3 tools/ram_layout.py` validates that every
partition is in bounds, contiguous, and non-overlapping, then writes a
machine-readable report when passed `--json`.

The initialization routine at `0800163C` clears the first `0x3FE0` IWRAM
bytes, installs the interrupt handler at `03000008`, and creates seven heap
arenas that partition EWRAM exactly:

| Arena | Address | Size |
|---|---:|---:|
| EWRAM heap 0 | `02000000` | `0x10000` |
| EWRAM heap 1 | `02010000` | `0x0E000` |
| EWRAM heap 2 | `0201E000` | `0x02000` |
| EWRAM heap 3 | `02020000` | `0x08000` |
| EWRAM heap 4 | `02028000` | `0x0C000` |
| EWRAM heap 5 | `02034000` | `0x08000` |
| EWRAM heap 6 | `0203C000` | `0x04000` |

Together they reserve all `0x40000` EWRAM bytes. This means there is no
unassigned EWRAM address space, but it does not mean every heap byte contains a
live allocation. Measuring live use and fragmentation requires allocator
telemetry while the game runs.

The allocator format is now decoded well enough to measure that live use. Dump
`02000000..0203FFFF` and `03000000..03007FFF` from an emulator at the point of
interest, then run:

```sh
python3 tools/ram_snapshot.py --ewram ewram.bin --iwram iwram.bin \
    --json reports/build/ram-snapshot.json
```

The report follows all seven EWRAM heap handles and the IWRAM heap handle. For
each initialized arena it reports allocated payload, free payload, allocator
metadata, block count, and largest free block. The last value helps distinguish
an out-of-memory failure from fragmentation. Invalid block boundaries cause a
hard error instead of producing plausible but incorrect totals.

IWRAM contains the main runtime block, map viewports, runtime root pointers,
MusicPlayer2000 tracks and players, copied mixer code, newlib BSS, a `0x1400`
byte engine heap, stack reserves, and the sound/interrupt words at
`03007FF0..03007FFF`. The manifest currently accounts for 31,800 bytes and
marks 968 bytes as unassigned. Nested named objects document 74 addresses used
directly by recovered C. Unknown fields remain inside their correctly bounded
parent partition until caller analysis establishes their types.

Resolve a crash address to the narrowest known object and its byte offset with:

```sh
python3 tools/ram_layout.py --address 03003CC4
```

The catalog names the three task managers, four KMP viewport slots, all nine
MusicPlayer2000 track pools and player structures, the PSG channels, the
sequence scratch area, and the sound-driver state. Heap allocations cannot have
stable source-level addresses, so snapshots report them as allocator blocks.

The matching ROM also exposes `HeapGetFreeBytes`, `HeapGetLargestFreeBlock`,
and `HeapGetAllocationSize` as readable C, which makes the same measurements
available to future in-game diagnostics.

`make`, `make stats`, and `make english` show these accounted categories next
to the ELF-linked subset. The linked subset alone is misleading because the
original game establishes nearly all RAM through fixed addresses and runtime
heap creation rather than normal ELF `.bss` sections.
