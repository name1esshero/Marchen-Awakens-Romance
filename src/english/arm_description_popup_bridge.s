@ English-only replacement for sub_080537D8 (0x080537D8). NOT currently
@ wired into the Makefile -- every symbol-override mechanism tried to make
@ this win over the original without touching code_0500C0.o's ~50 other
@ symbols breaks mar_english.elf's .rom section by exactly 16 bytes. See
@ src/english/arm_description_popup.c's header and "sub_080537D8 English
@ bridge: confirmed fix, blocked by a linker quirk" in
@ docs/decompilation-notes.md before repeating this investigation.
.syntax unified
.thumb

.text
.balign 4
.global sub_080537D8
.type sub_080537D8,%function
.thumb_func
sub_080537D8:
    ldr r3, .Ltarget
    bx r3
    .balign 4
.Ltarget:
    .word EnglishArmDescriptionPopup
.size sub_080537D8, . - sub_080537D8
