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
