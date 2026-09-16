#!/usr/bin/env python3
"""Bidirectional SPC/marscript conversion (see docs/marscript-language.md).

Three layers, each built and round-trip verified against the layer below it
before anything above it was trusted:
  1. A flow-tracing disassembler/reassembler for the CODE section's raw
     instruction stream (disassemble/reassemble) -- proven byte-exact
     against all 334 real scripts in tests/test_marscript_roundtrip.py.
  2. A surface-syntax compiler (compile_source) from readable marscript text
     down to tools/script_assembler.py's JSON IR -- tests/test_marscript_compiler.py.
  3. A pretty-printer (decompile_to_source) from a real script's bytecode
     back to readable marscript text -- proven, together with (2), to
     recompile every one of the 334 real scripts back to its exact original
     bytes in tests/test_marscript_decompile_roundtrip.py.
Run as a script (`python3 marscript.py decompile/compile ...`) for a
file-based CLI over layers 2/3.

Two header-relative bases exist in the real format and must not be
confused: FUNC relocation offsets (from script_events.py, already proven
against real scripts) are relative to the start of the CODE chunk payload
(stack_size + entry_offset + instructions). The script's own entry point is
relative to the start of the *instruction stream*, i.e. CODE payload + 6
(after the 4-byte stack_size and 2-byte entry_offset header fields) --
confirmed against real scripts where entry=0 lands exactly on the first
instruction. Every offset this module reports externally uses the FUNC
convention (CODE-payload-relative), matching script_events.py, with an
explicit +6 applied wherever the entry-relative header field is read or
written.
"""
import argparse
import json
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lz77
import script_events
import text_codec
from script_assembler import assemble, OPS
from extract_scrp_text import strings_in

# Opcode -> (mnemonic, operand-kind-string), the exact inverse of OPS. 0x80
# (native call, OPS's 'native_call') is fixed-width-5 per script_events.py's
# FIXED_WIDTHS and always emitted with a zeroed 4-byte field when built
# through the assembler's "native" convenience form; decoded here as a
# plain i32 so a real script's actual value (whatever it is) round-trips
# exactly rather than being assumed to be zero.
OPCODES = {opcode: (name, kinds) for name, (opcode, kinds) in OPS.items()}
# Write-side (mnemonic -> (opcode, kinds)); same table script_assembler.py
# itself emits from, so the low-level fallback syntax below never claims a
# shape that isn't already proven.
WRITABLE_OPS = dict(OPS)

OPERAND_WIDTH = {'operand': 1, 'u8': 1, 'u16': 2, 'i32': 4, 'target': 4}


class DecodeError(ValueError):
    pass


def unpack(blob):
    return lz77.decompress(blob)[0] if blob[0] == 0x10 else blob


def code_payload(raw):
    """Return (code_bytes, stack_size, header_field).

    `header_field` is the raw u16 at payload offset 4 -- NOT a resolved
    branch target. include/script.h's `struct ScrpHeader` only documents
    the outer SCRP/CODE framing; these two fields (a u32 this module calls
    stack_size and this u16) aren't named or explained anywhere in this
    project yet, because the script-loading C routine that reads them
    (distinct from the opcode-execution loop already decompiled in
    script_bytecode.c) hasn't been found. Across all 334 real scripts this
    field is only ever 0 or 1 (324 vs. 10) -- and real execution
    demonstrably starts at instruction-stream offset 0 in *both* cases (the
    first byte that decodes as a valid instruction is always there, never
    at stream offset 1). So whatever this field means, it is not "byte
    offset to begin execution" the way script_assembler.py's own `entry`
    JSON field is when *building* a new script from scratch -- that
    resemblance in the assembler's code is not evidence about how the
    original game's compiler used this field. Treated here as an opaque
    flag: preserved byte-for-byte, never interpreted as a target.
    """
    if raw[:4] != b'SCRP' or raw[8:12] != b'CODE':
        raise DecodeError('Expected SCRP/CODE container')
    size = struct.unpack_from('<I', raw, 12)[0]
    code = raw[16:16 + size]
    if len(code) != size:
        raise DecodeError('Truncated CODE payload')
    stack, header_field = struct.unpack_from('<IH', code, 0)
    return code, stack, header_field


def extra_chunks(raw, code_size):
    """Return the raw bytes of whatever chunk(s) sit between CODE and
    FUNC/TERM, verbatim -- empty for the overwhelming majority of real
    scripts, which go straight from CODE to FUNC (or TERM if they make no
    native calls at all). One real script (G_EV023.SPC) has been found
    with an NVAR chunk in this position; its internal structure isn't
    documented or interpreted anywhere in this project (script_events.py's
    calls() already treats it as opaque, skipped only to keep walking
    toward FUNC), so this preserves it byte-for-byte rather than guessing
    at its meaning -- the same "opaque but exact" treatment this module
    already gives the CODE header's own unexplained fields and truly
    undecodable instruction spans.

    Mirrors tools/script_events.py's calls() chunk walk exactly, including
    its NVAR quirk (an NVAR chunk's declared length field counts its own
    8-byte header, unlike CODE/FUNC which count payload bytes only) --
    raises DecodeError for any chunk tag that isn't NVAR, FUNC, or TERM,
    the same as calls() does, rather than silently swallowing bytes this
    project has never seen and doesn't understand.
    """
    start = 16 + code_size
    cursor = start
    while cursor + 4 <= len(raw):
        tag = raw[cursor:cursor + 4]
        if tag in (b'FUNC', b'TERM'):
            return bytes(raw[start:cursor])
        if tag != b'NVAR':
            raise DecodeError(f'Unsupported script chunk {tag!r} at {cursor}')
        if cursor + 8 > len(raw):
            raise DecodeError(f'Truncated NVAR chunk header at {cursor}')
        length = struct.unpack_from('<I', raw, cursor + 4)[0] - 8
        if length < 0 or cursor + 8 + length > len(raw):
            raise DecodeError(f'Truncated NVAR chunk at {cursor}')
        cursor += 8 + length
    raise DecodeError('SCRP container ends without a FUNC or TERM chunk')


def _fixed_operand(kind, code, offset):
    if kind in ('operand', 'u8'):
        return code[offset]
    if kind == 'u16':
        return struct.unpack_from('<H', code, offset)[0]
    if kind == 'i32':
        return struct.unpack_from('<i', code, offset)[0]
    if kind == 'target':
        # Stored instruction-stream-relative by the assembler's target()
        # helper (the same convention as the entry-point header field, see
        # code_payload()); +6 converts to this module's payload-relative
        # numbering, matching every other offset it reports.
        return struct.unpack_from('<I', code, offset)[0] + 6
    raise DecodeError('unknown operand kind ' + kind)


