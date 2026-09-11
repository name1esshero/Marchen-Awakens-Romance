/* Original 16 MB placement stays fixed; optional C/text occupies ROM expansion. */
INCLUDE ld_script.ld
SECTIONS
{
    .english 0x09000000 :
    {
        KEEP(*(.english.entry))
        *(.english.original)
        build/english/*.o(.text .text.* .rodata .rodata.*)
    } > ROM
}
ASSERT(SIZEOF(.english) < 0x1000000, "English extension exceeds the GBA ROM limit")
