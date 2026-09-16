"""Verify the marscript surface-syntax compiler (tools/marscript.py's
compile_source) against the already-proven disassemble/reassemble core.

Since compiled marscript output describes content that doesn't exist in the
real ROM, there's no baserom bytes to compare against the way
test_marscript_roundtrip.py checks real scripts. What's checked instead:
the compiler's output assembles without error, is fully reachable when
independently flow-traced by the same disassembler proven against all 334
real scripts (an unreachable region would mean the compiler emitted
something structurally wrong), and round-trips byte-for-byte through
disassemble/reassemble the same way real scripts do.
"""
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import marscript
from script_assembler import assemble


class MarscriptCompilerTests(unittest.TestCase):
    def test_compiles_every_verified_construct_and_self_round_trips(self):
        source = '''
        script TEST {
            sprite dorothy = spawn(container: 2, resource: "09A00", animation: 0, at: 120, 80)
            hitregion doorway = region(id: 3, at: 96, 64, size: 16, 24)

            change dorothy to resource "09A01" animation 0
            set dorothy property 5 to 12
            hit doorway rect at 100, 70 size 20, 30
            hit doorway set mode to 1
            load field "MAP01_A" at 320, 160
            call script "SP_M01.SPC"
            chain script "CH_M01.SPC"

            label loop:
            goto loop

            free doorway
            free all hitregions
            end
        }
        '''
        document, name = marscript.compile_source(source)
        self.assertEqual(name, 'TEST')
        raw = assemble(document)
        self.assertEqual(raw[:4], b'SCRP')

        decoded = marscript.disassemble(raw)
        self.assertEqual(decoded['unreached'], [],
                         'compiled output left unreachable bytes -- the compiler '
                         'emitted something the flow-tracer cannot make sense of')

        code, _stack, _header = marscript.code_payload(raw)
        rebuilt = marscript.reassemble(decoded)
        self.assertEqual(rebuilt, code)

    def test_rejects_unimplemented_and_invalid_constructs(self):
        cases = [
            'script T { move dorothy to 1, 2 over 3 frames end }',
            'script T { wait 5 frames end }',
            'script T { hitregion h = region(id: 1, at: 0, 0, size: 1, 1) hit h set foo to 1 end }',
            'script T { set nobody property 0 to 1 end }',
            'script T { free nobody end }',
        ]
        for source in cases:
            with self.subTest(source=source):
                with self.assertRaises(marscript.CompileError):
                    marscript.compile_source(source)

    def test_sprite_declaration_order_gives_predictable_ids(self):
        source = '''
        script T {
            sprite a = spawn(container: 0, resource: "X", animation: 0, at: 0, 0)
            sprite b = spawn(container: 0, resource: "Y", animation: 0, at: 0, 0)
            set b property 0 to 5
            end
        }
        '''
        document, _ = marscript.compile_source(source)
        set_ins = [i for i in document['instructions']
                  if i.get('op') == 'native' and i['name'] == 'SprSet' and i['args'][1] == 0
                  and i['args'][2] == 5]
        self.assertEqual(len(set_ins), 1)
        self.assertEqual(set_ins[0]['args'][0], 1)  # b is the second-declared sprite -> id 1

    def test_expressions_if_while_self_round_trip(self):
        """Exercises var initializers, while, if/else, if-without-else, and
        every comparison/logical/arithmetic operator (including the two
        derived ones with no direct opcode: `<` via (b-a)>0, and negated
        `!=` for the `!(x != 0)` case) in one program. Correctness is
        checked the same way as the simpler compiler test: the compiled
        program must be fully reachable (no unreached regions, which would
        mean control flow the compiler emitted doesn't actually connect)
        and must round-trip byte-for-byte through disassemble/reassemble.
        """
        source = '''
        script LOGIC_TEST {
            sprite hero = spawn(container: 0, resource: "X", animation: 0, at: 0, 0)
            var counter = 0
            var limit = 3

            while (counter < limit) {
                set hero property 0 to 1
            }

            if (counter == limit && limit > 0) {
                call script "A.SPC"
            } else {
                chain script "B.SPC"
            }

            if (!(counter != 0) || (limit % 2 == 1)) {
                call script "C.SPC"
            }

            end
        }
        '''
        document, name = marscript.compile_source(source)
        self.assertEqual(name, 'LOGIC_TEST')
        raw = assemble(document)
        self.assertEqual(raw[:4], b'SCRP')

        decoded = marscript.disassemble(raw)
        self.assertEqual(decoded['unreached'], [])

        code, _stack, _header = marscript.code_payload(raw)
        rebuilt = marscript.reassemble(decoded)
        self.assertEqual(rebuilt, code)

    def test_low_level_constructs_self_round_trip(self):
        """The raw/low-level statement forms the decoder will need to fall
        back on for anything without a dedicated high-level verb: string
        declarations, address-to-string-payload resolution (confirmed this
        session to need a +3 offset past the string's own 0x10 header --
        see docs/decompiled-flags.md), raw register ops, goto_if_zero, a
        generic native call with register/string/literal arguments mixed
        in one call (register-valued native arguments were a real gap this
        session found and closed in script_assembler.py -- real scripts
        commonly need it, per the map editor's own audit), and switch.
        """
        source = '''
        script RAW_TEST {
            string greeting: "Hi"
            var x = 1
            var y = 2

            add r0, r1

            goto_if_zero r0, skip

            native "DoThing"(r0, "arg", 42) -> r0

            switch r0 {
                1 -> caseA,
                2 -> caseB,
            }

            label caseA:
            goto skip

            label caseB:
            address r5, greeting

            label skip:
            end
        }
        '''
        document, name = marscript.compile_source(source)
        self.assertEqual(name, 'RAW_TEST')
        raw = assemble(document)

        decoded = marscript.disassemble(raw)
        self.assertEqual(decoded['unreached'], [])
        # The address instruction must resolve 3 bytes into the string's
        # payload, not at the string block's own start.
        address_ins = [i for i in decoded['instructions'] if i['op'] == 'address']
        string_ins = next(i for i in decoded['instructions'] if i['op'] == '.string')
        self.assertEqual(address_ins[-1]['args'][1], string_ins['offset'] + 3)

        code, _stack, _header = marscript.code_payload(raw)
        rebuilt = marscript.reassemble(decoded)
        self.assertEqual(rebuilt, code)


if __name__ == '__main__':
    unittest.main()
