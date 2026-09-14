/* 32-bit wrapping LCG: state = state * 1103515245 + 12345.
 * Random returns bits 16..30, a value in 0..32767, and retains the full state.
 */
#include "random.h"
#include "runtime_state.h"
#include "rom_section.h"
AT("0007A184")
void RandomInit(u32 seed)
{
    RandomSeed(seed);
}
AT("0007A184") const u8 RandomInitTail[2] = {0, 0};

AT("0007A1D8")
void RandomSeed(u32 seed)
{
    gRandomSeed = seed;
}

AT("0007A1E4")
u32 RandomGetSeed(void)
{
    return gRandomSeed;
}

AT("0007A1F0")
u32 Random(void)
{
    u32 next = gRandomSeed * 0x41C64E6D + 0x3039;

    gRandomSeed = next;
    return (next >> 16) & 0x7FFF;
}

/* A second generator running the same LCG, but over its own state inside the
 * secondary runtime allocation rather than gRandomSeed. What distinguishes the
 * two callers is not yet recovered; all known call sites are still in
 * asm/code/code_0080C0.s. */
AT("0000832C")
u32 RuntimeRandom(void)
{
    u8 **root = &gSecondaryRuntime;
    u32 *seed = (u32 *)(*root + 0xEA4);
    u32 value = 0x41C64E6D * (*seed);

    value += 12345;
    *seed = value;
    return (value << 1) >> 17;
}
