/* Script-native adapters and tracked sound controls used by scene scripts.
 * Native arguments are 32-bit VM slots; the adapter return value tells the
 * interpreter whether to continue immediately or wait for an engine task.
 */
#include "gba/types.h"
#include "game_state.h"
#include "runtime_accessors.h"
#include "runtime_misc.h"
#include "sound.h"
#include "task_constructors.h"

#include "rom_section.h"
extern const char gSceneNcdExtension[];
#define SCRIPT_CONTINUE 1
#define SCRIPT_WAIT 0x7FFF
#define sText_NcdExtension gSceneNcdExtension

extern s32 sub_080053E4(s32 first, s32 second, s32 third, s32 mode);
extern s32 sub_08006760(s32 first, s32 second);
extern s32 sub_08006E88(s32 value);
extern s32 SpriteResourceFindGroup(s32 type, const char *name);
extern void sub_08005530(s32 first, s32 second, s32 third, s32 fourth,
                         s32 fifth, s32 sixth);
extern void sub_08005498(s32 first, s32 second, s32 third, s32 fourth,
                         s32 fifth);
extern struct EngineTask *StartSongWithTransition(u32 playerIndex,
                                                  u32 songIndex,
                                                  u32 *completion);
extern void sub_0808053C(s16 *first, s16 *second, s16 *third);
extern void SpriteRuntimeSetFields8C4(s32 first, s32 second, s32 third);
extern char *strcpy(char *destination, const char *source);
extern char *strcat(char *destination, const char *source);

void StopTrackedSong(u32 song);

/** Native script command: forward to CreateInputWaitTask() with mode fixed
 * to 0. @return The task's own status. */
AT("00005B34") s32 ScriptNativeStartTask05378(u32 count, const s32 *args,
                                               s32 *result)
{
    return CreateInputWaitTask(args[0], args[1], 0);
}

/** Native script command: forward to sub_080053E4() with mode fixed to 0.
 * @return The delegated call's status. */
AT("00005B44") s32 ScriptNativeStartTask053E4(u32 count, const s32 *args,
                                               s32 *result)
{
    return sub_080053E4(args[0], args[1], args[2], 0);
}

/** Native script command: forward to GameStateTestFlagsAC().
 * @return Always SCRIPT_CONTINUE. */
AT("00005B58") s32 ScriptNativeTestGameFlag(u32 count, const s32 *args,
                                             s32 *result)
{
    *result = GameStateTestFlagsAC(args[0]);
    return SCRIPT_CONTINUE;
}

/** Native script command: forward two arguments to sub_08006760().
 * @return Always SCRIPT_CONTINUE. */
AT("00005B6C") s32 ScriptNativeSetGameValue(u32 count, const s32 *args,
                                             s32 *result)
{
    sub_08006760(args[0], args[1]);
    return SCRIPT_CONTINUE;
}

/** Native script command: read one of the game state's +0x2C indexed
 * pointers. @return Always SCRIPT_CONTINUE. */
AT("00005B7C") s32 ScriptNativeGetStatePointer(u32 count, const s32 *args,
                                               s32 *result)
{
    *result = (s32)GameStateGetPointer2C(args[0]);
    return SCRIPT_CONTINUE;
}

/** Native script command: write one of the game state's +0x2C indexed
 * pointers. @return Always SCRIPT_CONTINUE. */
AT("00005B90") s32 ScriptNativeSetStatePointer(u32 count, const s32 *args,
                                               s32 *result)
{
    GameStateSetPointer2C(args[0], (void *)args[1]);
    return SCRIPT_CONTINUE;
}

/** Native script command: advance one of the game state's +0x2C indexed
 * pointers by a byte offset and report its new value.
 * @return Always SCRIPT_CONTINUE. */
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

/** Native script command: forward to sub_08006E88().
 * @return Always SCRIPT_CONTINUE. */
AT("00005BCC") s32 ScriptNativeLookupResource(u32 count, const s32 *args,
                                              s32 *result)
{
    *result = sub_08006E88(args[0]);
    return SCRIPT_CONTINUE;
}