def disassemble_block(code, start, strings, visited, instructions, queue):
    """Walk one straight-line run from `start`, queuing branch continuations.

    `visited` marks byte offsets already decoded as instructions, so control
    flow that merges (a forward branch landing back on already-linear code)
    is only decoded once. Anything never reached from the entry point or a
    known branch is left undecoded on purpose -- it is very likely embedded
    data (string tables, SWITCH case tables reached only through a branch
    this pass hasn't followed yet), and guessing it is code would be exactly
    the kind of invention this project's standards forbid.
    """
    cursor = start
    while cursor < len(code):
        if cursor in visited:
            return
        opcode = code[cursor]
        if opcode == 0x10:
            if cursor + 3 > len(code):
                raise DecodeError(f'0x10 string block truncated at {cursor}')
            length = struct.unpack_from('<H', code, cursor + 1)[0]
            width = 3 + length
            if cursor + width > len(code):
                raise DecodeError(f'0x10 string block exceeds CODE at {cursor}')
            visited.update(range(cursor, cursor + width))
            instructions.append(dict(offset=cursor, op='.string', width=width,
                                     raw=bytes(code[cursor:cursor + width])))
            cursor += width
            continue
        if opcode == 0x15:
            # Opcode SWITCH (src/script_bytecode.c's ScriptCmdSwitch, fully
            # decompiled): reads an operand (u8) naming the register to test,
            # then a count (u8), then that many (candidate u32, destination
            # u32) entries -- jumps to the first entry whose candidate equals
            # the operand's resolved value, falls through if none match.
            # This module's byte layout was originally reverse-engineered
            # from real data alone (CH_M10.SPC: a dispatch target's +6 landed
            # exactly on an otherwise-unreachable instruction boundary) before
            # this authoritative C confirmation existed. That first pass
            # mislabeled the operand byte as unexplained/padding and
            # hardcoded it to 0 on write -- silently correct only because no
            # script in the current 334-script corpus happens to switch on a
            # nonzero register, not because the byte's meaning was actually
            # understood. Fixed here now that the real handler is known.
            if cursor + 3 > len(code):
                raise DecodeError(f'SWITCH truncated at {cursor}')
            switch_operand = code[cursor + 1]
            count = code[cursor + 2]
            width = 3 + count * 8
            if cursor + width > len(code):
                raise DecodeError(f'SWITCH exceeds CODE at {cursor}')
            entries = []
            for i in range(count):
                value, target = struct.unpack_from('<II', code, cursor + 3 + i * 8)
                entries.append((value, target + 6))
                if target + 6 not in visited:
                    queue.append(target + 6)
            visited.update(range(cursor, cursor + width))
            instructions.append(dict(offset=cursor, op='.switch', width=width,
                                     operand=switch_operand, entries=entries))
            cursor += width
            continue
        spec = OPCODES.get(opcode)
        if spec is None:
            raise DecodeError(f'Unrecognized opcode 0x{opcode:02X} at offset {cursor} '
                              f'reached from real control flow')
        name, kinds_str = spec
        kinds = [k for k in kinds_str.split(',') if k]
        width = 1 + sum(OPERAND_WIDTH[k] for k in kinds)
        if cursor + width > len(code):
            raise DecodeError(f'{name} at {cursor} exceeds CODE bounds')
        args = []
        pos = cursor + 1
        for kind in kinds:
            args.append(_fixed_operand(kind, code, pos))
            pos += OPERAND_WIDTH[kind]
        visited.update(range(cursor, cursor + width))
        instructions.append(dict(offset=cursor, op=name, args=args, width=width))
        if name in ('jump', 'jump_if_zero', 'call', 'address'):
            target = args[-1]
            if target not in visited:
                queue.append(target)
        if name in ('jump', 'return', 'restore_result'):
            # restore_result (0x8F) is confirmed a real terminator by direct
            # evidence, not assumption: it is consistently the last
            # instruction before code_length across real scripts (e.g.
            # BTIM00.SPC), and without stopping here the tracer walked past
            # it into two bytes of genuine trailing padding.
            return
        cursor += width


def disassemble(raw):
    """Decode a SCRP/CODE payload into a labeled instruction list.

    Only bytes actually reachable from the entry point (or a call/jump/
    address target reached transitively from it) are decoded. Everything
    else is reported separately as `unreached` spans, never guessed at.
    """
    code, stack, header_field = code_payload(raw)
    strings = dict(strings_in(code, lossless=True))
    visited = set(range(6))  # the 6-byte header is never re-decoded as code
    instructions = []
    queue = [6]  # real execution demonstrably starts here in every real script; see code_payload()
    seen_starts = set()
    while queue:
        start = queue.pop()
        if start in seen_starts or start in visited:
            continue
        seen_starts.add(start)
        try:
            disassemble_block(code, start, strings, visited, instructions, queue)
        except DecodeError:
            # Whatever this run decoded before failing is already recorded
            # (instructions/visited are mutated incrementally); leave the
            # rest of this run undecoded rather than aborting the whole
            # disassembly on one not-yet-understood pattern. It's picked up
            # by the unreached-span fallback below, opaque if it still
            # doesn't decode cleanly there either.
            continue
    instructions.sort(key=lambda i: i['offset'])
    all_offsets = {ins['offset']: ins for ins in instructions}
    # Contiguous spans no live control-flow path reaches.
    unreached = []
    prev_end = 6
    for start in sorted({ins['offset'] for ins in instructions}):
        if start > prev_end:
            unreached.append((prev_end, start))
        prev_end = max(prev_end, start + all_offsets[start]['width'])
    if prev_end < len(code):
        unreached.append((prev_end, len(code)))
    # Real scripts contain statically-unreachable but structurally valid
    # code (confirmed against CH_M03A.SPC: an unused dispatch-table jump
    # stub, byte-identical to two others the table does reference). These
    # bytes are still part of the file and must round-trip, so decode each
    # unreached span the same way as live code where it decodes cleanly
    # end-to-end, and only fall back to an opaque raw span where it
    # genuinely doesn't -- never guessing, always preserving exact bytes.
    still_unreached = []
    for start, end in unreached:
        span_instructions = []
        span_visited = set()
        cursor = start
        try:
            while cursor < end:
                before = len(span_instructions)
                disassemble_block(code, cursor, strings, span_visited, span_instructions, [])
                if len(span_instructions) == before:
                    raise DecodeError('no progress')
                cursor = max(i['offset'] + i['width'] for i in span_instructions)
            if cursor != end:
                raise DecodeError('did not land exactly on span end')
        except DecodeError:
            # Genuinely not decodable as instructions (e.g. zero-fill
            # alignment padding after restore_result). Preserved verbatim as
            # an opaque span so reassembly still reproduces these bytes
            # exactly -- reported in `unreached` for visibility, but very
            # much still part of the file.
            still_unreached.append((start, end))
            instructions.append(dict(offset=start, op='.raw', width=end - start,
                                     raw=bytes(code[start:end])))
            continue
        for ins in span_instructions:
            ins['orphaned'] = True
        instructions.extend(span_instructions)
    instructions.sort(key=lambda i: i['offset'])
    branch_targets = {ins['args'][-1] for ins in instructions
                      if ins['op'] in ('jump', 'jump_if_zero', 'call', 'address')}
    branch_targets.update(target for ins in instructions if ins['op'] == '.switch'
                          for _candidate, target in ins['entries'])
    return dict(stack_size=stack, header_field=header_field, instructions=instructions,
                branch_targets=branch_targets, code_length=len(code), unreached=still_unreached)


