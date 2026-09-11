/* Link map for the MAR decompilation.
 *
 * Every generated fragment declares a section named .rom.<offset>, so sorting
 * by name lays the whole cartridge out in ROM order with no explicit list.
 * The ASSERTs below turn a mis-sized fragment into a link error instead of a
 * silently shifted image.
 */
ENTRY(Entry)

MEMORY
{
    ROM (rx) : ORIGIN = 0x08000000, LENGTH = 32M
}

SECTIONS
{
    . = 0x08000000;

    .rom :
    {
        KEEP(*(SORT_BY_NAME(.rom.*)))
    } > ROM

    __rom_end = .;

    /DISCARD/ : { *(.comment) *(.ARM.attributes) }
}

ASSERT(__rom_end == 0x09000000, "ROM image is not 16 MB: a fragment changed size")