/** Native script command: look up a named sprite resource group within the
 * NCD container named by args[0] plus the scene's NCD extension.
 * @return Always SCRIPT_CONTINUE. */
AT("00005BE0") s32 ScriptNativeFindNamedResource(u32 count,
                                                  const s32 *args,
                                                  s32 *result)
{
    char name[16];
    strcpy(name, (const char *)args[0]);
    strcat(name, sText_NcdExtension);
    *result = SpriteResourceFindGroup(sub_08006E88((s32)name),
                                 (const char *)args[1]);
    return SCRIPT_CONTINUE;
}

/** Native script command: forward five arguments (three narrowed to u16)
 * to sub_08005530(), with its sixth argument fixed to 0.
 * @return Always SCRIPT_CONTINUE. */
AT("00005C14") s32 ScriptNativeStartTask05530(u32 count, const s32 *args,
                                               s32 *result)
{
    sub_08005530(args[0], (u16)args[1], (u16)args[2], (u16)args[3],
                  args[4], 0);
    return SCRIPT_CONTINUE;
}

/** Native script command: forward to RuntimeGetPointer6120Field800().
 * @return Always SCRIPT_CONTINUE. */
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
    SpriteRuntimeSetFields8C4(first, second, third);
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

/** Native script command: if the requested scene value differs from the
 * current menu selection (+0x12EE), start a transition to it and wait;
 * otherwise continue immediately.
 * @return SCRIPT_CONTINUE if already selected, otherwise SCRIPT_WAIT. */
AT("00005D24") s32 ScriptNativeSelectSceneValue(u32 count, const s32 *args,
                                                 s32 *result)
{
    s32 status;

    if (GameStateGetField12EE() == args[0]) {
        status = SCRIPT_CONTINUE;
    } else {
        StartSongWithTransition(0, args[0], 0);
        GameStateSetField12EE(args[0]);
        status = SCRIPT_WAIT;
    }
    return status;
}

/** Native script command: start a song on sound player 6.
 * @return Always SCRIPT_WAIT. */
AT("00005D50") s32 ScriptNativeStartIndexedSong(u32 count, const s32 *args,
                                                 s32 *result)
{
    struct SoundPlayer *player = gSoundPlayerTable[6].player;
    const void *song = gSongTable[args[0]].header;
    SoundPlayerStart(player, song);
    return SCRIPT_WAIT;
}

/** Native script command: forward to CreateSoundFadeTask(), with its
 * completePendingOnFinish and completion arguments fixed to 0.
 * @return Always SCRIPT_CONTINUE. */
AT("00005D78") s32 ScriptNativeStartTask056AC(u32 count, const s32 *args,
                                               s32 *result)
{
    CreateSoundFadeTask(args[1], args[0], args[2], 0, 0);
    return SCRIPT_CONTINUE;
}
AT("00005D78") const u8 ScriptNativeStartTask056ACTail[2] = {0};

/** Native script command: forward to CreateSoundWaitTask(), publishing its
 * immediate result through the VM's result slot.
 * @return Always SCRIPT_CONTINUE. */
AT("00005D98") s32 ScriptNativeStartTask057C0(u32 count, const s32 *args,
                                               s32 *result)
{
    CreateSoundWaitTask(args[0], args[1], result, 0);
    return SCRIPT_CONTINUE;
}
AT("00005D98") const u8 ScriptNativeStartTask057C0Tail[2] = {0};

/** Select one of the nine MusicPlayer2000 instances used by scene scripts and
 * apply the requested volume to every active track. */
AT("00005DAC") s32 ScriptNativeSetSoundPlayerVolume(
    u32 count, const s32 *args, s32 *result)
{
    struct SoundPlayer *player;

    switch (args[0]) {
    case 0: player = &gSoundPlayer0; break;
    case 1: player = &gSoundPlayer1; break;
    case 2: player = &gSoundPlayer2; break;
    case 3: player = &gSoundPlayer3; break;
    case 4: player = &gSoundPlayer4; break;
    case 5: player = &gSoundPlayer5; break;
    case 6: player = &gSoundPlayer6; break;
    case 7: player = &gSoundPlayer7; break;
    case 8: player = &gSoundPlayer8; break;
    default: player = &gSoundPlayer0; break;
    }
    SoundPlayerSetVolume(player, 0xFF,
                         *(const u16 *)((const u8 *)args + 8));
    return SCRIPT_CONTINUE;
}

