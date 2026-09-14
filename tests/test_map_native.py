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
            source=(ROOT/'src/map_native.c').read_text()
            (folder/'native.c').write_text(source)
            (folder/'test.c').write_text(r'''
#include "native.c"
#include <assert.h>
#include <string.h>
static int called, values[5];
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
 return 0;
}
''')
            subprocess.run(['gcc','-O2','-D','AT(x)=','-I'+str(ROOT/'include'),str(folder/'test.c'),'-o',str(folder/'test')],check=True)
            subprocess.run([str(folder/'test')],check=True)
if __name__=='__main__':unittest.main()
