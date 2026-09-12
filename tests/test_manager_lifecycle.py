"""Check recovered task/archive ownership with the game's 32-bit pointer ABI."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[1]

class ManagerTests(unittest.TestCase):
    def test_lifecycle_and_linked_insertion(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            header = root / 'hardware.h'
            header.write_text('#define gNfpState testNfpState\n#include "nfp.h"\nextern struct NfpState *testNfpState;\n')
            source = root / 'test.c'
            source.write_text(r'''
#include "task_manager.h"
#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)
struct NfpState *testNfpState;
static List queues[3];
static ListNode nodes[3];
static struct NfpMount mounts[2];
static void *result, *freed[8];
static struct Heap *allocationHeap, *freeHeap;
static u32 allocationSize, freeCount, stateWasClear;
void *HeapAlloc(struct Heap *heap,u32 size) {
    allocationHeap=heap; allocationSize=size;
    if(testNfpState) stateWasClear=(!testNfpState->mounts && !testNfpState->heap && !testNfpState->mount_count);
    return result;
}
void HeapFree(struct Heap *heap,void *ptr) {
    u32 i;
    freeHeap=heap; freed[freeCount++]=ptr;
    for(i=0;i<3;i++) if(ptr==&nodes[i]) nodes[i].next=(ListNode *)1;
}
void CpuFill(void *dest,u32 size,u32 value) {
    u32 i; for(i=0;i<size;i++) ((u8 *)dest)[i]=(u8)value;
}
static int run(void) {
    struct Heap heap;
    struct TaskManager manager;
    struct NfpState state;
    u32 i;
    CHECK(sizeof(List)==12 && sizeof(manager)==16 && sizeof(state)==12 && sizeof(mounts[0])==24);
    result=queues;
    TaskManagerInit(&manager,&heap,3);
    CHECK(allocationHeap==&heap && allocationSize==36);
    CHECK(manager.heap==&heap && manager.queueCount==3 && TaskManagerCount(&manager)==0);
    for(i=0;i<3;i++) CHECK(!queues[i].head && !queues[i].tail && !queues[i].count);
    ListAppend(&queues[0],&nodes[2]);
    ListInsertBeforeLinked(&queues[0],&nodes[2],&nodes[0]);
    ListInsertBeforeLinked(&queues[0],&nodes[2],&nodes[1]);
    CHECK(queues[0].head==&nodes[0] && queues[0].tail==&nodes[2] && queues[0].count==3);
    CHECK(!nodes[0].prev && nodes[0].next==&nodes[1] && nodes[1].prev==&nodes[0]);
    CHECK(nodes[1].next==&nodes[2] && nodes[2].prev==&nodes[1] && !nodes[2].next);
    manager.taskCount=3;
    CHECK(TaskManagerCount(&manager)==3);
    TaskManagerDestroy(&manager);
    CHECK(freeCount==4 && freeHeap==&heap);
    for(i=0;i<3;i++) CHECK(freed[i]==&nodes[i]);
    CHECK(freed[3]==queues && !manager.queues && !manager.taskCount);
    CHECK(manager.heap==&heap && manager.queueCount==3);
    freeCount=0;
    TaskManagerInit(&manager,&heap,0);
    CHECK(allocationSize==0);
    TaskManagerDestroy(&manager);
    CHECK(freeCount==1 && freed[0]==queues);
    for(i=0;i<sizeof(mounts);i++) ((u8 *)mounts)[i]=0xA5;
    CpuFill(&state,sizeof(state),0xA5);
    result=mounts;
    NfpInit(&state,&heap,2);
    CHECK(testNfpState==&state && stateWasClear && allocationSize==48 && allocationHeap==&heap);
    CHECK(state.mounts==mounts && state.heap==&heap && state.mount_count==2);
    for(i=0;i<sizeof(mounts);i++) CHECK(((u8 *)mounts)[i]==0xA5);
    freeCount=0;
    NfpShutdown();
    CHECK(freeCount==1 && freed[0]==mounts && freeHeap==&heap);
    CHECK(!testNfpState && !state.mounts && !state.heap && !state.mount_count);
    result=0;
    NfpInit(&state,&heap,2);
    CHECK(!state.mounts && state.heap==&heap && state.mount_count==2);
    return 0;
}
void _start(void) {
    int result=run();
    __asm__ volatile("int $0x80" : : "a"(1), "b"(result));
    __builtin_unreachable();
}
''')
            exe = str(root / 'test')
            subprocess.run(['cc','-m32','-nostdlib','-fno-pie','-no-pie',
                            '-fno-stack-protector','-fno-builtin','-D__attribute__(x)=',
                            '-I'+str(ROOT/'include'),'-include',str(header),str(source),
                            *(str(ROOT/'src'/name) for name in ('task_manager.c','nfp_lifecycle.c','list.c')),
                            '-o',exe],check=True)
            subprocess.run([exe],check=True)

if __name__ == '__main__': unittest.main()
