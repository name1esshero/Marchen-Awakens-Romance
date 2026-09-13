"""Round-trip the source-level SPC assembler through the production decoder."""
import json
from pathlib import Path
import sys
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import lz77
import script_assembler
import script_events
import script_map_catalog

ROOT=Path(__file__).resolve().parents[1]


class ScriptAssemblerTests(unittest.TestCase):
    def source(self):
        return {
            'version':1,'stack_size':1024,'entry':'start','instructions':[
                {'label':'start'},
                {'op':'native','name':'FldSet',
                 'args':[{'string':'MAP01_1A','register':0},0,0],'result':0},
                {'op':'return'}]}

    def test_native_string_and_func_relocation_decode(self):
        raw=script_assembler.assemble(self.source())
        self.assertEqual(raw[:4],b'SCRP')
        calls=script_events.calls(raw)
        self.assertEqual(len(calls),1)
        self.assertEqual(calls[0]['function'],'FldSet')
        self.assertEqual([a['value'] for a in calls[0]['decoded_arguments']],
                         ['MAP01_1A',0,0])
        self.assertFalse(calls[0]['editable'])
        analysis=script_events.semantic_summary(calls)
        self.assertEqual(analysis['field_loads'][0]['destination'],'MAP01_1A')
        self.assertEqual((analysis['field_loads'][0]['x'],analysis['field_loads'][0]['y']),(0,0))

    def test_compressed_output_decodes_identically(self):
        raw=script_assembler.assemble(self.source())
        self.assertEqual(script_events.calls(lz77.compress(raw)),script_events.calls(raw))

    def test_registry_is_complete_and_unique(self):
        entries=json.loads((ROOT/'scripts/native_commands.json').read_text())
        self.assertEqual(len(entries),128)
        self.assertEqual(len({e['name'] for e in entries}),128)
        self.assertIn('SprInit',{e['name'] for e in entries})
        self.assertIn('FldSet',{e['name'] for e in entries})

    def test_checked_in_map_catalog_is_reproducible(self):
        expected=json.loads((ROOT/'maps/script_catalog.json').read_text())
        self.assertEqual(script_map_catalog.build(ROOT),expected)
        self.assertEqual(expected['totals']['field_loads'],113)
        self.assertGreaterEqual(expected['totals']['sprite_resources'],200)

    def test_rejects_unknown_operations_and_labels(self):
        source=self.source();source['instructions']=[{'op':'mystery'}]
        with self.assertRaisesRegex(ValueError,'unknown opcode'):
            script_assembler.assemble(source)
        source=self.source();source['entry']='missing'
        with self.assertRaisesRegex(ValueError,'unknown label'):
            script_assembler.assemble(source)


if __name__=='__main__':unittest.main()
