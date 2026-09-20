#include "sram.h"

#include "gba/io_reg.h"

#define WAITCNT_SRAM_MASK 0xFFFC
#define WAITCNT_SRAM_8_CYCLES 3

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
