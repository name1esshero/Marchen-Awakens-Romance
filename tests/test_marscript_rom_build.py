"""Prove tools/marscript_rom_build.py's placement logic, without a full ROM
build: the no-override path must reproduce named_scripts.py's existing
output exactly, a same-size override must land at the original archive
slot unchanged in size, and an override that grows past its slot must
leave the original slot untouched and get routed to the expansion region
instead. tests/test_split_scripts_english.py and
tests/test_resource_catalog_english.py cover the other two stages this
feeds into; test_marscript_end_to_end_build below is the one test that
exercises the real toolchain across all three together.
"""
import json
import subprocess
import struct
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import marscript_rom_build as mrb
import named_scripts
import script_events


def manifest():
    return json.loads(named_scripts.MANIFEST.read_text())


class MarscriptRomBuildTests(unittest.TestCase):
    def test_no_override_matches_named_scripts_exactly(self):
        # A handful of scripts, mixing compressed/uncompressed, is enough to
        # prove the pass-through -- running all 334 is
        # test_marscript_decompile_roundtrip.py's job, not this one's.
        sample = manifest()[:15]
        per_script_output, expansions = mrb.plan(sample)
        self.assertEqual(expansions, {})
        for e in sample:
            original = (ROOT / e['path']).read_bytes()
            expected = script_events.build(
                named_scripts.rebuild(original, named_scripts.edits(ROOT / e['text'])),
                original, e['name'], ROOT)
            self.assertEqual(per_script_output[e['name']], expected)
            self.assertEqual(len(per_script_output[e['name']]), len(original))

    def _with_override(self, name, source, test):
        path = mrb.override_path(name)
        self.assertFalse(path.exists(), f'{path} already exists -- not overwriting a real override')
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(source, encoding='utf-8')
        try:
            test()
        finally:
            path.unlink()

    def test_same_size_override_lands_at_original_slot_unchanged_in_size(self):
        e = next(x for x in manifest() if x['name'] == 'BTOM00.SPC')
        original = (ROOT / e['path']).read_bytes()
        self.assertFalse(original[0] == 0x10, 'test assumes an uncompressed sample')
        import marscript
        source = marscript.decompile_to_source(original, 'BTOM00_SPC')

        def check():
            per_script_output, expansions = mrb.plan([e])
            self.assertEqual(expansions, {})
            self.assertEqual(len(per_script_output['BTOM00.SPC']), len(original))
            # No edits were actually made (straight decompile/recompile), so
            # this should reproduce the original bytes exactly -- the same
            # property test_marscript_decompile_roundtrip.py proves for the
            # whole corpus.
            self.assertEqual(per_script_output['BTOM00.SPC'], original)

        self._with_override('BTOM00.SPC', source, check)

    def test_growth_override_keeps_original_slot_and_routes_to_expansion(self):
        e = next(x for x in manifest() if x['name'] == 'BTOM00.SPC')
        original = (ROOT / e['path']).read_bytes()
        lines = ['script BTOM00_SPC {', '    stack_size 1024', '    entry 0']
        lines += ['    add r0, r0'] * 30
        lines += ['    restore_result', '}']
        source = '\n'.join(lines) + '\n'

        def check():
            per_script_output, expansions = mrb.plan([e])
            self.assertEqual(per_script_output['BTOM00.SPC'], original,
                             'the original archive slot must stay untouched when a script grows, '
                             'so nothing else in the ROM shifts')
            self.assertIn('BTOM00.SPC', expansions)
            self.assertGreater(len(expansions['BTOM00.SPC']), len(original))

        self._with_override('BTOM00.SPC', source, check)

    def test_compiled_bytes_rejects_mismatched_script_name(self):
        with self.assertRaises(ValueError):
            mrb.compiled_bytes('BTOM00.SPC', 'script SOME_OTHER_NAME { end }')


