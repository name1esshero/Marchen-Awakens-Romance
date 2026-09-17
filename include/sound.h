#ifndef MAR_SOUND_H
#define MAR_SOUND_H
#include "gba/types.h"

/* M4A-compatible player layout verified against 080789B0..08079280.
 * The player lock word is the little-endian signature "Smsh".
 * Song and sample data use ROM pointers, independently of the NFP archive.
 */
#define SOUND_PLAYER_READY 0x68736D53
#define SOUND_PLAYER_PAUSED 0x80000000
/* MusicPlayer2000's 80-byte sequence-track state.  The named prefix is used
 * by the volume/pitch and fade paths; the tail contains modulation and
 * command-runtime state that is still being decoded. */
struct SoundTrack
{
    u8 flags;                    /* +00: exists/start/volume/pitch change bits */
    u8 unknown_01[7];
    s8 key;                      /* +08: calculated MIDI key */
    u8 pitch;                    /* +09: calculated fractional pitch */
    s8 keyShift;                 /* +0A */
    s8 keyShiftExtra;            /* +0B */
    s8 tuning;                   /* +0C */
    u8 pitchExtra;               /* +0D */
    s8 bend;                     /* +0E */
    u8 bendRange;                /* +0F */
    u8 volumeRight;              /* +10 */
    u8 volumeLeft;               /* +11 */
    u8 volume;                   /* +12 */
    u8 fadeVolume;               /* +13 */
    s8 pan;                      /* +14 */
    s8 panExtra;                 /* +15 */
    s8 modulation;               /* +16 */
    u8 modulationDepth;          /* +17 */
    u8 modulationType;           /* +18: pitch=0, volume=1, pan=2 */
    u8 lfoSpeed;                 /* +19 */
    u8 lfoCounter;               /* +1A */
    u8 wait;                     /* +1B: sequence interpreter waits this tick */
    u8 unknown_1C[2];
    u8 pseudoEchoVolume;         /* +1E */
    u8 pseudoEchoLength;         /* +1F */
    void *channel;               /* +20: active mixer/CGB channel */
    u8 toneType;                 /* +24: voice/oscillator type */
    u8 unknown_25;
    u8 toneLength;               /* +26 */
    u8 tonePanSweep;             /* +27 */
    u32 toneWave;                /* +28: WaveData pointer encoded by XCMD */
    u8 toneAttack;               /* +2C */
    u8 toneDecay;                /* +2D */
    u8 toneSustain;              /* +2E */
    u8 toneRelease;              /* +2F */
    u8 unknown_30[16];
    const u8 *command;            /* +40: current sequence bytecode cursor */
    u8 unknown_44[12];
};
/* Twelve-byte MusicPlayer2000 instrument record.  The meaning of wave is
 * selected by type: it can name PCM sample data, a PSG oscillator, or a
 * subordinate key-split/direct-sound table. */
