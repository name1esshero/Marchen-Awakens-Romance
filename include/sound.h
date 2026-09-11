#ifndef MAR_SOUND_H
#define MAR_SOUND_H
#include "gba/types.h"

/* M4A-compatible player layout verified against 080789B0..08079280.
 * The player lock word is the little-endian signature "Smsh".
 * Song and sample data use ROM pointers, independently of the NFP archive.
 */
#define SOUND_PLAYER_READY 0x68736D53
#define SOUND_PLAYER_PAUSED 0x80000000
struct SoundTrack { u8 state[80]; };
struct SoundPlayer
{
    const void *song;             /* +00: current SongHeader */
    u32 status;                   /* +04: track bits, high bit pauses playback */
    u8 trackCount;
    u8 unknown_09[0x1B];
    u16 fadeInterval;             /* +24 */
    u16 fadeCounter;              /* +26 */
    u16 fadeVolume;               /* +28: volume << 2, low bits control fade */
    u16 unknown_2A;
    struct SoundTrack *tracks;    /* +2C */
    const void *voices;           /* +30: ToneData bank */
    u32 ident;                    /* +34: ready/locked signature */
};
struct SoundPlayerEntry
{
    struct SoundPlayer *player;
    struct SoundTrack *tracks;
    u8 trackCount;
    u8 padding;
    u16 flags;
};
struct SoundSongEntry { const void *header; u16 player; u16 otherPlayer; };
void SoundPlayerResume(struct SoundPlayer *player);
void SoundPlayerFadeOut(struct SoundPlayer *player, u16 interval);
void SoundPlayerFadeOutTemporary(struct SoundPlayer *player, u16 interval);
void SoundPlayerFadeIn(struct SoundPlayer *player, u16 interval);
void SoundPlayerStop(struct SoundPlayer *player);
void SoundSongStart(u16 song);
void SoundSongStop(u16 song);
void SoundUpdate(void);
#endif