class MarscriptEndToEndBuildTest(unittest.TestCase):
    """The one test in this file that touches the real toolchain and does a
    real link, proving tools/marscript_rom_build.py,
    tools/split_scripts_english.py and tools/resource_catalog.py's
    build-english mode work together end to end -- not just in isolation.
    Skipped if the toolchain isn't available (e.g. a machine without
    arm-none-eabi-* installed), since it's a real build, not a unit test.
    """
    def test_growth_override_builds_and_resolves_correctly(self):
        import shutil
        if shutil.which('arm-none-eabi-as') is None or not (ROOT / 'baserom.gba').exists():
            self.skipTest('real toolchain / baserom.gba not available')

        e = next(x for x in manifest() if x['name'] == 'BTOM00.SPC')
        original = (ROOT / e['path']).read_bytes()
        lines = ['script BTOM00_SPC {', '    stack_size 1024', '    entry 0']
        lines += ['    add r0, r0'] * 30
        lines += ['    restore_result', '}']
        override = mrb.override_path('BTOM00.SPC')
        self.assertFalse(override.exists())
        override.parent.mkdir(parents=True, exist_ok=True)
        override.write_text('\n'.join(lines) + '\n', encoding='utf-8')
        try:
            subprocess.run([sys.executable, 'tools/marscript_rom_build.py'], cwd=ROOT, check=True)
            subprocess.run([sys.executable, 'tools/split_scripts_english.py'], cwd=ROOT, check=True)
            subprocess.run([sys.executable, 'tools/resource_catalog.py', 'build-english'], cwd=ROOT, check=True)

            expansion_names = json.loads((ROOT / 'build/english/scripts_expansion_manifest.json').read_text())
            self.assertEqual(expansion_names, ['BTOM00.SPC'])

            grown = (ROOT / 'build/english/scripts_expansion/BTOM00.SPC.bin').read_bytes()
            self.assertGreater(len(grown), len(original))
            self.assertEqual((ROOT / 'build/english/scripts/nfp/BTOM00.SPC.bin').read_bytes(), original)

            catalog_english = (ROOT / 'build/generated/resource_catalog_english.inc').read_text()
            self.assertIn('gMarscriptScript_BTOM00_SPC - 0x08000000', catalog_english)
            externs = (ROOT / 'build/generated/resource_catalog_english_externs.inc').read_text()
            self.assertIn('extern const u8 gMarscriptScript_BTOM00_SPC[];', externs)

            subprocess.run(['make', 'mar_english.gba', f'-j{max(1, (__import__("os").cpu_count() or 1))}'],
                           cwd=ROOT, check=True, capture_output=True, text=True)

            rom = (ROOT / 'mar_english.gba').read_bytes()
            symbol_line = next(l for l in (ROOT / 'build/english/mar_english.map').read_text().splitlines()
                               if 'gMarscriptScript_BTOM00_SPC' in l and '0x' in l)
            symbol_addr = int(symbol_line.split()[0], 16)
            expansion_off = symbol_addr - 0x08000000
            self.assertEqual(rom[expansion_off:expansion_off + len(grown)], grown)
            self.assertEqual(rom[e['rom_offset']:e['rom_offset'] + len(original)], original)

            cat_off = 0x1C0960 + 16 * next(i for i, r in enumerate(
                json.loads((ROOT / 'data/resource_catalog.json').read_text())['records'])
                if r['name'] == 'BTOM00.SPC')
            stored_offset = struct.unpack_from('<I', rom, cat_off + 12)[0]
            self.assertEqual(stored_offset, expansion_off)
        finally:
            override.unlink(missing_ok=True)
            subprocess.run([sys.executable, 'tools/marscript_rom_build.py'], cwd=ROOT, check=True)
            subprocess.run([sys.executable, 'tools/split_scripts_english.py'], cwd=ROOT, check=True)
            subprocess.run([sys.executable, 'tools/resource_catalog.py', 'build-english'], cwd=ROOT, check=True)


if __name__ == '__main__':
    unittest.main()
