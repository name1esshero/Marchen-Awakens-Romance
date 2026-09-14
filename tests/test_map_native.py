"""Verify the actual matching native adapters against mocked engine helpers."""
from pathlib import Path
import subprocess
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
class MapNativeTests(unittest.TestCase):
    def test_arguments_truncation_free_dispatch_and_return_protocol(self):
        with tempfile.TemporaryDirectory() as temp:
            folder=Path(temp)
            source=(ROOT/'src/map_native.c').read_text().replace(
                '((const char *)0x08086D88)', '".KMP"')
            (folder/'native.c').write_text(source)
            (folder/'test.c').write_text(r'''
#include "native.c"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
static int called, values[5];
static int bgPhase;
char *strupr(char *text) {
 char *p=text;while(*p){*p=toupper((unsigned char)*p);p++;}return text;
}
void KmpLoadResource(const char *name,void *vram,s32 slot,s32 plane,s32 palette,s32 extra,s32 flags) {
 /* strupr's result is overwritten by a second, unmodified strcpy before use
  * in the real ROM, so the name reaching here keeps its original case. */
 assert(!strcmp(name,"map01_b.KMP"));
 assert((uintptr_t)vram==(slot==2?0x06008000u:slot==3?0x0600C000u:0x06000000u));
 assert(!plane && !palette && extra==7 && flags==3);called=6;values[0]=slot;
}
void KmpRenderViewport(struct KmpViewport *view,s32 x,s32 y) {
 /* Index by sizeof(struct KmpViewport) as the host compiles it, not the
  * hardcoded 0xFC GBA (32-bit pointer) stride -- host pointers are 8 bytes,
  * so the host struct size differs from the real ROM's. */
 assert(view==gKmpViewports+values[0]);
 assert(x==11<<16 && y==22<<16);bgPhase=1;
}
u8 gIwramBase[4];
u8 gMapGenerationRootOffset[1];
static char field_name_buffer[18];
void *HeapAlloc(void *heap,u32 size) {
 (void)heap;assert(size==18);return field_name_buffer;
}
void GameStateCopyString12F4(char *destination) {
 strcpy(destination,"MAP01_A");
}
void KmpLoadField(const char *name,s16 x,s16 y) {
 assert(!strcmp(name,"MAP01_A"));called=1;values[0]=x;values[1]=y;
}
void HitRegionInit(s32 id,s32 x,s32 y,s32 w,s32 h) {
 called=2;values[0]=id;values[1]=x;values[2]=y;values[3]=w;values[4]=h;
}
void HitRegionSetRect(s32 id,s32 x,s32 y,s32 w,s32 h) {
 HitRegionInit(id,x,y,w,h);called=3;
}
void HitRegionDisable(s32 id) {called=4;values[0]=id;}
void HitRegionDisableAll(void) {called=5;}
int main(void) {
 union MapArgument field[3];s32 out=123456, args[5]={3,-10,20,30,40};int i;
 field[0].string="MAP01_A";field[1].integer=65535;field[2].integer=32768;
 assert(ScriptNativeFieldSet(3,field,&out)==0x7FFF);
 assert(called==1 && values[0]==-1 && values[1]==-32768 && out==123456);
 assert(ScriptNativeHitInit(0,args,&out)==1 && called==2);
 for(i=0;i<5;i++)assert(values[i]==args[i]);
 assert(ScriptNativeHitRect(5,args,&out)==1 && called==3);
 for(i=0;i<5;i++)assert(values[i]==args[i]);
 assert(ScriptNativeHitFree(1,args,&out)==0x7FFF && called==4 && values[0]==3);
 args[0]=-1;
 assert(ScriptNativeHitFree(1,args,&out)==0x7FFF && called==5);
 assert(out==123456);
 {
  union MapArgument bg[6];s32 slot;
  bg[1].integer=0;bg[2].integer=7;bg[3].string="map01_b";
  bg[4].integer=11;bg[5].integer=22;
  for(slot=1;slot<=5;slot++){
   bgPhase=0;bg[0].integer=slot;
   assert(ScriptNativeBackgroundSet(6,bg,&out)==0x7FFF);
   assert(called==6 && values[0]==slot && bgPhase==1 && out==123456);
  }
 }
 return 0;
}
''')
            subprocess.run(['gcc','-O2','-D','AT(x)=','-I'+str(ROOT/'include'),str(folder/'test.c'),'-o',str(folder/'test')],check=True)
            subprocess.run([str(folder/'test')],check=True)
if __name__=='__main__':unittest.main()
