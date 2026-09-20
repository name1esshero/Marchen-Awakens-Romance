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
static s8 bindingIndices[4];
static struct SpriteBindingOwner bindingOwner;
static struct SpriteResourceDescriptor descriptors[2];
static struct SpriteResourceHeader headers[2];
static struct SpriteResourceLevel0 level0[3];
static struct SpriteResourceLevel1 level1[10];
static struct SpriteResourceLevel2 level2[10];
static struct SpriteResourceLevel3 level3[10];
static u8 table24[10 * 32];
static u8 table28[10 * 32];
static void *releasedData;
static u32 releaseCount;
static void *copiedDestination;
static const void *copiedSource;
static u32 copiedSize;
void sub_080869B8(void *data) { releasedData=data; releaseCount++; }
void CpuCopy(void *destination,const void *source,u32 size) { copiedDestination=destination; copiedSource=source; copiedSize=size; }
int __divsi3(int dividend,int divisor) { return dividend/divisor; }
char *strupr(char *s) { char *p=s; while (*p) { if (*p>='a' && *p<='z') *p-=32; p++; } return s; }
char *strcpy(char *destination, const char *source) { char *result=destination; while ((*destination++=*source++)) {} return result; }
s32 memcmp(const void *left, const void *right, u32 size) { const u8 *a=left; const u8 *b=right; while (size--) { if (*a != *b) return *a-*b; a++; b++; } return 0; }
static void customCopy(void *destination,const void *source,u32 size) { copiedDestination=destination; copiedSource=source; copiedSize=size+1; }
static int run(void) {
    struct SpriteResource resource;
    testSpriteEngineState=&state;
    state.oamEntries=entries;
    state.oamBoundaries[0]=0;
    state.oamBoundaries[1]=128;
    state.oamBoundaries[2]=128;
    CHECK(sizeof(void *)==4 && sizeof(struct SpriteEngineState)==0x628);
    CHECK((u8 *)&state.resources-(u8 *)&state==0x61C);
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
    state.bindings[2].owner=&bindingOwner;
    state.bindings[2].index=1;
    bindingOwner.bindingIndices=bindingIndices;
    bindingIndices[1]=2;
    SpriteEngineIncrementCounter(2,0);
    CHECK(SpriteEngineGetCounter(2,0)==0xFF);
    SpriteEngineSetCounter(2,0,1);
    SpriteEngineIncrementCounter(2,0);
    CHECK(SpriteEngineGetCounter(2,0)==2);
    SpriteEngineDecrementCounter(2,0);
    CHECK(SpriteEngineGetCounter(2,0)==1 && state.bindings[2].owner==&bindingOwner);
    SpriteEngineDecrementCounter(2,0);
    CHECK(SpriteEngineGetCounter(2,0)==0);
    CHECK(state.bindings[2].owner==0 && state.bindings[2].index==-1);
    CHECK(bindingIndices[1]==-1);
    SpriteEngineDecrementCounter(2,0);
    CHECK(SpriteEngineGetCounter(2,0)==0);
    state.bindings[3].owner=&bindingOwner;
    state.bindings[3].index=-1;
    SpriteEngineReleaseBinding(3);
    CHECK(state.bindings[3].owner==0 && state.bindings[3].index==-1);
    state.resources=descriptors;
    descriptors[1].header=&headers[1];
    descriptors[1].level0=level0;
    descriptors[1].level1=level1;
    descriptors[1].level2=level2;
    descriptors[1].level3=level3;
    descriptors[1].table24=table24;
    descriptors[1].table28=table28;
    headers[1].entryCount=7;
    level0[2].childBase=3;
    level1[4].childBase=2;
    level2[4].childBase=1;
    level3[4].table24Index=2;
    level3[4].table28Index=3;
    CHECK(SpriteResourceGetEntryCount(1)==7);
    CHECK(SpriteResourceGetLevel0(1,2)==&level0[2]);
    CHECK(SpriteResourceGetLevel1(1,2,1)==&level1[4]);
    CHECK(SpriteResourceGetLevel2(1,2,1,2)==&level2[4]);
    CHECK(SpriteResourceGetLevel3(1,2,1,2,3)==&level3[4]);
    CHECK(SpriteResourceGetTable24(1,2,1,2,3)==table24+64);
    CHECK(SpriteResourceGetTable28(1,2,1,2,3)==table28+96);
    SpriteEngineSetBuffer4(entries+4);
    SpriteEngineSetBuffer8(entries+8);
    SpriteEngineSetBufferC(entries+12);
    CHECK(SpriteEngineGetBuffer4()==entries+4);
    CHECK(SpriteEngineGetBuffer8()==entries+8);
    CHECK(SpriteEngineGetBufferC()==entries+12);
    SpriteEngineSetCopyCallback620(0);
    SpriteEngineSetCopyCallback624(0);
    CHECK(SpriteEngineGetCopyCallback620()==SpriteEngineDefaultCopy620);
    CHECK(SpriteEngineGetCopyCallback624()==SpriteEngineDefaultCopy624);
    SpriteEngineGetCopyCallback620()(entries+16,entries+32,24);
    CHECK(copiedDestination==entries+16 && copiedSource==entries+32 && copiedSize==24);
    SpriteEngineSetCopyCallback624(customCopy);
    CHECK(SpriteEngineGetCopyCallback624()==customCopy);
    SpriteEngineGetCopyCallback624()(entries+40,entries+48,8);
    CHECK(copiedDestination==entries+40 && copiedSource==entries+48 && copiedSize==9);
    SpriteEngineSetCopyCallback620(customCopy);
    SpriteEngineCopyToBuffer8(2,entries+200,3);
    CHECK(copiedDestination==entries+72 && copiedSource==entries+200 && copiedSize==97);
    SpriteEngineCopyToBufferC(1,entries+220);
    CHECK(copiedDestination==entries+44 && copiedSource==entries+220 && copiedSize==33);
    SpriteEngineSetAllFlags10(0);
    SpriteEngineSetFlag10(3,1);
    SpriteEngineSetFlag10(31,2);
    CHECK(SpriteEngineGetFlags10()==0x80000008 && SpriteEngineTestFlag10(3)==8);
    SpriteEngineSetFlag10(3,0);
    CHECK(SpriteEngineGetFlags10()==0x80000000);
    SpriteEngineSetAllFlags10(7);
    CHECK(SpriteEngineGetFlags10()==0xFFFFFFFF);
    SpriteEngineSetAllFlags14(0);
    SpriteEngineSetFlag14(5,-1);
    CHECK(SpriteEngineGetFlags14()==32 && SpriteEngineTestFlag14(5)==32);
    SpriteEngineSetFlag14(5,0);
    CHECK(SpriteEngineGetFlags14()==0);
    SpriteEngineSetAllFlags14(1);
    CHECK(SpriteEngineGetFlags14()==0xFFFFFFFF);
    SpriteEngineSetBuffer4(entries+4);
    CHECK((u8 *)SpriteEngineGetAffineOamMatrix(3)==entries+100);
    CHECK(SpriteMathDivide65536ByS16(256)==256);
    CHECK(SpriteMathDivide65536ByS16(-256)==-256);
    CHECK(SpriteMathDivide65536ByS16(0x10100)==256);
    CHECK(SpriteRecordSizeForCount(0)==32);
    CHECK(SpriteRecordSizeForCount(7)==256);
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
