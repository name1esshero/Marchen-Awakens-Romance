/* Small adapters shared by the renderer and sprite runtime. */
#include "runtime_leaf.h"
#include "runtime_accessors.h"
#include "sound.h"

#define AT(x) __attribute__((section(".rom." x)))

extern void LZ77UnCompVram(const void *source, void *destination);
extern void sub_0807BEC0(void *state);
extern s32 sub_0806E4E8();
extern s32 sub_08004DA8(void);
extern void sub_08078A70(u16 song);
extern void sub_08078BA4(void);
extern void StopSoundPlayer(u32 player);

/* This call site uses destination/source order opposite to the BIOS wrapper. */
AT("00002148") void Lz77UnCompVramSwapped(void *destination, const void *source)
{
    LZ77UnCompVram(source, destination);
}

/* Reset the embedded resource beginning eight bytes into a sprite sidecar. */
AT("00008BD8") void SpriteAuxiliaryReset(void *state)
{
    sub_0807BEC0((u8 *)state + 8);
}

/* Advance the procedural map generator and return its new state. */
AT("0006E52C") s32 MapGeneratorStep(void)
{
    return sub_0806E4E8();
}
AT("0006E52C") const u8 MapGeneratorStepTail[2] = {0};

AT("00005084") s32 RuntimeReadSignedByte(void)
{
    return (s8)sub_08004DA8();
}
AT("00005084") const u8 RuntimeReadSignedByteTail[2] = {0};

AT("00005E98") void SoundSongStartU16(u32 song)
{
    sub_08078A70((u16)song);
}
AT("00005E98") const u8 SoundSongStartU16Tail[2] = {0};

AT("00005EF0") void RuntimeResetSelection(void)
{
    sub_08078BA4();
    GameStateSetField12EE(-1);
}
AT("00005EF0") const u8 RuntimeResetSelectionTail[2] = {0};

AT("00018E30") void SoundStopPlayers4And5(void)
{
    StopSoundPlayer(4);
    StopSoundPlayer(5);
}
AT("00018E30") const u8 SoundStopPlayers4And5Tail[2] = {0};

/* The sprite renderer stores its current origin in its global work block. */
AT("0007D21C") void SpriteSetViewportOrigin(u16 x,u16 y)
{
    u8 *state=*(u8 **)0x03006118;
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

AT("0006E538") s32 Crc32Difference(const void *data,u32 size,s32 expected)
{
 return expected-sub_0806E4E8(data,size);
}
AT("0006E538") const u8 Crc32DifferenceTail[2]={0};