def reassemble(decoded):
    """Exact inverse of disassemble(): rebuild the CODE payload bytes.

    Labels are placed at every offset any instruction actually branches to,
    so this reproduces the original layout exactly when fed a diassembly
    that covered the whole reachable graph starting from the same entry.
    """
    instructions = sorted(decoded['instructions'], key=lambda i: i['offset'])
    out = bytearray(6)  # placeholder for stack_size/entry, filled at the end
    offset_map = {}
    for ins in instructions:
        offset_map[ins['offset']] = len(out)
        if ins['op'] in ('.string', '.raw'):
            # OP_SET_BYTECODE_ADDRESS (0x22, this module's 'address') commonly
            # targets a byte inside a string block's payload, not the block's
            # own start (see script.h: "the compiler uses [0x10] to skip
            # inline strings/data, then opcode 0x22 obtains the embedded
            # address"). Every byte the string block covers needs a mapping,
            # not just its first.
            base = len(out)
            for i in range(len(ins['raw'])):
                offset_map[ins['offset'] + i] = base + i
            out.extend(ins['raw'])
            continue
        if ins['op'] == '.switch':
            out.append(0x15)
            out.append(ins['operand'])
            out.append(len(ins['entries']))
            for value, target in ins['entries']:
                out.extend(struct.pack('<I', value))
                out.extend(struct.pack('<I', 0))  # patched below
            continue
        opcode, kinds_str = WRITABLE_OPS[ins['op']]
        out.append(opcode)
        kinds = [k for k in kinds_str.split(',') if k]
        for kind, value in zip(kinds, ins['args']):
            if kind in ('operand', 'u8'):
                out.append(value)
            elif kind == 'u16':
                out.extend(struct.pack('<H', value))
            elif kind == 'i32':
                out.extend(struct.pack('<i', value))
            elif kind == 'target':
                out.extend(struct.pack('<I', value))  # patched below
    # Patch branch targets now that every instruction's final position is known.
    cursor = 6
    for ins in instructions:
        if ins['op'] in ('.string', '.raw'):
            cursor += len(ins['raw'])
            continue
        if ins['op'] == '.switch':
            for i, (value, target) in enumerate(ins['entries']):
                if target not in offset_map:
                    raise DecodeError(f'switch target {target} was never decoded as an instruction')
                struct.pack_into('<I', out, cursor + 3 + i * 8 + 4, offset_map[target] - 6)
            cursor += ins['width']
            continue
        opcode, kinds_str = WRITABLE_OPS[ins['op']]
        kinds = [k for k in kinds_str.split(',') if k]
        pos = cursor + 1
        for kind, value in zip(kinds, ins['args']):
            if kind == 'target':
                if value not in offset_map:
                    raise DecodeError(f'branch target {value} was never decoded as an instruction')
                # Write back instruction-stream-relative, the inverse of the
                # +6 applied when this value was decoded in _fixed_operand().
                struct.pack_into('<I', out, pos, offset_map[value] - 6)
            pos += OPERAND_WIDTH[kind]
        cursor += ins['width']
    struct.pack_into('<IH', out, 0, decoded['stack_size'], decoded['header_field'])
    return bytes(out)


# ---------------------------------------------------------------------------
# Surface syntax: marscript text -> the JSON `document` shape
# tools/script_assembler.py's assemble() already accepts and has real,
# tested emission for (see docs/marscript-language.md for the language
# itself). This is deliberately a thin front end over that existing,
# already-verified bytecode emitter rather than a second code generator --
# lowering produces the same {"op": ..., "args": [...]} instruction dicts
# assemble() already knows how to encode.
#
# Scope, and why: only constructs whose native-call signature is confirmed
# elsewhere in this project are implemented. `move` (SprMove) and `wait` are
# deliberately NOT implemented -- SprMove is registered generically as
# ScriptNativeSpriteEffect with no independently-verified per-argument
# breakdown anywhere in this codebase (unlike SprInit/HitInit/FldSet, which
# all have one), and no real script has produced a confirmed wait/yield
# bytecode pattern. Compiling either would mean guessing a native's argument
# layout, which is exactly what this project's standards forbid.

class CompileError(ValueError):
    pass


_TOKEN_RE = re.compile(r"""
    (?P<ws>\s+)
  | (?P<comment>//[^\n]*)
  | (?P<string>"(?:[^"\\]|\\.)*")
  | (?P<hex>0[xX][0-9A-Fa-f]+)
  | (?P<int>-?\d+)
  | (?P<ident>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<op>==|!=|<=|>=|&&|\|\|)
  | (?P<punct>[{}():,.+\-=<>!&|^~*/%])
""", re.VERBOSE)


def _tokenize(text):
    tokens = []
    pos = 0
    while pos < len(text):
        m = _TOKEN_RE.match(text, pos)
        if not m:
            raise CompileError(f'unrecognized character {text[pos]!r} at byte {pos}')
        pos = m.end()
        kind = m.lastgroup
        if kind in ('ws', 'comment'):
            continue
        tokens.append((kind, m.group()))
    return tokens


