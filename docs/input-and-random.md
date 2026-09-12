# Input, packed flags, and random state

The functions at `0807A084..0807A210` now compile from readable C with agbcc.
The 396-byte range includes compiler literal pools and original alignment.

## Input state

`src/input.c` and `include/input.h` identify the pointer at `03006108` as the
base of eight 8-byte key-state slots. Initialization installs caller-owned
storage and clears all 64 bytes. Callers must supply a valid slot index.

| Offset | Field | Behavior |
| --- | --- | --- |
| +0 | held | Current active-high button mask |
| +2 | pressed | Newly pressed bits, consumed by readers |
| +4 | previous | Previous held mask |
| +6 | unused | Cleared at initialization; not accessed by these helpers |

`KeyInputPoll` inverts the active-low hardware halfword at `04000130` and
keeps ten key bits. `KeyInputSet` receives a mask directly and does not apply
that ten-bit limit. Both save the previous mask and calculate rising edges.

The directional filter has a significant original behavior: when left and
right are both pressed, the entire held mask becomes left (`0x20`). Otherwise,
if up and down are both pressed, it becomes up (`0x40`). Other held buttons
are discarded in those cases. C preserves this behavior exactly.

`KeyInputConsumePressed` returns `pressed & mask` and removes those matching
bits from the pressed state. It is not a read-only query. `KeyInputAnyHeld`
returns a Boolean and leaves input state untouched. English message pagination
uses the consuming API for its A-button advance.

## Packed flags

`src/bitset.c` sets, clears, and tests bits in caller-owned byte arrays.
Bit zero is the least significant bit of byte zero; bit eight is the least
significant bit of byte one. A test returns exactly zero or one. Neither
helper allocates storage or validates the index.

## Random state

`src/random.c` stores a 32-bit state at `0300610C`. `RandomInit` delegates to
`RandomSeed`; `RandomGetSeed` exposes the full state. Each `Random` call applies:

```c
state = state * 1103515245 + 12345; /* Unsigned 32-bit wraparound. */
return (state >> 16) & 0x7FFF;
```

Seed one produces `16838, 5758, 10113, 17515, 31051`. Returned values use only
bits 16..30, while the full updated 32-bit state is retained for the next call.

## Verification

`python3 tests/test_input_random.py` compiles the real C with host substitutes
for the hardware register and state locations. It checks all hardware button
combinations, independent slots, pressed-bit consumption, initialization bounds,
RNG vectors, and bit operations spanning byte boundaries. The normal ROM build
and `tools/audit_provenance.py` establish matching linked bytes separately.
