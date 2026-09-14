#include "gba/types.h"

/* MusicPlayer2000 PSG update module.  This module is intentionally compiled
 * separately: the original library boundary affects agbcc register allocation. */
#define SOUND_CHANNEL_SF_START 0x80
#define SOUND_CHANNEL_SF_STOP 0x40
#define SOUND_CHANNEL_SF_IEC 0x04
#define SOUND_CHANNEL_SF_ENV 0x03
#define SOUND_CHANNEL_SF_ENV_ATTACK 0x03
#define SOUND_CHANNEL_SF_ENV_DECAY 0x02
#define SOUND_CHANNEL_SF_ENV_SUSTAIN 0x01
#define SOUND_CHANNEL_SF_ENV_RELEASE 0x00
#define SOUND_CHANNEL_SF_ON 0xC7
#define CGB_CHANNEL_MO_PIT 2
#define CGB_CHANNEL_MO_VOL 1
#define CGB_NRx2_ENV_DIR_DEC 0
#define CGB_NRx2_ENV_DIR_INC 8
#define TONEDATA_TYPE_FIX 8
#define REG_ADDR_NR10 0x04000060
#define REG_ADDR_NR11 0x04000062
#define REG_ADDR_NR12 0x04000063
#define REG_ADDR_NR13 0x04000064
#define REG_ADDR_NR14 0x04000065
#define REG_ADDR_NR21 0x04000068
#define REG_ADDR_NR22 0x04000069
#define REG_ADDR_NR23 0x0400006C
#define REG_ADDR_NR24 0x0400006D
#define REG_ADDR_NR30 0x04000070
#define REG_ADDR_NR31 0x04000072
#define REG_ADDR_NR32 0x04000073
#define REG_ADDR_NR33 0x04000074
#define REG_ADDR_NR34 0x04000075
#define REG_ADDR_NR41 0x04000078
#define REG_ADDR_NR42 0x04000079
#define REG_ADDR_NR43 0x0400007C
#define REG_ADDR_NR44 0x0400007D
#define REG_NR51 (*(volatile u8*)0x04000081)
#define REG_SOUNDBIAS_H (*(volatile u8*)0x04000089)
#define REG_WAVE_RAM0 (*(volatile u32*)0x04000090)
#define REG_WAVE_RAM1 (*(volatile u32*)0x04000094)
#define REG_WAVE_RAM2 (*(volatile u32*)0x04000098)
#define REG_WAVE_RAM3 (*(volatile u32*)0x0400009C)
struct SoundCgbChannel {
    u8 statusFlags;
    u8 type;
    u8 rightVolume;
    u8 leftVolume;
    u8 attack;
    u8 decay;
    u8 sustain;
    u8 release;
    u8 key;
    u8 envelopeVolume;
    u8 envelopeGoal;
    u8 envelopeCounter;
    u8 pseudoEchoVolume;
    u8 pseudoEchoLength;
    u8 unused0E[2];
    u8 gateTime;
    u8 midiKey;
    u8 velocity;
    u8 priority;
    u8 rhythmPan;
    u8 unused15[4];
    u8 sustainGoal;
    u8 n4;
    u8 pan;
    u8 panMask;
    u8 modify;
    u8 length;
    u8 sweep;
    u32 frequency;
    u32 *wavePointer;
    u32 *currentPointer;
    void *track;
    void *prev;
    void *next;
    u8 unused38[8];
};

