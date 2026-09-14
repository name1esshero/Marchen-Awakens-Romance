/* Packed flag helpers; no allocation or implicit bounds checks. */
#include "bitset.h"
#include "rom_section.h"
/**
 * @brief Set or clear one bit in a packed flag array.
 * @param bits Packed byte array to modify.
 * @param index Zero-based bit index.
 * @param enabled Nonzero to set the bit; zero to clear it.
 */
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

/**
 * @brief Test one bit in a packed flag array.
 * @param bits Packed byte array to inspect.
 * @param index Zero-based bit index.
 * @return One when the bit is set, otherwise zero.
 */
AT("0007A1BC")
u32 BitTest(const u8 *bits, u32 index)
{
    const u8 *byte = bits + (index >> 3);
    u8 mask = 1 << (index & 7);

    return (*byte & mask) != 0;
}
