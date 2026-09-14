#include "sound.h"

#include "rom_section.h"

/* PCM sample and CGB waveform data referenced by the SoundToneData tables
 * below; see asm/sound_samples.s and asm/game_table_handlers.s. */
extern const u8 gCgbWaveform_08089CF0[];
extern const u8 gCgbWaveform_0808ABB4[];
extern const u8 gCgbWaveform_0808B034[];
extern const u8 gCgbWaveform_0808B054[];
extern const u8 gCgbWaveform_0808B064[];
extern const u8 gCgbWaveform_0808B0A4[];
extern const u8 gWave_0808B89C[];
extern const u8 gWave_0808E990[];
extern const u8 gWave_0808F9AC[];
extern const u8 gWave_08091F80[];
extern const u8 gWave_08094C24[];
extern const u8 gWave_08098418[];
extern const u8 gWave_0809A8B4[];
extern const u8 gWave_0809D3B8[];
extern const u8 gWave_0809FCBC[];
extern const u8 gWave_080A32BC[];
extern const u8 gWave_080A5950[];
extern const u8 gWave_080A7D4C[];
extern const u8 gWave_080A9928[];
extern const u8 gWave_080AAE64[];
extern const u8 gWave_080AD068[];
extern const u8 gWave_080AE1AC[];
extern const u8 gWave_080AF4E0[];
extern const u8 gWave_080B2BBC[];
extern const u8 gWave_080B3FF0[];
extern const u8 gWave_080B5144[];
extern const u8 gWave_080B53DC[];
extern const u8 gWave_080B5C5C[];
extern const u8 gWave_080B6F60[];
extern const u8 gWave_080B83F4[];
extern const u8 gWave_080BBB0C[];
extern const u8 gWave_080BDD90[];
extern const u8 gWave_080C22A4[];
extern const u8 gWave_080C3C6C[];
extern const u8 gWave_080C8600[];
extern const u8 gWave_080CB454[];
extern const u8 gWave_080CDE0C[];
extern const u8 gWave_080CFC68[];
extern const u8 gWave_080D69FC[];
extern const u8 gWave_080D89D0[];
extern const u8 gWave_080DF764[];
extern const u8 gWave_080E146C[];
extern const u8 gWave_080E1538[];
extern const u8 gWave_080E4090[];
extern const u8 gWave_080E5114[];
extern const u8 gWave_080E6620[];
extern const u8 gWave_080E73F8[];
extern const u8 gWave_080E7F0C[];
extern const u8 gWave_080E8408[];
extern const u8 gWave_080E989C[];
extern const u8 gWave_080EB138[];
extern const u8 gWave_080EF5CC[];
extern const u8 gWave_080EF9A8[];
extern const u8 gWave_080F1670[];
extern const u8 gWave_080F30AC[];
extern const u8 gWave_080F35D0[];
extern const u8 gWave_080F3E84[];
extern const u8 gWave_080F72D8[];
extern const u8 gWave_080FB4EC[];
extern const u8 gWave_080FBD14[];
extern const u8 gWave_080FF6C8[];
extern const u8 gWave_081030DC[];
extern const u8 gWave_0810594C[];
extern const u8 gWave_08106424[];
extern const u8 gWave_08107414[];
extern const u8 gWave_0810A564[];
extern const u8 gWave_0810B22C[];
extern const u8 gWave_0810C6A8[];
extern const u8 gWave_0810E2BC[];
extern const u8 gWave_08110EBC[];
extern const u8 gWave_08112BD0[];
extern const u8 gWave_08115A24[];
extern const u8 gWave_081165D4[];
extern const u8 gWave_08116F9C[];
extern const u8 gWave_0811D424[];
extern const u8 gWave_081222F8[];
extern const u8 gWave_0812798C[];
extern const u8 gWave_081282B0[];
extern const u8 gWave_0812AB64[];
extern const u8 gWave_0812ECD8[];
extern const u8 gWave_0812FD0C[];
extern const u8 gWave_08132310[];
extern const u8 gWave_08137498[];
extern const u8 gWave_08138240[];
extern const u8 gWave_08138730[];
extern const u8 gWave_0813C25C[];
extern const u8 gWave_0813D2D0[];
extern const u8 gWave_0813E10C[];
extern const u8 gWave_0813F560[];
extern const u8 gWave_08141978[];
extern const u8 gWave_0814718C[];
extern const u8 gWave_08148270[];
extern const u8 gWave_0814DF30[];
extern const u8 gWave_08151FA4[];
extern const u8 gWave_08153F00[];
extern const u8 gWave_08158E14[];
extern const u8 gWave_0815B328[];
extern const u8 gWave_0815D03C[];
extern const u8 gWave_081612D0[];
extern const u8 gWave_08163564[];
extern const u8 gWave_0816D8CC[];
extern const u8 gWave_08170180[];
extern const u8 gWave_08172AEC[];
extern const u8 gWave_0817D644[];
extern const u8 gWave_081833B8[];
extern const u8 gWave_08185E8C[];
extern const u8 gWave_08188608[];
extern const u8 gWave_0818A3FC[];
extern const u8 gWave_0818C700[];
extern const u8 gWave_0818D564[];
extern const u8 gWave_0818D8A8[];
extern const u8 gWave_0818EA0C[];
extern const u8 gWave_081930B0[];
extern const u8 gWave_08194BC4[];
extern const u8 gWave_08196338[];
extern const u8 gWave_08198F6C[];
extern const u8 gWave_0819B220[];
extern const u8 gWave_0819B524[];

