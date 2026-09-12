"""Exercise recovered sprite-engine state accessors with a 32-bit ABI."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class SpriteEngineStateTests(unittest.TestCase):
    def test_oam_entries_boundaries_and_signed_values(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            header = root / 'hardware.h'
            header.write_text(
                '#define gSpriteEngineState testSpriteEngineState\n'
                '#include "sprite_engine.h"\n'
                'extern struct SpriteEngineState *testSpriteEngineState;\n'
            )
            source = root / 'test.c'
            source.write_text(r'''
#include "sprite_engine.h"
#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)
struct SpriteEngineState *testSpriteEngineState;
static struct SpriteEngineState state;
static u8 entries[8 * 128];
static void *releasedData;
static u32 releaseCount;
void sub_080869B8(void *data) { releasedData=data; releaseCount++; }
static int run(void) {
    struct SpriteResource resource;
    testSpriteEngineState=&state;
    state.oamEntries=entries;
    state.oamBoundaries[0]=0;
    state.oamBoundaries[1]=128;
    state.oamBoundaries[2]=128;
    CHECK(sizeof(void *)==4 && sizeof(struct SpriteEngineState)==0x614);
    CHECK(SpriteEngineGetOamEntry(0)==entries);
    CHECK(SpriteEngineGetOamEntry(7)==entries+56);
    CHECK(SpriteEngineAllocateOamEntry()==entries);
    CHECK(SpriteEngineAllocateOamEntry()==entries+8);
    CHECK(SpriteEngineGetOamBoundary(0)==2);
    CHECK(SpriteEngineGetOamBoundary(1)==128);
    CHECK(SpriteEngineGetOamBoundary(2)==128);
    CHECK(SpriteEngineGetOamBoundary(-1)==0);
    CHECK(SpriteEngineGetOamBoundary(3)==0);
    SpriteEngineSetOamBoundary(0x123,1);
    CHECK(SpriteEngineGetOamBoundary(1)==0x23);
    SpriteEngineSetOamBoundary(77,3);
    CHECK(SpriteEngineGetOamBoundary(0)==2);
    CHECK(SpriteEngineGetOamBoundary(1)==0x23);
    CHECK(SpriteEngineGetOamBoundary(2)==128);
    SpriteEngineSetValue610(-1234);
    SpriteEngineSetValue612(2345);
    CHECK(SpriteEngineGetValue610()==-1234);
    CHECK(SpriteEngineGetValue612()==2345);
    resource.data=entries+24;
    resource.handle=0;
    SpriteResourceRelease(&resource);
    CHECK(releaseCount==0);
    SpriteResourceSetHandle(&resource,0x12345);
    CHECK(releaseCount==0 && SpriteResourceGetHandle(&resource)==0x2345);
    SpriteResourceSetHandle(&resource,-7);
    CHECK(releaseCount==1 && releasedData==entries+24);
    CHECK(SpriteResourceGetHandle(&resource)==-7);
    SpriteResourceRelease(&resource);
    CHECK(releaseCount==2);
    state.flags20C=0;
    SpriteEngineSetFlag20C(3,1);
    SpriteEngineSetFlag20C(15,-1);
    CHECK(SpriteEngineGetFlags20C()==0x8008);
    CHECK(SpriteEngineTestFlag20C(3)==8);
    CHECK(SpriteEngineTestFlag20C(2)==0);
    SpriteEngineSetFlag20C(3,0);
    CHECK(SpriteEngineGetFlags20C()==0x8000);
    SpriteEngineSetCounter(2,3,0xA5);
    CHECK(SpriteEngineGetCounter(2,3)==0xA5);
    SpriteEngineSetCounter((u8)258,0,(u8)0x1FF);
    CHECK(SpriteEngineGetCounter(2,0)==0xFF);
    return 0;
}
void _start(void) {
    int result=run();
    __asm__ volatile("int $0x80" : : "a"(1), "b"(result));
    __builtin_unreachable();
}
''')
            exe = str(root / 'test')
            subprocess.run([
                'cc', '-m32', '-nostdlib', '-fno-pie', '-no-pie',
                '-fno-stack-protector', '-fno-builtin', '-D__attribute__(x)=',
                '-I' + str(ROOT / 'include'), '-include', str(header),
                str(source), str(ROOT / 'src/sprite_engine_state.c'), '-o', exe,
            ], check=True)
            subprocess.run([exe], check=True)


if __name__ == '__main__':
    unittest.main()
