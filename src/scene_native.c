/* Script-native adapters and tracked sound controls used by scene scripts.
 * Native arguments are 32-bit VM slots; the adapter return value tells the
 * interpreter whether to continue immediately or wait for an engine task.
 */
#include "gba/types.h"
#include "runtime_accessors.h"
#include "runtime_misc.h"

#define AT(x) __attribute__((section(".rom." x)))
#define SCRIPT_CONTINUE 1
#define SCRIPT_WAIT 0x7FFF

extern s32 CreateInputWaitTask(s32 first, s32 second, s32 mode);
extern s32 sub_080053E4(s32 first, s32 second, s32 third, s32 mode);
extern s32 sub_08006760(s32 first, s32 second);
extern s32 sub_08006E88(s32 value);
extern s32 FindResourceByName(s32 type, const char *name);
extern s32 sub_08005848(s32 first, s32 second, s32 third, s32 mode);
extern s32 sub_080057C0(s32 first, s32 second, s32 third, s32 mode);
extern void sub_08005530(s32 first, s32 second, s32 third, s32 fourth,
                         s32 fifth, s32 sixth);
extern void sub_08005498(s32 first, s32 second, s32 third, s32 fourth,
                         s32 fifth);
extern void sub_080056AC(s32 first, s32 second, s32 third, s32 fourth,
                         s32 fifth);
extern void sub_080059C8(s32 first, s32 second, s32 third);
extern void sub_0808053C(s16 *first, s16 *second, s16 *third);
extern void sub_08080504(s32 first, s32 second, s32 third);
extern void sub_0807915C(void *player, const void *song);
extern void SoundPlayerStop(void *player);
extern char *strcpy(char *destination, const char *source);
extern char *strcat(char *destination, const char *source);

struct SoundPlayerEntry {
    void *player;
    u8 unused04[8];
};

struct SongEntry {
    const void *song;
    u32 unused04;
};

extern struct SoundPlayerEntry gSoundPlayerTable[];
extern struct SongEntry gSongTable[];
extern void sub_08078A70(u16 song);
extern void sub_08078A9C(u16 song);
extern void sub_08078B3C(u16 song);
void StopTrackedSong(u32 song);

AT("00005B34") s32 ScriptNativeStartTask05378(u32 count, const s32 *args,
                                               s32 *result)
{
    return CreateInputWaitTask(args[0], args[1], 0);
}

AT("00005B44") s32 ScriptNativeStartTask053E4(u32 count, const s32 *args,
                                               s32 *result)
{
    return sub_080053E4(args[0], args[1], args[2], 0);
}

AT("00005B58") s32 ScriptNativeTestGameFlag(u32 count, const s32 *args,
                                             s32 *result)
{
    *result = GameStateTestFlagsAC(args[0]);
    return SCRIPT_CONTINUE;
}

AT("00005B6C") s32 ScriptNativeSetGameValue(u32 count, const s32 *args,
                                             s32 *result)
{
    sub_08006760(args[0], args[1]);
    return SCRIPT_CONTINUE;
}

AT("00005B7C") s32 ScriptNativeGetStatePointer(u32 count, const s32 *args,
                                               s32 *result)
{
    *result = (s32)GameStateGetPointer2C(args[0]);
    return SCRIPT_CONTINUE;
}

AT("00005B90") s32 ScriptNativeSetStatePointer(u32 count, const s32 *args,
                                               s32 *result)
{
    GameStateSetPointer2C(args[0], (void *)args[1]);
    return SCRIPT_CONTINUE;
}

AT("00005BA0") s32 ScriptNativeAdvanceStatePointer(u32 count,
                                                   const s32 *args,
                                                   s32 *result)
{
    s32 slot = args[0];
    GameStateSetPointer2C(slot, (u8 *)GameStateGetPointer2C(slot) + args[1]);
    *result = (s32)GameStateGetPointer2C(args[0]);
    return SCRIPT_CONTINUE;
}
AT("00005BA0") const u8 ScriptNativeAdvanceStatePointerTail[2] = {0};

AT("00005BCC") s32 ScriptNativeLookupResource(u32 count, const s32 *args,
                                              s32 *result)
{
    *result = sub_08006E88(args[0]);
    return SCRIPT_CONTINUE;
}

AT("00005BE0") s32 ScriptNativeFindNamedResource(u32 count,
                                                  const s32 *args,
                                                  s32 *result)
{
    char name[16];
    strcpy(name, (const char *)args[0]);
    strcat(name, (const char *)0x08086A6C);
    *result = FindResourceByName(sub_08006E88((s32)name),
                                 (const char *)args[1]);
    return SCRIPT_CONTINUE;
}

AT("00005C14") s32 ScriptNativeStartTask05530(u32 count, const s32 *args,
                                               s32 *result)
{
    sub_08005530(args[0], (u16)args[1], (u16)args[2], (u16)args[3],
                  args[4], 0);
    return SCRIPT_CONTINUE;
}

AT("00005C74") s32 ScriptNativeGetSpriteRuntime(u32 count, const s32 *args,
                                                s32 *result)
{
    *result = (s32)RuntimeGetPointer6120Field800();
    return SCRIPT_CONTINUE;
}
AT("00005C74") const u8 ScriptNativeGetSpriteRuntimeTail[2] = {0};

