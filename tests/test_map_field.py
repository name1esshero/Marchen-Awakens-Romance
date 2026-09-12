"""Exercise actual recovered field-loader C with hardware/archive calls mocked."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

class MapFieldTests(unittest.TestCase):
    def test_field_name_resources_viewport_stride_and_signed_coordinates(self):
        with tempfile.TemporaryDirectory() as temp:
            folder=Path(temp)
            source=(ROOT/'src/map_field.c').read_text().replace('__attribute__((section(".rom." x)))','').replace('(const char *)0x08086A5C','".KMP"')
            (folder/'field.c').write_text(source)
            (folder/'test.c').write_text(r'''
#include "kmp.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
static int phase;
static int expectedX,expectedY;
void sub_08006A1C(const char *name) {assert(phase++==0);assert(!strcmp(name,"map01_a"));}
char *strupr(char *text) {char *p=text;while(*p){*p=toupper((unsigned char)*p);p++;}return text;}
void sub_08003178(char *name,void *vram,s32 slot,s32 plane,s32 palette,s32 extra,s32 flags) {
 assert(!strcmp(name,"MAP01_A.KMP"));assert((uintptr_t)vram==0x06000000);
 assert(slot==phase-1 && plane==slot);assert(!palette && !extra);
 assert(flags==(slot==0?3:0));phase++;
}
void KmpRenderViewport(struct KmpViewport *view,s32 x,s32 y) {
 assert((uintptr_t)view==0x03003BC4+(phase-3)*0xFC);
 assert(x==expectedX && y==expectedY);phase++;
}
int main(void) {
 struct KmpHeader header={0};struct KmpViewport view={0};
 phase=0;expectedX=-32768*65536;expectedY=32767*65536;
 KmpLoadField("map01_a",-32768,32767);assert(phase==5);
 phase=0;expectedX=0;expectedY=-65536;
 KmpLoadField("map01_a",0,-1);assert(phase==5);
 view.data=&header;header.widthTiles=64;header.heightTiles=128;
 KmpSetClip(&view,3,4,5,6);
 assert(view.clipX==3 && view.clipY==4 && view.clipWidth==5 && view.clipHeight==6);
 KmpResetClip(&view);
 assert(view.clipX==0 && view.clipY==0 && view.clipWidth==64 && view.clipHeight==128);
 return 0;
}
''')
            subprocess.run(['gcc','-O2','-I'+str(ROOT/'include'),str(folder/'field.c'),str(folder/'test.c'),'-o',str(folder/'test')],check=True)
            subprocess.run([str(folder/'test')],check=True)

if __name__=='__main__':unittest.main()
