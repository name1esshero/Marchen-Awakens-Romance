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
            implementation = Path(temp) / 'ncd_sprite.c'
            implementation.write_text((ROOT/'src/ncd_sprite.c').read_text().replace(
                'AT("00008A70") const u8 NcdSpriteContainerResetTail[2]={0};',
                'const u8 NcdSpriteContainerResetTail[2]={0};'))
            source.write_text(r'''
#include "ncd.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
void *HeapAlloc(void *heap, u32 size) {
    (void)heap;
    return malloc(size);
}
void CpuFill(void *dest, u32 size, u32 value) {
    assert(size == 52 && value == 0);
    memset(dest, 0, size);
}
void CpuCopy(void *dest, const void *src, u32 size) {
    assert(size == 52);
    memcpy(dest, src, size);
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
    {
        struct NcdSprite copy;
        data.sprite.copyMode27=3;
        data.sprite.flag27=1;
        NcdSpriteCopy(&copy,&data.sprite);
        assert(!memcmp(&copy,&data.sprite,39));
        assert(copy.copyMode27==1 && copy.flag27==1);
        assert(!memcmp((unsigned char *)&copy+40,(unsigned char *)&data.sprite+40,12));
    }
    return 0;
}
''')
            exe = Path(temp) / 'test'
            subprocess.run(['gcc','-O2','-I'+str(ROOT/'include'),str(source),str(implementation),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)
if __name__ == '__main__': unittest.main()
