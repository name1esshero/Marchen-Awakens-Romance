# Sound engine

The ROM uses Nintendo's MusicPlayer2000 engine for sampled music and sound
effects, plus the GBA's four programmable sound generator channels. The
matching declarations are in `include/sound.h`; recovered driver code lives in
`src/sound_m4a.c`, `src/sound_cgb_update.c`, and `src/sound.c`.

`struct SoundPlayer` is the 64-byte music-player state. Its signed status word
contains active-track bits and uses bit 31 for the paused/idle state. Each
player owns 80-byte `SoundTrack` records. A track contains its sequence cursor,
voice parameters, pitch, volume, pan, modulation, pseudo-echo settings, and its
active mixer or PSG channel.

The recovered driver path covers player creation and song selection, fade and
resume handling, tempo/volume/pitch/pan controls, modulation, sequence memory
commands, extended tone commands, and the complete PSG update loop. The PSG
routine updates square channels 1 and 2, wave channel 3, and noise channel 4,
including envelope state, frequency changes, pan/volume routing, length expiry,
and writes to the corresponding `SOUND1CNT` through `SOUND4CNT` registers.

Script-visible audio uses scheduler tasks in `src/sound_tasks.c`:

- `SoundFadeTask` waits for and performs a player fade.
- `SoundWaitTask` waits on the game's player-busy field.
- `SoundPlayerIdleTask` observes MusicPlayer2000's status word directly.
- `StartSongWithTransition` starts an idle player immediately or stops an active
  player and creates `SoundStartTask` for the delayed replacement.
- `ResumeSoundPlayer` selects a player from the nine-entry player table.

The ROM data is now typed at its source boundaries as well. `src/sound_tables.c`
contains the pitch, frequency, PSG, player, song, and three twelve-byte
instrument banks. `src/song_headers.c` exposes all 102 unique variable-sized
song headers and records every game-visible song ID that shares one. A
zero-track header is only its four-byte prefix; the PCM wave header immediately
after it must not be mistaken for a voices pointer. `src/song_tracks.c` owns the
fourteen sequence tracks used by songs 1 and 2 as separate bytecode arrays, so
their entry points and exact lengths can be edited and verified independently.
`gDynamicSoundPlayerOrder` documents the six player slots that dynamic effects
try in priority order.

All listed routines are compiled with agbcc and occupy their original ROM
addresses. `SoundUpdateCgbChannels` uses the older bundled agbcc frontend in a
separate translation unit because that reproduces the original module's code
generation. `make compare` verifies the linked bytes rather than accepting a
semantic-only reconstruction.

The remaining audio work includes a command-level assembler for the recovered
sequence bytecode and the rest of the song tracks. The earlier MusicPlayer2000
assembly before `0x08078948` is the SDK's hand-written mixed Thumb/ARM core and
is retained as such.
