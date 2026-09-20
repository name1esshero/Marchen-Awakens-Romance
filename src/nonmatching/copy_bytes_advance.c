/* CopyBytesAdvance, 0x0807E738. Called a dozen times by the 0x7E6xx-0x7F0xx
 * resource serializers to copy `size` bytes from `source` to `destination`,
 * returning the source pointer advanced past the copied bytes. Above 64
 * bytes it defers to CpuCopy (the BIOS DMA-backed bulk copy already used
 * elsewhere in this project); at or below 64 it copies byte by byte inline.
 *
 * Logically confirmed correct -- every read/write offset and the two return
 * values (source+size for the DMA path, the walked pointer for the byte
 * loop) match the ROM exactly -- but not byte-exact. The ROM keeps six
 * values in six distinct registers across the whole function: source twice
 * (r5, and a second copy r3 used only for the loop's own walk), destination
 * twice (r0, materialized unconditionally at the very top before any
 * branch, and r1, the untouched incoming parameter register used directly
 * by the loop), and size twice (r4, a copy used for both range checks, and
 * r2, the untouched incoming parameter register decremented directly by
 * the loop's own countdown).
 *
 * Every C shape tried collapses this to four registers instead of six,
 * because agbcc's optimizer proves several of those "copies" are
 * unnecessary and coalesces them:
 *   - A `const u8 *walker = source;` local coalesces with `source` itself
 *     wherever their live ranges do not overlap, giving one register (r5)
 *     instead of the ROM's two (r5 and r3).
 *   - Introducing an explicit `u8 *destCopy = destination;` local, used
 *     only in the DMA branch, does not force an early, unconditional copy;
 *     agbcc still proves it equals `destination` and only materializes it
 *     inside the branch that needs it, giving one register (r1) instead of
 *     the ROM's two (r0 materialized early, r1 used by the loop).
 *   - Writing the two range checks (`> 64`, then `> 0`) as independent,
 *     separately-named local comparisons rather than one guard does not
 *     stop agbcc from reading the same live value for both -- and does not
 *     stop the loop from decrementing that same register directly, giving
 *     one register (r4) instead of the ROM's two (r4 for the checks, r2
 *     for the loop's own countdown).
 *
 * This is the same class of difference already documented in
 * sound_idle_wait.c and sound_fade_create.c: the compiler finding a more
 * efficient register allocation than the ROM's original, not an
 * instruction-order or expression-grouping question reachable by
 * restructuring this candidate's C. Recovering it would need to explain why
 * the original source kept six values distinct where four suffice, not a
 * source-level trick aimed at the register allocator.
 */
#include "gba/types.h"

extern void CpuCopy(void *destination, const void *source, u32 size);

u8 *CopyBytesAdvance(const u8 *source, u8 *destination, u32 size)
{
    const u8 *walker = source;

    if (size > 64) {
        CpuCopy(destination, source, size);
        return (u8 *)source + size;
    }
    if (size > 0) {
        do {
            *destination++ = *walker++;
        } while (--size != 0);
    }
    return (u8 *)walker;
}