class _Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.i = 0

    def peek(self):
        return self.tokens[self.i] if self.i < len(self.tokens) else (None, None)

    def next(self):
        tok = self.peek()
        if tok[0] is None:
            raise CompileError('unexpected end of source')
        self.i += 1
        return tok

    def expect(self, kind, value=None):
        tok = self.next()
        if tok[0] != kind or (value is not None and tok[1] != value):
            raise CompileError(f'expected {value or kind}, got {tok[1]!r}')
        return tok

    def expect_ident(self, value):
        return self.expect('ident', value)

    def at(self, kind, value=None):
        tok = self.peek()
        return tok[0] == kind and (value is None or tok[1] == value)

    def integer(self):
        kind, text = self.next()
        if kind == 'hex':
            return int(text, 16)
        if kind == 'int':
            return int(text)
        raise CompileError(f'expected an integer, got {text!r}')

    def string(self):
        kind, text = self.next()
        if kind != 'string':
            raise CompileError(f'expected a string literal, got {text!r}')
        return json.loads(text)  # marscript string syntax matches JSON string syntax

    def ident(self):
        kind, text = self.next()
        if kind != 'ident':
            raise CompileError(f'expected an identifier, got {text!r}')
        return text

    def signed_coordinate(self):
        # +N / -N (relative movement macro) or a plain integer (absolute).
        if self.at('punct', '+') or self.at('punct', '-'):
            sign = 1 if self.next()[1] == '+' else -1
            return ('relative', sign * self.integer())
        return ('absolute', self.integer())

    def register(self):
        # Raw register syntax: r0 .. r127 (operand encoding is one byte;
        # bit 7 has its own meaning per ScriptResolveOperand in
        # script_bytecode.c -- this module doesn't interpret that, it just
        # carries whatever operand byte value is written here through
        # exactly, matching how the disassembler decodes it).
        kind, text = self.next()
        if kind != 'ident' or not re.fullmatch(r'r\d+', text):
            raise CompileError(f'expected a register (e.g. r0), got {text!r}')
        value = int(text[1:])
        if not 0 <= value <= 255:
            raise CompileError(f'register operand {text!r} out of range')
        return value


class _ExpressionCompiler:
    """Compiles marscript expressions to instructions, using only opcodes
    with confirmed semantics from src/script_bytecode.c (see
    docs/decompiled-flags.md). Every opcode used here is a direct,
    already-decompiled VM command (BINARY_COMMAND-family `dest OP= src`, or
    the *_zero family testing a single operand against zero) -- nothing
    here is inferred from bytecode patterns alone.

    Every method returns a register holding the result, always a *fresh
    temporary* the caller owns and must free with free_temp() once done --
    never a variable's own register, since these opcodes are destructive
    (`dest OP= src`) and mutating a variable as a side effect of merely
    reading it in an expression would be wrong. A bare variable reference is
    therefore always copied into a new temp before use, not returned as-is.

    Temporaries allocate downward from register 14 (register 15 is reserved
    elsewhere for native-call stack setup, matching script_assembler.py's
    own use of it) while declared `var`s allocate upward from 0, so the two
    pools can never collide -- either running out is a real compile error,
    not silent corruption.
    """

    def __init__(self, parser, instructions, variables, next_register):
        self.p = parser
        self.instructions = instructions
        self.variables = variables
        self.next_register = next_register  # shared [int] cursor with the declaration parser
        self.next_temp = [14]
        self.free_temps = []

    def alloc_temp(self):
        if self.free_temps:
            return self.free_temps.pop()
        if self.next_temp[0] < self.next_register[0]:
            raise CompileError('expression needs more temporary registers than are free '
                               '(conflicts with declared variables)')
        r = self.next_temp[0]
        self.next_temp[0] -= 1
        return r

    def free_temp(self, register):
        self.free_temps.append(register)

    def emit(self, op, args):
        self.instructions.append(dict(op=op, args=args))

    def compile(self):
        return self.compile_or()

    def compile_or(self):
        left = self.compile_and()
        while self.p.at('op', '||'):
            self.p.next()
            right = self.compile_and()
            self.emit('logical_or', [left, right])
            self.free_temp(right)
        return left

    def compile_and(self):
        left = self.compile_comparison()
        while self.p.at('op', '&&'):
            self.p.next()
            right = self.compile_comparison()
            self.emit('logical_and', [left, right])
            self.free_temp(right)
        return left

    def compile_comparison(self):
        left = self.compile_additive()
        p = self.p
        if p.at('op') and p.peek()[1] in ('==', '!=', '<=', '>='):
            opname = p.next()[1]
            right = self.compile_additive()
            self.emit('sub', [left, right])
            self.free_temp(right)
            zero_test = {'==': 'eq_zero', '!=': 'ne_zero', '<=': 'le_zero', '>=': 'ge_zero'}[opname]
            self.emit(zero_test, [left])
            return left
        if p.at('punct', '<') or p.at('punct', '>'):
            opname = p.next()[1]
            right = self.compile_additive()
            if opname == '>':
                self.emit('sub', [left, right])
                self.free_temp(right)
                self.emit('gt_zero', [left])
                return left
            # No lt_zero opcode exists (confirmed: script_bytecode.c only
            # implements <=0, >0, >=0, ==0, !=0). a < b is derived as
            # (b - a) > 0 -- a real derivation from confirmed primitives,
            # not a guessed opcode.
            self.emit('sub', [right, left])
            self.free_temp(left)
            self.emit('gt_zero', [right])
            return right
        return left

    def compile_additive(self):
        left = self.compile_multiplicative()
        while self.p.at('punct', '+') or self.p.at('punct', '-'):
            opname = self.p.next()[1]
            right = self.compile_multiplicative()
            self.emit('add' if opname == '+' else 'sub', [left, right])
            self.free_temp(right)
        return left

    def compile_multiplicative(self):
        left = self.compile_unary()
        while self.p.at('punct', '*') or self.p.at('punct', '/') or self.p.at('punct', '%'):
            opname = self.p.next()[1]
            right = self.compile_unary()
            self.emit({'*': 'mul', '/': 'div', '%': 'mod'}[opname], [left, right])
            self.free_temp(right)
        return left

    def compile_unary(self):
        if self.p.at('punct', '!'):
            self.p.next()
            value = self.compile_unary()
            self.emit('logical_not', [value])
            return value
        if self.p.at('punct', '-'):
            self.p.next()
            value = self.compile_unary()
            self.emit('negate', [value])
            return value
        if self.p.at('punct', '~'):
            self.p.next()
            value = self.compile_unary()
            self.emit('bit_not', [value])
            return value
        return self.compile_primary()

    def compile_primary(self):
        p = self.p
        if p.at('punct', '('):
            p.next()
            value = self.compile()
            p.expect('punct', ')')
            return value
        if p.at('int') or p.at('hex'):
            temp = self.alloc_temp()
            self.emit('set_i32', [temp, p.integer()])
            return temp
        if p.at('ident'):
            name = p.ident()
            if name not in self.variables:
                raise CompileError(f'{name!r} is not a declared variable')
            temp = self.alloc_temp()
            self.emit('copy', [temp, self.variables[name]])
            return temp
        raise CompileError(f'expected an expression, got {p.peek()[1]!r}')



