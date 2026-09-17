/* Readable source form for StartSongOnFreePlayer (0x08005F7C), whose actual
 * matching Thumb instructions are in asm/code/code_0000C0.s.  Removing the
 * former empty asm fence changes old_agbcc's literal-load scheduling: it
 * builds the selected song entry before loading the dynamic player order.
 */
#include "sound.h"

/** Try six player slots in priority order and return the selected order index,
 * or -1 when every eligible slot is active. */
s32 StartSongOnFreePlayer(void *context, void *arguments, u32 song)
{
    struct SoundPlayer *players[10];
    const struct SoundPlayerEntry *playerTable = gSoundPlayerTable;
    const struct SoundSongEntry *songTable = gSongTable;
    const s16 *order = gDynamicSoundPlayerOrder;
    const struct SoundSongEntry *songEntry = &songTable[song];
    s32 i;

    players[0] = &gSoundPlayer0;
    players[1] = &gSoundPlayer1;
    players[2] = &gSoundPlayer2;
    players[3] = &gSoundPlayer3;
    players[4] = &gSoundPlayer4;
    players[5] = &gSoundPlayer5;
    players[6] = &gSoundPlayer6;
    players[7] = &gSoundPlayer7;
    players[8] = &gSoundPlayer8;

    for (i = 0; i <= 5; i++)
    {
        s32 player = order[i];

        if ((s32)players[player]->status < 0)
        {
            SoundPlayerStart(playerTable[player].player, songEntry->header);
            return i;
        }
    }
    return -1;
}
