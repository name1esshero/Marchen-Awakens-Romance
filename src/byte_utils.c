/* Byte-oriented string length and XOR checksums, including high-bit bytes. */
#include "byte_utils.h"
#include "rom_section.h"

/** @brief Count bytes before the first zero terminator. */
AT("000025A4")
u32 CoreStringLength(const u8 *text)
{
    u32 length = 0;

    while (*text++)
        length++;
    return length;
}

/** @brief Count bytes before the first zero terminator. */
AT("0007A210")
u32 ByteStringLength(const u8 *text)
{
    u32 length = 0;

    while (*text++)
        length++;
    return length;
}

/** @brief XOR every byte in a fixed-size buffer. */
AT("0007A228")
u32 BufferXor(const u8 *data, u32 size)
{
    u32 value = 0, i;

    for (i = 0; i < size; i++)
        value ^= data[i];
    return value;
}

/** @brief XOR every buffer byte and then apply a one-byte seed. */
AT("0007A248")
u32 BufferXorSeeded(const u8 *data, u32 size, u8 seed)
{
    u32 value = 0, i;

    for (i = 0; i < size; i++)
        value ^= data[i];
    return value ^ seed;
}
