"""ROM-driven checks: execute the original mapping instructions for every u16.

This deliberately limited interpreter supports only instructions encountered
in 0807AE7C; unexpected opcodes fail instead of being guessed or skipped.
"""
import sys
from pathlib import Path
import struct
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import font
import nfp

ROM_PATH = font.ROOT / 'baserom.gba'
ROM = ROM_PATH.read_bytes() if ROM_PATH.exists() else b''


def original_mapping(code):
    r = [0] * 8
    r[0] = code
    pc = 0x7AE7C
    zero = carry = False
    for _ in range(160):
        h = struct.unpack_from('<H', ROM, pc)[0]
        nxt = pc + 2
        if h in (0xB510, 0xBC10, 0xBC02):
            pass  # Saved r4 and return address are irrelevant to the return value.
        elif h == 0x4708:
            return r[0]
        elif h & 0xF800 in (0, 0x0800):
            amount = (h >> 6) & 31
            src, dst = (h >> 3) & 7, h & 7
            if h & 0xF800 == 0:
                r[dst] = (r[src] << amount) & 0xFFFFFFFF
            else:
                r[dst] = r[src] >> (amount or 32)
        elif h & 0xF800 == 0x1800:
            src, dst = (h >> 3) & 7, h & 7
            operand = (h >> 6) & 7
            value = operand if h & 0x400 else r[operand]
            r[dst] = (r[src] - value if h & 0x200 else r[src] + value) & 0xFFFFFFFF
        elif h & 0xF800 == 0x2000:
            r[(h >> 8) & 7] = h & 255
        elif h & 0xF800 == 0x2800:
            a, b = r[(h >> 8) & 7], h & 255
            zero, carry = a == b, a >= b
        elif h & 0xF800 in (0x3000, 0x3800):
            dst = (h >> 8) & 7
            value = h & 255
            r[dst] = (r[dst] - value if h & 0x800 else r[dst] + value) & 0xFFFFFFFF
        elif h & 0xFFC0 == 0x4000:
            r[h & 7] &= r[(h >> 3) & 7]
        elif h & 0xFFC0 == 0x4280:
            a, b = r[h & 7], r[(h >> 3) & 7]
            zero, carry = a == b, a >= b
        elif h & 0xF800 == 0x4800:
            r[(h >> 8) & 7] = struct.unpack_from('<I', ROM, ((pc + 4) & ~3) + (h & 255) * 4)[0]
        elif h & 0xF000 == 0xD000:
            cond = (h >> 8) & 15
            take = {0: zero, 1: not zero, 8: carry and not zero, 9: not carry or zero}[cond]
            if take:
                disp = h & 255
                nxt = pc + 4 + (disp if disp < 128 else disp - 256) * 2
        elif h & 0xF800 == 0xE000:
            disp = h & 0x7FF
            nxt = pc + 4 + (disp if disp < 1024 else disp - 2048) * 2
        else:
            raise AssertionError(f'Unexpected instruction {h:04X} at {pc:06X}')
        pc = nxt
    raise AssertionError('Mapping did not return')


@unittest.skipUnless(ROM_PATH.exists(), 'baserom.gba is required for ROM comparison')
class FontTest(unittest.TestCase):
    def test_all_character_codes_against_rom(self):
        for code in range(65536):
            self.assertEqual(font.glyph_index(code), original_mapping(code), f'{code:04X}')

    def test_whole_font_roundtrip_and_pixel_edit(self):
        entry = next(e for e in nfp.entries(ROM) if e['name'] == 'FONT.NFT')
        blob = ROM[entry['rom_offset']:entry['end']]
        px = font.decode(blob)
        self.assertEqual(font.encode(blob, px), blob)
        px[0][0] ^= 1
        edited = font.encode(blob, px)
        diffs = [i for i, (a, b) in enumerate(zip(blob, edited)) if a != b]
        self.assertEqual(diffs, [0x60])
        self.assertEqual(blob[0x60] ^ edited[0x60], 0x80)
        px[-1][-1] = 1
        with self.assertRaises(ValueError):
            font.encode(blob, px)


if __name__ == '__main__':
    unittest.main()