/* Fixed-point pitch, sample-rate, PSG, and extended-command tables. */
AT("000895C8") const u8 gSoundScaleTable[180] = {
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
};

AT("0008967C") const u32 gSoundFrequencyTable[12] = {
    0x80000000, 0x879C7C97, 0x8FACD61E, 0x9837F052, 0xA14517CC, 0xAADC0848, 0xB504F334, 0xBFC886BB,
    0xCB2FF52A, 0xD744FCCB, 0xE411F03A, 0xF1A1BF39,
};

AT("000896AC") const u16 gPcmSamplesPerVBlankTable[12] = {
    0x0060, 0x0084, 0x00B0, 0x00E0, 0x0108, 0x0130, 0x0160, 0x01C0, 0x0210, 0x0260, 0x02A0, 0x02C0,
};

AT("000896C4") const u8 gCgbScaleTable[132] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
};

AT("00089748") const s16 gCgbFrequencyTable[12] = {
    -2004, -1891, -1785, -1685, -1591, -1501, -1417, -1337, -1262, -1192, -1125, -1062,
};

AT("00089760") const u8 gCgbNoiseTable[60] = {
    0xD7, 0xD6, 0xD5, 0xD4, 0xC7, 0xC6, 0xC5, 0xC4, 0xB7, 0xB6, 0xB5, 0xB4,
    0xA7, 0xA6, 0xA5, 0xA4, 0x97, 0x96, 0x95, 0x94, 0x87, 0x86, 0x85, 0x84,
    0x77, 0x76, 0x75, 0x74, 0x67, 0x66, 0x65, 0x64, 0x57, 0x56, 0x55, 0x54,
    0x47, 0x46, 0x45, 0x44, 0x37, 0x36, 0x35, 0x34, 0x27, 0x26, 0x25, 0x24,
    0x17, 0x16, 0x15, 0x14, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
};

AT("0008979C") const u8 gCgb3VolumeTable[68] = {
    0x00, 0x00, 0x60, 0x60, 0x60, 0x60, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80,
    0x80, 0x80, 0x20, 0x20, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x1C, 0x1E, 0x20, 0x24, 0x28, 0x2A, 0x2C,
    0x30, 0x34, 0x36, 0x38, 0x3C, 0x40, 0x42, 0x44, 0x48, 0x4C, 0x4E, 0x50,
    0x54, 0x58, 0x5A, 0x5C, 0x60, 0x00, 0x00, 0x00,
};

/* Extended-command dispatch table; each thumb-bit-set address is one of
 * these already-decompiled per-field track readers, declared generically
 * here since only their address (not their real signature) is needed. */
extern void CallRuntimeHandler(void);
extern void SoundTrackReadWavePointer(void);
extern void SoundTrackReadToneType(void);
extern void SoundTrackReadToneAttack(void);
extern void SoundTrackReadToneDecay(void);
extern void SoundTrackReadToneSustain(void);
extern void SoundTrackReadToneRelease(void);
extern void SoundTrackReadPseudoEchoVolume(void);
extern void SoundTrackReadPseudoEchoLength(void);
extern void SoundTrackReadToneLength(void);
extern void SoundTrackReadTonePanSweep(void);

AT("000897E0") void *const gSoundExtendedCommandTable[12] = {
    [0] = (void *)((u32)CallRuntimeHandler + 1), [1] = (void *)((u32)SoundTrackReadWavePointer + 1), [2] = (void *)((u32)SoundTrackReadToneType + 1), [3] = (void *)((u32)CallRuntimeHandler + 1),
    [4] = (void *)((u32)SoundTrackReadToneAttack + 1), [5] = (void *)((u32)SoundTrackReadToneDecay + 1), [6] = (void *)((u32)SoundTrackReadToneSustain + 1), [7] = (void *)((u32)SoundTrackReadToneRelease + 1),
    [8] = (void *)((u32)SoundTrackReadPseudoEchoVolume + 1), [9] = (void *)((u32)SoundTrackReadPseudoEchoLength + 1), [10] = (void *)((u32)SoundTrackReadToneLength + 1), [11] = (void *)((u32)SoundTrackReadTonePanSweep + 1),
};

