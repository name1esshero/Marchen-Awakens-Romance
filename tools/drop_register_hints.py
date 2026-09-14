#!/usr/bin/env python3
"""Remove forced-register hints that do not affect generated code.

Many functions in src/ pin locals to specific registers with
`register T x asm("rN")` or the `TARGET_REGISTER` macro. The decomp community
treats such hints as a red flag: they suggest the C does not genuinely compile
to the original instructions. Most of them are also simply unnecessary --
leftovers from an iteration that landed on the right C shape for other reasons.

This tool proves which hints are load-bearing. For each one it removes the pin,
recompiles the single translation unit with the project's real toolchain, and
keeps the removal only if the generated assembly is byte-identical. Removals are
applied cumulatively, because register allocation is global: a hint that looks
unnecessary on its own can become necessary once its neighbours are gone.

Usage:
    python3 tools/drop_register_hints.py src/foo.c [src/bar.c ...]
    python3 tools/drop_register_hints.py --dry-run src/foo.c
    python3 tools/drop_register_hints.py --all

A file is only ever left in a state whose assembly matches the baseline, so a
run can never break the byte-exact build. Always confirm with `make compare`.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# `register T name TARGET_REGISTER("r5") = init;` or the bare asm() spelling.
HINT = re.compile(r'\s*(?:TARGET_REGISTER\(\s*"r\d+"\s*\)|asm\s*\(\s*"r\d+"\s*\))')

# A scheduling fence: a whole statement built on an empty asm template, used to
# stop agbcc reordering the instructions around it. Files spell it directly as
# `asm volatile("" : "+r"(x));` and also hide it behind per-file macros.
FENCE = re.compile(r'^\s*asm\s+volatile\s*\(\s*""[^;]*\)\s*;?\s*$'
                   r'|^\s*asm\s*\(\s*""\s*:[^;]*\)\s*;\s*$')

# `#define NCD_ALLOCATION_BARRIER(value) asm volatile("" : "+r"(value))`
FENCE_MACRO_DEF = re.compile(
    r'^\s*#\s*define\s+(\w+)\s*\([^)]*\)\s*asm\s+volatile\s*\(\s*""', re.MULTILINE)


def fence_matcher(text):
    """A predicate matching fence statements, including this file's own macros."""
    names = FENCE_MACRO_DEF.findall(text)
    if not names:
        return FENCE.match
    call = re.compile(r'^\s*(?:%s)\s*\([^;]*\)\s*;\s*$' % '|'.join(names))
    return lambda line: FENCE.match(line) or call.match(line)

# Two translation units were built by the older agbcc snapshot; see the Makefile.
OLD_AGBCC = {'sound_m4a', 'sound_cgb_update'}


def compile_unit(relpath):
    """Compile one src/*.c the way the Makefile does; return its disassembly.

    The comparison has to be on machine code, not on the compiler's assembly
    text: a scheduling fence emits `.code 16` directives that vanish with it,
    so identical instructions would otherwise look like a difference.
    """
    stem = Path(relpath).stem
    cc1 = ROOT / 'tools/agbcc/bin' / ('old_agbcc' if stem in OLD_AGBCC else 'agbcc')
    flags = ['-O2', '-fhex-asm']
    if stem != 'libc_adapters':
        flags.insert(0, '-mthumb-interwork')
    with tempfile.TemporaryDirectory() as tmp:
        pre = Path(tmp) / 'unit.i'
        asm = Path(tmp) / 'unit.s'
        obj = Path(tmp) / 'unit.o'
        steps = [
            ['arm-none-eabi-cpp', '-nostdinc', '-undef', '-DAGBCC=1',
             '-Iinclude', '-Itools/agbcc/include', relpath, '-o', str(pre)],
            [str(cc1), *flags, str(pre), '-o', str(asm)],
            ['arm-none-eabi-as', '-mcpu=arm7tdmi', '-mthumb-interwork', '-I.',
             '-o', str(obj), str(asm)],
        ]
        for step in steps:
            if subprocess.run(step, cwd=ROOT,
                              capture_output=True, text=True).returncode != 0:
                return None
        dis = subprocess.run(['arm-none-eabi-objdump', '-d', str(obj)],
                             cwd=ROOT, capture_output=True, text=True)
        if dis.returncode != 0:
            return None
        # Drop the header lines, which name the temporary object file.
        return '\n'.join(dis.stdout.split('\n')[2:])


