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
            (folder/'field.c').write_text((ROOT/'src/map_field.c').read_text())
            (folder/'test.c').write_text(r'''
#include "kmp.h"
#include "map_events.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
/* The ROM resolves this through asm/game_table_handlers.s; the host link
 * supplies the literal it aliases. */
const char gMapArchiveKmpExtension[] = ".KMP";
static int phase;
static int expectedX,expectedY;
struct KmpViewport gKmpViewports[2];
struct TaskManager gMainTaskManager;
static union { void *alignment; u8 bytes[256]; } eventStorage;
static u32 eventCompletion;
void sub_08061F20(struct EngineTask *task) { (void)task; }
struct EngineTask *CreateTask(struct TaskManager *manager,
    void (*callback)(struct EngineTask *), u32 queue, u32 *completion, u32 size) {
 assert(manager == &gMainTaskManager && callback == sub_08061F20);
 assert(queue == 0 && completion == &eventCompletion && size == 160);
 memset(eventStorage.bytes, 0xA5, sizeof(eventStorage.bytes));
 return (struct EngineTask *)eventStorage.bytes;
}
void GameStateSetString12F4(const char *name) {assert(phase++==0);assert(!strcmp(name,"map01_a"));}
char *strupr(char *text) {char *p=text;while(*p){*p=toupper((unsigned char)*p);p++;}return text;}
void KmpLoadResource(const char *name,void *vram,s32 slot,s32 plane,s32 palette,s32 extra,s32 flags) {
 assert(!strcmp(name,"MAP01_A.KMP"));assert((uintptr_t)vram==0x06000000);
 assert(slot==phase-1 && plane==slot);assert(!palette && !extra);
 assert(flags==(slot==0?3:0));phase++;
}
void KmpRenderViewport(struct KmpViewport *view,s32 x,s32 y) {
 assert(view==&gKmpViewports[phase-3]);
 assert(x==expectedX && y==expectedY);phase++;
}
u32 GameStateGetResourceCounter(void) { return 7; }
void sub_08054350(void *a,void *b,s32 c,s32 d,s32 e,s32 f,s32 g) {
 (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;
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
 {
  struct EngineTask *task = CreateFieldEventTask(-32768,32767,-1,0,-123,
      &header,&eventCompletion);
  struct FieldEventTaskData *data = (void *)(eventStorage.bytes + ENGINE_TASK_HEADER_SIZE);
  unsigned i;
  assert(task == (struct EngineTask *)eventStorage.bytes);
  assert(data->firstCoordinate == -32768 && data->secondCoordinate == 32767);
  assert(data->thirdCoordinate == -1 && data->fourthCoordinate == 0);
  assert(data->value == -123 && data->objectData == &header);
  for (i = 0; i < ENGINE_TASK_HEADER_SIZE; i++) assert(eventStorage.bytes[i] == 0xA5);
  assert(data->unknown86[0] == 0xA5 && data->unknown94[0] == 0xA5);
 }
 return 0;
}
''')
            subprocess.run(['gcc','-O2','-ffunction-sections','-fdata-sections',
                            '-D','AT(x)=','-I'+str(ROOT/'include'),
                            str(folder/'field.c'),str(folder/'test.c'),
                            '-Wl,--gc-sections','-o',str(folder/'test')],check=True)
            subprocess.run([str(folder/'test')],check=True)

if __name__=='__main__':unittest.main()
