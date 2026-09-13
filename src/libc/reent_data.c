/* Newlib-compatible global reentrancy state used by the bundled libc. */
#include <reent.h>

#define AT(x) __attribute__((section(".rom." x)))
#define C_LOCALE_NAME ((const char *)0x081AC8BC)

/* This is _REENT_INIT with the existing shared "C" locale string named
 * explicitly, so no compiler-created duplicate string enters the ROM. */
AT("00F2AD00")
struct _reent gLibcReentrancy = {
    0,
    &gLibcReentrancy.__sf[0],
    &gLibcReentrancy.__sf[1],
    &gLibcReentrancy.__sf[2],
    0,
    "",
    0,
    C_LOCALE_NAME,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    { { 1, 0, "", { 0, 0, 0, 0, 0, 0, 0, 0 }, 0 } }
};

AT("00F2AFEC")
struct _reent *_impure_ptr = &gLibcReentrancy;
