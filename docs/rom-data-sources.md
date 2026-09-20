# ROM data sources

Tracked assembly is reserved for machine code and low-level symbols that still
need matching C. Assets use editable source formats and placement manifests.

## Build path

`data/rom_data_sections.json` records 842 independently placed ROM sections:

- 298 graphics, palette, tilemap, font, and sprite-container sections
- 190 map sections
- 334 script sections
- 8 Japanese artwork sections replaced by the English build
- 3 initialized regions whose formats are still being decoded
- 2 declarative sections for the decoded high-ROM zero/pointer pattern
- 7 large uniform padding spans

`sound/sample_sections.json` records 150 PCM spans rebuilt from the WAV files
and metadata under `sound/samples/`. Some samples cross the old disassembly
chunk boundaries, so one WAV payload can supply more than one placed span.

During a build, `tools/rom_data_sections.py` checks that every compiled payload
fits its original range and writes temporary linker input under
`build/generated/`. These files are disposable and `make clean` removes them.
The checked-in source remains PNG, PAL, JSON, text, Marscript, WAV, or a small
unresolved binary rather than `.incbin` assembly.

The Japanese build links every base group. The English build omits the
`japanese_localized_assets` and `script_assets` groups, then links localized
artwork and script objects at the same addresses. This preserves the Japanese
byte match while allowing the English ROM to grow in its separate expansion
region.

## What still needs decoding

The two remaining `data/data_*.bin` files are UI and menu table families
suitable for typed C. The third binary is now accurately named
`data/agb_debug_monitor.bin`; it is a dormant mixed ARM/Thumb SDK monitor and
must be disassembled before its routines can become C. Its 3,952-byte zero
tail and the former `data_FFF000.bin` now build from explicit fill and
repeated-word records. Current evidence and exact ranges are documented in
[`raw-data-inventory.md`](raw-data-inventory.md).

Large `0xFF` regions represent erased cartridge capacity, so expanding them
into hundreds of thousands of C initializers would reduce readability. Short
zero alignment gaps are ordinary named C arrays in `src/rom_padding.c`.

Run `make compare` after changing placement data. A successful result proves
that the generated payloads, ordering, alignment, and final ROM all remain
byte-identical to the reference ROM.