/* Voice records use the standard 12-byte ToneData layout.  The wave field
 * is a sample pointer for PCM voices, a small oscillator ID for PSG voices,
 * or a subordinate tone table for split/drum voices. */
AT("00089810") const struct SoundToneData gVoiceGroupMain[188] = {
    [0] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0808B89C, .attack = 255, .decay = 127, .sustain = 206, .release = 216 },
    [1] = { .type = 3, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_0808B034, .attack = 0, .decay = 0, .sustain = 12, .release = 3 },
    [2] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 1, .sustain = 8, .release = 4 },
    [3] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 6, .sustain = 0, .release = 0 },
    [4] = { .type = 2, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 14, .release = 0 },
    [5] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000000, .attack = 0, .decay = 0, .sustain = 6, .release = 0 },
    [6] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0808E990, .attack = 255, .decay = 127, .sustain = 206, .release = 216 },
    [7] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [8] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0808F9AC, .attack = 255, .decay = 127, .sustain = 206, .release = 216 },
    [9] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08091F80, .attack = 255, .decay = 127, .sustain = 206, .release = 216 },
    [10] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [11] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [12] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [13] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [14] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [15] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [16] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08094C24, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [17] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08098418, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [18] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0809A8B4, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [19] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [20] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [21] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [22] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [23] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [24] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [25] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [26] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [27] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [28] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [29] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [30] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [31] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [32] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [33] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [34] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [35] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [36] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [37] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [38] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [39] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [40] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0809D3B8, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [41] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [42] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [43] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0809FCBC, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [44] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [45] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [46] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [47] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [48] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080A32BC, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [49] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080A5950, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [50] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080A7D4C, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [51] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [52] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080A9928, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [53] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080AAE64, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [54] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080AD068, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [55] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080AE1AC, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [56] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080AF4E0, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [57] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [58] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B2BBC, .attack = 37, .decay = 127, .sustain = 206, .release = 188 },
    [59] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [60] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [61] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B3FF0, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [62] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [63] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [64] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [65] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [66] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [67] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [68] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B5144, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [69] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B53DC, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [70] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B5C5C, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [71] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B6F60, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [72] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [73] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080B83F4, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [74] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [75] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [76] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [77] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [78] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [79] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [80] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080BBB0C, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [81] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [82] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [83] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [84] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [85] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [86] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [87] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [88] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [89] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [90] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [91] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [92] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [93] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [94] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [95] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [96] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080BDD90, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [97] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [98] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080C22A4, .attack = 255, .decay = 127, .sustain = 206, .release = 242 },
    [99] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [100] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080C3C6C, .attack = 255, .decay = 188, .sustain = 255, .release = 216 },
    [101] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080C8600, .attack = 255, .decay = 188, .sustain = 255, .release = 216 },
    [102] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [103] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [104] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [105] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [106] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [107] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [108] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [109] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [110] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [111] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [112] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080CB454, .attack = 255, .decay = 127, .sustain = 206, .release = 216 },
    [113] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [114] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [115] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [116] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080CDE0C, .attack = 255, .decay = 127, .sustain = 206, .release = 216 },
    [117] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [118] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [119] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [120] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [121] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [122] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [123] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [124] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080CFC68, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [125] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080D69FC, .attack = 255, .decay = 127, .sustain = 206, .release = 188 },
    [126] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080D89D0, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [127] = { .type = 128, .key = 0, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_08089CF0, .attack = 0, .decay = 0, .sustain = 0, .release = 0 },
    [128] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [129] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [130] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [131] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [132] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [133] = { .type = 8, .key = 30, .length = 0, .panSweep = 192, .wave = (u32)gWave_080DF764, .attack = 255, .decay = 0, .sustain = 255, .release = 216 },
    [134] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E146C, .attack = 255, .decay = 0, .sustain = 255, .release = 165 },
    [135] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [136] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [137] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E1538, .attack = 255, .decay = 127, .sustain = 206, .release = 165 },
    [138] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E4090, .attack = 255, .decay = 0, .sustain = 255, .release = 204 },
    [139] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [140] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E5114, .attack = 255, .decay = 0, .sustain = 255, .release = 165 },
    [141] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E6620, .attack = 255, .decay = 0, .sustain = 255, .release = 165 },
    [142] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E73F8, .attack = 255, .decay = 0, .sustain = 255, .release = 127 },
    [143] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [144] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [145] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [146] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E7F0C, .attack = 255, .decay = 165, .sustain = 255, .release = 38 },
    [147] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [148] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [149] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [150] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E8408, .attack = 255, .decay = 165, .sustain = 255, .release = 38 },
    [151] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [152] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080E989C, .attack = 255, .decay = 0, .sustain = 206, .release = 188 },
    [153] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080EB138, .attack = 255, .decay = 188, .sustain = 206, .release = 204 },
    [154] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [155] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [156] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [157] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [158] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [159] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [160] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [161] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [162] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [163] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [164] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [165] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [166] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [167] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [168] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [169] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [170] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [171] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [172] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [173] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [174] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [175] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [176] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [177] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [178] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [179] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [180] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [181] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [182] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [183] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [184] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080EF5CC, .attack = 255, .decay = 127, .sustain = 206, .release = 165 },
    [185] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080EF9A8, .attack = 255, .decay = 127, .sustain = 206, .release = 242 },
    [186] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [187] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080F1670, .attack = 255, .decay = 127, .sustain = 206, .release = 242 },
};

