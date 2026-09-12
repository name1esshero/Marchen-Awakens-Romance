"""Exercise recovered heap metadata on a real 32-bit ABI and checksum vectors."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[1]

class HeapByteTests(unittest.TestCase):
    def test_heap_boundaries_failure_paths_and_byte_operations(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            header = root/'hardware.h'
            header.write_text('''#define gDefaultHeap testDefaultHeap
#include "heap.h"
extern struct Heap *testDefaultHeap;
''')
            source = root/'test.c'
            source.write_text(r'''
#include "byte_utils.h"
#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)
struct Heap *testDefaultHeap;
static union { u32 align; u8 bytes[128]; } storage, allocated;
static u32 allocationCalls, fillCalls, lastSize, lastMode;
static struct Heap *lastHeap;
static void *allocationResult;
void CpuFill(void *dest,u32 size,u32 value) {
    u32 i;
    fillCalls++;
    for (i=0;i<size;i++) ((u8 *)dest)[i]=(u8)value;
}
void *sub_0807A54C(struct Heap *heap,u32 size,u32 mode) {
    allocationCalls++;lastHeap=heap;lastSize=size;lastMode=mode;
    return allocationResult;
}
static int run(void) {
    struct Heap *heap=(struct Heap *)storage.bytes;
    u32 i;
    const u8 data[]={0x00,0x80,0xFF,0x55};
    const u8 text[]={'a',0x82,0xA0,0,'z'};
    CHECK(sizeof(void *)==4 && sizeof(struct Heap)==8 && sizeof(struct HeapBlock)==8);
    for(i=0;i<128;i++) storage.bytes[i]=0xA5;
    CHECK(HeapCreate(heap,24)==0 && allocationCalls==0);
    for(i=0;i<128;i++) CHECK(storage.bytes[i]==0xA5);
    CHECK(HeapCreate(heap,25)==heap);
    CHECK(heap->size==28 && heap->scanStart==(struct HeapBlock *)(storage.bytes+8));
    CHECK(heap->scanStart->sizeAndFlags==22 && heap->scanStart->metadata==0);
    for(i=16;i<128;i++) CHECK(storage.bytes[i]==0xA5);
    CHECK(HeapCreate(heap,0xFFFFFFFFu)==0); /* Original unsigned rounding wraps. */
    CHECK(HeapCreate(0,0)==0 && allocationCalls==0);
    allocationResult=0;
    CHECK(HeapCreate(0,25)==0 && allocationCalls==1);
    CHECK(lastHeap==0 && lastSize==28 && lastMode==0);
    allocationResult=allocated.bytes;
    CHECK(HeapCreate(0,29)==(struct Heap *)allocated.bytes);
    CHECK(lastSize==32 && ((struct Heap *)allocated.bytes)->size==32);
    testDefaultHeap=(struct Heap *)allocated.bytes;
    CHECK(HeapInitDefault(0,64)==0 && fillCalls==0);
    CHECK(HeapInitDefault(heap,20)==0 && fillCalls==1);
    CHECK(testDefaultHeap==(struct Heap *)allocated.bytes);
    for(i=0;i<16;i++) CHECK(storage.bytes[i]==0);
    CHECK(HeapInitDefault(heap,64)==heap && testDefaultHeap==heap);
    CHECK(heap->size==64 && heap->scanStart->sizeAndFlags==58);
    CHECK(HeapAlloc(heap,33)==allocated.bytes);
    CHECK(lastHeap==heap && lastSize==33 && lastMode==0);
    CHECK(ByteStringLength((const u8 *)"")==0 && ByteStringLength(text)==3);
    CHECK(BufferXor(0,0)==0 && BufferXor(data,4)==0x2A);
    CHECK(BufferXorSeeded(0,0,0xF0)==0xF0);
    CHECK(BufferXorSeeded(data,4,0xF0)==0xDA);
    return 0;
}
void _start(void) {
    int result=run();
    __asm__ volatile("int $0x80" : : "a"(1), "b"(result));
    __builtin_unreachable();
}
''')
            exe = str(root/'test')
            subprocess.run(['cc','-m32','-nostdlib','-fno-pie','-no-pie',
                            '-fno-stack-protector','-fno-builtin','-D__attribute__(x)=',
                            '-I'+str(ROOT/'include'),'-include',str(header),str(source),
                            str(ROOT/'src/heap.c'),str(ROOT/'src/byte_utils.c'),'-o',exe],check=True)
            subprocess.run([exe],check=True)

if __name__=='__main__':unittest.main()
