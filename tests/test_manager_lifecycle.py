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
s32 NfpMountIsActive(s32 handle) { return testNfpState->mounts[handle].active; }
void NfpSetMountActive(s32 handle,s32 active) { testNfpState->mounts[handle].active=active; }
char *strcpy(char *dest,const char *src) {
    char *start=dest;
    do { *dest++=*src; } while(*src++);
    return start;
}
char *strupr(char *text) {
    char *start=text;
    for(;*text;text++) if(*text>='a' && *text<='z') *text-='a'-'A';
    return start;
}
static List queues[3];
static union { u32 align; u8 bytes[64]; } taskStorage;
static void *failureArgument;
void sub_0807A4B8(void *argument) { failureArgument=argument; }
static void callback(struct EngineTask *task) { (void)task; }
static s32 findArchiveResult, findEntryResult;
static struct NfpEntry directory[3];
s32 NfpFindArchive(const char *name) { (void)name; return findArchiveResult; }
struct NfpHeader *NfpGetArchiveBase(s32 handle) {
    return testNfpState->mounts[handle].base;
}
struct NfpEntry *NfpGetEntry(s32 handle,s32 index) {
    (void)handle;
    return index < 0 ? 0 : &directory[index];
}
s32 NfpFindEntryIndex(s32 handle,const char *name) {
    (void)handle; (void)name; return findEntryResult;
}
static struct EngineTask scheduled[4];
static u32 visited[4], visitCount;
static void runCallback(struct EngineTask *task) {
    visited[visitCount++]=(u32)(task-scheduled);
    if(task==&scheduled[1]) {
        task->state=-1;
        ListAppend(&task->manager->queues[1],&scheduled[3].node);
        task->manager->taskCount++;
    }
}
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
    struct EngineTask *task;
    struct EngineTask anchor;
    u32 completion;
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
    mounts[0].active=0;
    mounts[1].active=1;
    CHECK(!NfpGetMountName(0) && NfpCountMounted()==1);
    NfpSetMountName(1,"mixed_09");
    CHECK(NfpGetMountName(1)==mounts[1].name);
    CHECK(mounts[1].name[0]=='M' && mounts[1].name[4]=='D' && mounts[1].name[5]=='_');
    CHECK(mounts[1].name[6]=='0' && mounts[1].name[7]=='9' && !mounts[1].name[8]);
    mounts[1].base=(struct NfpHeader *)0x1234;
    mounts[1].size=100;
    NfpUnmount(1);
    CHECK(!NfpGetMountName(1) && NfpCountMounted()==0);
    CHECK(mounts[1].base==(struct NfpHeader *)0x1234 && mounts[1].size==100 && mounts[1].name[0]=='M');
    mounts[0].active=1;
    mounts[1].active=2;
    CHECK(NfpCountMounted()==2);
    mounts[1].base=(struct NfpHeader *)taskStorage.bytes;
    mounts[1].base->count=3;
    findArchiveResult=1;
    CHECK(NfpGetEntryCountByName("ARC")==3);
    directory[2].offset=17;
    CHECK(NfpOpenByIndex("ARC",2)==taskStorage.bytes+17);
    findEntryResult=2;
    CHECK(NfpFindEntryByArchiveName("ARC","MEM")==2);
    findEntryResult=-1;
    CHECK(NfpFindEntryByArchiveName("ARC","MISS")==-1);
    findArchiveResult=-1;
    CHECK(NfpGetEntryCountByName("MISS")==0);
    CHECK(NfpOpenByIndex("MISS",2)==0);
    CHECK(NfpFindEntryByArchiveName("MISS","MEM")==0);
    state.mount_count=0;
    CHECK(NfpCountMounted()==0);
    state.mount_count=2;
    freeCount=0;
    NfpShutdown();
    CHECK(freeCount==1 && freed[0]==mounts && freeHeap==&heap);
    CHECK(!testNfpState && !state.mounts && !state.heap && !state.mount_count);
    result=queues;
    TaskManagerInit(&manager,&heap,3);
    result=taskStorage.bytes;
    for(i=0;i<64;i++) taskStorage.bytes[i]=0xA5;
    completion=99;
    CHECK(sizeof(struct EngineTask)==32);
    task=TaskCreateInQueue(&manager,callback,2,&completion,13);
    CHECK(task==(struct EngineTask *)taskStorage.bytes && allocationSize==45);
    CHECK(task->manager==&manager && task->callback==callback && task->priority==2);
    CHECK(task->state==1 && task->completion==&completion && completion==0);
    CHECK(queues[2].head==&task->node && queues[2].tail==&task->node && queues[2].count==1);
    CHECK(manager.taskCount==1);
    for(i=32;i<45;i++) CHECK(taskStorage.bytes[i]==0);
    CHECK(taskStorage.bytes[45]==0xA5);
    ListRemove(&queues[2],&task->node);
    manager.taskCount=0;
    anchor.priority=1;
    ListAppend(&queues[1],&anchor.node);
    task=TaskCreateBefore(&manager,callback,&anchor,0,0);
    CHECK(allocationSize==32 && task->priority==1 && !task->completion);
    CHECK(queues[1].head==&task->node && task->node.next==&anchor.node);
    CHECK(anchor.node.prev==&task->node && queues[1].tail==&anchor.node && queues[1].count==2);
    CHECK(manager.taskCount==1);
    result=0;
    completion=99;
    CHECK(!TaskCreateInQueue(&manager,callback,0,&completion,4));
    CHECK(failureArgument==(void *)0x00600000 && completion==99 && manager.taskCount==1);
    failureArgument=0;
    CHECK(!TaskCreateBefore(&manager,callback,&anchor,&completion,4));
    CHECK(failureArgument==(void *)0x00600000 && completion==99 && queues[1].count==2);
    result=queues;
    TaskManagerInit(&manager,&heap,3);
    result=taskStorage.bytes;
    task=CreateTask(&manager,callback,2,0,0);
    CHECK(task && task->priority==2 && manager.taskCount==1);
    task->state=0;
    FinishTask(task);
    CHECK(task->state==0);
    task->state=1;
    FinishTask(task);
    CHECK(task->state==-1);
    FinishTask(task);
    CHECK(task->state==-1);
    ListRemove(&queues[2],&task->node);
    manager.taskCount=0;
    anchor.priority=1;
    ListAppend(&queues[1],&anchor.node);
    task=CreateTask(&manager,callback,(u32)&anchor,0,0);
    CHECK(task && task->priority==1 && task->node.next==&anchor.node);
    result=queues;
    TaskManagerInit(&manager,&heap,3);
    CpuFill(scheduled,sizeof(scheduled),0);
    for(i=0;i<4;i++) {
        scheduled[i].manager=&manager;
        scheduled[i].priority=(i==0 ? 0 : 1);
        scheduled[i].callback=runCallback;
        scheduled[i].state=1;
    }
    scheduled[0].state=-1;
    ListAppend(&queues[0],&scheduled[0].node);
    ListAppend(&queues[1],&scheduled[1].node);
    ListAppend(&queues[1],&scheduled[2].node);
    manager.taskCount=3;
    freeCount=0;
    TaskManagerRun(&manager);
    CHECK(visitCount==3 && visited[0]==1 && visited[1]==2 && visited[2]==3);
    CHECK(freeCount==2 && freed[0]==&scheduled[0] && freed[1]==&scheduled[1]);
    CHECK(manager.taskCount==2 && queues[0].count==0 && queues[1].count==2);
    CHECK(queues[1].head==&scheduled[2].node && queues[1].tail==&scheduled[3].node);
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
                            *(str(ROOT/'src'/name) for name in ('task_manager.c','task_create.c','task_scheduler.c','nfp_lifecycle.c','nfp_mount_helpers.c','nfp_convenience.c','list.c')),
                            '-o',exe],check=True)
            subprocess.run([exe],check=True)

if __name__ == '__main__': unittest.main()
