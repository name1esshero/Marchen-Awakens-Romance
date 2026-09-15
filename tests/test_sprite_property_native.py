"""Native property adapters retain VM result and task parameter semantics."""
from pathlib import Path
import subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
class SpritePropertyNativeTests(unittest.TestCase):
 def test_adapters(self):
  with tempfile.TemporaryDirectory() as temp:
   p=Path(temp);combined=(ROOT/'src/script_sprite.c').read_text()
   source='#include "script_sprite.h"\n'+combined[
       combined.index('/* Native sprite property adapters'):combined.index('/* SprInit worker task')]
   (p/'worker.c').write_text(source)
   (p/'test.c').write_text(r'''
#include <assert.h>
#include "worker.c"
static int sets,gets;
s32 sub_08010E44(s32 id,s32 property,s32 p2,s32 p3,s32 value,s32 p5,s32 p6){
 assert(id==31&&property==0&&p2==0&&p3==0&&value==-32769&&p5==1&&p6==0);sets++;return 0x7fff;
}
s32 sub_08011174(s32 id,s32 property,s32 *result,s32 mode){assert(id==31&&property==0&&!mode);*result=-123;gets++;return -1;}
int main(void){s32 args[3]={31,0,-32769},result=99;
 assert(ScriptNativeSpriteSet(3,args,&result)==1&&sets==1&&result==99);
 assert(ScriptNativeSpriteGet(2,args,&result)==1&&gets==1&&result==-123);
 return 0;}
''')
   subprocess.run(['gcc','-O2','-D','AT(x)=','-I'+str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
