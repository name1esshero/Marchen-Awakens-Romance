/* Decoded M4A player controls. Not linked: agbcc register allocation and
 * leaf-function prologues do not yet match the cartridge. The table bases
 * below are confirmed by the song-start consumer at 08078A70. */
#include "sound.h"
#define AT(x) __attribute__((section(".rom." x)))
#define players ((const struct SoundPlayerEntry *)0x0808B144)
#define songs ((const struct SoundSongEntry *)0x0808B1B0)
extern void sub_08077DC0(void);
extern void sub_0807915C(struct SoundPlayer *, const void *);
extern void sub_08078644(struct SoundPlayer *, struct SoundTrack *);
extern void sub_080789CC(struct SoundPlayer *,u16);
extern void sub_08079240(struct SoundPlayer *);
void SoundPlayerResume(struct SoundPlayer *p)
{
    if (p->ident == SOUND_PLAYER_READY) p->status &= ~SOUND_PLAYER_PAUSED;
}

void SoundPlayerFadeOut(struct SoundPlayer *p,u16 interval)
{
    if (p->ident == SOUND_PLAYER_READY)
    {
        p->fadeCounter=interval;
        p->fadeInterval=interval;
        p->fadeVolume=256;
    }
}

void SoundPlayerFadeOutTemporary(struct SoundPlayer *p,u16 interval)
{
    if (p->ident == SOUND_PLAYER_READY)
    {
        p->fadeCounter=interval;
        p->fadeInterval=interval;
        p->fadeVolume=257;
    }
}

void SoundPlayerFadeIn(struct SoundPlayer *p,u16 interval)
{
    if (p->ident == SOUND_PLAYER_READY)
    {
        p->fadeCounter=interval;
        p->fadeInterval=interval;
        p->fadeVolume=2;
        p->status &= ~SOUND_PLAYER_PAUSED;
    }
}

void SoundSongStart(u16 song)
{
    const struct SoundSongEntry *s=&songs[song];
    sub_0807915C(players[s->player].player,s->header);
}

void SoundSongStop(u16 song)
{
    const struct SoundSongEntry *s=&songs[song];
    struct SoundPlayer *p=players[s->player].player;
    if (p->song==s->header) sub_08079240(p);
}

