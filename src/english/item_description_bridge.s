@ English-only tail bridge replacing the 16-byte ItemGetDescription accessor.
.syntax unified
.thumb
.section .rom.00056484,"ax"
.balign 4
.global ItemGetDescription
.type ItemGetDescription,%function
.thumb_func
ItemGetDescription:
    ldr r3, .Ltarget
    bx r3
    .space 8, 0
.Ltarget:
    .word EnglishItemGetDescription
.size ItemGetDescription, . - ItemGetDescription
