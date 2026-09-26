#include "sram.h"

#include "gba/io_reg.h"

#define WAITCNT_SRAM_MASK (3 << 0)
#define WAITCNT_SRAM_8 (3 << 0)

/* These are the Nintendo AGB SDK's agb_sram.c entry points, written here in
 * the SDK's own shape (the same code pret's pokeemerald/pokefirered keep).
 *
 * Why this is nonmatching: at the project's -O2, both agbcc snapshots emit
 * the ROM's code except for the WAITCNT read-modify-write. agbcc loads the
 * WAITCNT value into r1 and the 0xFFFC mask into r0 (`ldrh r1; ldr r0,
 * =0xFFFC; ands r0, r1`); the ROM loads the value into r0 and the mask into
 * r1. This register swap is unresolved.
 *
 * Evidence only, not a resolution: compiled at -O1, this same source
 * reproduces all three ROM routines byte for byte (ReadSram 64 bytes,
 * WriteSram 64, VerifySram 74 plus its zero tail). That is consistent with a
 * different library build policy, but it does not establish one. MAR has
 * repeatedly seen apparent compiler or configuration requirements disappear
 * once the source shape was better understood. No independent provenance
 * shows these routines were built differently, so the -O2 discrepancy stays
 * open and the build keeps its fixed flags. The exact routines remain named
 * assembly in asm/code/code_0780C0.s. */

/**
 * @brief Configure the cartridge bus for SRAM and copy bytes out of it.
 * @param src SRAM source.
 * @param dest Destination buffer.
 * @param size Number of bytes to copy.
 */
void ReadSram(const u8 *src, u8 *dest, u32 size)
{
    REG_WAITCNT = (REG_WAITCNT & ~WAITCNT_SRAM_MASK) | WAITCNT_SRAM_8;
    while (--size != -1)
        *dest++ = *src++;
}

/**
 * @brief Configure the cartridge bus for SRAM and copy bytes into it.
 * @param src Source buffer.
 * @param dest SRAM destination.
 * @param size Number of bytes to copy.
 */
void WriteSram(const u8 *src, u8 *dest, u32 size)
{
    REG_WAITCNT = (REG_WAITCNT & ~WAITCNT_SRAM_MASK) | WAITCNT_SRAM_8;
    while (--size != -1)
        *dest++ = *src++;
}

/**
 * @brief Return the first mismatching SRAM address.
 * @param src Bytes expected in SRAM.
 * @param dest SRAM bytes to compare.
 * @param size Number of bytes to compare.
 * @return The first mismatching SRAM address, or null when all bytes agree.
 */
u8 *VerifySram(const u8 *src, const u8 *dest, u32 size)
{
    REG_WAITCNT = (REG_WAITCNT & ~WAITCNT_SRAM_MASK) | WAITCNT_SRAM_8;
    while (--size != -1)
    {
        if (*dest++ != *src++)
            return (u8 *)(dest - 1);
    }
    return 0;
}
