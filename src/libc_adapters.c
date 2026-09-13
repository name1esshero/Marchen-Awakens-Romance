/* Small glue routines used by the bundled C library.  Keeping these in C
 * documents the callbacks and fixed arguments that were opaque in assembly.
 */
#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))

extern s32 strtol(const char *text, char **end, s32 radix);

AT("00082640")
s32 ParseDecimalInteger(const char *text)
{
    return strtol(text, 0, 10);
}