def compile_source(text):
    """Compile marscript source to the JSON document tools/script_assembler.py's
    assemble() accepts. Returns (document, script_name)."""
    tokens = _tokenize(text)
    p = _Parser(tokens)
    p.expect_ident('script')
    name = p.ident()
    p.expect('punct', '{')

    sprites = {}       # local name -> compiler-assigned sprite id
    hitregions = {}     # local name -> declared id
    variables = {}      # local name -> register (0-14)
    string_names = set()  # names declared via `string NAME: "..."`
    next_sprite_id = [0]
    next_register = [0]
    instructions = []
    header = {}  # optional stack_size/entry overrides, see the `stack_size`/`entry` statements below

    def alloc_register():
        if next_register[0] > 14:
            raise CompileError('script needs more live variables than the 15 available registers')
        r = next_register[0]
        next_register[0] += 1
        return r

    def native(fname, args, result=0):
        instructions.append(dict(op='native', name=fname, args=args, result=result))

    def resolve_sprite(local_name):
        if local_name not in sprites:
            raise CompileError(f'{local_name!r} is not a declared sprite')
        return sprites[local_name]

    def resolve_hitregion(local_name):
        if local_name not in hitregions:
            raise CompileError(f'{local_name!r} is not a declared hitregion')
        return hitregions[local_name]

    exprs = _ExpressionCompiler(p, instructions, variables, next_register)
    next_synthetic_label = [0]

    def synthetic_label(hint):
        next_synthetic_label[0] += 1
        return f'__{hint}_{next_synthetic_label[0]}'

    def parse_statement():
        kind, text = p.peek()
        if kind != 'ident':
            raise CompileError(f'expected a statement, got {text!r}')

        if text == 'sprite':
            p.next(); local_name = p.ident(); p.expect('punct', '='); p.expect_ident('spawn')
            p.expect('punct', '(')
            fields = {}
            while not p.at('punct', ')'):
                key = p.ident(); p.expect('punct', ':')
                if key == 'resource':
                    fields[key] = p.string()
                elif key == 'at':
                    fields['x'] = p.integer(); p.expect('punct', ','); fields['y'] = p.integer()
                else:
                    fields[key] = p.integer()
                if p.at('punct', ','):
                    p.next()
            p.expect('punct', ')')
            for required in ('container', 'resource', 'animation', 'x', 'y'):
                if required not in fields:
                    raise CompileError(f'sprite {local_name}: missing required field {required!r}')
            sprite_id = next_sprite_id[0]; next_sprite_id[0] += 1
            sprites[local_name] = sprite_id
            # `extra` (SprInit's 5th argument) has no confirmed meaning in this
            # codebase (see tools/map_editor/README.md: "Most sprite properties
            # remain undecoded") -- defaulting it to 0 is an explicit,
            # documented assumption, not a verified fact.
            native('SprInit', [sprite_id, fields['container'], {'string': fields['resource']},
                               fields['animation'], fields.get('extra', 0)])
            native('SprSet', [sprite_id, 0, fields['x']])
            native('SprSet', [sprite_id, 1, fields['y']])

        elif text == 'hitregion':
            p.next(); local_name = p.ident(); p.expect('punct', '='); p.expect_ident('region')
            p.expect('punct', '(')
            fields = {}
            while not p.at('punct', ')'):
                key = p.ident(); p.expect('punct', ':')
                if key == 'at':
                    fields['x'] = p.integer(); p.expect('punct', ','); fields['y'] = p.integer()
                elif key == 'size':
                    fields['width'] = p.integer(); p.expect('punct', ','); fields['height'] = p.integer()
                else:
                    fields[key] = p.integer()
                if p.at('punct', ','):
                    p.next()
            p.expect('punct', ')')
            for required in ('id', 'x', 'y', 'width', 'height'):
                if required not in fields:
                    raise CompileError(f'hitregion {local_name}: missing required field {required!r}')
            hitregions[local_name] = fields['id']
            native('HitInit', [fields['id'], fields['x'], fields['y'], fields['width'], fields['height']])

        elif text == 'var':
            p.next(); local_name = p.ident()
            register = alloc_register()
            variables[local_name] = register
            if p.at('punct', '='):
                p.next()
                temp = exprs.compile()
                instructions.append(dict(op='copy', args=[register, temp]))
                exprs.free_temp(temp)

        elif text == 'change':
            p.next(); local_name = p.ident()
            sprite_id = resolve_sprite(local_name)
            p.expect_ident('to'); p.expect_ident('resource')
            resource = p.string()
            p.expect_ident('animation')
            animation = p.integer()
            # Same undocumented-4th-argument caveat as SprInit's `extra`.
            native('SprChg', [sprite_id, 0, {'string': resource}, animation, 0])

        elif text == 'set':
            p.next(); local_name = p.ident()
            sprite_id = resolve_sprite(local_name)
            p.expect_ident('property'); prop = p.integer(); p.expect_ident('to')
            value = p.integer()
            native('SprSet', [sprite_id, prop, value])

        elif text == 'hit':
            p.next(); local_name = p.ident()
            region_id = resolve_hitregion(local_name)
            if p.at('ident', 'rect'):
                p.next(); p.expect_ident('at')
                x = p.integer(); p.expect('punct', ',')
                y = p.integer(); p.expect_ident('size')
                w = p.integer(); p.expect('punct', ',')
                h = p.integer()
                native('HitHitRect', [region_id, x, y, w, h])
            elif p.at('ident', 'set'):
                p.next()
                prop_name = p.ident(); p.expect_ident('to')
                value = p.integer()
                prop_numbers = dict(x=54, y=55, width=56, height=57, mode=61)
                if prop_name not in prop_numbers:
                    raise CompileError(f"hit ... set: {prop_name!r} is not one of HitSet's five "
                                       f"verified properties (x, y, width, height, mode)")
                native('HitSet', [region_id, prop_numbers[prop_name], value])
            else:
                raise CompileError('expected "rect" or "set" after a hitregion name')

        elif text == 'free':
            p.next()
            if p.at('ident', 'all'):
                p.next(); p.expect_ident('hitregions')
                native('HitFree', [-1])
            else:
                local_name = p.ident()
                region_id = resolve_hitregion(local_name)
                native('HitFree', [region_id])

        elif text == 'load':
            p.next(); p.expect_ident('field')
            field_name = p.string(); p.expect_ident('at')
            x = p.integer(); p.expect('punct', ','); y = p.integer()
            native('FldSet', [{'string': field_name}, x, y])

        elif text in ('call', 'chain'):
            p.next(); p.expect_ident('script')
            script_name = p.string()
            native(text, [{'string': script_name}])

        elif text == 'label':
            p.next(); label_name = p.ident(); p.expect('punct', ':')
            instructions.append(dict(label=label_name))

        elif text == 'goto':
            p.next(); label_name = p.ident()
            instructions.append(dict(op='jump', args=[label_name]))

        elif text == 'goto_if_zero':
            p.next()
            register = p.register(); p.expect('punct', ',')
            label_name = p.ident()
            instructions.append(dict(op='jump_if_zero', args=[register, label_name]))

        elif text == 'string':
            p.next(); local_name = p.ident(); p.expect('punct', ':')
            value = p.string()
            instructions.append(dict(label=local_name))
            instructions.append(dict(op='string', value=value))
            string_names.add(local_name)

        elif text == 'raw':
            # Opaque bytes that don't decode as instructions (e.g. the
            # zero-fill alignment padding real scripts sometimes have after
            # restore_result -- see docs/decompiled-flags.md). A hex string,
            # not interpreted, just carried through exactly on rebuild.
            p.next()
            hex_text = p.string()
            try:
                value = bytes.fromhex(hex_text)
            except ValueError:
                raise CompileError(f'raw {hex_text!r} is not valid hex')
            instructions.append(dict(op='raw', value=hex_text))

        elif text == 'address':
            p.next()
            register = p.register(); p.expect('punct', ',')
            label_name = p.ident()
            # A string's real address load points 3 bytes into its payload
            # (past the 0x10 opcode + u16 length header), confirmed against
            # real scripts this session -- see docs/decompiled-flags.md.
            # Addressing a plain code label (not a declared `string`) uses
            # the label as-is; that's a real, separate, confirmed use of
            # this opcode (ScriptCmdSetBytecodeAddress in script_bytecode.c
            # is a generic "load an address within this script's own
            # bytecode" instruction, not string-specific).
            target = f'{label_name}+3' if label_name in string_names else label_name
            instructions.append(dict(op='address', args=[register, target]))

        elif text == 'switch':
            p.next()
            register = p.register(); p.expect('punct', '{')
            entries = []
            while not p.at('punct', '}'):
                candidate = p.integer()
                p.expect('punct', '-'); p.expect('punct', '>')  # "->", matching native(...) -> rX
                label_name = p.ident()
                entries.append((candidate, label_name))
                if p.at('punct', ','):
                    p.next()
            p.expect('punct', '}')
            instructions.append(dict(op='switch', operand=register, entries=entries))

        elif text == 'native':
            p.next()
            fname = p.string(); p.expect('punct', '(')
            args = []
            while not p.at('punct', ')'):
                if p.at('string'):
                    args.append({'string': p.string()})
                elif p.at('ident') and re.fullmatch(r'r\d+', p.peek()[1]):
                    args.append({'register': p.register()})
                else:
                    args.append(p.integer())
                if p.at('punct', ','):
                    p.next()
            p.expect('punct', ')')
            result = 0
            if p.at('punct', '-'):
                p.next(); p.expect('punct', '>')
                result = p.register()
            native(fname, args, result)

        elif text == 'native_call':
            # Raw single-instruction form of opcode 0x80. Two shapes:
            # `native_call "NAME"` reproduces a real, FUNC-named call site
            # verbatim (registers a relocation against the immediately
            # preceding `push_i32 <argc>`, without also rebuilding that
            # call's own argument setup the way `native(...)` above does --
            # needed because the decoder can't always safely prove where a
            # real call's argument setup begins, see marscript.py's
            # _find_call_spans, but the relocation itself must never be
            # dropped). `native_call <int>` is the fully generic numeric
            # fallback for the (so far unobserved) case of a native_call
            # with no FUNC name at all, preserving whatever raw 4-byte
            # field value real bytecode actually has there.
            p.next()
            if p.at('string'):
                instructions.append(dict(op='native_call', name=p.string()))
            else:
                instructions.append(dict(op='native_call', args=[p.integer()]))

        elif kind == 'ident' and text in WRITABLE_OPS and text not in (
                'jump', 'jump_if_zero', 'call', 'return', 'native_call'):
            # Generic fallback for every other confirmed opcode
            # (arithmetic/bitwise/comparison/stack/frame ops) that doesn't
            # have its own dedicated statement form -- data-driven from the
            # same OPS table script_assembler.py itself emits from, so
            # nothing here claims a shape that isn't already proven.
            p.next()
            opcode, kinds_str = WRITABLE_OPS[text]
            arg_kinds = [k for k in kinds_str.split(',') if k]
            args = []
            for i, kind_name in enumerate(arg_kinds):
                if i > 0:
                    p.expect('punct', ',')
                if kind_name == 'operand':
                    args.append(p.register())
                elif kind_name == 'target':
                    args.append(p.ident())
                else:
                    args.append(p.integer())
            instructions.append(dict(op=text, args=args))

        elif text == 'if':
            p.next(); p.expect('punct', '(')
            cond = exprs.compile()
            p.expect('punct', ')')
            else_label = synthetic_label('if_else')
            end_label = synthetic_label('if_end')
            instructions.append(dict(op='jump_if_zero', args=[cond, else_label]))
            exprs.free_temp(cond)
            p.expect('punct', '{')
            parse_block()
            p.expect('punct', '}')
            has_else = p.at('ident', 'else')
            if has_else:
                instructions.append(dict(op='jump', args=[end_label]))
            instructions.append(dict(label=else_label))
            if has_else:
                p.next(); p.expect('punct', '{')
                parse_block()
                p.expect('punct', '}')
                instructions.append(dict(label=end_label))

        elif text == 'while':
            p.next()
            start_label = synthetic_label('while_start')
            end_label = synthetic_label('while_end')
            instructions.append(dict(label=start_label))
            p.expect('punct', '(')
            cond = exprs.compile()
            p.expect('punct', ')')
            instructions.append(dict(op='jump_if_zero', args=[cond, end_label]))
            exprs.free_temp(cond)
            p.expect('punct', '{')
            parse_block()
            p.expect('punct', '}')
            instructions.append(dict(op='jump', args=[start_label]))
            instructions.append(dict(label=end_label))

        elif text == 'wait':
            raise CompileError("'wait' is not implemented -- no real script has produced a "
                               "confirmed wait/yield bytecode pattern to compile it against "
                               "(see docs/marscript-language.md)")

        elif text == 'move':
            raise CompileError("'move' is not implemented -- SprMove's exact argument layout "
                               "is not independently verified anywhere in this codebase, unlike "
                               "SprInit/HitInit/FldSet (see docs/marscript-language.md)")

        elif text == 'end':
            p.next()
            instructions.append(dict(op='return'))

        elif text == 'stack_size':
            # Raw SCRP/CODE header field, opaque to this project (see
            # docs/decompiled-flags.md) -- only present so decompile_to_source()
            # can reproduce a real script's exact header bytes on recompile.
            # New hand-written scripts can omit this; assemble() defaults to 1024.
            p.next()
            header['stack_size'] = p.integer()

        elif text == 'entry':
            # The other raw header field (docs/decompiled-flags.md): confirmed
            # to always be 0 or 1 in real scripts and to never affect where
            # execution actually starts. Same purpose as stack_size above --
            # preserved for exact recompilation, not meaningful to write by hand.
            p.next()
            header['entry'] = p.integer()

        elif text == 'extra_chunks':
            # Raw bytes of whatever chunk(s) a real script has between CODE
            # and FUNC/TERM (see marscript.py's extra_chunks()) -- empty for
            # almost every real script, present (as an opaque, undocumented
            # NVAR chunk) for at least one (G_EV023.SPC). Same purpose as
            # stack_size/entry above: exact recompilation of a decoded real
            # script, not something a hand-written new script needs.
            p.next()
            header['extra_chunks_hex'] = p.string()

        else:
            raise CompileError(f'unknown statement {text!r}')

    def parse_block():
        while not p.at('punct', '}'):
            parse_statement()

    parse_block()
    p.expect('punct', '}')
    document = dict(version=1, instructions=instructions, **header)
    return document, name


