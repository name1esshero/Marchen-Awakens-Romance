#!/usr/bin/env python3
"""Ask agbcc what it does with a given C shape.

Matching a function is mostly a search over C spellings that all mean the same
thing but compile differently. Running that search against the ROM is slow and
teaches you nothing reusable. This compiles a snippet on its own and prints the
Thumb it produces, so a shape can be checked in about a second, and so the
compiler's habits can be written down once instead of rediscovered.

The compiler is GNU C 2.9-arm-000512 (thumb-elf), driven exactly as the
Makefile drives it.

Usage:
    python3 tools/agbcc_probe.py path/to/snippet.c
    python3 tools/agbcc_probe.py -              # read the snippet from stdin
    python3 tools/agbcc_probe.py -f Name file.c # print only that function
    python3 tools/agbcc_probe.py --diff a.c b.c # compare two spellings
    python3 tools/agbcc_probe.py --bytes -f Name file.c   # compare real bytes

A snippet is ordinary C. Nothing is included for you, so either include what you
need or use the short integer typedefs the project uses; `--types` prepends
`typedef signed int s32;` and friends for one-off experiments.

By default this prints agbcc's own assembly *text*. That text is not the same
thing as the bytes that end up in the ROM: an inline-asm scheduling fence
emits `.code 16` directives that vanish along with it, and some instructions
have more than one valid textual spelling for the identical encoding (a
two-operand `adds r0, r1` and the equivalent three-operand `adds r0, r0, r1`
disassemble from the same halfword). Two real cases from one session: a fence
removal that looked like a difference in the text but assembled to identical
bytes, and a `SpriteResourceFindGroup` register swap that looked identical in
the text (`adds r0, r0, r1` vs `adds r0, r1, r0`) but is a genuinely different
encoding (`0x1840` vs `0x1808`) -- opposite mistakes, both caused by trusting
the text. Pass `--bytes` to assemble the output with `arm-none-eabi-as` and
compare `objdump -d` instead, whenever a text match or mismatch needs to be
trusted for a real decision (see docs/COMPILER_HINT_CLEANUP.md's "compare
machine code, not the compiler's assembly text").
"""
import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

TYPEDEFS = """typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
"""


def compile_snippet(source, old=False):
    """Return the Thumb assembly agbcc emits for this snippet, or None."""
    cc1 = ROOT / 'tools/agbcc/bin' / ('old_agbcc' if old else 'agbcc')
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / 'probe.c'
        pre = Path(tmp) / 'probe.i'
        asm = Path(tmp) / 'probe.s'
        src.write_text(source)
        cpp = subprocess.run(
            ['arm-none-eabi-cpp', '-nostdinc', '-undef', '-DAGBCC=1',
             '-Iinclude', '-Itools/agbcc/include', str(src), '-o', str(pre)],
            cwd=ROOT, capture_output=True, text=True)
        if cpp.returncode != 0:
            return None, cpp.stderr
        cc = subprocess.run([str(cc1), '-mthumb-interwork', '-O2', '-fhex-asm',
                             str(pre), '-o', str(asm)],
                            cwd=ROOT, capture_output=True, text=True)
        if cc.returncode != 0:
            return None, cc.stderr
        return asm.read_text(), cc.stderr


def assemble_snippet(source, old=False):
    """Compile and assemble a snippet; return objdump -d output, or None.

    This is what actually lands in the ROM, unlike compile_snippet's assembly
    *text*: assembling with arm-none-eabi-as and reading the real bytes back
    with objdump collapses fence-only text differences (`.code 16` directives
    with nothing else changed) and, in the other direction, catches same-
    looking-text instructions that are genuinely different encodings. See
    tools/drop_register_hints.py's compile_unit, which this mirrors.
    """
    cc1 = ROOT / 'tools/agbcc/bin' / ('old_agbcc' if old else 'agbcc')
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / 'probe.c'
        pre = Path(tmp) / 'probe.i'
        asm = Path(tmp) / 'probe.s'
        obj = Path(tmp) / 'probe.o'
        src.write_text(source)
        steps = [
            ['arm-none-eabi-cpp', '-nostdinc', '-undef', '-DAGBCC=1',
             '-Iinclude', '-Itools/agbcc/include', str(src), '-o', str(pre)],
            [str(cc1), '-mthumb-interwork', '-O2', '-fhex-asm',
             str(pre), '-o', str(asm)],
            ['arm-none-eabi-as', '-mcpu=arm7tdmi', '-mthumb-interwork', '-I.',
             '-o', str(obj), str(asm)],
        ]
        for step in steps:
            r = subprocess.run(step, cwd=ROOT, capture_output=True, text=True)
            if r.returncode != 0:
                return None, r.stderr
        dis = subprocess.run(['arm-none-eabi-objdump', '-d', str(obj)],
                             cwd=ROOT, capture_output=True, text=True)
        if dis.returncode != 0:
            return None, dis.stderr
        return dis.stdout, ''


