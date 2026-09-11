@ Preserve the original constructor's span and ABI in the optional English ROM.
@ r0..r3 are the four original arguments. The call returns the original task.
.syntax unified
.thumb
.section .rom.00011790,"ax"
.global DialogueStart
.type DialogueStart,%function
.thumb_func
DialogueStart:
    push {r4, lr}
    ldr r4, .Lenglish
    bl .Lcall
    pop {r4}
    pop {r1}
    bx r1
.Lcall:
    bx r4
.align 2
.Lenglish:
    .word EnglishDialogueStart
    .space 224 - (. - DialogueStart), 0
.size DialogueStart, . - DialogueStart
