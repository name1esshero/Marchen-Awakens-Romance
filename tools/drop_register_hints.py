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

# Two translation units were built by the older agbcc snapshot; see the Makefile.
OLD_AGBCC = {'sound_m4a', 'sound_cgb_update'}


def compile_unit(relpath):
    """Compile one src/*.c the way the Makefile does; return its assembly text."""
    stem = Path(relpath).stem
    cc1 = ROOT / 'tools/agbcc/bin' / ('old_agbcc' if stem in OLD_AGBCC else 'agbcc')
    flags = ['-O2', '-fhex-asm']
    if stem != 'libc_adapters':
        flags.insert(0, '-mthumb-interwork')
    with tempfile.TemporaryDirectory() as tmp:
        pre, asm = Path(tmp) / 'unit.i', Path(tmp) / 'unit.s'
        cpp = subprocess.run(
            ['arm-none-eabi-cpp', '-nostdinc', '-undef', '-DAGBCC=1',
             '-Iinclude', '-Itools/agbcc/include', relpath, '-o', str(pre)],
            cwd=ROOT, capture_output=True, text=True)
        if cpp.returncode != 0:
            return None
        cc = subprocess.run([str(cc1), *flags, str(pre), '-o', str(asm)],
                            cwd=ROOT, capture_output=True, text=True)
        if cc.returncode != 0:
            return None
        return asm.read_text()


def strip_hint(line):
    """Drop the register pin from one declaration, and the now-pointless keyword."""
    stripped = HINT.sub('', line, count=1)
    if stripped == line:
        return line
    return re.sub(r'\bregister\s+', '', stripped, count=1)


def process(relpath, dry_run=False):
    full = ROOT / relpath
    original = full.read_text()
    lines = original.split('\n')

    baseline = compile_unit(relpath)
    if baseline is None:
        print(f'{relpath}: SKIPPED (does not compile standalone)')
        return 0

    # The macro definition itself is the mechanism, not a use site.
    sites = [i for i, line in enumerate(lines)
             if HINT.search(line) and not line.lstrip().startswith('#define')]

    removed = kept = 0
    for i in sites:
        pinned = lines[i]
        unpinned = strip_hint(pinned)
        if unpinned == pinned:
            continue
        lines[i] = unpinned
        full.write_text('\n'.join(lines))
        if compile_unit(relpath) == baseline:
            removed += 1
        else:
            lines[i] = pinned
            full.write_text('\n'.join(lines))
            kept += 1

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
            if HINT.search(p.read_text()))
    if not targets:
        parser.error('no files given; pass paths or --all')

    total = sum(process(t, args.dry_run) for t in targets)
    print(f'\ntotal hints removed: {total}')
    print('Now confirm the ROM with: make -j$(nproc) && make compare')
    return 0


if __name__ == '__main__':
    sys.exit(main())
