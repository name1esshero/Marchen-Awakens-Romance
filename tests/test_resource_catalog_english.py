"""Prove tools/resource_catalog.py's build_english() mode: unaffected
records must be byte-identical to the base build() output, and a record
whose script was routed to the expansion region must resolve as a linker
symbol expression instead of a literal, with a matching extern declaration.
"""
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import resource_catalog as rc


class ResourceCatalogEnglishTests(unittest.TestCase):
    def _with_expansion_manifest(self, names, test):
        original = rc.EXPANSION_MANIFEST.read_text() if rc.EXPANSION_MANIFEST.exists() else None
        rc.EXPANSION_MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        rc.EXPANSION_MANIFEST.write_text(json.dumps(names))
        try:
            test()
        finally:
            if original is not None:
                rc.EXPANSION_MANIFEST.write_text(original)
            else:
                rc.EXPANSION_MANIFEST.unlink(missing_ok=True)

    def test_no_expansions_matches_base_build_exactly(self):
        rc.build()
        base = rc.OUTPUT.read_text()

        def check():
            rc.build_english()
            self.assertEqual(rc.ENGLISH_OUTPUT.read_text(), base)
            self.assertEqual(rc.ENGLISH_EXTERNS_OUTPUT.read_text(), '')

        self._with_expansion_manifest([], check)

    def test_expanded_record_uses_symbol_expression(self):
        payload = json.loads(rc.METADATA.read_text())
        some_name = payload['records'][100]['name']

        def check():
            rc.build_english()
            text = rc.ENGLISH_OUTPUT.read_text()
            ident = rc.marscript_identifier(some_name)
            self.assertIn(f'(u32)gMarscriptScript_{ident} - 0x08000000', text)
            self.assertNotIn(f'"{some_name}", 0x', text)
            externs = rc.ENGLISH_EXTERNS_OUTPUT.read_text()
            self.assertEqual(externs, f'extern const u8 gMarscriptScript_{ident}[];\n')

            # Every record other than this one must still carry its
            # original literal -- expansion must never leak to unrelated
            # entries.
            rc.build()
            base_lines = rc.OUTPUT.read_text().splitlines()
            english_lines = text.splitlines()
            self.assertEqual(len(base_lines), len(english_lines))
            differing = [i for i in range(len(base_lines)) if base_lines[i] != english_lines[i]]
            self.assertEqual(len(differing), 1)

        self._with_expansion_manifest([some_name], check)

    def test_marscript_identifier_matches_marscript_rom_build(self):
        # Both this module and marscript_rom_build.py derive a marscript
        # script-name identifier from a manifest name independently; they
        # must agree, or a script's expansion symbol name here wouldn't
        # match what compile_source() actually produces/expects.
        import marscript_rom_build as mrb
        for name in ['BTOM04_4.SPC', 'CH_M01_3.SPC', 'plain.SPC']:
            self.assertEqual(rc.marscript_identifier(name), mrb.marscript_identifier(name))


if __name__ == '__main__':
    unittest.main()
