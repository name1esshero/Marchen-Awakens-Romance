#include "sram.h"

#include "gba/io_reg.h"

#define WAITCNT_SRAM_MASK (3 << 0)
#define WAITCNT_SRAM_8 (3 << 0)

/* These are the Nintendo AGB SDK's agb_sram.c entry points, written here in
 * the SDK's own shape (the same code pret's pokeemerald/pokefirered keep).
 *
 * Why this is nonmatching: the ROM's copies were built with the SDK's
 * library optimization level, not the game's -O2. Compiled at -O1 with the
 * project's agbcc, all three functions reproduce the ROM byte for byte
 * (ReadSram 64 bytes, WriteSram 64, VerifySram 74 plus its zero tail). At
 * -O2, both agbcc snapshots emit identical code except for the WAITCNT
 * read-modify-write: -O2 loads the WAITCNT value into r1 and the 0xFFFC
 * mask into r0 (`ldrh r1; ldr r0, =0xFFFC; ands r0, r1`), while -O1 and the
 * ROM load the value into r0 and the mask into r1. Earlier probes of five
 * different C spellings all hit the same swap because it is a property of
 * the optimization level, not of the source shape.
 *
 * PRET_STANDARDS.md forbids per-function compiler-flag changes as a matching
 * shortcut, so the exact routines remain named assembly in
 * asm/code/code_0780C0.s. Building a separate SDK object at -O1 (as pret
 * projects do for their SDK libraries) is a project-level decision for the
 * repository owner, not something to adopt from here. */

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
