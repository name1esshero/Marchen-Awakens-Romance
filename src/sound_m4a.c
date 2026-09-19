/* MusicPlayer2000 sequence-player arithmetic.  These routines use the older
 * agbcc executable, matching the compiler revision used by Nintendo's driver.
 */
#include "sound.h"

#include "rom_section.h"
#define TRACK_EXISTS       0x80
#define TRACK_START        0x40
#define TRACK_VOLUME_DIRTY 0x01
#define TRACK_PITCH_DIRTY  0x04
#define TRACK_MIX_DIRTY    0x03
#define FADE_TEMPORARY     0x01
#define FADE_IN            0x02
#define FADE_VOLUME_SHIFT  2
#define SOUND_CHANNEL_STOP       0x40
#define SOUND_CHANNEL_ECHO       0x04
#define SOUND_CHANNEL_ENVELOPE   0x03
#define SOUND_CHANNEL_ON         0xC7
#define CGB_MODIFY_PITCH         0x02
#define CGB_MODIFY_VOLUME        0x01
#define CGB_ENVELOPE_INCREASE    0x08
#define CGB_FIXED_PITCH          0x08

extern void sub_080783DC(void);
extern u32 sub_08077DB0(u32 first, u32 second);
extern void sub_08077E44(void);
extern void sub_080781DC(void *jumpTable);
extern void sub_080786B8(void);
extern void sub_080788B8(void);
extern void sub_08078920(void);
extern void sub_08078934(void);
extern void SoundTrackNoOp(struct SoundPlayer *, struct SoundTrack *);
extern void sub_08080BC8(struct SoundPlayer *, struct SoundTrack *, void *);
extern void SoundCallCallback5DAC(u32 argument);
extern void CpuSet(const void *source, void *destination, u32 mode);
extern const u8 gSoundPlayerCount[];
extern void *gSoundJumpTable[];
extern const u8 gSoundMaxLines[];
extern void SoundTrackMemoryCommand(struct SoundPlayer *, struct SoundTrack *);
extern void SoundTrackDispatchExtendedCommand(
    struct SoundPlayer *, struct SoundTrack *);
extern void SoundPlayerUpdateFade(struct SoundPlayer *);
extern void SoundTrackUpdateVolumeAndPitch(
    struct SoundPlayer *, struct SoundTrack *);
extern u32 SoundMidiKeyToCgbFrequency(u8 channel, u8 key, u8 fineAdjust);
extern void SoundDisableCgbOscillator(u8 channel);

struct SoundChannel
{
    u8 status;
    u8 type;
    u8 unknown_02[26];
    u8 panMask;                   /* +1C: NR51 left/right routing bits */
    u8 unknown_1D[35];
};

struct SoundCgbChannel
{
    u8 statusFlags;
    u8 type;
    u8 volumeRight;
    u8 volumeLeft;
    u8 attack;
    u8 decay;
    u8 sustain;
    u8 release;
    u8 key;
    u8 envelopeVolume;
    u8 envelopeTarget;
    u8 envelopeCounter;
    u8 pseudoEchoVolume;
    u8 pseudoEchoLength;
    u8 unknown_0E[2];
    u8 gateTime;
    u8 midiKey;
    u8 velocity;
    u8 priority;
    u8 rhythmPan;
    u8 unknown_15[4];
    u8 sustainTarget;
    u8 controlN4;
    u8 pan;
    u8 panMask;
    u8 modified;
    u8 length;
    u8 sweep;
    u32 frequency;
    u32 *wave;
    u32 *loadedWave;
    struct SoundTrack *track;
    void *previous;
    void *next;
    u8 unknown_38[8];
};

/* Kept as a call from the channel update loop to preserve the original
 * driver's register-allocation boundary. */
extern void SoundUpdateCgbChannelVolume(struct SoundCgbChannel *channel);

struct SoundDriverState
{
    u32 ident;
    volatile u8 pcmDmaCounter;    /* +04 */
    u8 reverb;                     /* +05 */
    u8 maxChannels;                /* +06 */
    u8 masterVolume;               /* +07 */
    u8 frequencyIndex;             /* +08 */
    u8 mode;                       /* +09 */
    u8 cycleCounter;               /* +0A */
    u8 pcmDmaPeriod;               /* +0B: VBlanks per PCM buffer */
    u8 maxLines;                   /* +0C: mixer scanline budget */
    u8 padding_0D[3];
    s32 pcmSamplesPerVBlank;       /* +10 */
    s32 pcmFrequency;              /* +14 */
    s32 divisionFrequency;         /* +18: mixer phase divisor */
    struct SoundCgbChannel *cgbChannels; /* +1C */
    void *mainHead;                /* +20 */
    struct SoundPlayer *playerHead; /* +24 */
    void *cgbSound;                /* +28 */
    void (*cgbOscillatorOff)(u8);  /* +2C */
    void *midiKeyToCgbFrequency;   /* +30 */
    void *jumpTable;               /* +34 */
    void *playNote;                /* +38 */
    void *extendedVolumePitch;     /* +3C */
    u8 padding_40[16];
    struct SoundChannel channels[12]; /* +50 */
};

/** Enable the four Game Boy-compatible PSG channels and replace the generic
 * sequence handlers with this driver's CGB-aware commands. */
