/* A shrinking index pool used for drawing without replacement. */
#include "random.h"
#include "heap.h"
#include "rom_section.h"

/**
 * @brief Fill an index pool with the values zero through count minus one.
 * @param buffer Caller-owned storage, or NULL to allocate from the default heap.
 * @param count Number of halfword entries to initialize; not modified.
 * @return The initialized pool. The original caller assumes allocation succeeds.
 */
AT("000036F0")
u16 *RandomPoolInitialize(void *buffer, const u16 *count)
{
    u16 *start;
    u16 *output;
    u16 i;

    if (!buffer)
        buffer = HeapAlloc(0, *count * sizeof(u16));
    start = buffer;
    output = start;
    for (i = 0; i < *count; i++)
        *output++ = i;
    return start;
}
AT("000036F0") const u8 RandomPoolInitializeTail[2] = {0, 0};

/**
 * @brief Draw a random entry and remove it from the live part of a pool.
 * @param buffer Initialized pool; the removed slot receives its last live entry.
 * @param count Live entry count, decremented after a successful draw.
 * @return Selected entry. An empty pool returns buffer[0] without advancing RNG.
 * Empty-pool behavior still requires buffer[0] to be readable, as in the ROM.
 */
AT("00003724")
u16 RandomPoolTake(u16 *buffer, u16 *count)
{
    u16 index;
    u16 value;

    if (*count == 0)
        return buffer[0];
    index = Random() % *count;
    if (index + 1 >= *count)
        value = buffer[index];
    else
    {
        value = buffer[index];
        buffer[index] = buffer[*count - 1];
    }
    (*count)--;
    return value;
}
