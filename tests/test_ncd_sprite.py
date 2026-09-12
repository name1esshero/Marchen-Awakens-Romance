"""Execute the recovered initializer and verify its complete 52-byte ABI."""
from pathlib import Path
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]

class NcdSpriteTests(unittest.TestCase):
    def test_initialization_layout_and_bounds(self):
        with tempfile.TemporaryDirectory() as temp:
            source = Path(temp) / 'test.c'
            source.write_text(r'''
#include "ncd.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
void CpuFill(void *dest, u32 size, u32 value) {
    assert(size == 52 && value == 0);
    memset(dest, 0, size);
}
int main(void) {
    struct {u32 before; struct NcdSprite sprite; u32 after;} data;
    unsigned char expected[52] = {0};
    memset(&data, 0xA5, sizeof(data));
    assert(sizeof(struct NcdSprite) == 52);
    assert(offsetof(struct NcdSprite, x) == 24);
    assert(offsetof(struct NcdSprite, cellHandles) == 48);
    NcdInitSprite(&data.sprite, 0x12345);
    expected[8] = expected[9] = 255;
    expected[10] = 0x45; expected[11] = 0x23;
    memset(expected + 16, 255, 8);
    expected[36] = 16; expected[39] = 8; expected[40] = 1;
    expected[45] = expected[47] = 1;
    assert(!memcmp(&data.sprite, expected, 52));
    assert(data.before == 0xA5A5A5A5 && data.after == 0xA5A5A5A5);
    NcdInitSprite(&data.sprite, -1);
    assert(data.sprite.allocationPool == -1);
    return 0;
}
''')
            exe = Path(temp) / 'test'
            subprocess.run(['gcc','-O2','-I'+str(ROOT/'include'),str(source),str(ROOT/'src/ncd_sprite.c'),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)
if __name__ == '__main__': unittest.main()
