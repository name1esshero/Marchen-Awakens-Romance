# Decompilation and recovery notes

This document preserves the detailed technical findings, recovery history,
asset provenance, measurements, and implementation references that were
previously kept in the project README. The main README is now a practical
setup and tool guide.


A work-in-progress GBA decompilation with a byte-identical ROM rebuild.
The normal build uses only checked-in source assets. The original ROM is an
optional local verification input and is not supplied by the project.

```sh
make -j4
make compare
```

Expected SHA-1: `5ed178bfbdf459867d64e5b91a9d9c72654e4051` (16,777,216 bytes).
`make compare` checks every ROM byte when `baserom.gba` is present. `make`
prints linked ROM, EWRAM, and IWRAM usage after the build. The RAM report combines
linked sections with `ram_layout.json`, which accounts for the game's fixed
IWRAM objects and all seven EWRAM heap arenas. `tools/ram_snapshot.py` measures
live allocations and fragmentation from emulator EWRAM/IWRAM dumps; see
`docs/ram-layout.md`.
ROM usage counts occupied content rather than the padded file length. Runs of
at least 32 identical `00` or `FF` fill bytes, plus cartridge address space
beyond the image, are reported as free capacity.
`make test` runs the host-side unit suite, `make test-english` runs its English
subset, and `make ci` builds both `mar.gba` and `mar_english.gba` before running
the same tests used by GitHub Actions. ROM-dependent source round-trip tests are
skipped in public CI and run automatically on a local checkout with
`baserom.gba`.

**Toolchain.** arm-none-eabi binutils for assembling and linking, and **agbcc**
for C. agbcc is the period compiler preserved by the pret projects, and it is
required rather than preferred: modern GCC allocates registers differently for
identical source, so C only reproduces the original bytes when agbcc compiles
it. Build and install it once:

```sh
git clone https://github.com/pret/agbcc && cd agbcc
./build.sh && ./install.sh /path/to/this/project
```

That places the compiler and its headers/libraries under `tools/agbcc`, which
is a local installation excluded from Git. Project headers in `include/`
remain source files and should be committed. Sources are preprocessed with
`arm-none-eabi-cpp`, compiled by agbcc, then assembled, because agbcc is a cc1
only. Python tools use the standard library.

## Published documentation

