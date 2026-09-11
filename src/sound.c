/* Matched sound-driver control routines. Stop pauses the player, locks it,
 * stops each 80-byte track through 08078644, then restores the ready signature.
 * Update invokes the mixer; fade-out delegates to the interval setter. */
#include "sound.h"
#define AT(x) __attribute__((section(".rom." x)))
#define players ((const struct SoundPlayerEntry *)0x0808B144)
#define songs ((const struct SoundSongEntry *)0x0808B1B0)
extern void sub_08077DC0(void);
extern void sub_0807915C(struct SoundPlayer *, const void *);
extern void sub_08078644(struct SoundPlayer *, struct SoundTrack *);
extern void sub_080789CC(struct SoundPlayer *,u16);
extern void sub_08079240(struct SoundPlayer *);
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
    sub_080789CC(p,interval);
}

AT("00078C08") const u8 SoundFadeOutTail[2]={0,0};
