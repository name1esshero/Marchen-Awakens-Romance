# Driver-referenced audio

Open [the sample browser](index.html) to listen to 115 editable instrument and
effect samples. `samples/*.wav` contains unsigned 8-bit mono PCM; the driver
uses signed PCM. `samples/manifest.json` supplies each WaveData header, loop
start, frequency, original span, and referring tone addresses.

`python3 tools/sound_assets.py build` compiles WAV + JSON into `build/sound`.
The ordinary ROM build includes those outputs. No original sample binary or
ROM is read by this build step. Sample lengths remain fixed. Looping samples
have an interpolation guard equal to the first loop sample; nonlooping guards
are zero. The compiler derives the guard and alignment from the edited source.

The song-start function at 08078A70 references the nine-player table at
0808B144 and 221-entry song table at 0808B1B0. Of these slots, 101 have tracks;
120 point to a zero-track dummy header. `songs.json` and
`voice_references.json` preserve the discovered references. They are inspection
metadata, not source builders for song sequencing. Bank views can overlap;
finding a valid tone does not prove every song plays it. PSG instruments are
not PCM samples and are not represented by WAV files.

The header and driver signature agree with the primary
[M4A structure definitions](https://raw.githubusercontent.com/pret/pokeemerald/master/include/gba/m4a_internal.h).
The original statistical extraction was incorrect: 148 of its 149 candidates
overlap named graphics/map/font members. The remaining FE2C20 span contains
ARM instructions and pointers and remains unclassified data, not audio.
The removed candidates and byte hashes are retained in
[the legacy audit](../reports/sound/legacy-audit.json).
