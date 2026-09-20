"""Documentation export must not lose files or expose unrelated project data."""
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import publish_docs as docs


class PublicationTests(unittest.TestCase):
    def test_static_links_reject_missing_and_outside_site(self):
        with tempfile.TemporaryDirectory() as tmp:
            site = Path(tmp) / 'site'; site.mkdir()
            (site/'image.png').write_bytes(b'png')
            (site/'index.html').write_text('<img src="image.png"><a href="https://example.com/">External</a>')
            self.assertFalse(docs.check_links(site)['missing'])
            (site/'index.html').write_text('<img src="missing.png"><a href="../outside.txt">Outside</a>')
            (site.parent/'outside.txt').write_text('must not publish')
            self.assertEqual(len(docs.check_links(site)['missing']), 2)

    def test_restore_preserves_current_reports_and_editable_assets(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)/'root'; archive = Path(tmp)/'archive'
            for rel, data in {'reports/current.json':'old', 'reports/history.json':'history',
                              'graphics/index.html':'gallery', 'graphics/art.png':'archive art'}.items():
                p=archive/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(data)
            (root/'reports').mkdir(parents=True)
            (root/'reports/current.json').write_text('new')
            with patch.object(docs,'ROOT',root): docs.restore(archive)
            self.assertEqual((root/'reports/current.json').read_text(),'new')
            self.assertEqual((root/'reports/history.json').read_text(),'history')
            self.assertTrue((root/'graphics/index.html').exists())
            self.assertFalse((root/'graphics/art.png').exists())

    def test_export_excludes_rom_and_preserves_existing_wiki_home(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)/'root';site=Path(tmp)/'site';wiki=Path(tmp)/'wiki'
            (root/'reports').mkdir(parents=True);wiki.mkdir()
            (root/'reports/README.md').write_text('# Evidence\n')
            (root/'reports/forbidden.gba').write_bytes(b'ROM')
            (root/'docs/media').mkdir(parents=True)
            (root/'docs/media/map-editor.gif').write_bytes(b'GIF89a')
            (root/'secret.txt').write_text('unrelated')
            (wiki/'Home.md').write_text('Existing user documentation\n')
            with patch.object(docs,'ROOT',root),patch.object(docs,'DOCS',{'Evidence':'reports/README.md'}),patch.object(docs,'check_links',return_value={'missing':[]}):
                docs.stage(site,wiki)
            self.assertFalse((site/'reports/forbidden.gba').exists())
            self.assertFalse((site/'secret.txt').exists())
            self.assertEqual((site/'docs/media/map-editor.gif').read_bytes(),b'GIF89a')
            self.assertTrue((wiki/'Home.md').read_text().startswith('Existing user documentation'))
            manifest=json.loads((site/'publication.json').read_text())
            self.assertEqual(set(manifest['files']),{'reports/README.md','docs/media/map-editor.gif'})


if __name__=='__main__':unittest.main()
