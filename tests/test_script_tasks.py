"""Run recovered VM C with a substituted context slot and mocked dispatch/pop.
ROM byte comparisons separately verify the actual agbcc code and literal pools.
"""
import ctypes
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ScriptTaskTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        folder = Path(cls.temp.name)
        source = (ROOT / 'src/script_tasks.c').read_text()
        # Replace only the hardware context-slot address, retaining the C logic.
        source = 'extern void *hostContext;\n' + source.replace('0x0300611C', '(unsigned long)&hostContext')
        (folder/'tasks.c').write_text(source)
        (folder/'mock.c').write_text(r'''
#include "gba/types.h"
#include <string.h>
u8 context[32], state[0x228], frames[3][0xB8];
void *hostContext;
s32 dispatchStatus, dispatchCalls, blockAfterDispatch, popCalls, nextBudget;
static void storePointer(u8 *dest,void *p) { memcpy(dest,&p,sizeof(p)); }
void resetVm(void) {
    int i;
    memset(context,0,sizeof(context));memset(state,0,sizeof(state));memset(frames,0,sizeof(frames));
    hostContext=context;storePointer(context+12,state);storePointer(state+12,frames[2]);
    for(i=1;i<3;i++)storePointer(frames[i]+64,frames[i-1]);
    *(u32 *)(state+0x214)=10;*(u32 *)(context+4)=1;
    dispatchStatus=1;dispatchCalls=blockAfterDispatch=popCalls=nextBudget=0;
}
s32 ScriptDispatchCurrentFrame(void) {
    dispatchCalls++;
    if(blockAfterDispatch)*(u32 *)(state+0x220)=1;
    if(nextBudget)*(u32 *)(state+0x214)=nextBudget;
    return dispatchStatus;
}
void ScriptPopFrame(void) {
    void *frame,*parent;
    memcpy(&frame,state+12,sizeof(frame));memcpy(&parent,(u8 *)frame+64,sizeof(parent));
    storePointer(state+12,parent);popCalls++;
}
void FinishTask(void *task) {}
void CpuFill(void *dest,u32 size,u32 value) {}
void clearState(void) { storePointer(context+12,0); }
int active(void) { return *(u32 *)(context+4); }
''')
        library = folder/'tasks.so'
        subprocess.run(['gcc','-shared','-fPIC','-O2','-fno-strict-aliasing',
                        '-D__attribute__(x)=','-I'+str(ROOT/'include'),
                        str(folder/'tasks.c'),str(folder/'mock.c'),'-o',str(library)],check=True)
        cls.lib = ctypes.CDLL(str(library))
        cls.lib.ScriptGetParentFrame.restype = ctypes.c_void_p
        cls.lib.ScriptCommandReturn.argtypes = [ctypes.c_uint32,ctypes.POINTER(ctypes.c_uint32)]
        for name in ('ScriptGetPendingTasks','ScriptGetStepBudget','ScriptGetResult'):
            getattr(cls.lib,name).restype = ctypes.c_uint32

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def setUp(self):
        self.lib.resetVm()

    def value(self,name,value=None):
        ref=ctypes.c_int.in_dll(self.lib,name)
        if value is not None:ref.value=value
        return ref.value

    def test_pending_operations_and_null_guards(self):
        self.lib.ScriptSetPendingTasks(3)
        self.lib.ScriptAddPendingTasks(2)
        self.lib.ScriptCompletePendingTasks(4)
        self.assertEqual(self.lib.ScriptGetPendingTasks(),1)
        self.lib.ScriptCompletePendingTasks(2)
        self.assertEqual(self.lib.ScriptGetPendingTasks(),0xFFFFFFFF)
        for absent_vm in (False,True):
            if absent_vm:ctypes.c_void_p.in_dll(self.lib,'hostContext').value=None
            else:self.lib.clearState()
            self.lib.ScriptSetPendingTasks(7)
            self.lib.ScriptAddPendingTasks(1)
            self.lib.ScriptCompletePendingTasks(1)
            self.lib.ScriptSetStepBudget(5)
            self.assertEqual(self.lib.ScriptGetPendingTasks(),0)
            self.assertEqual(self.lib.ScriptGetStepBudget(),0)
            self.assertIsNone(self.lib.ScriptGetParentFrame())

    def test_slice_budget_status_and_pending_block(self):
        self.lib.ScriptSetStepBudget(0)
        self.assertEqual(self.lib.ScriptGetStepBudget(),10)
        self.assertEqual(self.lib.ScriptRunSlice(),1)
        self.assertEqual(self.value('dispatchCalls'),10)
        self.lib.resetVm();self.lib.ScriptSetPendingTasks(1)
        self.assertEqual(self.lib.ScriptRunSlice(),1)
        self.assertEqual(self.value('dispatchCalls'),0)
        for status in (0,-1,2):
            self.lib.resetVm();self.lib.ScriptSetStepBudget(3)
            self.value('dispatchStatus',status)
            self.assertEqual(self.lib.ScriptRunSlice(),min(status,1))
            self.assertEqual(self.value('dispatchCalls'),2 if status==2 else 1)

    def test_slice_observes_dispatch_changes(self):
        for field in ('blockAfterDispatch','nextBudget'):
            self.lib.resetVm();self.value(field,1)
            self.assertEqual(self.lib.ScriptRunSlice(),1)
            self.assertEqual(self.value('dispatchCalls'),1)

    def test_return_unwinds_all_frames_and_publishes_result(self):
        self.assertIsNotNone(self.lib.ScriptGetParentFrame())
        arguments=(ctypes.c_uint32*1)(0x12345678)
        self.assertEqual(self.lib.ScriptCommandReturn(1,arguments),1)
        self.assertEqual(self.lib.ScriptGetResult(),0x12345678)
        self.assertEqual(self.value('popCalls'),3)
        self.assertIsNone(self.lib.ScriptGetParentFrame())
        self.lib.ScriptDeactivate()
        self.assertEqual(self.lib.active(),0)
        self.assertEqual(self.lib.ScriptGetResult(),0x12345678)


if __name__=='__main__':unittest.main()
