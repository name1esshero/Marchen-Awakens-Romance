# Game-state flags

The recovered flag API lives in `src/flags.c` and `include/flags.h`. Generic
packed-bit operations remain in `src/bitset.c`; game code should use the
game-state APIs and named bank constants where possible.

## Known banks

| Game-state offset | Capacity | Confirmed use |
|---|---:|---|
| `0x00AC` | 1,024 bits | Script-visible game flags used by the `TestGameFlag` and game-flag setter natives |
| `0x012C` | 10,000 bits | Background attribute enable flags |
| `0x26F8` | At least 448 bits | ÄRM/deck ownership flags |
| `0x2730` | Not yet proven | A separate packed bank accessed by the recovered `GameState*Flag2730` functions |

The +`0x2730` name deliberately retains its offset. Give it a gameplay name
only after scripts or runtime behavior prove what its bits represent.

## Public API

- `GameStateSetFlagsAC(bit, enabled)` sets or clears a script-visible game
  flag. `GameStateTestFlagsAC(bit)` tests it.
- `GameStateInitializeAttributeFlags()` enables every background attribute
  and resets the associated selection field.
- `GameStateSetAttributeFlag()`, `GameStateTestAttributeFlag()`, and
  `GameStateSetAttributeFlagRange()` edit the attribute bank.
- `GameStateSetFlag2730()` and `GameStateTestFlag2730()` access the unresolved
  +`0x2730` bank. `GameStateTestFlag26F8()` tests deck ownership.

Flag IDs are not yet assigned speculative names. Contributors should use a
numeric ID with a nearby evidence comment until the script, map event, or game
logic establishing its purpose has been traced. Once verified, add the name to
`include/flags.h` and record the evidence here.

All nine functions compile to their original ROM addresses and are included in
the byte-identical build. The bank offsets describe RAM layout and are not ROM
addresses.