AT("00078CA8") void SoundDriverEnableCgb(void *channelStorage)
{
    struct SoundCgbChannel *channels = channelStorage;
    struct SoundDriverState *sound;
    u32 ident;
    u32 zero;

    *(volatile u16 *)0x04000084 = 0x8F;
    *(volatile u16 *)0x04000080 = 0;
    *(volatile u8 *)0x04000063 = 8;
    *(volatile u8 *)0x04000069 = 8;
    *(volatile u8 *)0x04000079 = 8;
    *(volatile u8 *)0x04000065 = 0x80;
    *(volatile u8 *)0x0400006D = 0x80;
    *(volatile u8 *)0x0400007D = 0x80;
    *(volatile u8 *)0x04000070 = 0;
    *(volatile u8 *)0x04000080 = 0x77;

    sound = *(struct SoundDriverState **)0x03007FF0;
    ident = sound->ident;
    if (ident != SOUND_PLAYER_READY)
        return;
    sound->ident++;
    gSoundJumpTable[8] = SoundTrackMemoryCommand;
    gSoundJumpTable[17] = sub_08078920;
    gSoundJumpTable[19] = sub_08078934;
    gSoundJumpTable[28] = SoundTrackDispatchExtendedCommand;
    gSoundJumpTable[29] = sub_080788B8;
    gSoundJumpTable[30] = SoundDriverSetSampleFrequency;
    gSoundJumpTable[31] = SoundTrackReleaseChannels;
    gSoundJumpTable[32] = SoundPlayerUpdateFade;
    gSoundJumpTable[33] = SoundTrackUpdateVolumeAndPitch;
    sound->cgbChannels = channels;
    sound->cgbSound = SoundUpdateCgbChannels;
    sound->cgbOscillatorOff = SoundDisableCgbOscillator;
    sound->midiKeyToCgbFrequency = SoundMidiKeyToCgbFrequency;
    sound->maxLines = (u32)gSoundMaxLines;

    zero = 0;
    CpuSet(&zero, channels, 0x05000040);
    channels[0].type = 1;
    channels[0].panMask = 0x11;
    channels[1].type = 2;
    channels[1].panMask = 0x22;
    channels[2].type = 3;
    channels[2].panMask = 0x44;
    channels[3].type = 4;
    channels[3].panMask = 0x88;
    sound->ident = ident;
}


/** Interpolate the two adjacent MIDI scale entries, then scale the wave's
 * native frequency by the high half of each 32x32-bit product. */
AT("00078948") u32 SoundMidiKeyToFrequency(
    struct SoundWaveData *wave, u8 key, u8 fineAdjust)
{
    u32 lowerFrequency;
    u32 upperFrequency;
    u32 fine = fineAdjust << 24;

    if (key > 178) {
        key = 178;
        fine = 255 << 24;
    }
    lowerFrequency = gSoundScaleTable[key];
    lowerFrequency = gSoundFrequencyTable[lowerFrequency & 15]
                   >> (lowerFrequency >> 4);
    upperFrequency = gSoundScaleTable[key + 1];
    upperFrequency = gSoundFrequencyTable[upperFrequency & 15]
                   >> (upperFrequency >> 4);
    return sub_08077DB0(
        wave->frequency,
        lowerFrequency
            + sub_08077DB0(upperFrequency - lowerFrequency, fine));
}

/** Empty compatibility entry retained by this MusicPlayer2000 build. */
AT("000789AC") void SoundDriverUnusedNoOp(void)
{
}
AT("000789AC") const u8 SoundDriverUnusedNoOpTail[2] = {0};

/** Resume a paused sequence player while holding its identity lock. */
AT("000789B0") void SoundPlayerResume(struct SoundPlayer *player)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->ident++;
        player->status &= ~SOUND_PLAYER_PAUSED;
        player->ident = SOUND_PLAYER_READY;
    }
}

/** Begin a permanent fade to silence. */
AT("000789CC") void SoundPlayerFadeOut(struct SoundPlayer *player, u16 interval)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->ident++;
        player->fadeCounter = interval;
        player->fadeInterval = interval;
        player->fadeVolume = 64 << 2;
        player->ident = SOUND_PLAYER_READY;
    }
}

/** Install the relocatable mixer in IWRAM, initialize its driver/channel
 * state, and register all nine player slots with their shared MEMACC area. */
AT("000789EC") void SoundDriverInit(void)
{
    s32 i;

    CpuSet((void *)((s32)sub_08077E44 & ~1),
           (void *)0x03005018, 0x040000E0);
    SoundDriverStateInit((void *)0x030053A0);
    SoundDriverEnableCgb((void *)0x03005DB0);
    SoundDriverSetMode(0x0095EC00);

    for (i = 0; i < (u16)(u32)gSoundPlayerCount; i++) {
        struct SoundPlayer *player = gSoundPlayerTable[i].player;
        SoundPlayerOpen(player, gSoundPlayerTable[i].tracks,
                        gSoundPlayerTable[i].trackCount);
        player->priorityMode = gSoundPlayerTable[i].flags;
        player->memory = (void *)0x03006070;
    }
}

/** Materialize each just-started track's default runtime state.  Track
 * bytecode itself is left intact; the callback clears its transient tail. */
