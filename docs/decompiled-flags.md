# Decompiled flags and small fields

A catalog for numeric fields/flags whose *existence and byte position* are
established but whose full meaning isn't (yet), scattered otherwise across
whichever file happened to be the first place they came up. One entry per
field: what's confirmed, what isn't, and where the confirming evidence
lives. Move an entry into proper header documentation once its meaning is
fully resolved, rather than leaving it duplicated here.

## SCRP/CODE header, offset 4 (u16), `tools/marscript.py`'s `header_field`

**Location**: two bytes immediately after the 4-byte value this project
calls `stack_size` (offset 0, u32), at the very start of a script's `CODE`
chunk payload -- i.e. right before the instruction stream begins.
`include/script.h`'s `struct ScrpHeader` only documents the outer
`SCRP`/`CODE` framing (magic/size/chunk_magic/chunk_size, 16 bytes) and
stops there; these next 6 bytes (stack_size + this field) aren't named or
explained anywhere in the C source, because the script-*loading* routine
that reads them hasn't been found -- `src/script_bytecode.c` only has the
opcode-execution loop, not the loader.

**Confirmed** (`tools/marscript.py`, `tests/test_marscript_roundtrip.py`,
against all 334 real named scripts):
- The field is only ever `0` (324 scripts) or `1` (10 scripts). No other
  value appears anywhere in the real corpus.
- Real execution demonstrably starts at instruction-stream offset 0 in
  *both* cases -- the first byte that decodes as a valid instruction,
  reached by flow-tracing from there, is always at offset 0, never at
  offset 1 or anywhere else. Confirmed by achieving exact byte-for-byte
  disassemble/reassemble round-trips for all 334 scripts using "always
  start at offset 0" and treating this field as an opaque pass-through.

**Not established**: what the field actually means when it's `1`. It is
*not* "byte offset to begin execution" -- that resemblance to
`tools/script_assembler.py`'s own `entry` JSON field (used when building a
*new* script from scratch, where it legitimately is a byte-offset label
reference) is a naming coincidence in this project's own tooling, not
evidence about how the original game's compiler or loader used this field.
Candidates not yet checked against real evidence: a stack-layout variant
flag, a "requires special initialization" flag, something read only by a
loader routine not yet decompiled. Find that loader (grep for a read of a
`CODE` chunk immediately after its 8-byte tag+length, or for the constant
`0x400`/1024 -- see the stack_size entry below) before asserting more here.

## SCRP/CODE header, offset 0 (u32), `tools/marscript.py`'s `stack_size`

**Location**: the first 4 bytes of the `CODE` chunk payload, immediately
preceding the header field above.

**Confirmed** (full survey, all 334 scripts): not a fixed constant --
raw values range widely (1024, 66560, 132096, 263168, 590848, ...). But
**the low 16 bits are exactly `1024` (`0x400`) in every single one of the
334 scripts, with zero exceptions.** This is a packed field:
`(upper16 << 16) | 0x400`, where the upper 16 bits vary per script and the
low 16 bits are a fixed constant that just happens to look like a
plausible plain "stack size" on its own when the upper bits are zero
(which is why the 176 scripts with upper16=0 read as exactly `1024`).

**Not established**: what the varying upper 16 bits represent. A second,
per-script size/count (a genuine stack size, now that 1024 is ruled out as
the whole field?) is the obvious next guess, but unconfirmed without a
decompiled loader to check it against.

## Bytecode opcode `0x80` (native call), `include/script.h`'s `OP_NATIVE_CALL`

**Location**: 1-byte opcode + 4-byte field, fixed width 5
(`tools/script_events.py`'s `FIXED_WIDTHS`, confirmed independently by
`tools/marscript.py` against all 334 scripts).

**Confirmed**: `tools/script_assembler.py`'s own writer always emits this
4-byte field as zero when constructing a *new* native call
(`out.extend(b'\x80\0\0\0\0')`). `tools/marscript.py` decodes and preserves
whatever value real scripts actually contain there (not assumed to always
be zero) and this round-trips exactly for all 334 scripts.

**Not established**: what a *nonzero* value in that field would mean, if
one ever appears -- none does in the current corpus, so this is untested
territory, not a confirmed "always zero" invariant.

## Bytecode opcode `0x15` (`OP_DISPATCH_TABLE`, the `SWITCH` opcode)

**Fully confirmed** -- `ScriptCmdSwitch` in `src/script_bytecode.c` is
already decompiled, matching C: 1-byte opcode, 1-byte `operand` (which
register to test), 1-byte entry count `n`, then `n` 8-byte
`(u32 candidate, u32 target)` entries. At runtime: resolve `operand`,
compare against each `candidate` in order, jump to the first matching
entry's `target` (instruction-stream-relative, same `+6` convention as
`jump`/`jump_if_zero`/`call`/`address`); fall through if nothing matches.

Worth recording exactly how this was found, since it's a real example of a
reverse-engineering trap: this project's own tooling (`tools/marscript.py`)
originally reverse-engineered this opcode from real data alone, *before*
this C confirmation existed -- finding real scripts where a branch target's
`+6` landed exactly on an otherwise-unreachable instruction boundary. That
got the `target` field right, but the byte at position 1 (the actual
`operand` register) was misread as unexplained padding and hardcoded to
`0x00` on every write. The full 334/334 round-trip test still passed,
which looked like confirmation but wasn't one: it only proved no script in
the *current* corpus happens to switch on a nonzero register, not that the
byte's meaning was understood. Fixed once the real handler was found.
Lesson: a passing round-trip test proves the bytes you preserved are
right; it says nothing about a field you never varied.

## Bytecode opcode `0x8F` (`restore_result`)

**Confirmed**: consistently the last real instruction in a script before
its `CODE` payload ends (mid-trace evidence: stopping flow-tracing here,
rather than continuing linearly past it, is required for exact round-trip
on every script that has trailing bytes after it -- e.g. `BTIM00.SPC` has
2 zero-padding bytes after it that are not further instructions).

**Not established**: the exact semantics beyond "the script's real work is
done here" -- whether it's specifically about propagating a native call's
return value, an interpreter-frame teardown, or something else, isn't
decided by this evidence alone.
