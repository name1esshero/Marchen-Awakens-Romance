@ Wrappers around GBA BIOS calls.
@
@ Each is a bare `swi` followed by a return; there is no C way to express
@ "no prologue, no epilogue, just this one instruction" without a naked
@ function relying on an inline-asm body, which is the shortcut the pret
@ Golden Rule forbids. Writing them here instead, exactly as pret projects
@ write their own SWI wrappers (e.g. pokeemerald's libagbsyscall.s), needs no
@ such compiler cooperation: the assembler places precisely these bytes.
@ SWI numbers are the BIOS's own, listed in include/gba/bios.h.

	.section .rom.00079E98, "ax"
	.thumb_func
	.thumb
	.global ArcTan2
ArcTan2:
	svc 0x0A
	bx lr

	.section .rom.00079E9C, "ax"
	.thumb_func
	.thumb
	.global BgAffineSet
BgAffineSet:
	svc 0x0E
	bx lr

	.section .rom.00079EA0, "ax"
	.thumb_func
	.thumb
	.global CpuFastSet
CpuFastSet:
	svc 0x0C
	bx lr

	.section .rom.00079EA4, "ax"
	.thumb_func
	.thumb
	.global CpuSet
CpuSet:
	svc 0x0B
	bx lr

@ The ROM clears r2 first; the BIOS ignores it, but the instruction is part
@ of the function's bytes.
	.section .rom.00079EA8, "ax"
	.thumb_func
	.thumb
	.global IntrWait
IntrWait:
	movs r2, #0
	svc 0x04
	bx lr
	.byte 0x00
	.byte 0x00

	.section .rom.00079EB0, "ax"
	.thumb_func
	.thumb
	.global LZ77UnCompVram
LZ77UnCompVram:
	svc 0x12
	bx lr

	.section .rom.00079EB4, "ax"
	.thumb_func
	.thumb
	.global LZ77UnCompWram
LZ77UnCompWram:
	svc 0x11
	bx lr

	.section .rom.00079EB8, "ax"
	.thumb_func
	.thumb
	.global RLUnCompWram
RLUnCompWram:
	svc 0x14
	bx lr

	.section .rom.00079EBC, "ax"
	.thumb_func
	.thumb
	.global RegisterRamReset
RegisterRamReset:
	svc 0x01
	bx lr

@ Return to the BIOS and restart the cartridge.
@
@ The standard GBA soft-reset sequence: interrupts off, stack pointer back to
@ the top of the user stack, then RegisterRamReset followed by SoftReset. The
@ BIOS never returns, so there is nothing after the two swi instructions.
	.section .rom.00079EC0, "ax"
	.thumb_func
	.thumb
	.global SoftReset
SoftReset:
	ldr r3, _08079ED0 @ REG_IME
	movs r2, #0
	strb r2, [r3, #0]
	ldr r1, _08079ED4 @ top of the user stack
	mov sp, r1
	svc 0x01
	svc 0x00
	.align 2, 0
_08079ED0:
	.4byte 0x04000208
_08079ED4:
	.4byte 0x03007F00

	.section .rom.00079ED8, "ax"
	.thumb_func
	.thumb
	.global Sqrt
Sqrt:
	svc 0x08
	bx lr
