#include "sram.h"

#include "rom_section.h"
#define WAITCNT_ADDRESS ((vu16 *)0x04000204)
#define WAITCNT_SRAM_MASK 0xFFFC
#define WAITCNT_SRAM_8_CYCLES 3

/* Nintendo's SRAM library uses byte accesses and the cartridge bus's
 * eight-cycle SRAM setting.  These two identical entry points are retained
 * because SetSramFastFunc relocates ReadSram while callers also use
 * WriteSram directly from ROM. */
#define DEFINE_SRAM_COPY(name,address)                                      \
AT(address) void name(const u8 *source, u8 *destination, u32 size)          \
{                                                                           \
    register vu16 *waitcnt asm("r2") = WAITCNT_ADDRESS;                    \
    register u16 wait asm("r0") = *waitcnt;                                \
    register u16 mask asm("r1") = WAITCNT_SRAM_MASK;                       \
    wait &= mask;                                                            \
    wait |= WAITCNT_SRAM_8_CYCLES;                                          \
    *waitcnt = wait;                                                         \
    while (size--)                                                           \
        *destination++ = *source++;                                         \
}

DEFINE_SRAM_COPY(ReadSram, "00079EDC")
DEFINE_SRAM_COPY(WriteSram, "00079F1C")

/** Return the first mismatching SRAM address, or NULL when all bytes agree. */
AT("00079F5C")
u8 *VerifySram(const u8 *source, const u8 *destination, u32 size)
{
    register vu16 *waitcnt asm("r2") = WAITCNT_ADDRESS;
    register u16 wait asm("r0") = *waitcnt;
    register u16 mask asm("r1") = WAITCNT_SRAM_MASK;
    wait &= mask;
    wait |= WAITCNT_SRAM_8_CYCLES;
    *waitcnt = wait;

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
AT("00079F5C") const u8 VerifySramTail[2] = {0, 0};

/** Write and verify up to three times through the verifier copied to IWRAM. */
AT("0007A044")
u8 *WriteSramFast(const u8 *source, u8 *destination, u32 size)
{
    u8 *mismatch;
    u8 attempt = 0;

    while (attempt <= 2) {
        WriteSram(source, destination, size);
        mismatch = VerifySramFast(source, destination, size);
        if (mismatch == 0)
            break;
        attempt++;
    }
    return mismatch;
}
