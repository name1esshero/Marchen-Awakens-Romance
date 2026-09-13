"""Verify active-only NCD selection and the script record ABI."""
import subprocess,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class ScriptSpriteTests(unittest.TestCase):
    def test_selection(self):
        with tempfile.TemporaryDirectory() as temp:
            p=Path(temp)
            combined=(ROOT/'src/script_sprite.c').read_text()
            (p/'worker.c').write_text(combined[:combined.index('/* Native sprite properties')])
            (p/'test.c').write_text(r'''
#include "script_sprite.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
static struct ScriptSprite sprite;
static int looked, expectedContainer, answer;
void *GameStateGetRecord0B90(u32 id) {assert(id==16);return &sprite;}
char *strupr(char *p) {char *s=p;while(*s){if(*s>='a'&&*s<='z')*s-=32;s++;}return p;}
s32 SpriteResourceFindGroup(s32 c,const char *s) {
 assert(c==expectedContainer);assert(!strcmp(s,"PS_WK02"));looked++;return answer;
}
static int created;
void sub_08010AEC(s32 id,s32 container,const char *name,s32 animation,s32 extra,s32 mode,s32 flags) {
 assert(id==16 && container==0 && !strcmp(name,"ps_wk02"));
 assert(animation==7 && extra==9 && mode==1 && flags==0);created++;
}
int main(void) {
 struct ScriptSprite before;
 union SpriteArgument args[5];s32 result=9876;
 assert(sizeof(sprite)==40 && offsetof(struct ScriptSprite,container)==2);
 assert(offsetof(struct ScriptSprite,x)==10 && offsetof(struct ScriptSprite,y)==12);
 memset(&sprite,0,sizeof(sprite));before=sprite;
 ScriptSpriteSelect(16,0,"ps_wk02",2,3);
 assert(!looked && !memcmp(&before,&sprite,sizeof(sprite)));
 sprite.active=1;sprite.last=1;sprite.other=13;sprite.x=200;sprite.y=412;
 expectedContainer=-1;answer=123;
 ScriptSpriteSelect(16,65535,"ps_wk02",32768,65535);
 assert(looked==1 && sprite.container==-1 && sprite.group==123);
 assert(sprite.animation==-32768 && sprite.frame==-1);
 assert(sprite.x==200 && sprite.y==412 && sprite.active && sprite.last && sprite.other==13);
 assert(sprite.drawOrderBits==3);
 expectedContainer=0;answer=-1;
 ScriptSpriteSelect(16,0,"ps_wk02",0,0);
 assert(sprite.group==-1 && looked==2);
 args[0].integer=16;args[1].integer=0;args[2].string="ps_wk02";
 args[3].integer=7;args[4].integer=9;answer=44;
 assert(ScriptNativeSpriteChange(5,args,&result)==1 && result==9876);
 assert(sprite.group==44 && sprite.animation==7 && sprite.frame==9 && looked==3);
 assert(ScriptNativeSpriteInit(5,args,&result)==1 && result==9876 && created==1);
 return 0;
}
''')
            subprocess.run(['gcc','-O2','-I'+str(ROOT/'include'),str(p/'test.c'),str(p/'worker.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