# ---------------------------------------------------------------------------
# Decoder: disassembled instructions -> readable marscript text.
#
# Deliberately conservative about what it collapses into a nicer form.
# Native calls are the one pattern collapsed into a single readable
# statement (`native "NAME"(args)`) rather than left as raw push/pop
# instructions, since they're the single most common and most unreadable
# thing in real scripts -- everything else renders using the same
# low-level syntax the compiler already accepts, so it's provably
# round-trippable rather than a best-effort guess at prettiness.
#
# The collapse only covers a call's own dispatch sequence (the `add_i32
# r15,N` stack-allocation instruction through the trailing `pop`) -- not
# any string-literal setup that happens to precede it. A string argument
# renders as its own ordinary `string`/`address` statements right where
# they already are, and the call references whichever register that
# address ended up in. This sidesteps a genuinely ambiguous question (does
# a given preceding string block belong to *this* call, or to unrelated
# earlier code?) rather than guess at it.


def _find_call_spans(code, calls, instructions, branch_targets):
    """For each script_events.calls() entry, find the span this decoder
    will render as one collapsed `native` statement: from the `add_i32
    r15, count*4` stack-allocation instruction through the trailing pop,
    inclusive.

    Deliberately does NOT reuse script_events.py's own byte-pattern rfind
    search for this. That search only has to recover integer literal
    values, so it's safe for it to walk straight through an unrelated
    instruction sitting between the `add_i32` and the call's own pushes
    (real scripts do this -- a `jump` can sit right where a call's
    prologue would otherwise be, and script_events.py's symbolic
    executor silently treats an opcode it doesn't recognize as a no-op
    rather than rejecting the span). That's fine for value-recovery, but
    fatal here: collapsing a span across a real instruction would delete
    it from the rendered source rather than just rendering it less
    prettily. So this walks backward from `ref` only through instructions
    this module's own flow-tracing disassembler already decoded (proven
    byte-exact by test_marscript_roundtrip.py), accepting only the exact
    push/push_i32 shapes a call's own argument setup can consist of, and
    requiring the instruction immediately before those to be the matching
    `add_i32 r15, count*4` -- never a raw byte scan. Anything that doesn't
    fit this exactly is left uncollapsed for the ordinary per-instruction
    renderer to handle, safe but less pretty, rather than risk it.

    Also rejects a span that some other branch (a jump/switch elsewhere in
    the script) targets partway through -- confirmed to happen in real
    scripts (e.g. BTIM01_3.SPC, where a switch case lands on the second
    argument push of an otherwise-ordinary call's own setup). Collapsing
    a span like that would silently drop the label its target needs.
    """
    ordered = sorted(instructions, key=lambda i: i['offset'])
    by_offset = {ins['offset']: ins for ins in ordered}
    index_of = {ins['offset']: i for i, ins in enumerate(ordered)}
    spans = {}  # start_offset -> (end_offset, call, result_register)
    for call in calls:
        ref = call['offset']
        count = call['count']
        if len(call['decoded_arguments']) != count:
            continue
        argc_ins = by_offset.get(ref)
        if argc_ins is None or argc_ins['op'] != 'push_i32' or argc_ins['args'] != [count]:
            continue
        cursor_i = index_of[ref] - 1
        remaining = count
        while cursor_i >= 0 and remaining > 0:
            ins = ordered[cursor_i]
            if ins['op'] not in ('push', 'push_i32'):
                break
            remaining -= 1
            cursor_i -= 1
        if remaining != 0 or cursor_i < 0:
            continue
        prologue = ordered[cursor_i]
        if prologue['op'] != 'add_i32' or prologue['args'] != [15, count * 4]:
            continue
        start = prologue['offset']
        end = ref + 12  # push_i32 argcount (5) + native_call (5) + pop (2)
        if any(start < target < end for target in branch_targets):
            continue
        # script_assembler.py's native() form always emits this trailing
        # pop (opcode 0x2A at end-2, 1-byte register operand at end-1)
        # regardless of whether the source wrote `-> rX` -- an omitted
        # result clause defaults to r0, so the actual register must be
        # read back and rendered explicitly whenever it isn't 0, or a
        # nonzero real result register would silently become r0 on
        # recompile.
        result_register = code[end - 1]
        spans[start] = (end, call, result_register)
    return spans