AT("00078C60") void SoundPlayerImmediateInit(struct SoundPlayer *player)
{
    s32 trackCount = player->trackCount;
    struct SoundTrack *track = player->tracks;

    while (trackCount > 0) {
        if (track->flags & TRACK_EXISTS) {
            if (track->flags & TRACK_START) {
                SoundCallCallback5DAC((u32)track);
                track->flags = TRACK_EXISTS;
                track->bendRange = 2;
                track->fadeVolume = 64;
                track->lfoSpeed = 22;
                track->toneType = 1;
            }
        }
        trackCount--;
        track++;
    }
}

/** Fade to silence, leaving the player paused so it can be resumed. */
AT("00078C18") void SoundPlayerFadeOutTemporary(
    struct SoundPlayer *player, u16 interval)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->ident++;
        player->fadeCounter = interval;
        player->fadeInterval = interval;
        player->fadeVolume = (64 << 2) | 1;
        player->ident = SOUND_PLAYER_READY;
    }
}

/** Fade a ready player in and clear its paused flag. */
AT("00078C38") void SoundPlayerFadeIn(
    struct SoundPlayer *player, u16 interval)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->ident++;
        player->fadeCounter = interval;
        player->fadeInterval = interval;
        player->fadeVolume = 2;
        player->status &= ~SOUND_PLAYER_PAUSED;
        player->ident = SOUND_PLAYER_READY;
    }
}

/** Configure direct-sound timing for a frequency-table slot and synchronize
 * Timer 0's restart to the final visible scanline. */
AT("00078EB8") void SoundDriverSetSampleFrequency(u32 frequency)
{
    struct SoundDriverState *sound =
        *(struct SoundDriverState **)0x03007FF0;

    frequency = (frequency & 0xF0000) >> 16;
    sound->frequencyIndex = frequency;
    sound->pcmSamplesPerVBlank =
        gPcmSamplesPerVBlankTable[frequency - 1];
    sound->pcmDmaPeriod = 1584 / sound->pcmSamplesPerVBlank;
    sound->pcmFrequency =
        (597275 * sound->pcmSamplesPerVBlank + 5000) / 10000;
    sound->divisionFrequency =
        (16777216 / sound->pcmFrequency + 1) >> 1;

    *(volatile u16 *)0x04000102 = 0;
    *(volatile u16 *)0x04000100 =
        -(280896 / sound->pcmSamplesPerVBlank);
    SoundDriverVSyncOn();
    while (*(volatile u8 *)0x04000006 == 159) {
    }
    while (*(volatile u8 *)0x04000006 != 159) {
    }
    *(volatile u16 *)0x04000102 = 0x80;
}

/** Establish the core direct-sound state and hardware DMA/FIFO routing. */
AT("00078DEC") void SoundDriverStateInit(void *state)
{
    struct SoundDriverState *sound = state;
    u32 zero;

    sound->ident = 0;
    if (*(volatile u32 *)0x040000C4 & (1 << 25))
        *(volatile u32 *)0x040000C4 = 0x84400004;
    *(volatile u16 *)0x040000C6 = 0x0400;
    *(volatile u16 *)0x04000084 = 0x008F;
    *(volatile u16 *)0x04000082 = 0x0B0E;
    *(volatile u8 *)0x04000089 =
        (*(volatile u8 *)0x04000089 & 0x3F) | 0x40;
    *(volatile u32 *)0x040000BC = (u32)((u8 *)sound + 0x350);
    *(volatile u32 *)0x040000C0 = 0x040000A0;
    *(struct SoundDriverState **)0x03007FF0 = sound;

    zero = 0;
    CpuSet(&zero, sound, 0x05000260);
    sound->maxChannels = 8;
    sound->masterVolume = 15;
    sound->playNote = sub_080786B8;
    sound->cgbSound = SoundTrackNoOp;
    sound->cgbOscillatorOff = (void (*)(u8))SoundTrackNoOp;
    sound->midiKeyToCgbFrequency = SoundTrackNoOp;
    sound->extendedVolumePitch = SoundTrackNoOp;
    sub_080781DC(gSoundJumpTable);
    sound->jumpTable = gSoundJumpTable;
    SoundDriverSetSampleFrequency(0x40000);
    sound->ident = SOUND_PLAYER_READY;
}

/** Apply packed MusicPlayer2000 mode fields: reverb, channel count, master
 * volume, DAC resolution, and direct-sound sample-rate selection. */
AT("00078F5C") void SoundDriverSetMode(u32 mode)
{
    struct SoundDriverState *sound =
        *(struct SoundDriverState **)0x03007FF0;
    u32 value;

    if (sound->ident != SOUND_PLAYER_READY)
        return;
    sound->ident++;
    value = mode & 0xFF;
    if (value)
        sound->reverb = value & 0x7F;
    value = mode & 0xF00;
    if (value) {
        struct SoundChannel *channel;
        sound->maxChannels = value >> 8;
        value = 12;
        channel = &sound->channels[0];
        while (value != 0) {
            channel->status = 0;
            value--;
            channel++;
        }
    }
    value = mode & 0xF000;
    if (value)
        sound->masterVolume = value >> 12;
    value = mode & 0xB00000;
    if (value) {
        value = (value & 0x300000) >> 14;
        *(volatile u8 *)0x04000089 =
            (*(volatile u8 *)0x04000089 & 0x3F) | value;
    }
    value = mode & 0xF0000;
    if (value) {
        SoundDriverVSyncOff();
        SoundDriverSetSampleFrequency(value);
    }
    sound->ident = SOUND_PLAYER_READY;
}

