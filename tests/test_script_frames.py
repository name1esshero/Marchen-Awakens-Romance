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
        source='struct ScriptContext; extern struct ScriptContext *hostVm;\n'+source.replace('gScriptContext','hostVm')
        (folder/'frames.c').write_text(source)
        (folder/'mock.c').write_text(r'''
#include "script_vm.h"
#include <string.h>
struct ScriptContext context, *hostVm=&context;
struct ScriptExecutionState state;
struct ScriptFrame frame, parent;
int calls, status, freeCount, parentRestored;
void *freed[16], *heaps[16];
void resetFrames(void) {
 memset(&context,0,sizeof(context));memset(&state,0,sizeof(state));
 memset(&frame,0,sizeof(frame));memset(&parent,0,sizeof(parent));
 context.state=&state;state.frame=&frame;state.heap=(void *)0x1010;
 state.stepBudget=3;state.dispatchState=7;state.dispatchIndex=9;
 frame.parent=&parent;calls=freeCount=parentRestored=0;status=1;
}
s32 sub_08080070(struct ScriptFrame *arg) { if(arg!=&frame)return -99;calls++;return status; }
void HeapFree(void *heap,void *pointer) {
 if(state.frame==&parent)parentRestored++;
 if(freeCount<16){freed[freeCount]=pointer;heaps[freeCount]=heap;}
 freeCount++;
}
void CpuFill(void *destination,u32 size,u32 value) { memset(destination,value,size); }
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
int releasePoolsValid(void) {
 struct ScriptFramePoolEntry direct[1], nested[1];
 void *elements[3]={(void *)0x6060,0,(void *)0x7070};
 u8 *raw=(u8 *)&frame;
 int i;
 resetFrames();
 direct[0].count=2;direct[0].data=(void *)0x5050;
 nested[0].count=3;nested[0].data=elements;
 frame.count034=1;frame.table038=direct;
 frame.count036=1;frame.table03C=nested;
 frame.programCounter=9;memset(frame.work048,0xA5,sizeof(frame.work048));
 frame.field084=0;memset(frame.callbackAddresses,0xA5,sizeof(frame.callbackAddresses));
 frame.dispatchFlags=0xFFFF;frame.flags=0xFFFF;
 *(u32 *)(raw+0x28)=11;*(u32 *)(raw+0x2C)=13;
 ScriptFrameReleasePools();
 if(freeCount!=4 || freed[0]!=(void *)0x5050 || freed[1]!=(void *)0x6060 ||
    freed[2]!=(void *)0x7070 || freed[3]!=elements)return 0;
 for(i=0;i<4;i++)if(heaps[i]!=state.heap)return 0;
 if(direct[0].count || direct[0].data || nested[0].count || nested[0].data)return 0;
 if(frame.programCounter || frame.field084!=24 || frame.dispatchFlags || frame.flags)return 0;
 for(i=0;i<sizeof(frame.work048);i++)if(frame.work048[i])return 0;
 for(i=0;i<8;i++)if(frame.callbackAddresses[i])return 0;
 return state.dispatchState==(u32)(unsigned long)&frame && state.dispatchIndex==0;
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
            self.assertEqual(self.lib.freeOrderValid(owned),1)
            self.assertEqual(self.value('parentRestored'),4+owned)
            self.assertEqual(self.lib.popStateValid(),1)

    def test_pop_skips_null_allocations(self):
        self.lib.ScriptPopFrame()
        self.assertEqual(self.value('freeCount'),1)
        self.assertEqual(self.value('parentRestored'),1)
        self.assertEqual(self.lib.popStateValid(),1)

    def test_release_pools_frees_nested_allocations_and_resets_frame(self):
        self.assertEqual(self.lib.releasePoolsValid(),1)


if __name__=='__main__':unittest.main()