AT("0008A0E0") const struct SoundToneData gVoiceGroupSecondary[127] = {
    [0] = { .type = 4, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000001, .attack = 0, .decay = 0, .sustain = 12, .release = 4 },
    [1] = { .type = 3, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_0808B034, .attack = 0, .decay = 1, .sustain = 12, .release = 0 },
    [2] = { .type = 3, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_0808B064, .attack = 0, .decay = 0, .sustain = 12, .release = 0 },
    [3] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080F30AC, .attack = 64, .decay = 127, .sustain = 206, .release = 165 },
    [4] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080F35D0, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [5] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080F3E84, .attack = 64, .decay = 38, .sustain = 206, .release = 216 },
    [6] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080F72D8, .attack = 255, .decay = 38, .sustain = 206, .release = 216 },
    [7] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080FB4EC, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [8] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080FBD14, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [9] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_080FF6C8, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [10] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081030DC, .attack = 255, .decay = 127, .sustain = 206, .release = 204 },
    [11] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0810594C, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [12] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08106424, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [13] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08107414, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [14] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [15] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0810A564, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [16] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0810B22C, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [17] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0810C6A8, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [18] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0810E2BC, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [19] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08110EBC, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [20] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08112BD0, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [21] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08115A24, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [22] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081165D4, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [23] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08116F9C, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [24] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0811D424, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [25] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081222F8, .attack = 255, .decay = 127, .sustain = 206, .release = 127 },
    [26] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0812798C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [27] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [28] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081282B0, .attack = 255, .decay = 127, .sustain = 231, .release = 204 },
    [29] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0812AB64, .attack = 255, .decay = 127, .sustain = 231, .release = 204 },
    [30] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0812ECD8, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [31] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0812FD0C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [32] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08132310, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [33] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08137498, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [34] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08138240, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [35] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08138730, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [36] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0813C25C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [37] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0813D2D0, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [38] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0813E10C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [39] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0813F560, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [40] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08141978, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [41] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0814718C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [42] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08148270, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [43] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0814DF30, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [44] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08151FA4, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [45] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08153F00, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [46] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08158E14, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [47] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0815B328, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [48] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0815D03C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [49] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081612D0, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [50] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08163564, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [51] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [52] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [53] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0816D8CC, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [54] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08170180, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [55] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08172AEC, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [56] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0817D644, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [57] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081833B8, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [58] = { .type = 8, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08185E8C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [59] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08188608, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [60] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0818A3FC, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [61] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0818C700, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [62] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0818D564, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [63] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [64] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0818D8A8, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [65] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0818EA0C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [66] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_081930B0, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [67] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08194BC4, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [68] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08196338, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [69] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_08198F6C, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [70] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0819B220, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [71] = { .type = 0, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gWave_0819B524, .attack = 255, .decay = 127, .sustain = 231, .release = 127 },
    [72] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [73] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [74] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [75] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [76] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [77] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [78] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [79] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [80] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [81] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [82] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [83] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [84] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [85] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [86] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [87] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [88] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [89] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [90] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [91] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [92] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [93] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [94] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [95] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [96] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [97] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [98] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [99] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [100] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [101] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [102] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [103] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [104] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [105] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [106] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [107] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [108] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [109] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [110] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [111] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [112] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [113] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [114] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [115] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [116] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [117] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [118] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [119] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [120] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [121] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [122] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [123] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [124] = { .type = 4, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000000, .attack = 0, .decay = 0, .sustain = 12, .release = 1 },
    [125] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [126] = { .type = 2, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000003, .attack = 0, .decay = 1, .sustain = 12, .release = 0 },
};