/** Silence every software and CGB channel while holding the driver lock. */
AT("00078FF4") void SoundDriverClear(void)
{
    struct SoundDriverState *sound =
        *(struct SoundDriverState **)0x03007FF0;
    s32 i;
    void *channel;

    if (sound->ident != SOUND_PLAYER_READY)
        return;
    sound->ident++;
    i = 12;
    channel = &sound->channels[0];
    while (i > 0) {
        ((struct SoundChannel *)channel)->status = 0;
        i--;
        channel = (void *)((s32)channel + sizeof(struct SoundChannel));
    }
    channel = sound->cgbChannels;
    if (channel) {
        i = 1;
        while (i <= 4) {
            sound->cgbOscillatorOff(i);
            ((struct SoundChannel *)channel)->status = 0;
            i++;
            channel = (void *)((s32)channel + sizeof(struct SoundChannel));
        }
    }
    sound->ident = SOUND_PLAYER_READY;
}

/** Suspend FIFO DMA and clear the direct-sound PCM ring buffer.  The driver's
 * signature advances by ten while VBlank mixing is disabled. */
AT("00079048") void SoundDriverVSyncOff(void)
{
    struct SoundDriverState *sound =
        *(struct SoundDriverState **)0x03007FF0;
    u32 ident = sound->ident;
    u32 zero;

    if (ident >= SOUND_PLAYER_READY && ident <= SOUND_PLAYER_READY + 1) {
        sound->ident = ident + 10;
        if (*(volatile u32 *)0x040000C4 & (1 << 25))
            *(volatile u32 *)0x040000C4 = 0x84400004;
        *(volatile u16 *)0x040000C6 = 0x0400;
        zero = 0;
        CpuSet(&zero, (u8 *)sound + 0x350, 0x0500018C);
    }
}

/** Restart FIFO DMA after a temporary sound shutdown. */
AT("000790AC") void SoundDriverVSyncOn(void)
{
    struct SoundDriverState *sound =
        *(struct SoundDriverState **)0x03007FF0;
    u32 ident = sound->ident;

    if (ident == SOUND_PLAYER_READY)
        return;
    *(volatile u16 *)0x040000C6 = 0xB600;
    sound->pcmDmaCounter = 0;
    sound->ident = ident - 10;
}

/** Register a player and its track storage with the global M4A driver. */
AT("000790E4") void SoundPlayerOpen(
    struct SoundPlayer *player, struct SoundTrack *tracks, u8 trackCount)
{
    struct SoundDriverState *sound;

    if (trackCount == 0)
        return;
    if (trackCount > 16)
        trackCount = 16;

    sound = *(struct SoundDriverState **)0x03007FF0;
    if (sound->ident != SOUND_PLAYER_READY)
        return;
    sound->ident++;

    SoundCallCallback5DAC((u32)player);
    player->tracks = tracks;
    player->trackCount = trackCount;
    player->status = SOUND_PLAYER_PAUSED;

    while (trackCount != 0) {
        tracks->flags = 0;
        trackCount--;
        tracks++;
    }

    if (sound->mainHead != 0) {
        player->mainNext = sound->mainHead;
        player->playerNext = sound->playerHead;
        sound->mainHead = 0;
    }
    sound->playerHead = player;
    sound->mainHead = (void *)((u32)sub_080783DC + 1);
    sound->ident = SOUND_PLAYER_READY;
    player->ident = SOUND_PLAYER_READY;
}

/** Start a sequence after enforcing the player's replacement-priority policy.
 * Every active track is reset and pointed at its corresponding bytecode part;
 * surplus player tracks are explicitly retired. */
AT("0007915C") void SoundPlayerStart(
    struct SoundPlayer *player, const struct SoundSongHeader *song)
{
    s32 i;
    u8 priority;
    struct SoundTrack *track;

    if (player->ident != SOUND_PLAYER_READY)
        return;
    priority = player->priorityMode;
    if (!priority
     || ((!player->song || !(player->tracks[0].flags & TRACK_START))
      && ((player->status & 0xFFFF) == 0
       || (player->status & SOUND_PLAYER_PAUSED)))
     || player->priority <= song->priority) {
        player->ident++;
        player->status = 0;
        player->song = song;
        player->voices = song->voices;
        player->priority = song->priority;
        player->clock = 0;
        player->tempoBase = 150;
        player->tempoInterval = 150;
        player->tempoScale = 256;
        player->tempoCounter = 0;
        player->fadeInterval = 0;

        i = 0;
        track = player->tracks;
        while (i < song->trackCount && i < player->trackCount) {
            SoundTrackReleaseChannels(player, track);
            track->flags = TRACK_EXISTS | TRACK_START;
            track->channel = 0;
            track->command = song->parts[i];
            i++;
            track++;
        }
        while (i < player->trackCount) {
            SoundTrackReleaseChannels(player, track);
            track->flags = 0;
            i++;
            track++;
        }
        if (song->reverb & 0x80)
            SoundDriverSetMode(song->reverb);
        player->ident = SOUND_PLAYER_READY;
    }
}

