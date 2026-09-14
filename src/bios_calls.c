/* Wrappers around GBA BIOS calls.
 *
 * Each is a bare `swi` followed by a return, so they are written naked: the
 * compiler must not add a prologue or the bytes would no longer match. The
 * SWI numbers are the BIOS's own, listed in include/gba/bios.h. */

#include "gba/types.h"
#include "gba/bios.h"

#define AT_ROM(off) __attribute__((naked, section(".rom." off), used))

AT_ROM("00079E98") void ArcTan2(s16 x, s16 y)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_ARCTAN2));
}

AT_ROM("00079E9C") void BgAffineSet(void *src, void *dest, s32 count)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_BG_AFFINE_SET));
}

AT_ROM("00079EA0") void CpuFastSet(const void *src, void *dest, u32 control)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_CPU_FAST_SET));
}

AT_ROM("00079EA4") void CpuSet(const void *src, void *dest, u32 control)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_CPU_SET));
}

AT_ROM("00079EB0") void LZ77UnCompVram(const void *src, void *dest)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_LZ77_UNCOMP_VRAM));
}

AT_ROM("00079EB4") void LZ77UnCompWram(const void *src, void *dest)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_LZ77_UNCOMP_WRAM));
}
AT_ROM("00079EB8") void RLUnCompWram(const void *src, void *dest)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_RL_UNCOMP_WRAM));
}

AT_ROM("00079EBC") void RegisterRamReset(u32 flags)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_REGISTER_RAM_RESET));
}

AT_ROM("00079ED8") u32 Sqrt(u32 value)
{
    asm volatile("swi %0\n\tbx lr" :: "i"(SWI_SQRT));
}
/* Return to the BIOS and restart the cartridge.
 *
 * The standard GBA soft-reset sequence: interrupts off, stack pointer back to
 * the top of the user stack, then RegisterRamReset followed by SoftReset. It
 * has to be naked because the BIOS never returns, so a prologue would be dead
 * weight and would change the bytes.
 */
__attribute__((naked, section(".rom.00079EC0"), used))
void SoftReset(void)
{
    asm volatile(
        "ldr r3, =0x04000208\n"      /* REG_IME */
        "movs r2, #0\n"
        "strb r2, [r3, #0]\n"
        "ldr r1, =0x03007F00\n"      /* top of the user stack */
        "mov sp, r1\n"
        "swi %0\n"
        "swi %1\n"
        :: "i"(SWI_REGISTER_RAM_RESET), "i"(0));
}

/* Wait for an interrupt. The ROM clears r2 first; the BIOS ignores it, but the
 * instruction is part of the function's bytes. */
AT_ROM("00079EA8") void IntrWait(u32 discardOldFlags, u32 waitFlags)
{
    asm volatile("movs r2, #0\n\tswi %0\n\tbx lr" :: "i"(SWI_INTR_WAIT));
}
/* Six bytes of instructions, so the section needs its two zero bytes spelled
 * out; the assembler would otherwise pad the code section with a Thumb nop. */
__attribute__((section(".rom.00079EA8"), used))
const u8 IntrWaitTail[2] = {0};

/* Aliases for the labels the not-yet-decompiled assembly still calls. */
void sub_08079E98(s16, s16)              __attribute__((alias("ArcTan2")));
void sub_08079E9C(void *, void *, s32)   __attribute__((alias("BgAffineSet")));
void sub_08079EA0(const void *, void *, u32) __attribute__((alias("CpuFastSet")));
void sub_08079EA4(const void *, void *, u32) __attribute__((alias("CpuSet")));
void sub_08079EB0(const void *, void *)  __attribute__((alias("LZ77UnCompVram")));
void sub_08079EB4(const void *, void *)  __attribute__((alias("LZ77UnCompWram")));
void sub_08079EB8(const void *, void *)  __attribute__((alias("RLUnCompWram")));
void sub_08079EBC(u32)                   __attribute__((alias("RegisterRamReset")));
u32  sub_08079ED8(u32)                   __attribute__((alias("Sqrt")));
void sub_08079EC0(void)                  __attribute__((alias("SoftReset")));