AT("0008A6D4") const struct SoundToneData gVoiceGroupEffects[128] = {
    [0] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000003, .attack = 0, .decay = 2, .sustain = 9, .release = 4 },
    [1] = { .type = 2, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 2, .sustain = 9, .release = 5 },
    [2] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000000, .attack = 0, .decay = 2, .sustain = 10, .release = 4 },
    [3] = { .type = 3, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_0808B054, .attack = 0, .decay = 6, .sustain = 12, .release = 2 },
    [4] = { .type = 2, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000001, .attack = 0, .decay = 2, .sustain = 12, .release = 4 },
    [5] = { .type = 4, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000000, .attack = 0, .decay = 0, .sustain = 12, .release = 2 },
    [6] = { .type = 3, .key = 60, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_0808B0A4, .attack = 0, .decay = 4, .sustain = 6, .release = 6 },
    [7] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [8] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [9] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [10] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [11] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [12] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [13] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [14] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [15] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [16] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [17] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [18] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [19] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [20] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [21] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [22] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [23] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [24] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [25] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [26] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [27] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [28] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [29] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [30] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [31] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [32] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [33] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [34] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [35] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [36] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [37] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [38] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [39] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [40] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [41] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [42] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [43] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [44] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [45] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [46] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [47] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [48] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [49] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [50] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [51] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [52] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [53] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [54] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [55] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [56] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [57] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [58] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [59] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [60] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [61] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [62] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [63] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [64] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [65] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [66] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [67] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [68] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [69] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [70] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [71] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [72] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [73] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [74] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [75] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [76] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [77] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [78] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [79] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [80] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [81] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [82] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [83] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [84] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [85] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [86] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [87] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [88] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [89] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [90] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [91] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [92] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [93] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [94] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [95] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [96] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [97] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [98] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [99] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [100] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [101] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [102] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [103] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [104] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [105] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [106] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [107] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [108] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [109] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [110] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [111] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [112] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [113] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [114] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [115] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [116] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [117] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [118] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [119] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [120] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [121] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [122] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [123] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [124] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [125] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [126] = { .type = 1, .key = 60, .length = 0, .panSweep = 0, .wave = 0x00000002, .attack = 0, .decay = 0, .sustain = 15, .release = 0 },
    [127] = { .type = 128, .key = 0, .length = 0, .panSweep = 0, .wave = (u32)gCgbWaveform_0808ABB4, .attack = 0, .decay = 0, .sustain = 0, .release = 0 },
};
/* Nine MusicPlayer2000 instances and their fixed IWRAM track pools. */
extern struct SoundPlayer gSoundPlayer0, gSoundPlayer1, gSoundPlayer2, gSoundPlayer3,
    gSoundPlayer4, gSoundPlayer5, gSoundPlayer6, gSoundPlayer7, gSoundPlayer8;
extern struct SoundTrack gSoundPlayer0Tracks[], gSoundPlayer1Tracks[], gSoundPlayer2Tracks[],
    gSoundPlayer3Tracks[], gSoundPlayer4Tracks[], gSoundPlayer5Tracks[], gSoundPlayer6Tracks[],
    gSoundPlayer7Tracks[], gSoundPlayer8Tracks[];

AT("0008B144") const struct SoundPlayerEntry gSoundPlayerTable[9] = {
    [0] = { .player = &gSoundPlayer0, .tracks = gSoundPlayer0Tracks, .trackCount = 13 },
    [1] = { .player = &gSoundPlayer1, .tracks = gSoundPlayer1Tracks, .trackCount = 3 },
    [2] = { .player = &gSoundPlayer2, .tracks = gSoundPlayer2Tracks, .trackCount = 3 },
    [3] = { .player = &gSoundPlayer3, .tracks = gSoundPlayer3Tracks, .trackCount = 3 },
    [4] = { .player = &gSoundPlayer4, .tracks = gSoundPlayer4Tracks, .trackCount = 3 },
    [5] = { .player = &gSoundPlayer5, .tracks = gSoundPlayer5Tracks, .trackCount = 3 },
    [6] = { .player = &gSoundPlayer6, .tracks = gSoundPlayer6Tracks, .trackCount = 10 },
    [7] = { .player = &gSoundPlayer7, .tracks = gSoundPlayer7Tracks, .trackCount = 3 },
    [8] = { .player = &gSoundPlayer8, .tracks = gSoundPlayer8Tracks, .trackCount = 3 },
};

