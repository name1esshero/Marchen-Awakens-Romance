/* Small glue routines used by the bundled C library.  Keeping these in C
 * documents the callbacks and fixed arguments that were opaque in assembly.
 */
#include "gba/types.h"

#include "rom_section.h"

extern s32 strtol(const char *text, char **end, s32 radix);

/**
 * @brief Parse a base-ten signed integer.
 * @param text Null-terminated number text.
 * @return Parsed integer value using the bundled libc rules.
 */
AT("00082640")
s32 ParseDecimalInteger(const char *text)
{
    return strtol(text, 0, 10);
}