BYTE_LINE = re.compile(r'^\s*[0-9a-f]+:\s+([0-9a-f ]+)\t(.*)$')


def byte_instructions(objdump_output, function=None):
    """Reduce objdump -d text to (hex bytes, mnemonic) pairs worth comparing.

    Keeps the encoded bytes so two textually-identical-looking mnemonics that
    are actually different instructions (or vice versa) show up as different
    (or identical) here, which is the whole point of --bytes.
    """
    out, inside = [], function is None
    target = f'<{function}>:' if function else None
    for line in objdump_output.split('\n'):
        if target is not None:
            if line.endswith(target):
                inside = True
                continue
            if inside and re.match(r'^[0-9a-f]+ <.*>:$', line):
                inside = False
        if not inside:
            continue
        m = BYTE_LINE.match(line)
        if not m:
            continue
        raw_bytes, text = m.group(1).split(), m.group(2)
        # Drop the PC-relative "@ (offset <label>)" annotation: it repeats
        # the byte-address (not meaning) and shifts if the snippet is placed
        # at a different address than the ROM, which is not a real mismatch.
        text = re.sub(r'\t@ \(.*\)$', '', text).strip()
        out.append((''.join(raw_bytes), text))
    return out


def instructions(asm, function=None):
    """Strip directives and labels down to the instructions worth comparing."""
    out, inside = [], function is None
    for line in asm.split('\n'):
        if function is not None:
            if line.startswith(function + ':'):
                inside = True
                continue
            if inside and re.match(r'^\w+:', line) and not line.startswith('.L'):
                inside = False
        if not inside:
            continue
        stripped = line.strip()
        if not stripped or stripped.startswith('@'):
            continue
        if stripped.startswith('.') and not stripped.startswith('.L'):
            continue
        out.append(stripped)
    return out


def read(path):
    return sys.stdin.read() if path == '-' else Path(path).read_text()


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('files', nargs='+', help='snippet paths, or - for stdin')
    parser.add_argument('-f', '--function', help='show only this function')
    parser.add_argument('--diff', action='store_true',
                        help='compare the first two snippets instead of printing')
    parser.add_argument('--types', action='store_true',
                        help='prepend the project s8/u32/... typedefs')
    parser.add_argument('--old', action='store_true',
                        help='use old_agbcc, as sound_m4a.c does')
    parser.add_argument('--bytes', action='store_true',
                        help='compare real assembled bytes (objdump -d) '
                             'instead of agbcc\'s assembly text; slower, '
                             'but the only trustworthy answer for a real '
                             'match/mismatch decision -- see the module '
                             'docstring')
    args = parser.parse_args()

    def build(path):
        source = read(path)
        if args.types:
            source = TYPEDEFS + source
        if args.bytes:
            out, err = assemble_snippet(source, args.old)
            if out is None:
                print(f'{path}: failed to assemble\n{err}', file=sys.stderr)
                sys.exit(1)
            pairs = byte_instructions(out, args.function)
            return [f'{b}  {t}' for b, t in pairs]
        asm, err = compile_snippet(source, args.old)
        if asm is None:
            print(f'{path}: failed to compile\n{err}', file=sys.stderr)
            sys.exit(1)
        return instructions(asm, args.function)

    if args.diff:
        if len(args.files) < 2:
            parser.error('--diff needs two snippets')
        left, right = build(args.files[0]), build(args.files[1])
        if left == right:
            print('identical' + (' (real bytes)' if args.bytes else ''))
            return 0
        import difflib
        print('\n'.join(difflib.unified_diff(
            left, right, args.files[0], args.files[1], lineterm='')))
        return 1

    for path in args.files:
        if len(args.files) > 1:
            print(f'=== {path}')
        print('\n'.join(build(path)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