/* Song header symbols are declared as opaque byte arrays here: each one's
 * real type (from src/song_headers.c) varies with its track count, but
 * only the address is needed to build this table. */
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_001[];
extern const u8 gSongHeader_002[];
extern const u8 gSongHeader_003[];
extern const u8 gSongHeader_004[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_007[];
extern const u8 gSongHeader_008[];
extern const u8 gSongHeader_009[];
extern const u8 gSongHeader_010[];
extern const u8 gSongHeader_011[];
extern const u8 gSongHeader_012[];
extern const u8 gSongHeader_013[];
extern const u8 gSongHeader_014[];
extern const u8 gSongHeader_015[];
extern const u8 gSongHeader_016[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_018[];
extern const u8 gSongHeader_019[];
extern const u8 gSongHeader_020[];
extern const u8 gSongHeader_021[];
extern const u8 gSongHeader_022[];
extern const u8 gSongHeader_023[];
extern const u8 gSongHeader_024[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_026[];
extern const u8 gSongHeader_027[];
extern const u8 gSongHeader_028[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_030[];
extern const u8 gSongHeader_031[];
extern const u8 gSongHeader_032[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_101[];
extern const u8 gSongHeader_102[];
extern const u8 gSongHeader_103[];
extern const u8 gSongHeader_104[];
extern const u8 gSongHeader_105[];
extern const u8 gSongHeader_106[];
extern const u8 gSongHeader_107[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_109[];
extern const u8 gSongHeader_110[];
extern const u8 gSongHeader_111[];
extern const u8 gSongHeader_112[];
extern const u8 gSongHeader_113[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_115[];
extern const u8 gSongHeader_116[];
extern const u8 gSongHeader_117[];
extern const u8 gSongHeader_118[];
extern const u8 gSongHeader_119[];
extern const u8 gSongHeader_120[];
extern const u8 gSongHeader_121[];
extern const u8 gSongHeader_122[];
extern const u8 gSongHeader_123[];
extern const u8 gSongHeader_124[];
extern const u8 gSongHeader_125[];
extern const u8 gSongHeader_126[];
extern const u8 gSongHeader_127[];
extern const u8 gSongHeader_128[];
extern const u8 gSongHeader_129[];
extern const u8 gSongHeader_130[];
extern const u8 gSongHeader_131[];
extern const u8 gSongHeader_132[];
extern const u8 gSongHeader_133[];
extern const u8 gSongHeader_134[];
extern const u8 gSongHeader_135[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_140[];
extern const u8 gSongHeader_141[];
extern const u8 gSongHeader_142[];
extern const u8 gSongHeader_143[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_150[];
extern const u8 gSongHeader_151[];
extern const u8 gSongHeader_152[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_160[];
extern const u8 gSongHeader_161[];
extern const u8 gSongHeader_162[];
extern const u8 gSongHeader_163[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_170[];
extern const u8 gSongHeader_171[];
extern const u8 gSongHeader_172[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_180[];
extern const u8 gSongHeader_181[];
extern const u8 gSongHeader_182[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_190[];
extern const u8 gSongHeader_191[];
extern const u8 gSongHeader_192[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_000[];
extern const u8 gSongHeader_200[];
extern const u8 gSongHeader_201[];
extern const u8 gSongHeader_202[];
extern const u8 gSongHeader_203[];
extern const u8 gSongHeader_204[];
extern const u8 gSongHeader_205[];
extern const u8 gSongHeader_206[];
extern const u8 gSongHeader_207[];
extern const u8 gSongHeader_208[];
extern const u8 gSongHeader_209[];
extern const u8 gSongHeader_210[];
extern const u8 gSongHeader_211[];
extern const u8 gSongHeader_212[];
extern const u8 gSongHeader_213[];
extern const u8 gSongHeader_214[];
extern const u8 gSongHeader_215[];
extern const u8 gSongHeader_216[];
extern const u8 gSongHeader_217[];
extern const u8 gSongHeader_218[];
extern const u8 gSongHeader_219[];
extern const u8 gSongHeader_220[];

/* Song IDs are direct indexes into this table. Header addresses identify the
 * editable sequence that each game-side caller selects. */
AT("0008B1B0") const struct SoundSongEntry gSongTable[221] = {
    [0] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [1] = { .header = gSongHeader_001, .player = 0, .otherPlayer = 0 },
    [2] = { .header = gSongHeader_002, .player = 0, .otherPlayer = 0 },
    [3] = { .header = gSongHeader_003, .player = 0, .otherPlayer = 0 },
    [4] = { .header = gSongHeader_004, .player = 0, .otherPlayer = 0 },
    [5] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [6] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [7] = { .header = gSongHeader_007, .player = 0, .otherPlayer = 0 },
    [8] = { .header = gSongHeader_008, .player = 0, .otherPlayer = 0 },
    [9] = { .header = gSongHeader_009, .player = 0, .otherPlayer = 0 },
    [10] = { .header = gSongHeader_010, .player = 0, .otherPlayer = 0 },
    [11] = { .header = gSongHeader_011, .player = 0, .otherPlayer = 0 },
    [12] = { .header = gSongHeader_012, .player = 0, .otherPlayer = 0 },
    [13] = { .header = gSongHeader_013, .player = 0, .otherPlayer = 0 },
    [14] = { .header = gSongHeader_014, .player = 0, .otherPlayer = 0 },
    [15] = { .header = gSongHeader_015, .player = 0, .otherPlayer = 0 },
    [16] = { .header = gSongHeader_016, .player = 0, .otherPlayer = 0 },
    [17] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [18] = { .header = gSongHeader_018, .player = 0, .otherPlayer = 0 },
    [19] = { .header = gSongHeader_019, .player = 0, .otherPlayer = 0 },
    [20] = { .header = gSongHeader_020, .player = 0, .otherPlayer = 0 },
    [21] = { .header = gSongHeader_021, .player = 0, .otherPlayer = 0 },
    [22] = { .header = gSongHeader_022, .player = 0, .otherPlayer = 0 },
    [23] = { .header = gSongHeader_023, .player = 0, .otherPlayer = 0 },
    [24] = { .header = gSongHeader_024, .player = 0, .otherPlayer = 0 },
    [25] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [26] = { .header = gSongHeader_026, .player = 0, .otherPlayer = 0 },
    [27] = { .header = gSongHeader_027, .player = 0, .otherPlayer = 0 },
    [28] = { .header = gSongHeader_028, .player = 0, .otherPlayer = 0 },
    [29] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [30] = { .header = gSongHeader_030, .player = 0, .otherPlayer = 0 },
    [31] = { .header = gSongHeader_031, .player = 0, .otherPlayer = 0 },
    [32] = { .header = gSongHeader_032, .player = 0, .otherPlayer = 0 },
    [33] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [34] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [35] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [36] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [37] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [38] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [39] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [40] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [41] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [42] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [43] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [44] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [45] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [46] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [47] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [48] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [49] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [50] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [51] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [52] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [53] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [54] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [55] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [56] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [57] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [58] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [59] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [60] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [61] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [62] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [63] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [64] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [65] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [66] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [67] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [68] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [69] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [70] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [71] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [72] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [73] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [74] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [75] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [76] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [77] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [78] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [79] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [80] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [81] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [82] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [83] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [84] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [85] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [86] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [87] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [88] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [89] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [90] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [91] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [92] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [93] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [94] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [95] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [96] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [97] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [98] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [99] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [100] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [101] = { .header = gSongHeader_101, .player = 1, .otherPlayer = 1 },
    [102] = { .header = gSongHeader_102, .player = 1, .otherPlayer = 1 },
    [103] = { .header = gSongHeader_103, .player = 1, .otherPlayer = 1 },
    [104] = { .header = gSongHeader_104, .player = 1, .otherPlayer = 1 },
    [105] = { .header = gSongHeader_105, .player = 1, .otherPlayer = 1 },
    [106] = { .header = gSongHeader_106, .player = 1, .otherPlayer = 1 },
    [107] = { .header = gSongHeader_107, .player = 1, .otherPlayer = 1 },
    [108] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [109] = { .header = gSongHeader_109, .player = 6, .otherPlayer = 6 },
    [110] = { .header = gSongHeader_110, .player = 6, .otherPlayer = 6 },
    [111] = { .header = gSongHeader_111, .player = 2, .otherPlayer = 2 },
    [112] = { .header = gSongHeader_112, .player = 2, .otherPlayer = 2 },
    [113] = { .header = gSongHeader_113, .player = 2, .otherPlayer = 2 },
    [114] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [115] = { .header = gSongHeader_115, .player = 2, .otherPlayer = 2 },
    [116] = { .header = gSongHeader_116, .player = 2, .otherPlayer = 2 },
    [117] = { .header = gSongHeader_117, .player = 2, .otherPlayer = 2 },
    [118] = { .header = gSongHeader_118, .player = 2, .otherPlayer = 2 },
    [119] = { .header = gSongHeader_119, .player = 3, .otherPlayer = 3 },
    [120] = { .header = gSongHeader_120, .player = 6, .otherPlayer = 6 },
    [121] = { .header = gSongHeader_121, .player = 3, .otherPlayer = 3 },
    [122] = { .header = gSongHeader_122, .player = 3, .otherPlayer = 3 },
    [123] = { .header = gSongHeader_123, .player = 3, .otherPlayer = 3 },
    [124] = { .header = gSongHeader_124, .player = 3, .otherPlayer = 3 },
    [125] = { .header = gSongHeader_125, .player = 0, .otherPlayer = 0 },
    [126] = { .header = gSongHeader_126, .player = 6, .otherPlayer = 6 },
    [127] = { .header = gSongHeader_127, .player = 6, .otherPlayer = 6 },
    [128] = { .header = gSongHeader_128, .player = 0, .otherPlayer = 0 },
    [129] = { .header = gSongHeader_129, .player = 6, .otherPlayer = 6 },
    [130] = { .header = gSongHeader_130, .player = 4, .otherPlayer = 4 },
    [131] = { .header = gSongHeader_131, .player = 4, .otherPlayer = 4 },
    [132] = { .header = gSongHeader_132, .player = 4, .otherPlayer = 4 },
    [133] = { .header = gSongHeader_133, .player = 4, .otherPlayer = 4 },
    [134] = { .header = gSongHeader_134, .player = 4, .otherPlayer = 4 },
    [135] = { .header = gSongHeader_135, .player = 4, .otherPlayer = 4 },
    [136] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [137] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [138] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [139] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [140] = { .header = gSongHeader_140, .player = 4, .otherPlayer = 4 },
    [141] = { .header = gSongHeader_141, .player = 4, .otherPlayer = 4 },
    [142] = { .header = gSongHeader_142, .player = 2, .otherPlayer = 2 },
    [143] = { .header = gSongHeader_143, .player = 5, .otherPlayer = 5 },
    [144] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [145] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [146] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [147] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [148] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [149] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [150] = { .header = gSongHeader_150, .player = 4, .otherPlayer = 4 },
    [151] = { .header = gSongHeader_151, .player = 4, .otherPlayer = 4 },
    [152] = { .header = gSongHeader_152, .player = 5, .otherPlayer = 5 },
    [153] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [154] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [155] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [156] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [157] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [158] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [159] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [160] = { .header = gSongHeader_160, .player = 4, .otherPlayer = 4 },
    [161] = { .header = gSongHeader_161, .player = 4, .otherPlayer = 4 },
    [162] = { .header = gSongHeader_162, .player = 4, .otherPlayer = 4 },
    [163] = { .header = gSongHeader_163, .player = 4, .otherPlayer = 4 },
    [164] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [165] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [166] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [167] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [168] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [169] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [170] = { .header = gSongHeader_170, .player = 4, .otherPlayer = 4 },
    [171] = { .header = gSongHeader_171, .player = 4, .otherPlayer = 4 },
    [172] = { .header = gSongHeader_172, .player = 4, .otherPlayer = 4 },
    [173] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [174] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [175] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [176] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [177] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [178] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [179] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [180] = { .header = gSongHeader_180, .player = 4, .otherPlayer = 4 },
    [181] = { .header = gSongHeader_181, .player = 4, .otherPlayer = 4 },
    [182] = { .header = gSongHeader_182, .player = 5, .otherPlayer = 5 },
    [183] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [184] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [185] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [186] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [187] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [188] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [189] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [190] = { .header = gSongHeader_190, .player = 4, .otherPlayer = 4 },
    [191] = { .header = gSongHeader_191, .player = 4, .otherPlayer = 4 },
    [192] = { .header = gSongHeader_192, .player = 4, .otherPlayer = 4 },
    [193] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [194] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [195] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [196] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [197] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [198] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [199] = { .header = gSongHeader_000, .player = 0, .otherPlayer = 0 },
    [200] = { .header = gSongHeader_200, .player = 4, .otherPlayer = 4 },
    [201] = { .header = gSongHeader_201, .player = 4, .otherPlayer = 4 },
    [202] = { .header = gSongHeader_202, .player = 4, .otherPlayer = 4 },
    [203] = { .header = gSongHeader_203, .player = 4, .otherPlayer = 4 },
    [204] = { .header = gSongHeader_204, .player = 4, .otherPlayer = 4 },
    [205] = { .header = gSongHeader_205, .player = 4, .otherPlayer = 4 },
    [206] = { .header = gSongHeader_206, .player = 4, .otherPlayer = 4 },
    [207] = { .header = gSongHeader_207, .player = 4, .otherPlayer = 4 },
    [208] = { .header = gSongHeader_208, .player = 4, .otherPlayer = 4 },
    [209] = { .header = gSongHeader_209, .player = 4, .otherPlayer = 4 },
    [210] = { .header = gSongHeader_210, .player = 4, .otherPlayer = 4 },
    [211] = { .header = gSongHeader_211, .player = 5, .otherPlayer = 5 },
    [212] = { .header = gSongHeader_212, .player = 5, .otherPlayer = 5 },
    [213] = { .header = gSongHeader_213, .player = 4, .otherPlayer = 4 },
    [214] = { .header = gSongHeader_214, .player = 4, .otherPlayer = 4 },
    [215] = { .header = gSongHeader_215, .player = 4, .otherPlayer = 4 },
    [216] = { .header = gSongHeader_216, .player = 4, .otherPlayer = 4 },
    [217] = { .header = gSongHeader_217, .player = 4, .otherPlayer = 4 },
    [218] = { .header = gSongHeader_218, .player = 4, .otherPlayer = 4 },
    [219] = { .header = gSongHeader_219, .player = 4, .otherPlayer = 4 },
    [220] = { .header = gSongHeader_220, .player = 4, .otherPlayer = 4 },
};

