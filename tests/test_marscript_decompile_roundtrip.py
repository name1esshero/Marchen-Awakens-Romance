"""Prove marscript's decoder (bytecode -> readable text) is safe to trust.

test_marscript_roundtrip.py already proves the instruction-level model
(disassemble/reassemble) is byte-exact for all 334 real scripts.
test_marscript_compiler.py proves compile_source() (readable text ->
bytecode) is correct for every construct it supports, by self-consistency.

This test closes the last gap: decompile_to_source() (bytecode -> readable
text) must produce text that, when fed back through compile_source() and
reassembled, reconstructs the exact original bytes. That's the same
methodology the user asked for when this was still being designed --
convert real scripts, then prove a "recompile" gives back the original --
now run for real against the whole corpus instead of by hand on a few
samples.

Deliberately compares the *entire* rebuilt SCRP container against the
original, not just the CODE chunk payload -- comparing only the CODE
payload once let a real bug through silently: a native_call site the
decoder couldn't safely collapse into a pretty `native "NAME"(...)` line
rendered as a bare opcode with no FUNC relocation at all, so the CODE
bytes still matched exactly while the call's name silently vanished from
the FUNC chunk, which would have made the call unresolved at runtime. A
whole-file comparison is the only one immune to that specific way of
"passing" while quietly breaking a different chunk.

"Entire" means the logical SCRP container (its own header + CODE + FUNC +
TERM, sized by the header's own declared chunks-size field), not
necessarily the whole on-disk file: real files in scripts/nfp/ carry
trailing zero-byte padding past that logical end (archive-slot alignment,
confirmed against real files -- unrelated to script content, and not this
module's concern), so the file can legitimately be a few bytes longer than
the SCRP container reconstructed from it. Checked explicitly below rather
than assumed.
"""
import json
import struct
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import lz77
import marscript
from script_assembler import assemble


class MarscriptDecompileRoundtripTests(unittest.TestCase):
    def test_decompile_then_recompile_matches_original_bytes(self):
        manifest = json.loads((ROOT / 'scripts/nfp/manifest.json').read_text())
        mismatches = []
        errors = []
        exact = 0
        for e in manifest:
            blob = (ROOT / e['path']).read_bytes()
            raw = lz77.decompress(blob)[0] if blob[0] == 0x10 else blob
            name = Path(e['name']).stem.replace('.', '_').replace('-', '_')
            try:
                source = marscript.decompile_to_source(raw, name)
            except marscript.DecodeError as ex:
                errors.append((e['name'], 'decompile: ' + str(ex)))
                continue
            try:
                document, parsed_name = marscript.compile_source(source)
            except marscript.CompileError as ex:
                errors.append((e['name'], 'compile_source: ' + str(ex)))
                continue
            self.assertEqual(parsed_name, name)
            try:
                rebuilt_raw = assemble(document)
            except ValueError as ex:
                errors.append((e['name'], 'assemble: ' + str(ex)))
                continue
            logical_len = 8 + struct.unpack_from('<I', raw, 4)[0]
            trailing_padding = raw[logical_len:]
            if trailing_padding.strip(b'\0'):
                errors.append((e['name'], f'unexpected non-zero trailing bytes past the '
                               f'declared SCRP size: {trailing_padding!r}'))
                continue
            if rebuilt_raw == raw[:logical_len]:
                exact += 1
            else:
                mismatches.append((e['name'], logical_len, len(rebuilt_raw)))
        print(f"\nmarscript decompile round-trip: {exact}/{len(manifest)} exact, "
              f"{len(mismatches)} mismatched, {len(errors)} decode/compile/assemble errors")
        if errors:
            print("first errors:")
            for entry_name, msg in errors[:10]:
                print(f"  {entry_name}: {msg}")
        if mismatches:
            print("first mismatches (name, original_len, rebuilt_len):")
            for row in mismatches[:10]:
                print(f"  {row}")
        self.assertEqual(errors, [])
        self.assertEqual(mismatches, [])
        self.assertEqual(exact, len(manifest))


if __name__ == '__main__':
    unittest.main()
