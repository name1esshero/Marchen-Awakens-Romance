/* Matched sound-driver control routines. Stop pauses the player, locks it,
 * stops each 80-byte track through 08078644, then restores the ready signature.
 * Update invokes the mixer; fade-out delegates to the interval setter. */
#include "sound.h"
#include "rom_section.h"
#define players gSoundPlayerTable
#define songs gSongTable
extern void sub_08077DC0(void);
extern void sub_08078644(struct SoundPlayer *, struct SoundTrack *);
extern void sub_08079240(struct SoundPlayer *);
extern u8 gIwramBase[];
extern u8 gSoundIrqModeOffset[];

/** Two driver-owned callbacks installed in IWRAM.  agbcc emits the shared
 * _call_via_r1 trampoline for these indirect calls. */
AT("00078DC4") void SoundCallCallback5DA8(u32 argument)
{
 void (*callback)(u32)=*(void (**)(u32))0x03005DA8;
 callback(argument);
}
AT("00078DD8") void SoundCallCallback5DAC(u32 argument)
{
 void (*callback)(u32)=*(void (**)(u32))0x03005DAC;
 callback(argument);
}

/** Stop DMA1 while servicing the software mixer, preserving IME state used by
 * the original interrupt path. */
AT("000010C8") void DisableDma1AndUpdateSound(void)
{
 volatile u16 *ime=(u16 *)0x04000208;
 *ime=0;
 *(volatile u16 *)0x0400010A=0;
 *ime=1;
 SoundUpdate();
}

/** Service the sound IRQ/DMA path. Before the mixer is enabled, this programs
 * DMA1's control halves; afterwards the same IRQ acknowledgement is followed
 * by a mixer update. */
AT("00001158") void SoundIrqService(void)
{
    u8 *base = gIwramBase;
    u32 offset = (u32)gSoundIrqModeOffset;
    s8 enabled = *(volatile s8 *)(base + offset);

    if (enabled == 0) {
        *(volatile u16 *)0x04000208 = 0;
        *(volatile u16 *)0x04000108 = 32;
        *(volatile u16 *)0x0400010A = 192;
        *(volatile u16 *)0x03007FF8 |= 1;
        *(volatile u16 *)0x04000208 = 1;
    } else {
        *(volatile u16 *)0x04000208 = 0;
        *(volatile u16 *)0x03007FF8 |= 1;
        *(volatile u16 *)0x04000208 = 1;
        SoundUpdate();
    }
}
AT("00078A64")
void SoundUpdate(void)
{
    sub_08077DC0();
}

AT("00078A64") const u8 SoundUpdateTail[2]={0,0};
AT("00079240")
void SoundPlayerStop(struct SoundPlayer *p)
{
    s32 count;
    struct SoundTrack *track;
    if (p->ident == SOUND_PLAYER_READY)
    {
        p->ident++;
        p->status |= SOUND_PLAYER_PAUSED;
        count=p->trackCount;
        track=p->tracks;
        while (count > 0)
        {
            sub_08078644(p,track);
            count--;
            track++;
        }
        p->ident=SOUND_PLAYER_READY;
    }
}

AT("00078C08")
void SoundFadeOut(struct SoundPlayer *p,u16 interval)
{
    SoundPlayerFadeOut(p,interval);
}

AT("00078C08") const u8 SoundFadeOutTail[2]={0,0};
