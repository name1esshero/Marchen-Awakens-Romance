"""Prove marscript's disassembler/reassembler round-trips real scripts exactly.

This is deliberately the first and only test written before any marscript
surface syntax exists: disassemble a real script's CODE payload, reassemble
it with no edits, and the bytes must match exactly. A failure here means
the instruction-level model is wrong before a single line of human-readable
syntax gets built on top of it.
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


class MarscriptRoundtripTests(unittest.TestCase):
    def test_disassemble_reassemble_matches_original_bytes(self):
        manifest = json.loads((ROOT / 'scripts/nfp/manifest.json').read_text())
        mismatches = []
        errors = []
        exact = 0
        for e in manifest:
            blob = (ROOT / e['path']).read_bytes()
            raw = lz77.decompress(blob)[0] if blob[0] == 0x10 else blob
            code, stack, entry = marscript.code_payload(raw)
            try:
                decoded = marscript.disassemble(raw)
            except marscript.DecodeError as ex:
                errors.append((e['name'], str(ex)))
                continue
            try:
                rebuilt = marscript.reassemble(decoded)
            except marscript.DecodeError as ex:
                errors.append((e['name'], 'reassemble: ' + str(ex)))
                continue
            if rebuilt == code:
                exact += 1
            else:
                mismatches.append((e['name'], len(code), len(rebuilt), len(decoded['unreached'])))
        print(f"\nmarscript round-trip: {exact}/{len(manifest)} exact, "
              f"{len(mismatches)} mismatched, {len(errors)} decode/encode errors")
        if errors:
            print("first errors:")
            for name, msg in errors[:10]:
                print(f"  {name}: {msg}")
        if mismatches:
            print("first mismatches (name, original_len, rebuilt_len, unreached_span_count):")
            for row in mismatches[:10]:
                print(f"  {row}")
        self.assertEqual(errors, [])
        self.assertEqual(mismatches, [])
        self.assertEqual(exact, len(manifest))


if __name__ == '__main__':
    unittest.main()