/** Start the requested song on the player selected by its song-table entry. */
AT("00078A70") void SoundSongStart(u16 songNumber)
{
    const struct SoundPlayerEntry *playerTable = gSoundPlayerTable;
    const struct SoundSongEntry *songTable = gSongTable;
    const struct SoundSongEntry *song = &songTable[songNumber];
    const struct SoundPlayerEntry *player = &playerTable[song->player];

    SoundPlayerStart(player->player, song->header);
}

/** Start a song unless that player is already running the same unpaused song. */
AT("00078A9C") void SoundSongStartOrChange(u16 songNumber)
{
    const struct SoundPlayerEntry *playerTable = gSoundPlayerTable;
    const struct SoundSongEntry *songTable = gSongTable;
    const struct SoundSongEntry *song = &songTable[songNumber];
    const struct SoundPlayerEntry *player = &playerTable[song->player];

    if (player->player->song != song->header) {
        SoundPlayerStart(player->player, song->header);
    } else if ((player->player->status & 0xFFFF) == 0
            || (player->player->status & SOUND_PLAYER_PAUSED)) {
        SoundPlayerStart(player->player, song->header);
    }
}

/** Continue a paused copy of the requested song, restarting it if it ended or
 * if this player currently owns a different song. */
AT("00078AE8") void SoundSongStartOrContinue(u16 songNumber)
{
    const struct SoundPlayerEntry *playerTable = gSoundPlayerTable;
    const struct SoundSongEntry *songTable = gSongTable;
    const struct SoundSongEntry *song = &songTable[songNumber];
    const struct SoundPlayerEntry *player = &playerTable[song->player];

    if (player->player->song != song->header)
        SoundPlayerStart(player->player, song->header);
    else if ((player->player->status & 0xFFFF) == 0)
        SoundPlayerStart(player->player, song->header);
    else if (player->player->status & SOUND_PLAYER_PAUSED)
        SoundPlayerResume(player->player);
}

/** Stop or resume a song only when its selected player still owns it. */
AT("00078B3C") void SoundSongStop(u16 songNumber)
{
    const struct SoundPlayerEntry *playerTable = gSoundPlayerTable;
    const struct SoundSongEntry *songTable = gSongTable;
    const struct SoundSongEntry *song = &songTable[songNumber];
    const struct SoundPlayerEntry *player = &playerTable[song->player];

    if (player->player->song == song->header)
        SoundPlayerStop(player->player);
}

AT("00078B70") void SoundSongContinue(u16 songNumber)
{
    const struct SoundPlayerEntry *playerTable = gSoundPlayerTable;
    const struct SoundSongEntry *songTable = gSongTable;
    const struct SoundSongEntry *song = &songTable[songNumber];
    const struct SoundPlayerEntry *player = &playerTable[song->player];

    if (player->player->song == song->header)
        SoundPlayerResume(player->player);
}

/** This title defines nine MusicPlayer2000 player slots. */
AT("00078BA4") void SoundStopAllPlayers(void)
{
    s32 i;

    for (i = 0; i < (u16)(u32)gSoundPlayerCount; i++)
        SoundPlayerStop(gSoundPlayerTable[i].player);
}

AT("00078BD0") void SoundResumePlayer(struct SoundPlayer *player)
{
    SoundPlayerResume(player);
}
AT("00078BD0") const u8 SoundResumePlayerTail[2] = {0};

/** Resume every one of the engine's nine MusicPlayer2000 slots. */
AT("00078BDC") void SoundResumeAllPlayers(void)
{
    s32 i;

    for (i = 0; i < (u16)(u32)gSoundPlayerCount; i++)
        SoundPlayerResume(gSoundPlayerTable[i].player);
}

/** Extended sequence command: read a little-endian WaveData pointer.  The
 * original driver intentionally constructs each byte independently. */
AT("00079DA8") void SoundTrackReadWavePointer(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    u32 wave;
    const u8 *command = track->command;

    wave &= 0xFFFFFF00;
    wave |= command[0];
    {
        u32 byte = command[1];
        byte <<= 8;
        wave &= 0xFFFF00FF;
        wave |= byte;
    }
    {
        u32 byte = command[2];
        byte <<= 16;
        wave &= 0xFF00FFFF;
        wave |= byte;
    }
    {
        u32 byte = command[3];
        byte <<= 24;
        wave &= 0x00FFFFFF;
        wave |= byte;
    }
    track->toneWave = wave;
    track->command += 4;
}

/** One-byte extended commands configuring a programmable instrument. */
AT("00079DF0") void SoundTrackReadToneType(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->toneType = *track->command;
    track->command++;
}
AT("00079DF0") const u8 SoundTrackReadToneTypeTail[2] = {0};

AT("00079E04") void SoundTrackReadToneAttack(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->toneAttack = *track->command;
    track->command++;
}
AT("00079E04") const u8 SoundTrackReadToneAttackTail[2] = {0};

/** One-byte extended command: set the current programmable instrument's
 * decay rate. */
AT("00079E18") void SoundTrackReadToneDecay(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->toneDecay = *track->command;
    track->command++;
}
AT("00079E18") const u8 SoundTrackReadToneDecayTail[2] = {0};

/** One-byte extended command: set the current programmable instrument's
 * sustain level. */
