"""Run the recovered input/RNG/bitset C with RAM and key-register substitutes."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[1]

class InputRandomTests(unittest.TestCase):
    def test_input_edges_direction_filtering_rng_sequence_and_bit_boundaries(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            header = root/'hardware.h'
            header.write_text('''#define gKeyStates testKeyStates
#define gKeyInputRegister testKeyRegister
#define gRandomSeed testRandomSeed
#include "input.h"
#include "random.h"
extern struct KeyState *testKeyStates;
extern vu16 testKeyRegister;
extern u32 testRandomSeed;
''')
            source = root/'test.c'
            source.write_text(r'''
#include <assert.h>
#include <string.h>
#include "bitset.h"
struct KeyState *testKeyStates;
vu16 testKeyRegister;
u32 testRandomSeed;
/* RuntimeRandom keeps its state in the secondary runtime allocation, which the
 * ROM link resolves; give the host build somewhere for it to point. */
static u8 runtimeAllocation[0x1000];
u8 *gSecondaryRuntime = runtimeAllocation;
void CpuFill(void *p, u32 size, u32 value) {
    assert(size==64 && value==0);
    memset(p, 0, size);
}
int main(void) {
    struct { u32 before; struct KeyState slots[8]; u32 after; } storage;
    u32 mask, expected, i;
    u8 bits[4] = {0,0,0,0};
    static const u32 states[] = {0x41C67EA6,0x967EB0E7,0x2781E494,0xC46B9B3D,0xF94BDF32};
    static const u32 values[] = {16838,5758,10113,17515,31051};
    memset(&storage, 0x5A, sizeof(storage));
    KeyInputInit(storage.slots);
    assert(testKeyStates==storage.slots);
    assert(storage.before==0x5A5A5A5A && storage.after==0x5A5A5A5A);
    for (i=0;i<8;i++)
        assert(!storage.slots[i].held && !storage.slots[i].pressed && !storage.slots[i].previous && !storage.slots[i].unused);
    for (mask=0;mask<1024;mask++) {
        /* Original horizontal filtering wins when all four directions are held. */
        expected=mask;
        if ((mask & 0x30)==0x30) expected=0x20;
        else if ((mask & 0xC0)==0xC0) expected=0x40;
        storage.slots[3].held=0x155;
        KeyInputSet(mask,3);
        assert(storage.slots[3].held==expected);
        assert(storage.slots[3].previous==0x155);
        assert(storage.slots[3].pressed==(expected & ~0x155u));
        storage.slots[4].held=0x155;
        testKeyRegister=(u16)~mask;
        KeyInputPoll(4);
        assert(storage.slots[4].held==expected);
        assert(storage.slots[4].pressed==storage.slots[3].pressed);
    }
    KeyInputSet(0,0);
    KeyInputSet(3,0);
    assert(KeyInputConsumePressed(1,0)==1);
    assert(KeyInputConsumePressed(1,0)==0);
    assert(storage.slots[0].pressed==2);
    assert(KeyInputAnyHeld(1,0)==1 && KeyInputAnyHeld(4,0)==0);
    assert(KeyInputConsumePressed(0,0)==0 && storage.slots[0].pressed==2);
    KeyInputSet(3,0);
    assert(storage.slots[0].pressed==0); /* Held buttons do not repeat edges. */
    KeyInputSet(0x8001,7);
    assert(storage.slots[7].held==0x8001); /* Set does not mask to ten keys. */
    RandomInit(1);
    for(i=0;i<5;i++) {
        assert(Random()==values[i]);
        assert(RandomGetSeed()==states[i]);
    }
    RandomSeed(0);
    assert(Random()==0 && RandomGetSeed()==12345);
    BitSet(bits,7,1); BitSet(bits,8,-1); BitSet(bits,31,1);
    assert(bits[0]==128 && bits[1]==1 && bits[2]==0 && bits[3]==128);
    assert(BitTest(bits,7)==1 && BitTest(bits,8)==1 && BitTest(bits,31)==1);
    assert(BitTest(bits,6)==0 && BitTest(bits,9)==0);
    BitSet(bits,7,0);
    assert(bits[0]==0 && bits[1]==1 && bits[3]==128);
    return 0;
}
''')
            exe = str(root/'test')
            subprocess.run(['cc','-D__attribute__(x)=','-I'+str(ROOT/'include'),
                            '-include',str(header),str(source),
                            *[str(ROOT/'src'/name) for name in ['input.c','random.c','bitset.c']],
                            '-o',exe],check=True)
            subprocess.run([exe],check=True)

if __name__=='__main__':unittest.main()