struct SoundToneData
{
    u8 type;
    u8 key;
    u8 length;
    u8 panSweep;
    u32 wave;
    u8 attack;
    u8 decay;
    u8 sustain;
    u8 release;
};
struct SoundPlayer
{
    const void *song;             /* +00: current SongHeader */
    u32 status;                   /* +04: track bits, high bit pauses playback */
    u8 trackCount;
    u8 priority;                  /* +09: active song priority */
    u8 unknown_0A;
    u8 priorityMode;              /* +0B: enables replacement priority checks */
    u32 clock;                    /* +0C */
    u8 unknown_10[8];
    u8 *memory;                   /* +18: sequence MEMACC scratch area */
    u16 tempoBase;                /* +1C */
    u16 tempoScale;               /* +1E */
    u16 tempoInterval;            /* +20 */
    u16 tempoCounter;             /* +22 */
    u16 fadeInterval;             /* +24 */
    u16 fadeCounter;              /* +26 */
    u16 fadeVolume;               /* +28: volume << 2, low bits control fade */
    u16 unknown_2A;
    struct SoundTrack *tracks;    /* +2C */
    const void *voices;           /* +30: ToneData bank */
    u32 ident;                    /* +34: ready/locked signature */
    void *mainNext;               /* +38: intrusive MPlayMain list */
    struct SoundPlayer *playerNext; /* +3C: intrusive player list */
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
/* Sequence header stored in ROM.  MusicPlayer2000 supports at most sixteen
 * tracks, so a fixed array preserves the original compiler's indexed loads. */
struct SoundSongHeader
{
    u8 trackCount;
    u8 blockCount;
    u8 priority;
    u8 reverb;
    const void *voices;
    const u8 *parts[16];
};
extern const struct SoundPlayerEntry gSoundPlayerTable[9];
extern struct SoundPlayer gSoundPlayer0;
extern struct SoundPlayer gSoundPlayer1;
extern struct SoundPlayer gSoundPlayer2;
extern struct SoundPlayer gSoundPlayer3;
extern struct SoundPlayer gSoundPlayer4;
extern struct SoundPlayer gSoundPlayer5;
extern struct SoundPlayer gSoundPlayer6;
extern struct SoundPlayer gSoundPlayer7;
extern struct SoundPlayer gSoundPlayer8;
extern const struct SoundSongEntry gSongTable[221];
extern const s16 gDynamicSoundPlayerOrder[6];
extern const u8 gSoundScaleTable[180];
extern const u32 gSoundFrequencyTable[12];
extern const u16 gPcmSamplesPerVBlankTable[12];
extern const u8 gCgbScaleTable[132];
extern const s16 gCgbFrequencyTable[12];
extern const u8 gCgbNoiseTable[60];
extern const u8 gCgb3VolumeTable[68];
extern void *const gSoundExtendedCommandTable[12];
extern const struct SoundToneData gVoiceGroupMain[188];
extern const struct SoundToneData gVoiceGroupSecondary[127];
extern const struct SoundToneData gVoiceGroupEffects[128];
struct SoundWaveData
{
    u32 type;
    u32 frequency;               /* +04: native playback frequency */
};
u32 SoundMidiKeyToFrequency(struct SoundWaveData *wave, u8 key,
                            u8 fineAdjust);
void SoundDriverUnusedNoOp(void);
void SoundDriverInit(void);
void SoundPlayerOpen(struct SoundPlayer *player, struct SoundTrack *tracks,
                     u8 trackCount);
void SoundPlayerImmediateInit(struct SoundPlayer *player);
void SoundDriverSetSampleFrequency(u32 frequency);
void SoundDriverStateInit(void *sound);
void SoundDriverSetMode(u32 mode);
void SoundDriverClear(void);
void SoundDriverEnableCgb(void *channels);
void SoundDriverCopyJumpTableSwi(void);
void SoundUpdateCgbChannels(void);
void SoundDriverVSyncOff(void);
void SoundDriverVSyncOn(void);
void SoundPlayerStart(struct SoundPlayer *player,
                      const struct SoundSongHeader *song);
void SoundPlayerResume(struct SoundPlayer *player);
void SoundPlayerFadeOut(struct SoundPlayer *player, u16 interval);
void SoundPlayerFadeOutTemporary(struct SoundPlayer *player, u16 interval);
void SoundPlayerFadeIn(struct SoundPlayer *player, u16 interval);
void SoundPlayerStop(struct SoundPlayer *player);
void SoundSongStart(u16 song);
void SoundSongStartOrChange(u16 song);
void SoundSongStartOrContinue(u16 song);
void SoundSongStop(u16 song);
void SoundSongContinue(u16 song);
void SoundStopAllPlayers(void);
void SoundResumePlayer(struct SoundPlayer *player);
void SoundResumeAllPlayers(void);
struct EngineTask;
struct EngineTask *CreateSoundWaitTask(s32 playerIndex, s32 wait,
                                       s32 *result, s32 *completion);
struct EngineTask *CreateSoundFadeTask(s32 countdown, s32 playerIndex,
                                       s32 completePendingOnFinish,
                                       s32 stopPlayerOnFinish,
                                       u32 *completion);
struct EngineTask *CreateSoundPlayerIdleWait(u32 playerIndex, u32 wait,
                                             s32 *result, u32 *completion);
s32 StartSongOnFreePlayer(void *context, void *arguments, u32 song);
void SoundUpdate(void);
void SoundCallCallback5DA8(u32 argument);
void SoundCallCallback5DAC(u32 argument);
void DisableDma1AndUpdateSound(void);
#endif