AT("00079E2C") void SoundTrackReadToneSustain(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->toneSustain = *track->command;
    track->command++;
}
AT("00079E2C") const u8 SoundTrackReadToneSustainTail[2] = {0};

/** One-byte extended command: set the current programmable instrument's
 * release rate. */
AT("00079E40") void SoundTrackReadToneRelease(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->toneRelease = *track->command;
    track->command++;
}
AT("00079E40") const u8 SoundTrackReadToneReleaseTail[2] = {0};

/** One-byte extended command: set the current track's pseudo-echo
 * volume. */
AT("00079E54") void SoundTrackReadPseudoEchoVolume(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->pseudoEchoVolume = *track->command;
    track->command++;
}

/** One-byte extended command: set the current track's pseudo-echo
 * length. */
AT("00079E60") void SoundTrackReadPseudoEchoLength(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->pseudoEchoLength = *track->command;
    track->command++;
}

/** One-byte extended command: set the current programmable instrument's
 * note length. */
AT("00079E6C") void SoundTrackReadToneLength(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->toneLength = *track->command;
    track->command++;
}
AT("00079E6C") const u8 SoundTrackReadToneLengthTail[2] = {0};

/** One-byte extended command: set the current track's pan sweep rate. */
AT("00079E80") void SoundTrackReadTonePanSweep(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    track->tonePanSweep = *track->command;
    track->command++;
}
AT("00079E80") const u8 SoundTrackReadTonePanSweepTail[2] = {0};

/** Reserved extended opcode. */
AT("00079E94") void SoundTrackNoOp(
    struct SoundPlayer *player, struct SoundTrack *track)
{
}
AT("00079E94") const u8 SoundTrackNoOpTail[2] = {0};

/** Advance a player's fade timer.  A completed permanent fade stops and
 * disables every track; a temporary fade pauses the player for resumption.
 */
AT("00079280") void SoundPlayerUpdateFade(struct SoundPlayer *player)
{
    s32 count;
    struct SoundTrack *track;
    u16 fadeVolume;

    if (player->fadeInterval == 0)
        return;
    if (--player->fadeCounter != 0)
        return;

    player->fadeCounter = player->fadeInterval;

    if (player->fadeVolume & FADE_IN) {
        if ((u16)(player->fadeVolume += (4 << FADE_VOLUME_SHIFT))
            >= (64 << FADE_VOLUME_SHIFT)) {
            player->fadeVolume = (64 << FADE_VOLUME_SHIFT);
            player->fadeInterval = 0;
        }
    } else {
        if ((s16)(player->fadeVolume -= (4 << FADE_VOLUME_SHIFT)) <= 0) {
            count = player->trackCount;
            track = player->tracks;

            while (count > 0) {
                u32 temporary;

                SoundTrackReleaseChannels(player, track);
                temporary = FADE_TEMPORARY;
                fadeVolume = player->fadeVolume;
                temporary &= fadeVolume;
                if (!temporary)
                    track->flags = 0;
                count--;
                track++;
            }

            if (player->fadeVolume & FADE_TEMPORARY)
                player->status |= SOUND_PLAYER_PAUSED;
            else
                player->status = SOUND_PLAYER_PAUSED;
            player->fadeInterval = 0;
            return;
        }
    }

    count = player->trackCount;
    track = player->tracks;
    while (count > 0) {
        if (track->flags & TRACK_EXISTS) {
            fadeVolume = player->fadeVolume;
            track->fadeVolume = fadeVolume >> FADE_VOLUME_SHIFT;
            track->flags |= TRACK_MIX_DIRTY;
        }
        count--;
        track++;
    }
}

/** Recalculate the left/right mix and final pitch after sequence commands or
 * modulation mark either half of a track dirty. */
AT("00079348") void SoundTrackUpdateVolumeAndPitch(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    if (track->flags & TRACK_VOLUME_DIRTY) {
        s32 volume;
        s32 pan;

        volume = (u32)(track->volume * track->fadeVolume) >> 5;
        if (track->modulationType == 1)
            volume = (u32)(volume * (track->modulation + 128)) >> 7;

        pan = 2 * track->pan + track->panExtra;
        if (track->modulationType == 2)
            pan += track->modulation;
        if (pan < -128)
            pan = -128;
        else if (pan > 127)
            pan = 127;

        track->volumeRight = (u32)((pan + 128) * volume) >> 8;
        track->volumeLeft = (u32)((127 - pan) * volume) >> 8;
    }

    if (track->flags & TRACK_PITCH_DIRTY) {
        s32 bend = track->bend * track->bendRange;
        s32 pitch = (track->tuning + bend) * 4
                  + (track->keyShift << 8)
                  + (track->keyShiftExtra << 8)
                  + track->pitchExtra;

        if (track->modulationType == 0)
            pitch += 16 * track->modulation;
        track->key = pitch >> 8;
        track->pitch = pitch;
    }

    track->flags &= ~(TRACK_PITCH_DIRTY | TRACK_VOLUME_DIRTY);
}

/** Convert a MIDI key and 8-bit fine adjustment to the GBA PSG frequency
 * encoding.  Channel four uses the hardware noise-period lookup instead. */
