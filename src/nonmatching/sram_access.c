#include "sram.h"

#include "gba/io_reg.h"

#define WAITCNT_SRAM_MASK 0xFFFC
#define WAITCNT_SRAM_8_CYCLES 3

/* Everything past the WAITCNT setup already matches byte-for-byte: the
 * `while (size--) *destination++ = *source++;` shape reproduces the ROM's
 * distinctive pre-decrement-then-compare-against-(-1) loop sentinel exactly
 * (`subs size,#1; movs r0,#1; negs r0,r0; cmp size,r0; beq done`), not just
 * approximately. What remains is narrow: the ROM loads the WAITCNT value
 * into r0 and the 0xFFFC mask into r1 (`ldrh r0,[waitcnt]; ldr r1,=0xFFFC;
 * ands r0,r0,r1`), while five C shapes tried here -- the compound `wait &=
 * mask;`, one combined `(REG_WAITCNT & 0xFFFC) | 3` expression, a named
 * `mask` local declared before or after the register read, and a raw
 * `volatile u16 *` pointer instead of the `REG_WAITCNT` macro -- all
 * produce the same result with the two registers swapped (mask in r0,
 * WAITCNT's value in r1). The AND's logical operands are already in the
 * ROM's order (WAITCNT's value first, the mask second); only the physical
 * register each lands in differs. */
/** Configure the cartridge bus for SRAM and copy bytes from source to destination. */
#define DEFINE_SRAM_COPY(name)                                      \
void name(const u8 *source, u8 *destination, u32 size)              \
{                                                                   \
    u16 wait = REG_WAITCNT;                                         \
    wait &= WAITCNT_SRAM_MASK;                                      \
    wait |= WAITCNT_SRAM_8_CYCLES;                                  \
    REG_WAITCNT = wait;                                             \
    while (size--)                                                  \
        *destination++ = *source++;                                 \
}

/* SetSramFastFunc relocates ReadSram to IWRAM, while other callers invoke
 * WriteSram from ROM. The SDK therefore provides two identical entry points. */
DEFINE_SRAM_COPY(ReadSram)
DEFINE_SRAM_COPY(WriteSram)

/**
 * @brief Return the first mismatching SRAM address.
 * @param source Bytes expected in SRAM.
 * @param destination SRAM bytes to compare.
 * @param size Number of bytes to compare.
 * @return The first mismatching SRAM address, or null when all bytes agree.
 */
u8 *VerifySram(const u8 *source, const u8 *destination, u32 size)
{
    u16 wait = REG_WAITCNT;

    wait &= WAITCNT_SRAM_MASK;
    wait |= WAITCNT_SRAM_8_CYCLES;
    REG_WAITCNT = wait;

    while (size--) {
        u8 actual = *destination;
        u8 expected = *source;
        source++;
        destination++;
        if (actual != expected)
            return (u8 *)(destination - 1);
    }
    return 0;
}
