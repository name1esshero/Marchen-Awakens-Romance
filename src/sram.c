#include "sram.h"

#include "rom_section.h"

/** Write and verify up to three times through the verifier copied to IWRAM. */
AT("0007A044")
u8 *WriteSramFast(const u8 *source, u8 *destination, u32 size)
{
    u8 *mismatch;
    u8 attempt = 0;

    while (attempt <= 2) {
        WriteSram(source, destination, size);
        mismatch = VerifySramFast(source, destination, size);
        if (mismatch == 0)
            break;
        attempt++;
    }
    return mismatch;
}