def strip_hint(line):
    """Drop the register pin from one declaration, and the now-pointless keyword."""
    stripped = HINT.sub('', line, count=1)
    if stripped == line:
        return line
    return re.sub(r'\bregister\s+', '', stripped, count=1)


def group_by_function(lines, indices):
    """Bucket line indices by the function body that encloses them.

    Register allocation is per-function, so hints only interact within one.
    Function bodies are found by brace depth: a top-level `{` opens one.
    """
    owner, depth, current = {}, 0, None
    for n, line in enumerate(lines):
        if depth == 0 and line.startswith('{'):
            current = n
        owner[n] = current
        depth += line.count('{') - line.count('}')
        if depth <= 0:
            depth, current = 0, None
    groups = {}
    for i in indices:
        groups.setdefault(owner.get(i), []).append(i)
    return list(groups.values())


def process(relpath, dry_run=False):
    full = ROOT / relpath
    original = full.read_text()
    lines = original.split('\n')

    baseline = compile_unit(relpath)
    if baseline is None:
        print(f'{relpath}: SKIPPED (does not compile standalone)')
        return 0

    is_fence = fence_matcher(original)

    # The macro definition itself is the mechanism, not a use site.
    sites = [i for i, line in enumerate(lines)
             if (HINT.search(line) and not line.lstrip().startswith('#define'))
             or (is_fence(line) and not line.lstrip().startswith('#define'))]

    def rewrite(state):
        full.write_text('\n'.join(l for l in state if l is not None))

    def replacement(i):
        """What line i becomes when its hint is dropped; None means delete."""
        if is_fence(lines[i]):
            return None                 # a fence is a whole statement
        dropped = strip_hint(lines[i])
        return dropped if dropped != lines[i] else lines[i]

    def accepts(indices):
        """Does dropping every hint in `indices` leave the assembly unchanged?"""
        trial = list(lines)
        for i in indices:
            trial[i] = replacement(i)
        rewrite(trial)
        return compile_unit(relpath) == baseline

    removed = kept = 0
    pending = list(sites)

    # Fences usually come in sets that only work as a set: dropping one while
    # its partner still pins the schedule changes the instruction order. Try
    # each function's fences together before falling back to one at a time.
    # Register allocation is per-function, so that is the right grouping.
    for group in group_by_function(lines, [i for i in pending
                                           if is_fence(lines[i])]):
        if accepts(group):
            for i in group:
                lines[i] = replacement(i)
            removed += len(group)
            pending = [i for i in pending if i not in set(group)]

    for i in pending:
        if replacement(i) == lines[i]:
            continue
        if accepts([i]):
            lines[i] = replacement(i)
            removed += 1
        else:
            kept += 1
    rewrite(lines)

    identical = compile_unit(relpath) == baseline
    if dry_run or not identical:
        full.write_text(original)
    if not identical:
        print(f'{relpath}: REVERTED (final assembly diverged)')
        return 0

    verb = 'would remove' if dry_run else 'removed'
    print(f'{relpath}: {verb} {removed}, {kept} load-bearing')
    return removed


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('files', nargs='*', help='paths under src/')
    parser.add_argument('--dry-run', action='store_true',
                        help='report what would be removed without editing')
    parser.add_argument('--all', action='store_true',
                        help='scan every src/*.c that contains a hint')
    args = parser.parse_args()

    targets = args.files
    if args.all:
        targets = sorted(
            str(p.relative_to(ROOT)) for p in (ROOT / 'src').glob('*.c')
            if any(HINT.search(l) or fence_matcher(p.read_text())(l)
                   for l in p.read_text().split('\n')))
    if not targets:
        parser.error('no files given; pass paths or --all')

    total = sum(process(t, args.dry_run) for t in targets)
    print(f'\ntotal hints removed: {total}')
    print('Now confirm the ROM with: make -j$(nproc) && make compare')
    return 0


if __name__ == '__main__':
    sys.exit(main())
