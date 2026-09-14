"""Verify waiting, teardown and reset behavior of the original sprite workers."""
from pathlib import Path
import subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
class SpriteResetTests(unittest.TestCase):
 def test_reset_lifecycle(self):
  with tempfile.TemporaryDirectory() as temp:
   p=Path(temp);combined=(ROOT/'src/script_sprite.c').read_text()
   source='#include "script_sprite.h"\n'+combined[combined.index('/* Deferred script-sprite reset workers'):]
   (p/'worker.c').write_text(source)
   (p/'test.c').write_text(r'''
#include <assert.h>
#include <string.h>
#include "worker.c"
static union {long double align;u8 bytes[32*40+16];} pool;
static int torn,freed,completed,finished;
static u8 auxiliary[72];
void *GameStateGetRecord0B90(u32 id){assert(id<32);return pool.bytes+id*40;}
void SpriteAuxiliaryReset(void *p){assert(p==auxiliary);torn++;}
void HeapFree(void *heap,void *p){assert(!heap&&p==auxiliary&&torn>freed);freed++;}
void CpuFill(void *p,u32 n,u32 value){assert(n==40&&!value);memset(p,0,n);}
void ScriptCompletePendingTasks(u32 n){assert(n==1);completed++;}
void FinishTask(void *p){assert(p);finished++;}
static void defaults(struct ScriptSprite *s){assert(!s->active&&!s->x&&!s->y&&s->drawOrderBits==3);assert(((struct ResetFields *)s)->value20==256&&((struct ResetFields *)s)->value22==256);}
int main(void){
 union {long double align;u8 bytes[64];} task={0};s32 result=123;
 struct ScriptSprite *s=GameStateGetRecord0B90(31);
 *(s32 *)(task.bytes+32)=31;*(s32 **)(task.bytes+24)=&result;
 s->active=1;s->x=99;s->y=-10;*(u16 *)((u8 *)s+26)=1;*(void **)((u8 *)s+36)=auxiliary;
 ScriptSpriteResetTask(task.bytes);assert(!torn&&!freed&&!completed&&!finished&&result==123&&s->x==99);
 *(u16 *)((u8 *)s+26)=0;ScriptSpriteResetTask(task.bytes);
 assert(torn==1&&freed==1&&completed==1&&finished==1&&result==-1);defaults(s);
 memset(pool.bytes,0,sizeof(pool.bytes));result=77;
 ((struct ScriptSprite *)GameStateGetRecord0B90(1))->active=1;((struct ScriptSprite *)GameStateGetRecord0B90(1))->x=100;
 s=GameStateGetRecord0B90(3);s->active=1;*(u16 *)((u8 *)s+26)=2;
 ((struct ScriptSprite *)GameStateGetRecord0B90(5))->x=42; /* inactive slot must be left untouched by reset-all */
 ScriptSpriteResetAllTask(task.bytes);defaults(GameStateGetRecord0B90(1));assert(completed==1&&result==77&&s->active&&((struct ScriptSprite *)GameStateGetRecord0B90(5))->x==42);
 *(u16 *)((u8 *)s+26)=0;ScriptSpriteResetAllTask(task.bytes);defaults(s);assert(completed==2&&finished==2&&result==-1);
 /* Single reset clears even an inactive record; it does not await its stale mask. */
 s=GameStateGetRecord0B90(31);s->active=0;s->x=7;*(u16 *)((u8 *)s+26)=1;*(s32 **)(task.bytes+24)=0;
 ScriptSpriteResetTask(task.bytes);defaults(s);assert(completed==3&&finished==3&&freed==1);
 return 0;
}
''')
   subprocess.run(['gcc','-O2','-fno-strict-aliasing','-D','AT(x)=','-I'+str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
