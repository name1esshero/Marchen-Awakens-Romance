@ GBA cartridge header.
@
@ The first word is the reset branch; bytes 0x04..0x9F are the Nintendo logo,
@ which the BIOS checksums on boot and which therefore has to be reproduced
@ byte for byte. The remaining fields describe the cartridge to the BIOS.

	.section .rom.00000000, "ax"
	.arm
	.global rom_header
rom_header:
	b Entry                       @ 0x00 reset vector

	.incbin "data/nintendo_logo.bin"   @ 0x04 Nintendo logo (BIOS-verified)

	.ascii "MARHEAVEN1"
	.space 2                          @ 0xA0 game title, NUL padded to 12
	.ascii "BM9J"                    @ 0xAC game code
	.ascii "A4"                      @ 0xB0 maker code
	.byte 0x96                       @ 0xB2 fixed value
	.byte 0x00                       @ 0xB3 main unit code
	.byte 0x00                       @ 0xB4 device type
	.space 7                           @ 0xB5 reserved
	.byte 0x00                       @ 0xBC software version
	.byte 0x02                       @ 0xBD header checksum
	.space 2                           @ 0xBE reserved
