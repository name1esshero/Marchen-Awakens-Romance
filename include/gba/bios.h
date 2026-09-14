#ifndef GBA_BIOS_H
#define GBA_BIOS_H

#include "gba/types.h"

/* BIOS calls the game reaches through the thin wrappers in src/bios_calls.c.
 * The SWI numbers are fixed by the GBA BIOS, not by this game. */
#define SWI_ARCTAN2          0x0A
#define SWI_CPU_SET          0x0B
#define SWI_CPU_FAST_SET     0x0C
#define SWI_BG_AFFINE_SET    0x0E
#define SWI_OBJ_AFFINE_SET   0x0F
#define SWI_LZ77_UNCOMP_WRAM 0x11
#define SWI_LZ77_UNCOMP_VRAM 0x12
#define SWI_RL_UNCOMP_WRAM   0x14
#define SWI_REGISTER_RAM_RESET 0x01
#define SWI_SQRT             0x08
#define SWI_INTR_WAIT        0x04

/* CpuSet / CpuFastSet control word flags */
#define CPU_SET_SRC_FIXED 0x01000000
#define CPU_SET_16BIT     0x00000000
#define CPU_SET_32BIT     0x04000000

#endif /* GBA_BIOS_H */
