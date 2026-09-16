"""Prove tools/split_scripts_english.py's two generated outputs are correct,
without a full build: the English script-assets variant must be
structurally identical to the checked-in base file except for its incbin
paths, and the expansion asm must declare exactly one symbol per script in
the expansion manifest.
"""
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import split_scripts_english as sse


class SplitScriptsEnglishTests(unittest.TestCase):
    def test_english_script_assets_mirrors_base_structure(self):
        base = sse.BASE_SCRIPT_ASSETS.read_text()
        sse.write_script_assets_english()
        english = sse.ENGLISH_SCRIPT_ASSETS.read_text()

        base_sections = [line for line in base.splitlines() if line.strip().startswith('.section .rom.')]
        english_sections = [line for line in english.splitlines() if line.strip().startswith('.section .rom.')]
        self.assertEqual(base_sections, english_sections,
                         'every original ROM section/address must be reproduced exactly, '
                         'or something in the base ROM layout would shift')

        base_incbins = [line for line in base.splitlines() if '.incbin' in line]
        english_incbins = [line for line in english.splitlines() if '.incbin' in line]
        self.assertEqual(len(base_incbins), len(english_incbins))
        self.assertTrue(all('build/scripts/nfp/' in line for line in base_incbins))
        self.assertTrue(all('build/english/scripts/nfp/' in line for line in english_incbins))
        # Same filenames, only the directory differs.
        base_files = [line.split('build/scripts/nfp/')[1] for line in base_incbins]
        english_files = [line.split('build/english/scripts/nfp/')[1] for line in english_incbins]
        self.assertEqual(base_files, english_files)

    def test_expansion_asm_declares_one_symbol_per_expanded_script(self):
        sse.EXPANSION_MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        original = sse.EXPANSION_MANIFEST.read_text() if sse.EXPANSION_MANIFEST.exists() else None
        try:
            sse.EXPANSION_MANIFEST.write_text(json.dumps(['FOO.SPC', 'BAR-1.SPC']))
            sse.write_expansion_asm()
            text = sse.EXPANSION_ASM.read_text()
            self.assertIn('.global gMarscriptScript_FOO_SPC', text)
            self.assertIn('gMarscriptScript_FOO_SPC:', text)
            self.assertIn('.incbin "build/english/scripts_expansion/FOO.SPC.bin"', text)
            self.assertIn('.global gMarscriptScript_BAR_1_SPC', text)
        finally:
            if original is not None:
                sse.EXPANSION_MANIFEST.write_text(original)
            else:
                sse.EXPANSION_MANIFEST.unlink(missing_ok=True)
            sse.write_expansion_asm()

    def test_expansion_asm_is_empty_but_valid_with_no_expansions(self):
        original = sse.EXPANSION_MANIFEST.read_text() if sse.EXPANSION_MANIFEST.exists() else None
        try:
            sse.EXPANSION_MANIFEST.write_text(json.dumps([]))
            sse.write_expansion_asm()
            text = sse.EXPANSION_ASM.read_text()
            self.assertNotIn('.global', text)
            self.assertNotIn('.incbin', text)
        finally:
            if original is not None:
                sse.EXPANSION_MANIFEST.write_text(original)
            else:
                sse.EXPANSION_MANIFEST.unlink(missing_ok=True)
            sse.write_expansion_asm()


if __name__ == '__main__':
    unittest.main()
