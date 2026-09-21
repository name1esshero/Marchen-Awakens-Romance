@ English-only tail bridges replacing the 24-byte consumable text accessors.
.syntax unified
.thumb

.section .rom.00057108,"ax"
.balign 4
.global ConsumableGetName
.type ConsumableGetName,%function
.thumb_func
ConsumableGetName:
    ldr r3, .LnameTarget
    bx r3
    .space 16, 0
.LnameTarget:
    .word EnglishConsumableGetName
.size ConsumableGetName, . - ConsumableGetName

.section .rom.00057120,"ax"
.balign 4
.global ConsumableGetDescription
.type ConsumableGetDescription,%function
.thumb_func
ConsumableGetDescription:
    ldr r3, .LdescriptionTarget
    bx r3
    .space 16, 0
.LdescriptionTarget:
    .word EnglishConsumableGetDescription
.size ConsumableGetDescription, . - ConsumableGetDescription

.section .rom.0005715C,"ax"
.balign 4
.global ConsumableGetResourceName
.type ConsumableGetResourceName,%function
.thumb_func
ConsumableGetResourceName:
    ldr r3, .LresourceNameTarget
    bx r3
    .space 16, 0
.LresourceNameTarget:
    .word EnglishConsumableGetResourceName
.size ConsumableGetResourceName, . - ConsumableGetResourceName
