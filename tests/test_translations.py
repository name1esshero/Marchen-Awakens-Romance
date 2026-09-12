"""Translation lookup must preserve source bytes and scene-specific context."""
import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import translate_comments


class TranslationTest(unittest.TestCase):
    def test_blank_color_records_are_not_counted_as_translations(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/'text/nfp').mkdir(parents=True)
            (root/'text/translation').mkdir()
            (root/'text/translation/english.json').write_text('{}')
            path=root/'text/nfp/BLANK.SPC.txt'
            original='@000001  C0D04 　C0F04   // TODO: English translation\n@000020  C0F04 未翻訳  // TODO: English translation\n'
            path.write_text(original)
            with patch.object(translate_comments,'ROOT',root),contextlib.redirect_stdout(io.StringIO()):
                translate_comments.main()
                first=path.read_text()
                translate_comments.main()
            self.assertEqual(first,path.read_text())
            self.assertIn('// FORMAT:',first.splitlines()[0])
            self.assertIn('// TODO:',first.splitlines()[1])
            self.assertNotIn('// EN:',first)
            self.assertEqual([l.partition('  //')[0] for l in first.splitlines()],
                             [l.partition('  //')[0] for l in original.splitlines()])

    def test_explicit_empty_particle_reaches_runtime_without_placeholder(self):
        import build_english
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'text/nfp').mkdir(parents=True)
            (root/'text/translation').mkdir()
            (root/'text/translation/english.json').write_text('{}')
            (root/'text/translation/scripts.json').write_text(json.dumps({'ITEM.SPC': {'を': ''}}))
            path = root/'text/nfp/ITEM.SPC.txt'
            original = '@000001 を  // TODO: English translation\n@000020 未翻訳  // TODO: English translation\n'
            path.write_text(original)
            with patch.object(translate_comments, 'ROOT', root), contextlib.redirect_stdout(io.StringIO()):
                translate_comments.main()
                annotated = path.read_text()
                translate_comments.main()
            self.assertEqual(annotated, path.read_text())
            self.assertEqual(annotated.splitlines()[0], '@000001 を  // EN:')
            self.assertIn('// TODO:', annotated.splitlines()[1])
            self.assertEqual([l.partition('  //')[0] for l in original.splitlines()],
                             [l.partition('  //')[0] for l in annotated.splitlines()])
            with patch.object(build_english.english_layout, 'load_mapping', return_value={}):
                accepted, rejected = build_english.collect(root)
            self.assertEqual(rejected, [])
            self.assertEqual(dict(accepted), {b'': [b'\0'], build_english.text_codec.encode('を'): [b'\0']})
            self.assertIn('""', '\n'.join(build_english.render(accepted)))

    def test_script_scope_and_source_spacing(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            folder=root/'text/nfp';folder.mkdir(parents=True)
            dictionaries=root/'text/translation';dictionaries.mkdir()
            (dictionaries/'english.json').write_text(json.dumps({'ギンタ':'Ginta'}))
            (dictionaries/'scripts.json').write_text(json.dumps({'ONE.SPC':{'いいんだな？':'...right?'}}))
            original='@000001  C0F04 いいんだな？   // TODO: English translation\n@000020 ［ギンタ］  // TODO: English translation\n'
            for name in ('ONE.SPC.txt','TWO.SPC.txt'):(folder/name).write_text(original)
            legacy=root/'text/script_123.txt';legacy.write_text(original)
            with patch.object(translate_comments,'ROOT',root),contextlib.redirect_stdout(io.StringIO()):
                translate_comments.main()
                first=(folder/'ONE.SPC.txt').read_text()
                translate_comments.main()
            self.assertEqual(first,(folder/'ONE.SPC.txt').read_text(),'Annotation must be idempotent')
            self.assertIn('// EN: ...right?',first)
            for path in (folder/'TWO.SPC.txt',legacy):
                text=path.read_text()
                self.assertIn('// TODO: English translation',text)
                self.assertNotIn('// EN: ...right?',text)
            for path in (folder/'ONE.SPC.txt',folder/'TWO.SPC.txt',legacy):
                actual=path.read_text()
                self.assertEqual([l.partition('  //')[0] for l in actual.splitlines()],
                                 [l.partition('  //')[0] for l in original.splitlines()])
                self.assertIn('// EN: [Ginta]',actual)


if __name__=='__main__':unittest.main()