[Wiki](https://github.com/name1esshero/Marchen-Awakens-Romance/wiki) ·
[Interactive galleries and reports](https://name1esshero.github.io/Marchen-Awakens-Romance/).
The wiki holds documentation; the separate `gh-pages` branch holds viewing
copies and historical audit snapshots. Local `reports/` and gallery HTML
are ignored outputs. Editable graphics, sound, translations, manifests, and
their generators remain tracked here.

On a fresh checkout, `make docs-fetch` restores missing published report and
gallery outputs without replacing existing files. `make galleries` regenerates
galleries from the current editable manifests without extracting assets.
`make site` stages a complete site in `build/site` and wiki pages in
`build/wiki`, checking HTML links and animation frame references. It does not
publish. See [publishing instructions](../tools/site/README.md).

## Map editor

![The map editor opening a field, changing pages, and playing a moving event object](media/map-editor.gif)

This GIF is built from screenshots of the real editor running in Chromium. It
shows a field opening, the Map/Collision/Connections/Events pages, and the Play
button previewing a decoded 32-frame `SprMove` command.

Run `make map-editor`, or `py tools/map_editor/server.py` on Windows.
The server automatically opens the editor in your default browser;
`--no-browser` disables this. The full URL is also printed in the terminal.
The basic workflow is:

1. Choose a map on the left. Use the mouse wheel over the map to zoom, drag the
   scrollbars to pan, or press **Fit whole map**.
2. Choose **Map** to paint visual tiles, **Collision** to inspect or paint raw
   tile attributes, **Events** to inspect objects and run decoded commands, or
   **Connections** to preview fields loaded by scripts.
3. In Events, choose a script and press **Play**. Literal sprite positions and
   movement targets animate with the extracted game frames. Unresolved runtime
   values are counted and shown instead of being guessed.
4. Press **Save sources**, then run `make` or `make english`. The editor writes
   reviewable JSON source overrides; it never edits the ROM directly.

The editor supports 49 field and special-scene maps, a visual tile selector,
palette banks, flips, layer visibility, raw attributes, draggable literal
events, undo/redo, and source saves. Plane 0 is shown above plane 1 by default,
matching the recovered BG0/BG1 setup. Numbered maps link their same-name,
spawn, character-event, and history-event scripts; inferred filename links are
labeled. The current catalog covers 113 field loads, 201 sprite resources,
1,690 sprite properties, and 78 sprite moves.

Dynamic register values, script branches, creating new events, and complete
warp semantics still require more engine decoding. See the
[plain editor guide](../tools/map_editor/README.md), the
[map editor roadmap](map-editor-roadmap.md), and the
[sprite rendering trace](sprite-rendering.md) for the verified details and
remaining work.
The verified [script source language](script-language.md) and
`scripts/source/example.json` compile JSON control flow and native calls into
standalone SPC files with rebuilt FUNC relocations.

## Browse the recovered assets

[Graphics folder guide](../graphics/README.md) · [Named background art](https://name1esshero.github.io/Marchen-Awakens-Romance/graphics/backgrounds/index.html).
The frame manifests retain 9,252 records while sharing 6,627 distinct PNG
sources. Consolidation removed 2,625 identical copies and 104 redundant
palette sidecars; named palettes remain authoritative.

- [Character animations](https://name1esshero.github.io/Marchen-Awakens-Romance/graphics/battle/characters/index.html): 2,498 editable frames.
- [Battle effects](https://name1esshero.github.io/Marchen-Awakens-Romance/graphics/battle/effects/index.html): 4,817 editable frames.
- [UI, item art, and panels](https://name1esshero.github.io/Marchen-Awakens-Romance/graphics/ui/index.html): 1,937 editable frames.
- [152 dialogue portraits](https://name1esshero.github.io/Marchen-Awakens-Romance/graphics/portraits/index.html): original resource names,
  all expressions, and links to their palette banks.
- [84 icon frames](https://name1esshero.github.io/Marchen-Awakens-Romance/graphics/icons/index.html): `ICON` and `ICONMINI`, 42 each.
- [1,734 named sprite previews](https://name1esshero.github.io/Marchen-Awakens-Romance/reports/sprites/index.html): first frame of each
  named group's first animation, assembled using its actual cells and palettes.
- [Graphics audit and colored tile previews](https://name1esshero.github.io/Marchen-Awakens-Romance/reports/graphics/index.html).
- [Font preview](../graphics/fonts/preview.png), [editable font sheet](../graphics/fonts/font.png),
  [Unicode-labelled glyph index](../graphics/fonts/glyphs.tsv).

Preview frame composition reproduces stored cell positions and ordinary flip
bits. Runtime affine transforms, animation playback, and priority behavior
have not been checked against an emulator.

## Editable build sources

| Content | Edit here | Notes |
|---|---|---|
| Portrait pixels | `graphics/portraits/F*.png` | 152 individual 64×64 indexed PNGs |
| Icon pixels | `graphics/icons/ICON_*.png`, `ICONMINI_*.png` | Individual indexed frames |
| Assembled character/effect frames | `graphics/battle/*/frames/*.png` | Indexed, transparent, original cell palettes |
| Assembled UI frames | `graphics/ui/frames/*.png` | Includes item art and portrait resources |
| Individual sprite cells | `graphics/battle/*/cells/*/*.png`, `graphics/ui/cells/*/*.png` | Complete pixels, including occluded layers |
| Animation and layout tables | Each sprite category’s `source/*.json` | Groups, animations, frames, cells, and container metadata |
| Sprite palettes | Each sprite category’s `palettes/*.pal` | 762 original 16-color BGR555 banks |
| Background tile pixels | Paths in `assets.json` marked with `archive_name` | 104 named KCG/TCG resources |
| Background palettes | `graphics/palettes/*.pal` | 105 named KCL/TCL members |
| Font pixels | `graphics/fonts/font.png` | 8×8 glyphs, 1bpp, 32 slots per row |
| Script strings | `text/nfp/*.SPC.txt` | All 334 named SPC members, compressed and uncompressed |
| Tilemap entries | `graphics/tilemaps/nfp/*.TSC.bin` | 90 named members, one 32x32 screenblock each |
| Map data | `maps/nfp/*.KMP.bin` | 193 named members; internal layout not yet decoded |
| Matching C | `src/*.c` | Addresses and occupied ranges in `src/decompiled.json` |

Keep PNG dimensions and palette indices intact. Sprite PNG palette colors are
viewing aids; edit the linked `.pal` file to change the actual ROM colors.
Portrait and icon edits are merged into SYSTEM.NCD; unchanged copies do not
mask changes to individual cell images. Incompatible edits to shared bytes are
rejected. Shared sprite tiles and palette banks can affect multiple images.

The animation galleries run locally without a server. Playback FPS is a preview
control; stored frame durations are displayed without claiming exact game timing.
Run `python3 tools/scenes.py extract` to regenerate the galleries; existing frame
PNGs with authored edits are preserved; unchanged views refresh from the cell sources. Frame edits must stay within existing cells and use their
palette indices. Edits that cannot be represented because of overlapping or
shared tiles are rejected; use the individual cell images for those changes.

Builds stage files under `build/` and do not overwrite source art. Every PNG
is converted and compressed with the matching VRAM-safe encoder. Timestamp or
display-palette changes cannot alter pixel indices. Compressed pixel edits
must fit the original reserved span. Intentional pixel, palette, or Japanese
text edits change the ROM; comments alone do not.

NCD containers are compiled from individual PNGs, palettes and readable JSON
tables; no original NCD or large tile-sheet image is a build input. The font
currently retains its original `.nft` header source. Files under
`reports/` and contact-sheet/preview PNGs are inspection artifacts, not editable
ROM inputs.

## What was recovered

The initialization code registers `MAR.NFP` at ROM `[0x1C0920, 0xF28410)`.
Its `NFP2.0` directory contains 830 members. Each directory record has a
12-byte filename and a 32-bit archive-relative offset. Directory counts and
table access are recovered from `0x0807AAD0`, `0x0807AB08`, `0x0807AC28`, and
`0x0807AC3C`; `tools/nfp.py` validates the directory and member boundaries.

The three `NCD` containers contain 32,265 sprite cells. Frame viewers assemble
cell centers into top-left bounds using the engine’s half-size offsets, fixing
detached parts on mixed-size sprites; original cell records remain unchanged. Their source-level build
is documented in [the graphics guide](../graphics/README.md). The old
`graphics/ncd` directory has been removed. Their group, animation,
frame, cell, palette, and tile tables are resolved by `0x0807B96C`. The palette
lookup at `0x0807BAF8` uses `cell.paletteIndex * 32`; the following graphics
lookup uses `cell.tileIndex * 32`. Every cell's geometry and palette index
validates, and the maximum referenced tile end exactly matches each NCD member
boundary. See [include/ncd.h](../include/ncd.h) and
[src/nonmatching/ncd.c](../src/nonmatching/ncd.c).

All 105 KCL/TCL palette members and all 762 NCD palette banks round-trip to
BGR555 without losing bits. Backgrounds may select multiple banks; the gallery
exposes bank variants instead of asserting one palette for an entire tile
sheet. Five 8bpp TCG previews place their palette at index 192 based on the
observed pixel-index range; this placement is marked as inferred in the manifest.

### The old raw graphics folder

`graphics/raw` was generated by a statistical detector, not a resource parser.
Its 300 PNGs included map bytes, animation tables, and tile-pool fragments,
including fragments starting halfway through a tile. Those images and seven
false LZ77 image detections have been removed with `reports/graphics/legacy`.
Their hashes, classifications and manifest records remain in
`reports/graphics/retired-scan.json`; they are not build inputs. Future heuristic
extractions report JSON candidates without creating image/source files.

The 104 named KCG/TCG resources use archive filenames. Of these, 91 now build
from 158 assembled image layers under `graphics/backgrounds`, using decoded
KMP dimensions, tile indices, flips and palette banks, or affine TSC byte indices. The remaining 13 retain
tile-sheet sources under `graphics/tilesets` pending other map profiles. See
[the mapped-image guide](../graphics/backgrounds/README.md). The previous
sprite/tilemap labels were shape guesses.
Run `python3 tools/audit_graphics_sources.py` to account for every active PNG
and detect missing, untracked or misfiled sources. Unknown map fields and
unclassified bytes outside named resources remain unresolved.

## Font and charmap

`FONT.NFT` begins at ROM `0x7BA990`. Its 96-byte header/palette area is followed
by exactly 63,872 bitmap bytes: 7,984 slots × 8 bytes. There are 950 deliberately
blank slots. The extracted font ends at `0x7CA370`, exactly the next archive
member boundary; unrelated data is not attached to the font image.

[char­map.txt](../charmap.txt) documents the engine reader, full glyph mapping,
private codes, and fallback behavior. [engine_charmap.tsv](../graphics/fonts/engine_charmap.tsv)
contains all 65,536 16-bit inputs. `tests/test_font.py` executes the original
THUMB mapping routine and verifies every result against the readable port.
The private codes `F056` and `F040` select Ä and heart glyphs. Text files retain
these as `<F0><56>` and `<F0><40>` so the ROM bytes are preserved.

Full-width Shift-JIS Latin characters select the clean Latin glyphs seen in
the preview. Single-byte ASCII uses a different index formula; the tools do
not silently substitute full-width input. The glyph mapping is complete;
the dialogue reader at 08011AF4 interprets Cxxxx as ink/shadow colors and
Txxxx as delay. Ordinary ASCII is skipped on that path. See the
[English layout notes](../text/translation/README.md).

## Text and translations

[Browse scripts with English comments and portrait references](https://name1esshero.github.io/Marchen-Awakens-Romance/reports/text/index.html).
The browser sorts records by CODE offset and links exact portrait resource IDs;
it does not reconstruct branches or infer which portrait is currently displayed.
Regenerate it after annotation changes with `python3 tools/text_gallery.py`.

`text/nfp` contains 8,640 conservatively detected, length-framed string records
from all 334 named scripts. This includes 1,021 records missed by the older
compressed-only/standard-Shift-JIS extraction. A `0x10` opcode, 16-bit length,
and exact single terminator establish each candidate's framing. This is not
claimed to be a complete bytecode disassembly; some strings are resource names
or other internal literals rather than dialogue.

Record addresses are CODE-local opcode offsets. Two spaces followed by `//`
introduce a comment; original trailing spaces before those two separators are
significant. English annotations use `// EN:`; unresolved translations retain
`// TODO:`. ASCII literals have separate `// LITERAL:` notes and are not counted
as translated dialogue. Names are transliterations, not asserted official
localization spellings.

All **5,562 translatable records in the current named-script extraction** now
have reviewed English mappings; **zero remain pending**. Two mappings explicitly
omit the Japanese object particle, which has no English equivalent. Another
3,073 records are ASCII literals and five contain formatting only, accounting
for all 8,640 extracted records across 334 scripts. This completes annotations
for this extraction, not proof that every ROM string has been found or that the
English runtime covers every screen. The detailed counts are generated in
[reports/text/audit.json](https://name1esshero.github.io/Marchen-Awakens-Romance/reports/text/audit.json). Reviewed exact-string
translations shared across scripts are maintained in `text/translation/english.json`.
Scene-specific wording is maintained in `text/translation/scripts.json`, keyed by
the original SPC filename; these entries override shared wording only in that
script. Keep sentence fragments here to avoid changing unrelated dialogue. Run
`python3 tools/translate_comments.py` to apply them without changing Japanese.
Legacy `text/scrp_*.txt` files are retained for reference; edit `text/nfp` for
script changes. Loose ROM text outside named sources uses `text/script_*`;
executable-region UI prompts recovered for the English runtime live in
`text/runtime_strings.txt`.

ÄRM names/descriptions and consumable/material text are fixed-layout data
tables rather than scripts. Their complete editable sources are
`text/arm_definitions.txt` (445 records) and `text/item_definitions.txt`
(13 records). `python3 tools/definition_text.py` verifies every populated name
and description against its exact record field. All 916 populated fields have
English mappings, including the ÄRM Select names and descriptions.

## Decompilation status

The provenance audit verifies **1,573 source-compiled C ranges (216,447 bytes,
1.2901% of the complete 16 MiB ROM) and 10 BIOS assembly wrappers**; each declared
range is linked from its expected object and matches the Japanese ROM. Recent
batches decode the save block, CRC-32 validation, asynchronous SRAM write and
load/verification paths, MusicPlayer2000 sound routines including the complete
four-channel PSG update loop, script-facing player transitions, instrument
banks, song tables, song headers, all 280 recovered song-track arrays, sprite
resource allocation, affine transforms, and sprite
interpolation work arrays. The map work also identifies the generated-map
connection attribute classes and their north/east/south/west mask bits.
Equivalent C candidates that made agbcc choose different instruction bytes
were rejected from the manifest. This is a verified function count, not a
percentage of all game code. Earlier batches include eight list helpers, the `SprSet` and `SprGet`
native adapters, and 13 item-table accessors. Item names and descriptions are
identified; other fields retain offsets until their gameplay meaning is
verified. List tests cover insertion and removal at every list position, and
item tests cover the 128-byte record layout, signed fields, and ID narrowing.

The next batch adds five input helpers (256 bytes), two packed-bit helpers
(72 bytes), and four RNG routines (68 bytes), all compiled from C. Input tests
cover all 1,024 hardware key combinations and pressed-bit consumption; RNG
and bitset tests check fixed sequences and byte boundaries. English dialogue
pagination now calls the named input API. See [input and random behavior](input-and-random.md).

Heap construction and three byte utilities now add six more matching functions
(232 bytes). The allocation search itself remains assembly. The 32-bit heap
test covers rounding, block metadata, default-heap publication, allocation
failure, and unsigned size overflow. See [heap and byte utilities](heap-and-byte-utils.md).

Task-manager initialization, destruction and counting, an alternate list insertion
helper, and archive initialization/shutdown add six matching C functions (276
bytes). The 32-bit lifecycle test checks node ownership, traversal while freeing,
list links, mount-table allocation and state clearing. See
[task and archive lifetime](task-and-archive-lifetime.md).

The consolidated `src/task_manager.c` contains task-manager lifecycle,
two task-creation helpers, the public creation wrapper, and the scheduler. The
helpers append to a priority queue or insert before an existing task. Tests
cover header and payload initialization, optional completion words, queue links,
allocation failure, removal before/after callbacks, and tasks appended during
the current pass.

Four archive mount helpers add 152 bytes of matching C in
`src/nfp_mount_helpers.c`: active-name lookup, uppercase-name assignment,
unmounting and active-slot counting. Unmount preserves the archive pointer,
size and name; it only clears the active flag.

Task completion, three archive convenience entry points, and adjacent list
reordering add another five matching functions (224 bytes). The archive APIs
provide name-based member counts, index-based opening, and archive/member-name
index lookup. Tests cover their distinct missing-archive and missing-member
results as well as task-state transitions and adjacent link reordering.

Eight sprite-engine state helpers add 264 bytes of matching C. They allocate
and address 8-byte OAM work entries, read and write three indexed boundaries,
and expose the signed state fields at offsets `0x610` and `0x612`. Unknown
field meanings retain offset-based names. See
[sprite rendering evidence](sprite-rendering.md).

Eight more sprite-engine helpers add 224 bytes of matching C. They manage a
16-byte resource record and its signed handle, expose the renderer's 16-bit
flag field at `0x20C`, and read or write its 16-by-4 byte counter table at
`0x1CC`. Behavior tests cover conditional resource release, handle truncation
and sign extension, individual flag changes, mask-valued flag tests, and
counter indexing.

Three resource-group lifetime helpers add another 152 bytes. A 16-entry binding
table at `0x14C` keeps signed-byte back-references into resource-owner records.
Its recovered routines release those references and maintain four saturating
reference counters per group, automatically releasing a binding when a counter
falls to zero.

Seven typed resource-table accessors add 396 bytes of matching C. They expose
the renderer's 32-byte resource descriptors and traverse indexed 16-, 8-, 8-,
20-, and 32-byte records without opaque pointer arithmetic at their call sites.
The neutral level names remain until the NCD fields using each table establish
their final animation or graphics roles.

Twelve renderer buffer and copy-callback helpers add 208 bytes of matching C.
Six get or set the three buffer pointers at state offsets +4, +8 and +12. Two
callback pairs at +`0x620` and +`0x624` accept custom transfer functions and
restore their own `CpuCopy` wrappers when passed null. This is the first 12 of
the current 100-function decompilation batch. Two higher-level transfer helpers
bring the batch to 14 functions: one copies an arbitrary count of 32-byte
records into buffer +8, while the other copies one 32-byte record into buffer
+12 through their independently configurable callbacks.

Eight more renderer-mask helpers bring the active batch to 22 functions. They
set, clear and test individual bits or replace the complete 32-bit masks at
state offsets +`0x10` and +`0x14`. Whole-mask setters preserve the original
binary behavior: zero clears the mask and any nonzero value fills it with ones.

Three leaf helpers and the NCD shallow-clone routine brought the active batch to
26 functions. The leaf helpers address 32-byte OAM work records, calculate
storage for `count + 1` records, and divide 65,536 by a sign-extended 16-bit
value. The clone copies all 52 bytes of an NCD runtime sprite and then sets its
two-bit copy mode to one; recovering it also corrected the byte-`0x27` layout.

The completed 100-function batch also contains NCD resource registration and
deep cloning. Registration resolves the container's six relative table offsets
and allocates one signed binding slot per palette. Deep clones allocate and copy
their own per-cell handle arrays, distinguishing them from shallow mode-one
clones.

Procedural-map state, native field/layer commands, and inventory-facing map
adapters now live together in `src/mapping.c`. The generator routines
identify the generator's private RNG and its state block at engine offset
`0x1304`, including six working pointers, three indexed tables, two signed
coordinate arrays, per-direction values, and four signed four-component
vectors. Names retain offsets where caller analysis has not established the
game-level meaning. See [map generation and script bytecode](map-and-script-runtime.md).

Sixty-four bytecode routines now live in `src/script_bytecode.c`. They decode
little-endian operands, advance the instruction cursor, operate the VM's stack,
resolve variables through the remaining operand resolver, and implement jump,
call, return, switch, assignment, arithmetic, and bitwise commands. This turns
the central script operations into editable C while preserving every original
instruction byte.

Seven more native-script routines now expose random range generation, RNG
seeding, integer parsing, string comparison and length, and right/substring
extraction. Thirty-six accessors in `src/game_state.c` describe the signed and
unsigned fields around main-state offsets `0x4240..0x42C4`. Another 28 routines
in `src/runtime_buffers.c` expose the secondary allocation's buffers, scalar
fields, and its 1,672-byte indexed record stride. Offset-based names remain
until their callers establish stable gameplay meanings.

**The build compiles C with agbcc**, the period compiler the pret projects
preserve. This is what lets ordinary C reproduce the original instruction
bytes: modern GCC allocates registers differently for identical source and
will not match. Installed under `tools/agbcc`, and the effect is immediate.
`NfpGetArchiveBase` matches instruction for instruction, literal pool
included, where modern GCC differed on two counts in every function:

| | agbcc / ROM | modern GCC |
|---|---|---|
| Leaf prologue | `push {lr}` | `push {r4, lr}`, saving a register it never uses |
| `index * 24` | `((i * 2) + i) * 8` with shifts | load 24, then `muls` |

The current manifest declares 1,547 linked ranges: 1,537 compiled-C ranges and
ten BIOS inline-assembly wrapper ranges. Compiled C owns 204,767 bytes, including
functions, typed tables, literal pools, and alignment, or 1.2205% of the complete
16 MiB ROM. This whole-ROM figure is the authoritative progress percentage: code,
data, assets, and padding all count in its denominator. The audit also reports a
narrower 29.0099% assembly-provider diagnostic to show conversion progress within
the remaining `asm/code` and compiled-C body, but that is not the headline score.
One range may contain multiple contiguous
functions and alignment bytes. The [build provenance audit](https://github.com/name1esshero/Marchen-Awakens-Romance/wiki/Build-verification)
checks their linked object providers and distinguishes them from preserved
assembly literals and original compressed data. A forced rebuild and deliberate
C/sprite/font source mutations verified that source changes reach the final ROM.

### A note on section padding

A function whose body is not a multiple of four bytes needs its alignment tail
declared in the same section. Left alone the assembler pads the tail with a
THUMB nop (`0xC046`) where the cartridge holds zeros. Contiguous functions
should share one section so the compiler's own `.align 2, 0` supplies the gap,
as `SetEntityRenderOverride` and `GetEntityRenderOverride` now do. Pinning
each function to its own section is an artifact of this project's layout, not
how the original was built.

### The filesystem

`src/nfp.c` and `include/nfp.h` decompile the accessors every asset load goes
through: `NfpGetArchiveBase`, `NfpGetDirectory`, `NfpGetData` and
`NfpGetEntryCount`, from `0x0807AAA0`, `0x0807AAD0`, `0x0807AAE0` and
`0x0807AB08`. The header fields they read at `+0x34`, `+0x38` and `+0x3C` are
exactly the count, directory and data offsets the extraction tools rely on, so
the C and the tooling agree on one documented structure.

Two details worth keeping: the IWRAM global at `0x03006114` points at
filesystem state whose first field is the mount array, so there are two
indirections before the index; and mounts are 24-byte records indexed by
handle, so the design supports several archives open at once even though this
cartridge ships one.

The glyph-index function, font glyph addressing, and engine character reading
now compile as matching C. Readable nonmatching C still documents NCD table
lookup outside the matching build. `src/nonmatching/archive.c` holds partial readings of the
background loader: `0x08027EAE` is inside `ArchiveTaskStep`, and its corrected
structures are in `include/archive.h`.

The analyzer's 19,634 candidate entries are not a reliable function count. The
refreshed pass seeds its graph with matching C and decoded native-command
handlers, while several historical labels are still inside routines. For
example `0x080032C4`, `0x080032CC`, `0x080032DC`, and
`0x080032F2` belong to the routine beginning at `0x080032B8`. Check control flow
before choosing a decompilation unit.

To replace assembly with C, add the source and alias, record the original ROM
address and occupied size in `src/decompiled.json`, regenerate the code split with `tools/split.py`'s `write_code` API, and run
`make compare`. Full extraction can reset edited asset sources; do not use it
as a routine C-only regeneration step.

## Filesystem coverage

Every NFP member now builds from a named source. `tools/named_maps.py` covers
the last two types that were still reaching the ROM as anonymous chunks, the
90 `TSC` tilemaps and 193 `KMP` maps, with extract, build and verify modes.

| Source of the ROM image | Bytes |
|---|---|
| Named assets | 14.43 MB |
| Remaining typed-but-undecoded `data/` chunks | 23,956 bytes |

A `TSC` member is not always a 2048-byte screenblock. The five decoded 8bpp resources use 256/1024-byte affine maps with one byte per tile. Regular background maps use 16-bit entries,
with the tile index in bits 0-9, horizontal and vertical flip at bits 10 and
11, and the palette bank in bits 12-15.

The remaining raw payloads are catalogued in
[`docs/raw-data-inventory.md`](raw-data-inventory.md). Renderer lookup
tables at `0x08F28410` and the consumable ID-zero sentinel now compile from C.
The bytecode VM's complete 256-entry opcode dispatch and its 19 named built-in
functions also compile as symbolic C tables.
The bundled libc's newlib `_reent` initialization and `_impure_ptr` now use their
real source structures as well. Long erased-flash spans use linker fills, leaving
four initialized raw regions totalling 23,956 bytes. These include UI/menu tables
and two unusual high-ROM tables that still need their consumers identified.

## Checks and regeneration

```sh
python3 -m unittest discover -s tests -v
make graphics-audit
python3 tools/named_scripts.py audit
make -j4 compare
make clean
make -j4 compare
```

`make extract` re-extracts original assets from `baserom.gba` and regenerates the
split. Extraction can reset edited graphics; ordinary builds do not. Named
script extraction preserves existing records/comments and adds missing records.
The generated `asm/data` directory is grouped by source type: graphics, maps,
scripts, still-undecoded raw data, and explicit padding. Each payload retains
its address-named linker section, so this organization does not alter ROM
placement. PCM spans interleaved with the original code region live together in
`asm/sound_samples.s`. Japanese artwork replaced by `make english` is isolated
in `asm/data/japanese_localized_assets.s`, allowing the English linker to swap
the complete localized set as one object. Uniform zero and `0xFF` regions use
assembly fill directives rather than meaningless `.bin` files.

### Recovery snapshots

`make snapshot` creates a verified archive under `backups/clean-<UTC time>/`.
It includes editable graphics, palettes, animation tables, translations, C,
assembly, raw data, analysis, tools, the bundled agbcc compiler and the original
ROM. It excludes earlier backups, build output, caches and local agent/Git
configuration. Every archived file is checked against its SHA-256; file modes
and symbolic links are preserved. Errors stop snapshot creation.

Each backup includes `SHA256SUMS`, a per-file `manifest.json`, and `RESTORE.txt`.
Restore into an empty directory, then run `make -j4 compare` there. System
Python dependencies and the ARM binutils/preprocessor still need to be installed.
Snapshots do not automatically delete earlier recovery points.

The obsolete `assets_rle/` scan output is no longer a build input. See
[the graphics notes](../graphics/README.md#retired-rle-scan-output) for the retained
unclassified region and the limits of the former RLE identification.

## Audio and translation audit

[115 editable driver-referenced WAV samples](../sound/README.md) replace the old
statistical sound guesses. Audio now rebuilds from WAV and header JSON, including
computed loop interpolation guards. The driver-side MusicPlayer2000 sequencer,
mixing helpers, fades, and PSG channel updater are recovered in C. Editable song
sequence authoring and several game-side task constructors remain unfinished.
The decoded structures, execution path, and remaining boundary are documented
in [docs/sound-engine.md](sound-engine.md).

[Translation and English layout notes](../text/translation/README.md) explain the
actual dialogue printer, its double-byte English font mapping, and strict row
limits. The optional `make english` build hooks the recovered dialogue constructor
and the direct item name and description accessors used by menus. The ARM deck's
fixed-width category, element, equipment, and stat labels are decoded in
`src/menu_text.c`; its English labels stay within the original pointer-addressed
slots. Table descriptions are also mapped after the runtime `T0000 C0F04`
printer prefix; longer translations wrap into A-button pages. Emulator validation
and complete script/text discovery are still unfinished. Run `python3 tools/audit_setup.py`
for reproducible archive ownership and audio source checks.

### Generated compression files and cleanup

`make english` builds `mar_english.gba`; `make compare` verifies `mar.gba`.

English UI artwork uses sibling `*_en.png` sources in `graphics/ui/frames`
or `graphics/ui/cells`. Only `make english` selects them, compiling a separate
SYSTEM container under `build/english/graphics`; Japanese PNGs and palettes
remain the matching sources. All 206 user-reported UI frames now have English
artwork, including the material panel and credits. Thirteen credit-name
romanizations are provisional and explicitly flagged in the
[credit mappings](../graphics/ui/credits_translation.json); their Japanese originals
are retained there for correction. This count covers the reported set, not every
text-bearing graphic in the ROM.
Preserve palette indices and dimensions when editing ordinary variants. Staff
credits use explicit [English OAM layouts](../graphics/ui/source/english/credits_layouts.json)
so full Latin names fit without Japanese surname-tile sharing or gaps. These
layouts reuse the original cell records and reserved tile pool; they affect only
the English build and still need an in-game credits review.
The New/Continue/run options preserve their shared foreground words and
background layers.
[UI translation inventory](../graphics/ui/english.json) tracks all 206 user-reported frames and name-reading review work.
[Artwork editing notes](../graphics/ui/english.md) explain transparency and review.
The English link audit independently rebuilds both containers and verifies the
linked English tile bytes and credit layouts, while requiring Japanese sources
to match the ROM. Credit metadata changes are limited to OAM positions/shapes,
tile pointers, and the derived cell-tile reference count; palettes stay unchanged.

`make clean` removes both ROM outputs and `build/`, including generated `.lz`,
`.4bpp`, `.8bpp`, objects and localization tables.

All 344 loose source `.lz` files have been removed: 240 duplicate script copies
and 104 graphics templates. Named SPC sources under `scripts/nfp` still own the
script bytecode and compressed framing; edit their text through `text/nfp`.
Graphics compression is fully generated: the VRAM-safe encoder reproduces all
104 original streams from PNG/layout sources, including its nearest-match tie
policy. No ROM, original LZ file, or compression recipe supplies output bytes.
Edited graphics must fit their original compressed allocation; shorter streams
receive zero fill within that allocation. The default unedited sources match
both the original streams and complete ROM byte for byte.

BA06 background correction: `BA06.TCG` contains the two `BA06_00/01` planes. The three flame planes `BA06_BG0/1/2` use `BA06_BG.TCG` and its own palette; their editable images are now `graphics/backgrounds/BA06_BG.TCG.png`, `.layer1.png`, and `.layer2.png`. Both resources retain identical compressed bytes. Numeric-only TSC suffix matching prevents the former cross-resource pairing.

English battle-effect artwork: frames 0386–0389 translate 封印中 as “Sealed”; frames 0402–0406 translate パラメータ as “Stats” while preserving the expanding banner. Editable `_en.png` overrides are linked only by `make english` through a separate EFFECT archive object. `python3 tools/effect_text.py` explicitly regenerates these labels from the extracted Latin font.

ROM-tail audit: `python3 tools/audit_rom_tails.py` records section endings and scans the complete post-NFP region in `reports/rom/tails.json`. Valid LZ token streams are candidates, not proven assets; most current hits belong to SPC scripts. The final raw section contains repeated words, and the previously flagged ARM-code block remains unclassified. MWA now has two mapped textbox-border images using the KMP-declared palette bank 15; its compressed bytes are unchanged.

ROM audit (2026-09-12): `python3 tools/audit_rom_coverage.py` checks linked coverage and Japanese ROM equality. The current map covers all 16 MiB with 1,239 nonoverlapping sections and no gaps. Fresh graphics checks reproduce all 104 compressed inputs and account for 19,394 PNGs, including English overrides and authoring backgrounds. These are provenance checks, not proof of complete semantic decoding; 11 named graphics layouts remain unresolved. See `reports/audit/README.md` for current results and historical limitations.

MAP07_A now has two editable 2704×1960 cave-map layers. Its 338×245 tile dimensions exceed the old extractor-only limit, but all tile references and KMP plane bounds are valid. Raw tiles, map planes, and compressed output round-trip exactly. Ten named layouts remain unresolved; the renderer adds viewport tile/palette offsets before writing screen entries, so out-of-resource tile references must not simply be replaced with blank tiles.

Normal field maps use one KCG/KCL tileset/palette pair shared by two KMP planes. `KmpLoadField` loads the graphics once and reuses them for the second plane; these are not separate primary and secondary tilesets. Lower-level scene loaders can choose VRAM destinations and tile/palette offsets. See `tools/map_editor/README.md` for the traced parameters and remaining special-scene questions.

Special-scene map tracing: the PW screen loads PW_BG01 at VRAM 06000000 and PW_BOX at 06004000, using separate viewports. The box uses palette offset 1 and scroll (-60, -20). Verified call parameters are recorded in `maps/runtime_scenes.json` and exposed by the map editor API. Out-of-resource tile references remain flagged until their full VRAM context is decoded.

Sprite-authoring foundation: SprInit at 08011ECC is now readable C with an exact 40-byte agbcc match. Its native argument forwarding is host-tested, and the full Japanese ROM still matches. Sprite/event insertion and new-map registration remain unfinished; current editing is limited to verified existing data and literal arguments.

The SprInit creation worker (08010B6C, 160 bytes) is now matching C as well. Its asynchronous preparation/wait/completion behavior is host-tested. The preceding preparation task and full event/control-flow authoring remain undecoded; new sprite insertion is not yet enabled.

Both deferred sprite-reset workers are now matching C (228 bytes combined). Tests verify busy-slot waits, auxiliary teardown, record clearing and the single/all-slot inactive-record difference. SprInit preparation clears coordinates, so new placement must happen after that reset; sprite/event insertion and map registration are still unfinished.

English startup/shop backgrounds: `make english` consumes
`graphics/backgrounds/{T_C01,T_TTL06,SM_BG12}.KCG_en.png`. The English-only
object rebuilds both compressed tiles and their KMP map entries, so translated
letters can use independent tiles. Japanese PNGs, palettes, and map sources
remain the inputs to the matching build. Generated artwork references live in
`graphics/backgrounds/source/english/`; edit the indexed `_en.png` files.
The builder rejects palette-bank crossings, images larger than one 16 KiB
character block, and compressed streams exceeding the existing ROM allocation.
Removing an override restores that background's Japanese tiles and map.

English PNGs are authored sources. `make clean`, `make tidy`, and
`graphics-clean` remove build outputs, not these files. Graphics cleanup refuses
to delete `_en.png` files or Japanese PNGs with English companions; frame
consolidation retains their paths so overrides remain connected. The explicit
UI authoring commands `ui_text.py --write --replace` and
`ui_layered_text.py --refresh-frames` can intentionally regenerate English art.

Object resource-group family at 0x08028118: `ObjectSetResourceGroupByName`, `ObjectSetResourceGroup`, and `ObjectCopyFieldsFromTemplate` are now matching C in src/object.c, extending the struct in include/object.h with four newly-identified fields (unk_04, unk_08, unk_0C, unk_10, unk_18). `ObjectSetResourceGroup` conditionally updates whichever of resource/group/offset a caller passes something other than -1 for, then always re-derives a cached element count (unk_18) from `SpriteResourceGetLevel1`. `ObjectCopyFieldsFromTemplate` bulk-overwrites the 64-byte object from a caller's template but preserves the manager-owned record pointer and stashes the template pointer itself at +0x04.

Two neighboring functions in that same family, `ObjectFreeNcdResources` (0x080281E0) and `ObjectFreeAuxiliaryResources` (0x0802824C), are understood but not yet byte-matching; they are recorded in src/nonmatching/object_free.c rather than forced. Both free an active object's heap record after releasing whatever it holds (via `NcdRuntimeSpriteReleaseAllocation` or `SpriteAuxiliaryReset` respectively, over `unk_18` entries spaced 72 bytes apart). What blocks the match is a single heap-handle load: the ROM computes it as two literal-pool words (0x03000000 and 0x00003FB4) added at runtime, while agbcc folds every C shape tried (a flat cast, split pointer arithmetic, a struct-with-filler member access, an extern-symbol-plus-offset) into one combined literal instead. Each function compiles exactly 12 bytes short of its ROM slot as a result.

libgcc runtime cluster around 0x08080BFC: identified as `__divsi3` (signed division, 148 bytes) followed immediately by the default `__div0` handler (a 2-byte `mov pc, lr` stub) at 0x08080C90, both verified byte-for-byte against the real objects in tools/agbcc/lib/libgcc.a's `_divsi3.o` and `_dvmd_tls.o` (ignoring only the `bl` operand to `__div0`, which is relocation-dependent and resolves correctly). `__modsi3` (0x08080C94), `__udivsi3` (0x08080DD4), and `__umodsi3` (0x08080E4C) were already aliased in asm/iwram_symbols.s but still referenced by their raw `sub_` addresses at every call site in src/resource_native.c, src/script_bytecode.c, src/script_native.c, src/script_resource_table.c, and across asm/code/*.s; both sets of call sites (division helpers and __div0, a new alias) now use the readable names. These are genuine assembly library routines, not decompiled C — no C source was written for them, consistent with them being hand-written libgcc code in the original toolchain too.

sub_08003AE4/sub_08003E94 (0x08003AE4-0x08004230, ~1,600 bytes combined): identified but not yet decompiled to C. This is a task-based interpolation system built on CreateTask/FinishTask (0x0807A77C/0x0807A7AC): sub_08003AE4 creates the task, storing per-axis start/end coordinate pointers, a frame count, and a mode (0-4) selected via a 5-way jump table; sub_08003E94 is the resulting task's per-frame step callback, decrementing the frame counter and, on each call, writing one interpolated coordinate through __divsi3 and (for modes 1-4) a squared/curve lookup table, matching one of five easing curves. This looks like the engine behind smooth multi-frame sprite movement (the map editor's scripted-event movement preview is a plausible caller). Not attempted as matching C yet: the five interpolation-mode bodies are large, near-duplicated, and dispatch through a jump table, which is exactly the class of construct (switch-generated jump tables) most likely to resist agbcc byte-matching without a dedicated session; recorded here so the next pass does not have to re-derive this from scratch.

Register-forcing cleanup: audited every `register T x asm("rN")`/`TARGET_REGISTER` use across the matching build (53 functions outside the legitimate BIOS SWI wrappers in src/bios_calls.c) to check which were genuinely load-bearing versus leftover decompiling scaffolding. Removed them (verified with a full `make compare`, not just a section-size check, since several cases matched in size but differed by a handful of bytes in operand order) from: all of src/battle_task_create.c (7 functions, including the DEFINE_SEQUENTIAL_MODE_TASK macro shared by 5 of them), src/encounter_task.c's CreateEncounterSpriteTask (whose header comment claiming the hints were required was stale and has been removed), all of src/save.c except CreateSaveWriteTask, src/font.c's GetFontGlyph, src/sound_m4a.c's SoundTrackReadWavePointer, and src/script_effect_native.c's ScriptNativeFieldEffectStart. Confirmed still genuinely necessary (reverted after testing, left alone): src/font.c's FontCharacterToGlyph, src/save.c's CreateSaveWriteTask, src/sound_fade_create.c's CreateSoundFadeTask, src/sound_m4a.c's SoundPlayerResume/FadeOut/FadeOutTemporary/FadeIn, src/resource_native.c's ScriptNativeQueryModeResource/SetModeResource, src/nfp.c's NfpFindEntryIndex/NfpGetEntrySizeByName, and src/ncd_sprite.c's NcdRuntimeSpriteReleaseAllocation. Also standardized src/nfp.c from raw `__attribute__((section(...)))` to the shared `AT()` macro, matching every other file. Remaining unaudited: the sprite_affine_matrix.c/sprite_transform.c/sprite_affine_slots.c cluster (9-17 register hints per function), sprite_tile_allocator.c, sprite_interpolation.c, script_resource_table.c, script_resources.c, ncd_sprite.c's other functions, and resource_native.c's ScriptNativeSetFriendArms.
