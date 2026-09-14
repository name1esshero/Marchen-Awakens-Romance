#ifndef ROM_SECTION_H
#define ROM_SECTION_H

/* Pins a function or object to the ROM offset it matches, so the linker can
 * place it at the exact address the original binary used. Host-side tests
 * that link several matching functions into one executable predefine AT as
 * empty (e.g. -D'AT(x)=') to avoid the section-per-address placement, which
 * a host toolchain has no use for and which conflicts when a function and
 * its const tail share one section name. */
#ifndef AT
#define AT(x) __attribute__((section(".rom." x)))
#endif

#endif /* ROM_SECTION_H */
