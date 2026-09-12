/* Byte-oriented string length and XOR checksums, including high-bit bytes. */
#include "byte_utils.h"
#define AT(x) __attribute__((section(".rom." x)))

AT("0007A210")
u32 ByteStringLength(const u8 *text)
{
    u32 length = 0;

    while (*text++)
        length++;
    return length;
}

AT("0007A228")
u32 BufferXor(const u8 *data, u32 size)
{
    u32 value = 0, i;

    for (i = 0; i < size; i++)
        value ^= data[i];
    return value;
}

AT("0007A248")
u32 BufferXorSeeded(const u8 *data, u32 size, u8 seed)
{
    u32 value = 0, i;

    for (i = 0; i < size; i++)
        value ^= data[i];
    return value ^ seed;
}
