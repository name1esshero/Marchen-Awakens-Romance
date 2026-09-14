/* Generic bit-scan helper shared by the map runtime's flag words. */
#include "gba/types.h"

#include "rom_section.h"

#define BIT_SCAN_LAST_BIT 31
#define BIT_SCAN_NOT_FOUND (-1)

/**
 * @brief Return the index of the lowest set bit of a word.
 * @param value Word to scan.
 * @return Bit index 0..31, or -1 when no bit is set.
 *
 * The gotos reproduce the original basic-block order: the ROM emits the
 * "found" return between the zero guard and the scan loop, and reaches the
 * not-found return through a separate branch rather than by inverting the
 * guard.  Writing the same logic as a plain nested if/for compiles to the
 * identical instructions in a different block order, which is a byte
 * mismatch.
 */
AT("000705F0") s32 FindLowestSetBit(s32 value)
{
 s32 index=0;
 s32 i;

 if (value!=0)
  goto scan;
 goto notFound;
found:
 return index;
scan:
 for (i=0;i<=BIT_SCAN_LAST_BIT;i++) {
  if (value&1)
   goto found;
  value>>=1;
  index++;
 }
notFound:
 return BIT_SCAN_NOT_FOUND;
}