def _render_arg(arg):
    if arg['kind'] == 'integer':
        return str(arg['value'])
    if arg['kind'] in ('string', 'address', 'operand'):
        register = arg.get('register')
        if register is not None:
            return f'r{register}'
    raise DecodeError(f'cannot render native-call argument {arg!r}')


def decompile_to_source(raw, script_name):
    """The inverse of compile_source(): disassembled bytecode -> readable
    marscript text. Always fully round-trippable (every rendered line uses
    syntax compile_source() already accepts) even where it isn't able to
    prettify something -- see the module docstring above.
    """
    decoded = disassemble(raw)
    code, stack_size, header_field = code_payload(raw)
    extra = extra_chunks(raw, len(code))
    calls = script_events.calls(unpack(raw))
    instructions = sorted(decoded['instructions'], key=lambda i: i['offset'])
    branch_targets = decoded['branch_targets']
    call_spans = _find_call_spans(code, calls, instructions, branch_targets)
    # Every native_call site's FUNC-relocated name, keyed by the native_call
    # instruction's own offset (call['offset'] is the *preceding*
    # push_i32(argc) instruction, 5 bytes earlier -- see script_events.py's
    # calls()). Used below so a call this pass can't safely collapse into a
    # pretty `native "NAME"(...)` line still keeps its relocation on
    # recompile, rather than silently becoming an unresolved call.
    call_name_by_native_call_offset = {call['offset'] + 5: call['function'] for call in calls}

    string_names = {}  # block offset -> synthetic name, for every .string block
    for ins in instructions:
        if ins['op'] == '.string':
            string_names[ins['offset']] = f'str_{ins["offset"]:X}'

    def label_name(offset):
        return f'L_{offset:X}'

    def render_target(offset):
        # A string's payload is always 3 bytes past its own block's start
        # (see docs/decompiled-flags.md); recognize that and reference the
        # string by name so `address` round-trips through the same +3
        # convention compile_source() already applies automatically,
        # instead of a plain numeric label a human would have to decode.
        string_offset = offset - 3
        if string_offset in string_names:
            return string_names[string_offset]
        return label_name(offset)

    lines = [f'script {script_name} {{',
             f'    stack_size {stack_size}',
             f'    entry {header_field}']
    if extra:
        lines.append(f'    extra_chunks "{extra.hex()}"')
    i = 0
    skip_until = -1
    while i < len(instructions):
        ins = instructions[i]
        offset = ins['offset']
        i += 1
        if offset < skip_until:
            continue
        if offset in branch_targets:
            lines.append(f'    label {label_name(offset)}:')

        if offset in call_spans:
            end, call, result_register = call_spans[offset]
            skip_until = end
            args = ', '.join(_render_arg(a) for a in call['decoded_arguments'])
            # Always written explicitly (even when 0, the default a bare
            # `native "NAME"(...)` with no `-> rX` clause would compile to)
            # so recompiling never silently changes a real nonzero result
            # register back to r0.
            lines.append(f'    native "{call["function"]}"({args}) -> r{result_register}')
            continue

        op = ins['op']
        if op == '.string':
            # Script strings are Shift-JIS with embedded engine control
            # codes (see tools/text_codec.py), not ASCII -- most real
            # dialogue text is Japanese. decode_lossless() is the same
            # codec extract_scrp_text.py already uses for this exact byte
            # layout, escaping control/private bytes as `<XX>` so the
            # result is both human-readable and exactly invertible via
            # text_codec.encode(); json.dumps then safely quotes it as a
            # marscript string literal (parsed back via json.loads, see
            # _Parser.string()).
            text = text_codec.decode_lossless(ins['raw'][3:-1])
            # ensure_ascii=False so real Japanese dialogue prints as actual
            # characters in the .marscript file instead of \uXXXX escapes
            # -- json.loads() (_Parser.string()) accepts either form, so
            # this is purely a readability choice, not a round-trip risk.
            lines.append(f'    string {string_names[offset]}: '
                         f'{json.dumps(text, ensure_ascii=False)}')
        elif op == '.raw':
            lines.append(f'    raw "{ins["raw"].hex()}"')
        elif op == '.switch':
            entries = ', '.join(f'{value} -> {label_name(target)}' for value, target in ins['entries'])
            lines.append(f'    switch r{ins["operand"]} {{ {entries} }}')
        elif op == 'jump':
            lines.append(f'    goto {label_name(ins["args"][0])}')
        elif op == 'jump_if_zero':
            register, target = ins['args']
            lines.append(f'    goto_if_zero r{register}, {label_name(target)}')
        elif op == 'address':
            register, target = ins['args']
            lines.append(f'    address r{register}, {render_target(target)}')
        elif op == 'return':
            lines.append('    end')
        elif op == 'native_call':
            # A native_call opcode reached outside any recognized call span
            # -- the dispatch-tail half of a call this pass declined to
            # collapse into a pretty `native "NAME"(...)` line (see
            # _find_call_spans). Its FUNC relocation must still be
            # preserved exactly -- an unresolved native_call would be
            # broken at runtime -- so this renders the named low-level
            # form whenever a real relocation names this site, and only
            # falls back to the raw numeric field for the (so far
            # unobserved) case of a genuinely unnamed one.
            call_name = call_name_by_native_call_offset.get(offset)
            if call_name is not None:
                lines.append(f'    native_call {json.dumps(call_name)}')
            else:
                lines.append(f'    native_call {ins["args"][0]}')
        else:
            spec = WRITABLE_OPS.get(op)
            if spec is None:
                raise DecodeError(f'no render rule for instruction {ins!r}')
            kinds = [k for k in spec[1].split(',') if k]
            rendered = []
            for kind, value in zip(kinds, ins['args']):
                if kind == 'operand':
                    rendered.append(f'r{value}')
                elif kind == 'target':
                    rendered.append(label_name(value))
                else:
                    rendered.append(str(value))
            lines.append(f'    {op} {", ".join(rendered)}')

    lines.append('}')
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)

    dec = sub.add_parser('decompile', help='SPC bytecode -> readable marscript text')
    dec.add_argument('input', type=Path)
    dec.add_argument('output', type=Path)
    dec.add_argument('--name', help='script name (default: input file stem)')

    comp = sub.add_parser('compile', help='marscript text -> SPC bytecode')
    comp.add_argument('input', type=Path)
    comp.add_argument('output', type=Path)
    comp.add_argument('--compress', action='store_true')

    args = parser.parse_args()
    if args.command == 'decompile':
        raw = unpack(args.input.read_bytes())
        name = args.name or args.input.stem
        source = decompile_to_source(raw, name)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(source, encoding='utf-8')
        print(f'wrote {args.output}: {len(source)} chars')
    else:
        document, name = compile_source(args.input.read_text(encoding='utf-8'))
        raw = assemble(document)
        data = lz77.compress(raw) if args.compress else raw
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(data)
        print(f'wrote {args.output}: {name}, {len(raw)} raw bytes, {len(data)} stored bytes')


if __name__ == '__main__':
    main()
