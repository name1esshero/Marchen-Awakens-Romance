"""Exercise the actual localization C with generated mappings on the host.
This verifies lookup/fallback/row bounds, not a complete emulator playthrough.
"""
import ctypes
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import build_english
import english_layout

ROOT=build_english.ROOT


class EnglishRuntimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory();folder=Path(cls.temp.name)
        cls.entries,cls.rejected=build_english.collect()
        lines=build_english.render(cls.entries)
        lines += [r"""
#include "dialogue.h"
#include <stdlib.h>
#include <string.h>
int captured_count, captured_mode, pending, key, clear_count, fail_alloc, fail_draw;
const char *captured_rows[3];
static void *parent, *clear_task;
static void (*parent_callback)(void *), (*clear_callback)(void *);
static int *child_result;
static unsigned char draw_task[540], vram[3072];
int vram_byte(int index) { return vram[index]; }
int child_ink(void) { return ((struct DialogueState *)(draw_task+32))->ink; }
int parent_finished;
void ScriptAddPendingTasks(unsigned n) { pending += n; }
void ScriptCompletePendingTasks(unsigned n) { pending -= n; }
unsigned sub_0807A134(unsigned mask,unsigned index) { int v=key;key=0;return v & mask; }
void CpuFill(void *dest,unsigned size,unsigned pattern) {
    unsigned long offset=(unsigned long)dest-0x0600C020;
    if(offset+size>sizeof(vram) || size%4 || offset%4 || pattern!=0x11111111) abort();
    memset(vram+offset,0x11,size);
}
void *CreateTask(void *manager,void *callback,unsigned priority,int *result,unsigned size) {
    void *task;
    if(fail_alloc) { fail_alloc--; return 0; }
    task=calloc(1,32+size);
    if(result)*result=0;
    if(manager==(void *)0x030032D4) { clear_task=task;clear_callback=callback; }
    else { parent=task;parent_callback=callback; }
    return task;
}
void FinishTask(void *task) { if(task==parent) parent_finished=1; }
void tick(void) { if(parent && !parent_finished) parent_callback(parent); }
void vblank(void) { if(clear_task) { clear_callback(clear_task);clear_count++;free(clear_task);clear_task=0; } }
void complete_draw(void) { if(child_result) { *child_result=-1;child_result=0;pending--; } }
void reset_mock(void) {
    free(parent);free(clear_task);parent=clear_task=0;child_result=0;
    memset(vram,0xAA,sizeof(vram));
    captured_count=captured_mode=pending=key=clear_count=parent_finished=fail_alloc=fail_draw=0;
}
void *DialogueStartOriginal(int mode,int count,const char **rows,int *result) {
    int i;
    if(fail_draw) { fail_draw--;return 0; }
    captured_mode=mode;captured_count=count;
    for(i=0;i<count && i<3;i++)captured_rows[i]=rows[i];
    if(parent) { child_result=result;if(result)*result=0;pending++; }
    else if(result)*result=77;
    if(parent) { memset(draw_task,0,sizeof(draw_task));return draw_task; }
    return (void *)0x1234;
}
"""]
        source=folder/'data.c';source.write_text('\n'.join(lines))
        library=folder/'runtime.so'
        subprocess.run(['gcc','-shared','-fPIC','-O2','-I'+str(ROOT/'include'),
                        '-D__attribute__(x)=', str(ROOT/'src/dialogue.c'), str(ROOT/'src/font.c'),
                        str(ROOT/'src/english/dialogue_runtime.c'),str(source),'-o',str(library)],check=True)
        cls.lib=ctypes.CDLL(str(library))
        cls.lib.EnglishTranslateRows.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_char_p)]
        cls.lib.EnglishDialogueStart.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_int)]
        cls.lib.EnglishDialogueStart.restype=ctypes.c_void_p

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def setUp(self):
        self.lib.reset_mock()

    def call(self,mode,rows):
        source=(ctypes.c_char_p*len(rows))(*rows)
        output=(ctypes.c_char_p*3)()
        n=self.lib.EnglishTranslateRows(mode,len(rows),source,output)
        return n,list(output)[:n]

    def test_every_generated_mapping_reaches_actual_c_output(self):
        self.assertGreater(len(self.entries),1000)
        for raw,rows in self.entries:
            n,output=self.call(0,[raw])
            if len(rows)>3:
                self.assertEqual(n,0)
                continue
            self.assertEqual(output,[r[:-1] for r in rows],raw)
            self.assertEqual(n,len(rows))
            for row in rows:
                visible=build_english.PREFIX.sub(b'',row[:-1],count=1)
                self.assertLessEqual(len(visible),42)
                self.assertEqual(len(visible)%2,0)
                self.assertLess(len(row),128)

    def test_capacity_modes_unknown_and_ambiguous_rows(self):
        one=next(raw for raw,rows in self.entries if len(rows)==1)
        two=next(raw for raw,rows in self.entries if len(rows)==2)
        self.assertEqual(self.call(0,[one,one,one])[0],3)
        self.assertEqual(self.call(1,[one,one])[0],2)
        self.assertEqual(self.call(1,[one,one,one])[0],0)
        self.assertEqual(self.call(1,[two,one])[0],0)
        self.assertEqual(self.call(0,[two,two])[0],0)
        self.assertEqual(self.call(2,[one])[0],0)
        self.assertEqual(self.call(0,[b'unmapped test string'])[0],0)
        self.assertEqual(self.call(0,[None])[0],0)

    def test_wrapper_preserves_abi_and_falls_back_whole_message(self):
        raw,expected=next((raw,rows) for raw,rows in self.entries if len(rows)==2)
        source=(ctypes.c_char_p*1)(raw);result=ctypes.c_int(-1)
        ptr=self.lib.EnglishDialogueStart(0,1,source,ctypes.byref(result))
        self.assertEqual(ptr,0x1234);self.assertEqual(result.value,77)
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'captured_count').value,2)
        source=(ctypes.c_char_p*2)(raw,b'unknown second row')
        self.lib.EnglishDialogueStart(1,2,source,ctypes.byref(result))
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'captured_mode').value,1)
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'captured_count').value,2)
        captured=(ctypes.c_char_p*3).in_dll(self.lib,'captured_rows')
        self.assertEqual(list(captured)[:2],[raw,b'unknown second row'])

    def value(self,name,value=None):
        ref=ctypes.c_int.in_dll(self.lib,name)
        if value is not None:ref.value=value
        return ref.value

    def captured(self):
        return list((ctypes.c_char_p*3).in_dll(self.lib,'captured_rows'))[:self.value('captured_count')]

    def test_pages_wait_for_button_and_vblank_and_hold_script(self):
        raw,rows=next((raw,rows) for raw,rows in self.entries if len(rows)==2)
        for mode in (0,1):
            self.lib.reset_mock()
            source=(ctypes.c_char_p*2)(raw,raw);result=ctypes.c_int(99)
            self.assertNotEqual(self.lib.EnglishDialogueStart(mode,2,source,ctypes.byref(result)),0x1234)
            self.assertEqual(result.value,0);self.assertEqual(self.value('pending'),1)
            self.lib.tick()
            all_rows=[r[:-1] for r in rows]*2;capacity=3-mode
            self.assertEqual(self.captured(),all_rows[:capacity])
            self.assertEqual(self.value('pending'),2)
            self.lib.complete_draw();self.value('key',1);self.lib.tick()
            self.assertEqual(self.value('clear_count'),0)
            self.value('key',0)
            for _ in range(5):self.lib.tick()
            self.assertEqual(result.value,0);self.assertEqual(self.value('pending'),1)
            self.value('key',1);self.lib.tick()
            for _ in range(5):self.lib.tick()
            self.assertEqual(self.captured(),all_rows[:capacity])
            self.lib.vblank();self.lib.tick()
            self.assertEqual(self.captured(),all_rows[capacity:])
            self.assertEqual(self.value('clear_count'),1)
            for y in range(32):
                for x in range(96):
                    offset=(y//8)*768+(x//4)*32+(y%8)*4+x%4
                    expected=0xAA if mode==1 and y<11 else 0x11
                    self.assertEqual(self.lib.vram_byte(offset),expected)
            self.lib.complete_draw();self.lib.tick()
            self.assertEqual(result.value,-1);self.assertEqual(self.value('pending'),0)
            self.assertEqual(self.value('parent_finished'),1)

    def test_allocation_failure_retries_without_losing_rows(self):
        raw,rows=next((raw,rows) for raw,rows in self.entries if len(rows)==2)
        source=(ctypes.c_char_p*2)(raw,raw);result=ctypes.c_int(99)
        self.lib.EnglishDialogueStart(1,2,source,ctypes.byref(result))
        self.value('fail_draw',1);self.lib.tick()
        self.assertEqual(self.value('pending'),1)
        self.lib.tick();self.assertEqual(self.captured(),[r[:-1] for r in rows])
        self.lib.complete_draw();self.lib.tick()
        self.value('key',1);self.value('fail_alloc',1);self.lib.tick();self.lib.tick()
        self.lib.vblank();self.value('fail_draw',1);self.lib.tick();self.lib.tick()
        self.assertEqual(self.captured(),[r[:-1] for r in rows])
        self.lib.complete_draw();self.lib.tick()
        self.assertEqual(result.value,-1);self.assertEqual(self.value('pending'),0)

    def test_style_carries_into_unformatted_rows_on_next_page(self):
        styled,first=next((raw,rows) for raw,rows in self.entries
                         if raw.startswith(b' C0D04 ') and len(rows)==2)
        plain,second=next((raw,rows) for raw,rows in self.entries
                         if not build_english.PREFIX.match(raw).group() and len(rows)==2)
        source=(ctypes.c_char_p*2)(styled,plain)
        self.lib.EnglishDialogueStart(1,2,source,None)
        self.lib.tick();self.lib.complete_draw();self.lib.tick()
        self.value('key',1);self.lib.tick();self.lib.vblank();self.lib.tick()
        self.assertEqual(self.captured(),[r[:-1] for r in second])
        self.assertEqual(self.lib.child_ink(),13)
        self.lib.complete_draw();self.lib.tick()
        self.assertEqual(self.value('pending'),0)

    def test_single_mapping_larger_than_one_page(self):
        raw,rows=next((raw,rows) for raw,rows in self.entries if len(rows)>3)
        source=(ctypes.c_char_p*1)(raw);result=ctypes.c_int(0)
        self.lib.EnglishDialogueStart(0,1,source,ctypes.byref(result))
        observed=[]
        for start in range(0,len(rows),3):
            self.lib.tick();observed+=self.captured()
            self.lib.complete_draw();self.lib.tick()
            if start+3<len(rows):
                self.value('key',1);self.lib.tick();self.lib.vblank()
        self.assertEqual(observed,[r[:-1] for r in rows])
        self.assertEqual(result.value,-1);self.assertEqual(self.value('pending'),0)


if __name__=='__main__':unittest.main()