AT("000793FC") u32 SoundMidiKeyToCgbFrequency(
    u8 channel, u8 key, u8 fineAdjust)
{
    if (channel == 4) {
        if (key <= 20) {
            key = 0;
        } else {
            key -= 21;
            if (key > 59)
                key = 59;
        }
        return gCgbNoiseTable[key];
    } else {
        s32 lower;
        s32 upper;

        if (key <= 35) {
            fineAdjust = 0;
            key = 0;
        } else {
            key -= 36;
            if (key > 130) {
                key = 130;
                fineAdjust = 255;
            }
        }

        lower = gCgbScaleTable[key];
        lower = gCgbFrequencyTable[lower & 15] >> (lower >> 4);
        upper = gCgbScaleTable[key + 1];
        upper = gCgbFrequencyTable[upper & 15] >> (upper >> 4);
        return lower + ((fineAdjust * (upper - lower)) >> 8) + 2048;
    }
}

/** Silence one of the four Game Boy-compatible oscillators. */
AT("000794A4") void SoundDisableCgbOscillator(u8 channel)
{
    switch (channel) {
    case 1:
        *(volatile u8 *)0x04000063 = 8;
        *(volatile u8 *)0x04000065 = 0x80;
        break;
    case 2:
        *(volatile u8 *)0x04000069 = 8;
        *(volatile u8 *)0x0400006D = 0x80;
        break;
    case 3:
        *(volatile u8 *)0x04000070 = 0;
        break;
    default:
        *(volatile u8 *)0x04000079 = 8;
        *(volatile u8 *)0x0400007D = 0x80;
    }
}



/* The four-channel PSG update loop is kept in a separate translation
 * unit because the original sound library compiled each module in isolation. */
static inline int SoundCgbChooseHardPan(struct SoundCgbChannel *channel)
{
    u32 right = channel->volumeRight;
    u32 left = channel->volumeLeft;

    if ((right = (u8)right) >= (left = (u8)left)) {
        if (right / 2 >= left) {
            channel->pan = 0x0F;
            return 1;
        }
    } else if (left / 2 >= right) {
        channel->pan = 0xF0;
        return 1;
    }
    return 0;
}

/** Derive the PSG envelope and stereo routing from the channel's left/right
 * mix.  Strongly one-sided mixes use hardware panning; balanced mixes retain
 * both speakers and clamp the four-bit envelope. */
AT("000794F4") void SoundUpdateCgbChannelVolume(
    struct SoundCgbChannel *channel)
{
    if (!SoundCgbChooseHardPan(channel)) {
        channel->pan = 0xFF;
        channel->envelopeTarget =
            (u32)(channel->volumeRight + channel->volumeLeft) / 16;
    } else {
        channel->envelopeTarget =
            (u32)(channel->volumeRight + channel->volumeLeft) / 16;
        if (channel->envelopeTarget > 15)
            channel->envelopeTarget = 15;
    }

    channel->sustainTarget =
        (channel->envelopeTarget * channel->sustain + 15) >> 4;
    channel->pan &= channel->panMask;
}

/** Change sequence speed.  tempoBase is the song tempo, tempoScale is the
 * caller-controlled multiplier, and tempoInterval is the effective value. */
AT("000799A8") void SoundPlayerSetTempo(
    struct SoundPlayer *player, u16 tempo)
{
    if (player->ident == SOUND_PLAYER_READY) {
        player->ident++;
        player->tempoScale = tempo;
        player->tempoInterval =
            (player->tempoBase * player->tempoScale) >> 8;
        player->ident = SOUND_PLAYER_READY;
    }
}

/** Apply a fade volume to every existing track selected by trackBits, on
 * a ready player. */
AT("000799D0") void SoundPlayerSetVolume(
    struct SoundPlayer *player, u16 trackBits, u16 volume)
{
    s32 count;
    u32 bit;
    struct SoundTrack *track;

    if (player->ident != SOUND_PLAYER_READY)
        return;
    player->ident++;
    count = player->trackCount;
    track = player->tracks;
    bit = 1;
    while (count > 0) {
        if ((trackBits & bit) && (track->flags & TRACK_EXISTS)) {
            track->fadeVolume = volume / 4;
            track->flags |= TRACK_MIX_DIRTY;
        }
        count--;
        track++;
        bit <<= 1;
    }
    player->ident = SOUND_PLAYER_READY;
}

/** Apply a pitch offset to every existing track selected by trackBits, on
 * a ready player. */
AT("00079A38") void SoundPlayerSetPitch(
    struct SoundPlayer *player, u16 trackBits, s16 pitch)
{
    s32 count;
    u32 bit;
    struct SoundTrack *track;

    if (player->ident != SOUND_PLAYER_READY)
        return;
    player->ident++;
    count = player->trackCount;
    track = player->tracks;
    bit = 1;
    while (count > 0) {
        if ((trackBits & bit) && (track->flags & TRACK_EXISTS)) {
            track->keyShiftExtra = pitch >> 8;
            track->pitchExtra = pitch;
            track->flags |= TRACK_PITCH_DIRTY | 0x08;
        }
        count--;
        track++;
        bit <<= 1;
    }
    player->ident = SOUND_PLAYER_READY;
}

/** Apply a pan offset to every existing track selected by trackBits, on a
 * ready player. */
AT("00079AAC") void SoundPlayerSetPan(
    struct SoundPlayer *player, u16 trackBits, s8 pan)
{
    s32 count;
    u32 bit;
    struct SoundTrack *track;