#ifdef NONMATCHING
AT("00005C88") s32 ScriptNativeSetRuntimeCoordinate(u32 count,
                                                     const s32 *args,
                                                     s32 *result)
{
    s16 first;
    s16 second;
    s16 third;
    sub_0808053C(&first, &second, &third);
    switch (args[0]) {
    case 0:
        first = args[1];
        break;
    case 1:
        second = args[1];
        break;
    case 2:
        third = args[1];
        break;
    }
    sub_08080504(first, second, third);
    return SCRIPT_CONTINUE;
}

AT("00005CDC") s32 ScriptNativeGetRuntimeCoordinate(u32 count,
                                                     const s32 *args,
                                                     s32 *result)
{
    s16 first;
    s16 second;
    s16 third;
    sub_0808053C(&first, &second, &third);
    switch (args[0]) {
    case 0:
        *result = first;
        break;
    case 1:
        *result = second;
        break;
    case 2:
        *result = third;
        break;
    }
    return SCRIPT_CONTINUE;
}
AT("00005CDC") const u8 ScriptNativeGetRuntimeCoordinateTail[2] = {0};
#endif

AT("00005D24") s32 ScriptNativeSelectSceneValue(u32 count, const s32 *args,
                                                 s32 *result)
{
    s32 status;

    if (GameStateGetField12EE() == args[0]) {
        status = SCRIPT_CONTINUE;
    } else {
        sub_080059C8(0, args[0], 0);
        GameStateSetField12EE(args[0]);
        status = SCRIPT_WAIT;
    }
    return status;
}

AT("00005D50") s32 ScriptNativeStartIndexedSong(u32 count, const s32 *args,
                                                 s32 *result)
{
    void *player = gSoundPlayerTable[6].player;
    const void *song = gSongTable[args[0]].song;
    sub_0807915C(player, song);
    return SCRIPT_WAIT;
}

AT("00005D78") s32 ScriptNativeStartTask056AC(u32 count, const s32 *args,
                                               s32 *result)
{
    sub_080056AC(args[1], args[0], args[2], 0, 0);
    return SCRIPT_CONTINUE;
}
AT("00005D78") const u8 ScriptNativeStartTask056ACTail[2] = {0};

AT("00005D98") s32 ScriptNativeStartTask057C0(u32 count, const s32 *args,
                                               s32 *result)
{
    sub_080057C0(args[0], args[1], (s32)result, 0);
    return SCRIPT_CONTINUE;
}
AT("00005D98") const u8 ScriptNativeStartTask057C0Tail[2] = {0};

AT("00005E3C") s32 ScriptNativeStartTask05848(u32 count, const s32 *args,
                                               s32 *result)
{
    sub_08005848(args[0], args[1], (s32)result, 0);
    return SCRIPT_CONTINUE;
}
AT("00005E3C") const u8 ScriptNativeStartTask05848Tail[2] = {0};

AT("00005E50") s32 ScriptNativeStopTrackedSong(u32 count, const s32 *args,
                                                s32 *result)
{
    StopTrackedSong(args[0]);
    return SCRIPT_WAIT;
}

AT("00005E64") s32 ScriptNativeResetNineChannels(u32 count, const s32 *args,
                                                  s32 *result)
{
    s32 channel;
    for (channel = 0; channel <= 8; channel++)
        sub_080056AC(16, channel, 1, 1, 0);
    GameStateSetField12EE(-1);
    return SCRIPT_WAIT;
}

AT("00005EA8") void StartTrackedSong(u32 song, s32 force)
{
    if (force)
        sub_08078A70((u16)song);
    else if (GameStateGetField12EE() != (s32)song)
        sub_08078A70((u16)song);
    GameStateSetField12EE(song);
}
AT("00005EA8") const u8 StartTrackedSongTail[2] = {0};

AT("00005ED8") void StopTrackedSong(u32 song)
{
    sub_08078B3C((u16)song);
    GameStateSetField12EE(-1);
}
AT("00005ED8") const u8 StopTrackedSongTail[2] = {0};

AT("00005F04") void StartSecondaryTrackedSong(u32 song, s32 force)
{
    if (force)
        sub_08078A70((u16)song);
    else if (GameStateGetField42C4() != (s32)song)
        sub_08078A70((u16)song);
    GameStateSetField42C4(song);
}
AT("00005F04") const u8 StartSecondaryTrackedSongTail[2] = {0};

AT("00005F34") void SoundSongStartAlternate(u32 song)
{
    sub_08078A9C((u16)song);
}
AT("00005F34") const u8 SoundSongStartAlternateTail[2] = {0};

AT("00005F44") void SoundSongStopU16(u32 song)
{
    sub_08078B3C((u16)song);
}
AT("00005F44") const u8 SoundSongStopU16Tail[2] = {0};

AT("00005F54") void StartIndexedSong(u32 playerIndex, u32 songIndex)
{
    void *player = gSoundPlayerTable[playerIndex].player;
    const void *song = gSongTable[songIndex].song;
    sub_0807915C(player, song);
}

AT("00006018") void StopSoundPlayer(u32 playerIndex)
{
    void *player = gSoundPlayerTable[playerIndex].player;
    SoundPlayerStop(player);
}
