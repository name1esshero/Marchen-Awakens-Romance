/* Small adapters shared by the renderer and sprite runtime. */
#include "runtime_leaf.h"
#include "runtime_accessors.h"
#include "sound.h"
#include "ncd.h"
#include "sprite_engine.h"

#include "rom_section.h"

extern void LZ77UnCompVram(const void *source, void *destination);
extern s32 sub_08004DA8(void);
extern void StopSoundPlayer(u32 player);

/** This call site uses destination/source order opposite to the BIOS wrapper. */
AT("00002148") void Lz77UnCompVramSwapped(void *destination, const void *source)
{
    LZ77UnCompVram(source, destination);
}

/** Reset the embedded resource beginning eight bytes into a sprite sidecar. */
AT("00008BD8") void SpriteAuxiliaryReset(void *state)
{
    NcdRuntimeSpriteReleaseAllocation((struct NcdSprite *)((u8 *)state + 8));
}

/** Public save-checksum entry point; the worker below contains the CRC loop. */
AT("0006E52C") u32 CalculateSaveCrc32(const void *data, u32 size)
{
    return CalculateCrc32(data, size);
}
AT("0006E52C") const u8 CalculateSaveCrc32Tail[2] = {0};

AT("00005084") s32 RuntimeReadSignedByte(void)
{
    return (s8)sub_08004DA8();
}
AT("00005084") const u8 RuntimeReadSignedByteTail[2] = {0};

/** Narrow-argument and narrow-result adapter for RuntimeTestFlag(). */
AT("00005094") u32 RuntimeTestFlagU8(u32 bit)
{
    return (u8)RuntimeTestFlag((u8)bit);
}
AT("00005094") const u8 RuntimeTestFlagU8Tail[2] = {0};

/** Narrow-argument wrapper so callers with a full-width song id can still
 * reach SoundSongStart(), which only takes a u16. */
AT("00005E98") void SoundSongStartU16(u32 song)
{
    SoundSongStart((u16)song);
}
AT("00005E98") const u8 SoundSongStartU16Tail[2] = {0};

/** Stop all sound players and clear the current menu selection (field12EE)
 * back to its "nothing selected" sentinel of -1. */
AT("00005EF0") void RuntimeResetSelection(void)
{
    SoundStopAllPlayers();
    GameStateSetField12EE(-1);
}
AT("00005EF0") const u8 RuntimeResetSelectionTail[2] = {0};

/** Stop the two sound players reserved for whatever caller needs both silent
 * at once; players 4 and 5 have no other meaning recovered yet. */
AT("00018E30") void SoundStopPlayers4And5(void)
{
    StopSoundPlayer(4);
    StopSoundPlayer(5);
}
AT("00018E30") const u8 SoundStopPlayers4And5Tail[2] = {0};

/** The sprite renderer stores its current origin in its global work block. */
AT("0007D21C") void SpriteSetViewportOrigin(u16 x,u16 y)
{
    u8 *state=(u8 *)gSpriteEngineState;
    *(u16 *)(state+328)=x;
    *(u16 *)(state+330)=y;
}

extern void HeapFree(void *heap,void *allocation);
AT("00003770") void HeapFreeDefault(void *allocation)
{
 if (allocation)
  HeapFree(0,allocation);
}
AT("00003770") const u8 HeapFreeDefaultTail[2]={0};

/** Compare a buffer's CRC-32 against an expected value.
 * @return Zero when they match, otherwise their signed difference. */
AT("0006E538") s32 Crc32Difference(const void *data,u32 size,s32 expected)
{
 return expected-CalculateCrc32(data,size);
}
AT("0006E538") const u8 Crc32DifferenceTail[2]={0};

/** Standard reflected CRC-32 used to validate the cartridge save block. */
AT("0006E4E8") u32 CalculateCrc32(const void *data, u32 size)
{
    const u8 *bytes;
    u32 length;
    u32 crc;
    u32 offset;

    bytes = data;
    length = size;
    crc = ~0u;

    for (offset = 0; offset < length; offset++) {
        u32 bit;

        crc ^= bytes[offset];
        for (bit = 0; bit <= 7; bit++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}