    if (player->ident != SOUND_PLAYER_READY)
        return;
    player->ident++;
    count = player->trackCount;
    track = player->tracks;
    bit = 1;
    while (count > 0) {
        if ((trackBits & bit) && (track->flags & TRACK_EXISTS)) {
            track->panExtra = pan;
            track->flags |= TRACK_MIX_DIRTY;
        }
        count--;
        track++;
        bit <<= 1;
    }
    player->ident = SOUND_PLAYER_READY;
}

/** Reset a track's LFO modulation state and flag it dirty (pitch or mix,
 * depending on modulation type). Called when modulation depth reaches
 * zero. */
AT("00079B14") static void SoundTrackClearModulation(struct SoundTrack *track)
{
    track->lfoCounter = 0;
    track->modulation = 0;
    if (track->modulationType == 0)
        track->flags |= TRACK_PITCH_DIRTY | 0x08;
    else
        track->flags |= TRACK_MIX_DIRTY;
}
AT("00079B14") const u8 SoundTrackClearModulationTail[2] = {0};

/** Set the LFO modulation depth on every existing track selected by
 * trackBits, on a ready player; clearing depth to zero also resets that
 * track's modulation state. */
AT("00079B34") void SoundPlayerSetModulationDepth(
    struct SoundPlayer *player, u16 trackBits, u8 depth)
{
    s32 count;
    u32 bit;
    struct SoundTrack *track;

    if (player->ident != SOUND_PLAYER_READY)
        return;
    player->ident++;
    count = player->trackCount;
    track = player->tracks;
    bit = 1;
    while (count > 0) {
        if ((trackBits & bit) && (track->flags & TRACK_EXISTS)) {
            track->modulationDepth = depth;
            if (!track->modulationDepth)
                SoundTrackClearModulation(track);
        }
        count--;
        track++;
        bit <<= 1;
    }
    player->ident = SOUND_PLAYER_READY;
}

/** Set the LFO speed on every existing track selected by trackBits, on a
 * ready player. */
AT("00079BA8") void SoundPlayerSetLfoSpeed(
    struct SoundPlayer *player, u16 trackBits, u8 speed)
{
    s32 count;
    u32 bit;
    struct SoundTrack *track;

    if (player->ident != SOUND_PLAYER_READY)
        return;
    player->ident++;
    count = player->trackCount;
    track = player->tracks;
    bit = 1;
    while (count > 0) {
        if ((trackBits & bit) && (track->flags & TRACK_EXISTS)) {
            track->lfoSpeed = speed;
            if (!track->lfoSpeed)
                SoundTrackClearModulation(track);
        }
        count--;
        track++;
        bit <<= 1;
    }
    player->ident = SOUND_PLAYER_READY;
}

#define SOUND_MEMORY_BRANCH(condition) \
    if (condition)                     \
        goto branchTaken;              \
    else                               \
        goto branchNotTaken

/** MEMACC is the sequence language's tiny state machine.  It can assign and
 * add/subtract immediate or scratch-memory bytes, then conditionally execute
 * the normal four-byte GOTO command. */
AT("00079C1C") void SoundTrackMemoryCommand(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    u32 operation;
    u8 *address;
    u8 data;

    operation = *track->command;
    track->command++;
    address = player->memory + *track->command;
    track->command++;
    data = *track->command;
    track->command++;

    switch (operation) {
    case 0:  *address = data; return;
    case 1:  *address += data; return;
    case 2:  *address -= data; return;
    case 3:  *address = player->memory[data]; return;
    case 4:  *address += player->memory[data]; return;
    case 5:  *address -= player->memory[data]; return;
    case 6:  SOUND_MEMORY_BRANCH(*address == data); return;
    case 7:  SOUND_MEMORY_BRANCH(*address != data); return;
    case 8:  SOUND_MEMORY_BRANCH(*address > data); return;
    case 9:  SOUND_MEMORY_BRANCH(*address >= data); return;
    case 10: SOUND_MEMORY_BRANCH(*address <= data); return;
    case 11: SOUND_MEMORY_BRANCH(*address < data); return;
    case 12: SOUND_MEMORY_BRANCH(*address == player->memory[data]); return;
    case 13: SOUND_MEMORY_BRANCH(*address != player->memory[data]); return;
    case 14: SOUND_MEMORY_BRANCH(*address > player->memory[data]); return;
    case 15: SOUND_MEMORY_BRANCH(*address >= player->memory[data]); return;
    case 16: SOUND_MEMORY_BRANCH(*address <= player->memory[data]); return;
    case 17: SOUND_MEMORY_BRANCH(*address < player->memory[data]); return;
    default: return;
    }

branchTaken:
    sub_08080BC8(player, track, *(void **)0x03005D24);
    return;

branchNotTaken:
    track->command += 4;
}

/** Dispatch an extended command through the table immediately following the
 * driver's pitch data. */
AT("00079D74") void SoundTrackDispatchExtendedCommand(
    struct SoundPlayer *player, struct SoundTrack *track)
{
    u32 operation = *track->command;
    track->command++;
    sub_08080BC8(player, track, gSoundExtendedCommandTable[operation]);
}

/* The original object filled halfword alignment gaps with zero rather than
 * the assembler's Thumb NOP.  Keep those bytes explicit and executable so
 * each recovered section remains identical. */
