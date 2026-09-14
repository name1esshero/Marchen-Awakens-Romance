#include "sound.h"
#include "rom_section.h"
#ifdef __GNUC__
#define SOUND_TABLE_BARRIER(songTable, order) ((void)0)
#else
#define SOUND_TABLE_BARRIER(songTable, order) \
    asm volatile("" : : "r" (songTable), "r" (order))
#endif
extern struct SoundPlayer gSoundPlayer0;
extern struct SoundPlayer gSoundPlayer1;
extern struct SoundPlayer gSoundPlayer2;
extern struct SoundPlayer gSoundPlayer3;
extern struct SoundPlayer gSoundPlayer4;
extern struct SoundPlayer gSoundPlayer5;
extern struct SoundPlayer gSoundPlayer6;
extern struct SoundPlayer gSoundPlayer7;
extern struct SoundPlayer gSoundPlayer8;
/* Try the six player slots in the engine's priority order and return the
 * order position used, or -1 when every eligible player is active. */
AT("00005F7C") s32 StartSongOnFreePlayer(void *context, void *arguments, u32 song)
{
    struct SoundPlayer *players[10];
    const s16 *order;
    const struct SoundSongEntry *songEntry;
    const struct SoundSongEntry *songTable;
    const struct SoundPlayerEntry *playerTable;
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
    i = 0;
    playerTable = gSoundPlayerTable;
    songTable = gSongTable;
    order = gDynamicSoundPlayerOrder;
    SOUND_TABLE_BARRIER(songTable, order);
    songEntry = &songTable[song];
    for (; i <= 5; i++) {
        s32 player = order[i];
        if ((s32)players[player]->status < 0) {
            SoundPlayerStart(playerTable[player].player, songEntry->header);
            return i;
        }
    }
    return -1;
}
