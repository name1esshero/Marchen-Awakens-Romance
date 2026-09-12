"""Exercise recovered frame C with host-sized pointers and named layouts.
Matching agbcc builds separately verify every GBA field offset and instruction.
"""
import ctypes
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]


class ScriptFrameTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory();folder=Path(cls.temp.name)
        source=(ROOT/'src/script_frames.c').read_text()
        source='struct ScriptContext; extern struct ScriptContext *hostVm;\n'+source.replace('(*(struct ScriptContext **)0x0300611C)','hostVm')
        (folder/'frames.c').write_text(source)
        (folder/'mock.c').write_text(r'''
#include "script_vm.h"
#include <string.h>
struct ScriptContext context, *hostVm=&context;
struct ScriptExecutionState state;
struct ScriptFrame frame, parent;
int calls, status, cleanupCalls, freeCount, parentRestored;
void *freed[5], *heaps[5];
void resetFrames(void) {
 memset(&context,0,sizeof(context));memset(&state,0,sizeof(state));
 memset(&frame,0,sizeof(frame));memset(&parent,0,sizeof(parent));
 context.state=&state;state.frame=&frame;state.heap=(void *)0x1010;
 state.stepBudget=3;state.dispatchState=7;state.dispatchIndex=9;
 frame.parent=&parent;calls=cleanupCalls=freeCount=parentRestored=0;status=1;
}
s32 sub_08080070(struct ScriptFrame *arg) { if(arg!=&frame)return -99;calls++;return status; }
void sub_0807EC58(void) { cleanupCalls++; }
void HeapFree(void *heap,void *pointer) {
 if(state.frame==&parent)parentRestored++;
 if(freeCount<5){freed[freeCount]=pointer;heaps[freeCount]=heap;}
 freeCount++;
}
void setPending(u32 n) { state.pendingTasks=n; }
void clearFrame(void) { state.frame=0; }
void setFlags(u16 n) { frame.flags=n; }
u32 getFlags(void) { return frame.flags; }
u32 getBudget(void) { return state.stepBudget; }
void setOwnedBlocks(int owned) {
 frame.storage=(void *)0x2020;frame.table038=(void *)0x3030;
 frame.table03C=(void *)0x4040;frame.resource=(void *)0x5050;frame.ownsResource=owned;
}
int popStateValid(void) { return state.frame==&parent && state.dispatchState==0 && state.dispatchIndex==-1; }
int freeOrderValid(int owned) {
 int i;
 void *expected[5]={(void *)0x2020,(void *)0x3030,(void *)0x4040,(void *)0x5050,&frame};
 if(!owned)expected[3]=&frame;
 if(freeCount!=4+owned)return 0;
 for(i=0;i<freeCount;i++) {
  if(freed[i]!=expected[i])return 0;
  if(heaps[i]!=(owned && i==3 ? 0 : state.heap))return 0;
 }
 return 1;
}
''')
        library=folder/'frames.so'
        subprocess.run(['gcc','-shared','-fPIC','-O2','-D__attribute__(x)=',
                        '-I'+str(ROOT/'include'),str(folder/'frames.c'),str(folder/'mock.c'),
                        '-o',str(library)],check=True)
        cls.lib=ctypes.CDLL(str(library))
        cls.lib.ScriptRunWorkBatch.restype=ctypes.c_uint32
        cls.lib.ScriptSetFrameFlag.argtypes=[ctypes.c_uint32]

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def setUp(self):self.lib.resetFrames()

    def value(self,name,value=None):
        field=ctypes.c_int.in_dll(self.lib,name)
        if value is not None:field.value=value
        return field.value

    def test_dispatch_frame_and_flag_bounds(self):
        self.assertEqual(self.lib.ScriptDispatchCurrentFrame(),1)
        self.assertEqual(self.value('calls'),1)
        self.lib.setFlags(0x4000)
        for index in (0,7):self.assertEqual(self.lib.ScriptSetFrameFlag(index),0)
        self.assertEqual(self.lib.getFlags(),0x4081)
        for index in (8,0xFFFFFFFF):self.assertEqual(self.lib.ScriptSetFrameFlag(index),-1)
        self.assertEqual(self.lib.getFlags(),0x4081)
        self.lib.clearFrame()
        self.assertEqual(self.lib.ScriptDispatchCurrentFrame(),0)
        self.assertEqual(self.value('calls'),1)
        self.assertEqual(self.lib.ScriptSetFrameFlag(0),-1)

    def test_work_batch_is_distinct_from_pending_aware_slice(self):
        self.lib.setPending(1)
        self.assertEqual(self.lib.ScriptRunWorkBatch(),3)
        self.assertEqual(self.value('calls'),3)
        self.lib.ScriptSetStepBudgetUnchecked(0)
        self.assertEqual(self.lib.getBudget(),10)
        for status,work,calls in [(2,4,2),(0,0,1),(-1,0xFFFFFFFF,1)]:
            self.lib.resetFrames();self.value('status',status)
            self.assertEqual(self.lib.ScriptRunWorkBatch(),work)
            self.assertEqual(self.value('calls'),calls)

    def test_pop_owned_and_borrowed_resources(self):
        for owned in (0,1):
            self.lib.resetFrames();self.lib.setOwnedBlocks(owned)
            self.lib.ScriptPopFrame()
            self.assertEqual(self.value('cleanupCalls'),1)
            self.assertEqual(self.lib.freeOrderValid(owned),1)
            self.assertEqual(self.value('parentRestored'),4+owned)
            self.assertEqual(self.lib.popStateValid(),1)

    def test_pop_skips_null_allocations(self):
        self.lib.ScriptPopFrame()
        self.assertEqual(self.value('freeCount'),1)
        self.assertEqual(self.value('parentRestored'),1)
        self.assertEqual(self.lib.popStateValid(),1)


if __name__=='__main__':unittest.main()
