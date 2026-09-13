@ English-only tail bridge replacing the 16-byte ItemGetName accessor.
.syntax unified
.thumb
.section .rom.00056474,"ax"
.balign 4
.global ItemGetName
.type ItemGetName,%function
.thumb_func
ItemGetName:
    ldr r3, .Ltarget
    bx r3
    .space 8, 0
.Ltarget:
    .word EnglishItemGetName
.size ItemGetName, . - ItemGetName
