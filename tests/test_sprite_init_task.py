"""Exercise the recovered creation worker's wait/completion lifecycle."""
from pathlib import Path
import subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
class SpriteInitTaskTests(unittest.TestCase):
 def test_wait_and_completion(self):
  with tempfile.TemporaryDirectory() as temp:
   p=Path(temp)
   combined=(ROOT/'src/script_sprite.c').read_text()
   source='#include "script_sprite.h"\n'+combined[
       combined.index('/* SprInit worker task'):combined.index('/* Deferred script-sprite reset workers')]
   (p/'worker.c').write_text(source)
   (p/'test.c').write_text(r'''
#include <assert.h>
#include <string.h>
#include "worker.c"
static union {long double align;u8 bytes[64];} storage;
static u8 auxiliary[72];
static int prepared,allocated,completed,finished;
void *GameStateGetRecord0B90(u32 id){assert(id==3);return storage.bytes;}
void sub_0801097C(s32 id,s32 wait,s32 *ready){assert(id==3 && wait==1);*ready=0;prepared++;}
void *HeapAlloc(void *heap,u32 size){assert(!heap && size==72);allocated++;return auxiliary;}
void CpuFill(void *dest,u32 size,u32 value){assert(dest==auxiliary && size==72 && !value);memset(dest,0,size);}
void NcdSpriteContainerReset(void *block){unsigned i;assert(block==auxiliary);for(i=0;i<72;i++)assert(!auxiliary[i]);}
s32 SpriteResourceFindGroup(s32 container,const char *name){assert(container==2 && !strcmp(name,"TEST"));return 7;}
void ScriptCompletePendingTasks(u32 count){assert(count==1);completed++;}
void FinishTask(void *task){assert(task);finished++;}
int main(void){
 struct SpriteInitTask task={0};s32 result=123;
 struct ScriptSprite *sprite=(struct ScriptSprite *)storage.bytes;
 task.id=3;task.wait=1;task.container=2;task.animation=4;task.frame=0;task.result=&result;strcpy(task.name,"TEST");
 memset(auxiliary,0xff,sizeof(auxiliary));sprite->x=17;sprite->y=-20;
 ScriptSpriteInitTask(&task);
 assert(task.state==16 && prepared==1 && !allocated && !finished && result==123);
 ScriptSpriteInitTask(&task);assert(prepared==1 && !allocated);
 task.finished=1;ScriptSpriteInitTask(&task);
 assert(allocated==1 && completed==1 && finished==1 && result==-1);
 assert(sprite->active && sprite->container==2 && sprite->group==7 && sprite->animation==4 && sprite->frame==0);
 assert(sprite->x==17 && sprite->y==-20 && sprite->drawOrderBits==3 && !sprite->hitBoundsEnabled);
 assert((storage.bytes[0]&2) && (sprite->flags1&1));
 task.state=99;ScriptSpriteInitTask(&task);assert(allocated==1);
 return 0;
}
''')
   subprocess.run(['gcc','-O2','-fno-strict-aliasing','-D','AT(x)=','-I'+str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