struct SoundDriverState {
    u32 ident;
    u8 dma;
    u8 reverb;
    u8 maxChannels;
    u8 masterVolume;
    u8 frequency;
    u8 mode;
    u8 cycleCounter;
    u8 period;
    u8 maxLines;
    u8 unused0D[3];
    s32 samples;
    s32 pcmFrequency;
    s32 divisionFrequency;
    struct SoundCgbChannel *cgbChannels;
};
extern void SoundUpdateCgbChannelVolume(struct SoundCgbChannel *);
extern void SoundDisableCgbOscillator(u8);
extern const u8 gCgb3VolumeTable[68];
__attribute__((section(".rom.0007955C"))) void SoundUpdateCgbChannels(void)
{
    s32 ch;
    struct SoundCgbChannel *channels;
    s32 prevC15;
    struct SoundDriverState *soundInfo = (*(struct SoundDriverState **)0x03007FF0);
    vu8 *nrx0ptr;
    vu8 *nrx1ptr;
    vu8 *nrx2ptr;
    vu8 *nrx3ptr;
    vu8 *nrx4ptr;
    s32 envelopeStepTimeAndDir;

    // The signed byte comparisons that cast to s8 perform 'and' by 0xFF.
    int mask = 0xff;

    if (soundInfo->cycleCounter)
        soundInfo->cycleCounter--;
    else
        soundInfo->cycleCounter = 14;

    for (ch = 1, channels = soundInfo->cgbChannels; ch <= 4; ch++, channels++)
    {
        if (!(channels->statusFlags & SOUND_CHANNEL_SF_ON))
            continue;

        /* 1. determine hardware channel registers */
        switch (ch)
        {
        case 1:
            nrx0ptr = (vu8 *)(REG_ADDR_NR10);
            nrx1ptr = (vu8 *)(REG_ADDR_NR11);
            nrx2ptr = (vu8 *)(REG_ADDR_NR12);
            nrx3ptr = (vu8 *)(REG_ADDR_NR13);
            nrx4ptr = (vu8 *)(REG_ADDR_NR14);
            break;
        case 2:
            nrx0ptr = (vu8 *)(REG_ADDR_NR10+1);
            nrx1ptr = (vu8 *)(REG_ADDR_NR21);
            nrx2ptr = (vu8 *)(REG_ADDR_NR22);
            nrx3ptr = (vu8 *)(REG_ADDR_NR23);
            nrx4ptr = (vu8 *)(REG_ADDR_NR24);
            break;
        case 3:
            nrx0ptr = (vu8 *)(REG_ADDR_NR30);
            nrx1ptr = (vu8 *)(REG_ADDR_NR31);
            nrx2ptr = (vu8 *)(REG_ADDR_NR32);
            nrx3ptr = (vu8 *)(REG_ADDR_NR33);
            nrx4ptr = (vu8 *)(REG_ADDR_NR34);
            break;
        default:
            nrx0ptr = (vu8 *)(REG_ADDR_NR30+1);
            nrx1ptr = (vu8 *)(REG_ADDR_NR41);
            nrx2ptr = (vu8 *)(REG_ADDR_NR42);
            nrx3ptr = (vu8 *)(REG_ADDR_NR43);
            nrx4ptr = (vu8 *)(REG_ADDR_NR44);
            break;
        }

        prevC15 = soundInfo->cycleCounter;
        envelopeStepTimeAndDir = *nrx2ptr;

        /* 2. calculate envelope volume */
        if (channels->statusFlags & SOUND_CHANNEL_SF_START)
        {
            if (!(channels->statusFlags & SOUND_CHANNEL_SF_STOP))
            {
                channels->statusFlags = SOUND_CHANNEL_SF_ENV_ATTACK;
                channels->modify = CGB_CHANNEL_MO_PIT | CGB_CHANNEL_MO_VOL;
                SoundUpdateCgbChannelVolume(channels);
                switch (ch)
                {
                case 1:
                    *nrx0ptr = channels->sweep;
                    // fallthrough
                case 2:
                    *nrx1ptr = ((u32)channels->wavePointer << 6) + channels->length;
                    goto init_env_step_time_dir;
                case 3:
                    if (channels->wavePointer != channels->currentPointer)
                    {
                        *nrx0ptr = 0x40;
                        REG_WAVE_RAM0 = channels->wavePointer[0];
                        REG_WAVE_RAM1 = channels->wavePointer[1];
                        REG_WAVE_RAM2 = channels->wavePointer[2];
                        REG_WAVE_RAM3 = channels->wavePointer[3];
                        channels->currentPointer = channels->wavePointer;
                    }
                    *nrx0ptr = 0;
                    *nrx1ptr = channels->length;
                    if (channels->length)
                        channels->n4 = 0xC0;
                    else
                        channels->n4 = 0x80;
                    break;
                default:
                    *nrx1ptr = channels->length;
                    *nrx3ptr = (u32)channels->wavePointer << 3;
                init_env_step_time_dir:
                    envelopeStepTimeAndDir = channels->attack + CGB_NRx2_ENV_DIR_INC;
                    if (channels->length)
                        channels->n4 = 0x40;
                    else
                        channels->n4 = 0x00;
                    break;
                }
                channels->envelopeCounter = channels->attack;
                if ((s8)(channels->attack & mask))
                {
                    channels->envelopeVolume = 0;
                    goto envelope_step_complete;
                }
                else
                {
                    // skip attack phase if attack is instantaneous (=0)
                    goto envelope_decay_start;
                }
            }
            else
            {
                goto oscillator_off;
            }
        }
        else if (channels->statusFlags & SOUND_CHANNEL_SF_IEC)
        {
            channels->pseudoEchoLength--;
            if ((s8)(channels->pseudoEchoLength & mask) <= 0)
            {
            oscillator_off:
                SoundDisableCgbOscillator(ch);
                channels->statusFlags = 0;
                goto channel_complete;
            }
            goto envelope_complete;
        }
        else if ((channels->statusFlags & SOUND_CHANNEL_SF_STOP) && (channels->statusFlags & SOUND_CHANNEL_SF_ENV))
        {
            channels->statusFlags &= ~SOUND_CHANNEL_SF_ENV;
            channels->envelopeCounter = channels->release;
            if ((s8)(channels->release & mask))
            {
                channels->modify |= CGB_CHANNEL_MO_VOL;
                if (ch != 3)
                    envelopeStepTimeAndDir = channels->release | CGB_NRx2_ENV_DIR_DEC;
                goto envelope_step_complete;
            }
            else
            {
                goto envelope_pseudoecho_start;
            }
        }
        else
        {
        envelope_step_repeat:
            if (channels->envelopeCounter == 0)
            {
                if (ch == 3)
                    channels->modify |= CGB_CHANNEL_MO_VOL;

                SoundUpdateCgbChannelVolume(channels);
                if ((channels->statusFlags & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_RELEASE)
                {
                    channels->envelopeVolume--;
                    if ((s8)(channels->envelopeVolume & mask) <= 0)
                    {
                    envelope_pseudoecho_start:
                        channels->envelopeVolume = ((channels->envelopeGoal * channels->pseudoEchoVolume) + 0xFF) >> 8;
                        if (channels->envelopeVolume)
                        {
                            channels->statusFlags |= SOUND_CHANNEL_SF_IEC;
                            channels->modify |= CGB_CHANNEL_MO_VOL;
                            if (ch != 3)
                                envelopeStepTimeAndDir = 0 | CGB_NRx2_ENV_DIR_INC;
                            goto envelope_complete;
                        }
                        else
                        {
                            goto oscillator_off;
                        }
                    }
                    else
                    {
                        channels->envelopeCounter = channels->release;
                    }
                }
                else if ((channels->statusFlags & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_SUSTAIN)
                {
                envelope_sustain:
                    channels->envelopeVolume = channels->sustainGoal;
                    channels->envelopeCounter = 7;
                }
                else if ((channels->statusFlags & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_DECAY)
                {
                    int envelopeVolume, sustainGoal;

                    channels->envelopeVolume--;
                    envelopeVolume = (s8)(channels->envelopeVolume & mask);
                    sustainGoal = (s8)(channels->sustainGoal);
                    if (envelopeVolume <= sustainGoal)
                    {
                    envelope_sustain_start:
                        if (channels->sustain == 0)
                        {
                            channels->statusFlags &= ~SOUND_CHANNEL_SF_ENV;
                            goto envelope_pseudoecho_start;
                        }
                        else
                        {
                            channels->statusFlags--;
                            channels->modify |= CGB_CHANNEL_MO_VOL;
                            if (ch != 3)
                                envelopeStepTimeAndDir = 0 | CGB_NRx2_ENV_DIR_INC;
                            goto envelope_sustain;
                        }
                    }
                    else
                    {
                        channels->envelopeCounter = channels->decay;
                    }
                }
                else
                {
                    channels->envelopeVolume++;
                    if ((u8)(channels->envelopeVolume & mask) >= channels->envelopeGoal)
                    {
                    envelope_decay_start:
                        channels->statusFlags--;
                        channels->envelopeCounter = channels->decay;
                        if ((u8)(channels->envelopeCounter & mask))
                        {
                            channels->modify |= CGB_CHANNEL_MO_VOL;
                            channels->envelopeVolume = channels->envelopeGoal;
                            if (ch != 3)
                                envelopeStepTimeAndDir = channels->decay | CGB_NRx2_ENV_DIR_DEC;
                        }
                        else
                        {
                            goto envelope_sustain_start;
                        }
                    }
                    else
                    {
                        channels->envelopeCounter = channels->attack;
                    }
                }
            }
        }

    envelope_step_complete:
        // every 15 frames, envelope calculation has to be done twice
        // to keep up with the hardware envelope rate (1/64 s)
        channels->envelopeCounter--;
        if (prevC15 == 0)
        {
            prevC15--;
            goto envelope_step_repeat;
        }

    envelope_complete:
        /* 3. apply pitch to HW registers */
        if (channels->modify & CGB_CHANNEL_MO_PIT)
        {
            if (ch < 4 && (channels->type & TONEDATA_TYPE_FIX))
            {
                int dac_pwm_rate = REG_SOUNDBIAS_H;

                if (dac_pwm_rate < 0x40)        // if PWM rate = 32768 Hz
                    channels->frequency = (channels->frequency + 2) & 0x7fc;
                else if (dac_pwm_rate < 0x80)   // if PWM rate = 65536 Hz
                    channels->frequency = (channels->frequency + 1) & 0x7fe;
            }

            if (ch != 4)
                *nrx3ptr = channels->frequency;
            else
                *nrx3ptr = (*nrx3ptr & 0x08) | channels->frequency;
            channels->n4 = (channels->n4 & 0xC0) + (*((u8 *)(&channels->frequency) + 1));
            *nrx4ptr = (s8)(channels->n4 & mask);
        }

        /* 4. apply envelope & volume to HW registers */
        if (channels->modify & CGB_CHANNEL_MO_VOL)
        {
            REG_NR51 = (REG_NR51 & ~channels->panMask) | channels->pan;
            if (ch == 3)
            {
                *nrx2ptr = gCgb3VolumeTable[channels->envelopeVolume];
                if (channels->n4 & 0x80)
                {
                    *nrx0ptr = 0x80;
                    *nrx4ptr = channels->n4;
                    channels->n4 &= 0x7f;
                }
            }
            else
            {
                u32 envMask = 0xF;
                *nrx2ptr = (envelopeStepTimeAndDir & envMask) + (channels->envelopeVolume << 4);
                *nrx4ptr = channels->n4 | 0x80;
                if (ch == 1 && !(*nrx0ptr & 0x08))
                    *nrx4ptr = channels->n4 | 0x80;
            }
        }

    channel_complete:
        channels->modify = 0;
    }
}


/* The original module used a zero halfword for final alignment. */
__attribute__((section(".rom.0007955C"))) const u8 SoundUpdateCgbChannelsTail[2] = {0};