/** Native script command: forward to CreateSoundPlayerIdleWait(), publishing
 * its immediate result through the VM's result slot.
 * @return Always SCRIPT_CONTINUE. */
AT("00005E3C") s32 ScriptNativeStartTask05848(u32 count, const s32 *args,
                                               s32 *result)
{
    CreateSoundPlayerIdleWait(args[0], args[1], result, 0);
    return SCRIPT_CONTINUE;
}
AT("00005E3C") const u8 ScriptNativeStartTask05848Tail[2] = {0};

/** Native script command: forward to StopTrackedSong().
 * @return Always SCRIPT_WAIT. */
AT("00005E50") s32 ScriptNativeStopTrackedSong(u32 count, const s32 *args,
                                                s32 *result)
{
    StopTrackedSong(args[0]);
    return SCRIPT_WAIT;
}

/** Native script command: fade out all nine sound channels and clear the
 * current menu selection (+0x12EE).
 * @return Always SCRIPT_WAIT. */
AT("00005E64") s32 ScriptNativeResetNineChannels(u32 count, const s32 *args,
                                                  s32 *result)
{
    s32 channel;
    for (channel = 0; channel <= 8; channel++)
        CreateSoundFadeTask(16, channel, 1, 1, 0);
    GameStateSetField12EE(-1);
    return SCRIPT_WAIT;
}

/** Start a song and remember it as the current menu selection (+0x12EE),
 * skipping the restart when it's already playing unless force is set. */
AT("00005EA8") void StartTrackedSong(u32 song, s32 force)
{
    if (force)
        SoundSongStart((u16)song);
    else if (GameStateGetField12EE() != (s32)song)
        SoundSongStart((u16)song);
    GameStateSetField12EE(song);
}
AT("00005EA8") const u8 StartTrackedSongTail[2] = {0};

/** Stop a song and clear the current menu selection (+0x12EE) back to its
 * "nothing selected" sentinel of -1. */
AT("00005ED8") void StopTrackedSong(u32 song)
{
    SoundSongStop((u16)song);
    GameStateSetField12EE(-1);
}
AT("00005ED8") const u8 StopTrackedSongTail[2] = {0};

/** StartTrackedSong()'s counterpart tracking a second, independent
 * selection field (+0x42C4) instead of +0x12EE. */
AT("00005F04") void StartSecondaryTrackedSong(u32 song, s32 force)
{
    if (force)
        SoundSongStart((u16)song);
    else if (GameStateGetField42C4() != (s32)song)
        SoundSongStart((u16)song);
    GameStateSetField42C4(song);
}
AT("00005F04") const u8 StartSecondaryTrackedSongTail[2] = {0};

/** Narrow-argument wrapper around SoundSongStartOrChange(). */
AT("00005F34") void SoundSongStartAlternate(u32 song)
{
    SoundSongStartOrChange((u16)song);
}
AT("00005F34") const u8 SoundSongStartAlternateTail[2] = {0};

/** Narrow-argument wrapper around SoundSongStop(). */
AT("00005F44") void SoundSongStopU16(u32 song)
{
    SoundSongStop((u16)song);
}
AT("00005F44") const u8 SoundSongStopU16Tail[2] = {0};

/** Start a song by table index on a given sound player. */
AT("00005F54") void StartIndexedSong(u32 playerIndex, u32 songIndex)
{
    struct SoundPlayer *player = gSoundPlayerTable[playerIndex].player;
    const void *song = gSongTable[songIndex].header;
    SoundPlayerStart(player, song);
}

/** Stop the sound player at a given index in the engine's nine-player
 * table. */
AT("00006018") void StopSoundPlayer(u32 playerIndex)
{
    struct SoundPlayer *player = gSoundPlayerTable[playerIndex].player;
    SoundPlayerStop(player);
}
