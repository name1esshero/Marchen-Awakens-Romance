/* Readable source forms for the four SoundPlayer lifecycle helpers that
 * remain in asm/code/code_0780C0.s.  See COMPILER_HINT_CLEANUP.md: the
 * current old_agbcc reconstruction always coalesces the ready-signature
 * value with another temporary instead of preserving the ROM's r3 load.
 */
#include "sound.h"

#define FADE_TEMPORARY 0x01
#define FADE_IN 0x02
#define FADE_VOLUME_SHIFT 2
#define SOUND_FADE_VOLUME_FULL (64 << FADE_VOLUME_SHIFT)

/** Resume a paused sequence player. */
void SoundPlayerResume(struct SoundPlayer *player)
{
    if (player->ident == SOUND_PLAYER_READY)
        player->status &= ~SOUND_PLAYER_PAUSED;
}

/** Begin a permanent fade to silence. */
void SoundPlayerFadeOut(struct SoundPlayer *player, u16 interval)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->fadeCounter = interval;
        player->fadeInterval = interval;
        player->fadeVolume = SOUND_FADE_VOLUME_FULL;
    }
}

/** Mark a fade as temporary so completion pauses rather than retires tracks. */
void SoundPlayerFadeOutTemporary(struct SoundPlayer *player, u16 interval)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->fadeCounter = interval;
        player->fadeInterval = interval;
        player->fadeVolume = SOUND_FADE_VOLUME_FULL | FADE_TEMPORARY;
    }
}

/** Fade a ready player in over interval frames and clear its paused flag. */
void SoundPlayerFadeIn(struct SoundPlayer *player, u16 interval)
{
    if (player->ident == SOUND_PLAYER_READY)
    {
        player->fadeCounter = interval;
        player->fadeInterval = interval;
        player->fadeVolume = FADE_IN;
        player->status &= ~SOUND_PLAYER_PAUSED;
    }
}
