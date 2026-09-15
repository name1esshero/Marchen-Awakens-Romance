/* sub_08078644, called from SoundPlayerStop() (src/sound.c) as
 * `sub_08078644(p, track)` for each of the player's tracks. Detaches every
 * CGB channel a track owns: for each node of the singly-linked channel list
 * at track->channel (struct SoundCgbChannel, walked via its own `next`
 * field), disable the CGB oscillator if the channel is one of the four CGB
 * types (`channel->type & 7`, matching SoundDriverEnableCgb()'s types 1-4 in
 * src/sound_m4a.c), clear the channel's own active flag and its back-
 * reference to this track, then clear the track's own list head once every
 * channel has been visited.
 *
 * sub_08078634 -- the target of the one `bl` in the body -- is not a real
 * function: like the ten `sub_0808XXXX: bx rN` register-indirect-call
 * trampolines already known in this codebase (needed because ARMv4T Thumb
 * has no `blx` with a register operand; a `bl` to a `bx rN` stub is how the
 * compiler calls a function pointer that isn't already sitting in a register
 * a direct `bl` could target), `sub_08078634: bx r3` exists purely so that
 * `bl`'s automatic link-register write gives the indirect call a return
 * address. It corresponds to nothing more than calling
 * `sound->cgbOscillatorOff(...)` directly, which is what the C below does;
 * the compiler regenerates the identical trampoline call for that ordinary
 * function-pointer call.
 *
 * The logic below is confirmed correct against every branch, field access,
 * and the resolved call target, and matches the ROM almost completely --
 * every instruction is identical except the very first condition. The ROM
 * tests `track->flags & TRACK_EXISTS` with a bare `tst r0, r1; beq` (does
 * not materialize the masked result). The identical source idiom
 * `if (track->flags & TRACK_EXISTS)` in the already-matching
 * SoundPlayerImmediateInit() (a few functions earlier in
 * src/sound_m4a.c, confirmed by disassembling its linked bytes) instead
 * compiles to `ands r0, r1; cmp r0, #0; beq`, so this is not simply "the
 * idiom picked the wrong instruction" -- the same C shape produces both
 * forms elsewhere in this very file, and no variant tried here (`if (x & y)`,
 * `if (y & x)`, wrapping the whole body in the positive condition instead of
 * an early return) reproduces the bare `tst`. Left as a one-instruction
 * mismatch for a future pass with a working hypothesis (see
 * docs/PRET_AUDIT.md) rather than guessed at further.
 */

#include "gba/types.h"
#include "sound.h"

#define TRACK_EXISTS 0x80

/* Both structs are defined locally in src/sound_m4a.c, not in sound.h;
 * duplicated here (fields relevant to this function only) so this reference
 * file stands alone. Keep in sync with src/sound_m4a.c if those change. */
struct SoundCgbChannel
{
    u8 statusFlags;
    u8 type;
    u8 unknown_02[42];
    struct SoundTrack *track;
    void *previous;
    struct SoundCgbChannel *next;
};

struct SoundDriverState
{
    u8 unknown_00[44];
    void (*cgbOscillatorOff)(u8);
};

void SoundTrackReleaseChannels(struct SoundPlayer *player, struct SoundTrack *track)
{
    struct SoundCgbChannel *channel;

    if (track->flags & TRACK_EXISTS) {
        channel = track->channel;
        if (channel != 0) {
            do {
                if (channel->statusFlags != 0) {
                    u8 channelType = channel->type & 7;
                    if (channelType) {
                        struct SoundDriverState *sound = *(struct SoundDriverState **)0x03007FF0;
                        sound->cgbOscillatorOff(channelType);
                    }
                    channel->statusFlags = 0;
                }
                channel->track = 0;
                channel = channel->next;
            } while (channel != 0);
        }
        track->channel = 0;
    }
}
