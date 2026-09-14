/* Packed flag helpers; no allocation or implicit bounds checks. */
#include "bitset.h"
#include "rom_section.h"
AT("0007A190")
void BitSet(u8 *bits, u32 index, s32 enabled)
{
    u8 *byte = bits + (index >> 3);
    u8 mask = 1 << (index & 7);

    if (enabled)
        *byte |= mask;
    else
        *byte &= ~mask;
}
AT("0007A190") const u8 BitSetTail[2] = {0, 0};

AT("0007A1BC")
u32 BitTest(const u8 *bits, u32 index)
{
    const u8 *byte = bits + (index >> 3);
    u8 mask = 1 << (index & 7);

    return (*byte & mask) != 0;
}
