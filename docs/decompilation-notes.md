# Decompilation and recovery notes

This document preserves the detailed technical findings, recovery history,
asset provenance, measurements, and implementation references that were
previously kept in the project README. The main README is now a practical
setup and tool guide.

Some historical entries describe forced-register experiments that predate the
current PRET standard. They are retained as investigation records, not as
approved techniques. Any matching C that still depends on those constructs is
listed by `make pret-audit` and must eventually be rewritten as natural C or
returned to assembly with its readable candidate kept under `src/nonmatching/`.


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

`ObjectFreeNcdResources` (0x080281E0, 108 bytes) and
`ObjectFreeAuxiliaryResources` (0x0802824C, 104 bytes) now match in
`src/object.c`. Both release the active object's records before freeing and
clearing the array. The multiple-record flag is 0x0200, the stride is 72 bytes,
and the NCD sprite lies eight bytes into each record. The loop treats the
record count as signed: zero and negative counts skip per-record cleanup but
still free the array.

The earlier reference C used an unsigned comparison and folded the heap root
into one literal. Separate linker symbols for the IWRAM base and the object
heap root offset preserve the original two-load address calculation. Both
agbcc snapshots produced matching code after these corrections; the normal
compiler remains in use. The obsolete reference file and 212 bytes of
assembly bodies were removed. Host tests cover inactive objects, null records,
single records, multiple records, and zero/negative counts.

libgcc runtime cluster around 0x08080BFC: identified as `__divsi3` (signed division, 148 bytes) followed immediately by the default `__div0` handler (a 2-byte `mov pc, lr` stub) at 0x08080C90, both verified byte-for-byte against the real objects in tools/agbcc/lib/libgcc.a's `_divsi3.o` and `_dvmd_tls.o` (ignoring only the `bl` operand to `__div0`, which is relocation-dependent and resolves correctly). `__modsi3` (0x08080C94), `__udivsi3` (0x08080DD4), and `__umodsi3` (0x08080E4C) were already aliased in asm/iwram_symbols.s but still referenced by their raw `sub_` addresses at every call site in src/resource_native.c, src/script_bytecode.c, src/script_native.c, src/script_resource_table.c, and across asm/code/*.s; both sets of call sites (division helpers and __div0, a new alias) now use the readable names. These are genuine assembly library routines, not decompiled C — no C source was written for them, consistent with them being hand-written libgcc code in the original toolchain too.

sub_08003AE4/sub_08003E94 (0x08003AE4-0x08004230, ~1,600 bytes combined): identified but not yet decompiled to C. This is a task-based interpolation system built on CreateTask/FinishTask (0x0807A77C/0x0807A7AC): sub_08003AE4 creates the task, storing per-axis start/end coordinate pointers, a frame count, and a mode (0-4) selected via a 5-way jump table; sub_08003E94 is the resulting task's per-frame step callback, decrementing the frame counter and, on each call, writing one interpolated coordinate through __divsi3 and (for modes 1-4) a squared/curve lookup table, matching one of five easing curves. This looks like the engine behind smooth multi-frame sprite movement (the map editor's scripted-event movement preview is a plausible caller). Not attempted as matching C yet: the five interpolation-mode bodies are large, near-duplicated, and dispatch through a jump table, which is exactly the class of construct (switch-generated jump tables) most likely to resist agbcc byte-matching without a dedicated session; recorded here so the next pass does not have to re-derive this from scratch.

[PARTLY SUPERSEDED -- the removals stand, but the "confirmed still genuinely necessary" lists do not; see the REGISTER-HINT CORRECTION at the end of this file.] Register-forcing cleanup: audited every `register T x asm("rN")`/`TARGET_REGISTER` use across the matching build (53 functions outside the legitimate BIOS SWI wrappers in src/bios_calls.c) to check which were genuinely load-bearing versus leftover decompiling scaffolding. Removed them (verified with a full `make compare`, not just a section-size check, since several cases matched in size but differed by a handful of bytes in operand order) from: all of src/battle_task_create.c (7 functions, including the DEFINE_SEQUENTIAL_MODE_TASK macro shared by 5 of them), src/encounter_task.c's CreateEncounterSpriteTask (whose header comment claiming the hints were required was stale and has been removed), all of src/save.c except CreateSaveWriteTask, src/font.c's GetFontGlyph, src/sound_m4a.c's SoundTrackReadWavePointer, and src/script_effect_native.c's ScriptNativeFieldEffectStart. Confirmed still genuinely necessary (reverted after testing, left alone): src/font.c's FontCharacterToGlyph, src/save.c's CreateSaveWriteTask, src/sound_fade_create.c's CreateSoundFadeTask, src/sound_m4a.c's SoundPlayerResume/FadeOut/FadeOutTemporary/FadeIn, src/resource_native.c's ScriptNativeQueryModeResource/SetModeResource, src/nfp.c's NfpFindEntryIndex/NfpGetEntrySizeByName, and src/ncd_sprite.c's NcdRuntimeSpriteReleaseAllocation. Also standardized src/nfp.c from raw `__attribute__((section(...)))` to the shared `AT()` macro, matching every other file. Remaining unaudited: the sprite_affine_matrix.c/sprite_transform.c/sprite_affine_slots.c cluster (9-17 register hints per function), sprite_tile_allocator.c, sprite_interpolation.c, script_resource_table.c, script_resources.c, ncd_sprite.c's other functions, and resource_native.c's ScriptNativeSetFriendArms.

[PARTLY SUPERSEDED -- the removals stand, but the "confirmed still genuinely necessary" lists do not; see the REGISTER-HINT CORRECTION at the end of this file.] Register-forcing cleanup, round two: continued the audit into script_resources.c, ncd_sprite.c, script_resource_table.c, and sprite_tile_allocator.c. Cleaned (hints removed, byte-exact verified): ScriptResourceResetArray, NcdResetResource, ScriptResourceSet (7 hints, the largest single clean removal so far), SpriteTileAllocatorInit, and SpriteTileAllocatorFreeTotal. Confirmed still genuinely necessary: NcdQueueSprite, SpriteTileAllocatorRelease, SpriteInterpolationInit, and resource_native.c's ScriptNativeSetFriendArms (all tried, all produced real content differences after removal, all reverted). Also standardized ncd_sprite.c from raw `__attribute__((section(...)))` to `AT()`. Combined with the first round, 17 of the originally-flagged 63 register-forced matching functions no longer need the technique. Not attempted this round, and expected to be the hardest remaining cases based on how every dense/loop-heavy candidate has gone so far: sprite_affine_slots.c (both functions combine register forcing with nested nested nested loops across nested register-forced blocks), sprite_transform.c's three SpriteVectorRotate* functions and SpriteProjectPoint/SpritePackAffinePosition/SpriteBuildAffineMatrix (all stack register hints with explicit `asm("" : "+r"(...))` compiler fences, a stronger signal of genuine necessity than a bare register hint alone), and sprite_affine_matrix.c's three 17-hint Write functions.

game_tables.c pret-standards cleanup: eliminated every raw hex pointer per docs/PRET_STANDARDS.md. Named constants replace magic numbers (FRIEND_ARM_NO_OWNERSHIP_BIT, BATTLE_PRESET_SIZE, bit-shift forms for gResourceSlotMasks, ARRAY_COUNT-style *_COUNT defines for every table). gScriptNativeCommands (128 entries) now uses designated initializers with named string constants (gScriptNativeName_*) and, for 124 of 128 handlers, their real already-decompiled C names (discovered that the stored handler addresses have the THUMB bit set, i.e. real_addr = stored-1, which is what let the cross-reference against decompiled.json succeed) -- the remaining 4 (BgSet, PmbDeckMake, DeckMake, ShuffleDeckCopy) and all of gBattleActionHandlers/gEngineStartupHandlers (267 combined unique addresses, none yet decompiled) use sub_ADDR placeholders. Rather than editing the fragile misdecoded-as-code asm region those names' bytes live in (attempted once, reverted: literal-pool cross-references from unrelated code elsewhere turned out to point into the exact byte ranges being renamed, so deleting them broke distant "ldr rX, =label" loads), sub_ADDR symbols are declared as plain `.set` absolute aliases in the new asm/game_table_handlers.s, the same technique asm/iwram_symbols.s already uses -- zero risk to existing disassembly since no bytes or labels are touched, only a new symbol table entry added. gTwoDigitResourceNames/gGeneratedEffectNames strings are named the same way, via #define aliases over their existing fixed addresses (gTwoDigitName00..47, gEffectNameEF_GEN01..13), rather than AT()-pinning new const char[] copies, because the two tables' bytes turned out to be interleaved with unrelated strings in the same ASCII pool and other code's literal pool loads reference addresses inside that pool too. Verified with make compare after every step; two earlier attempts (deleting the misdecoded asm region outright, and an arithmetic slip that put the array's own AT() at the wrong address) were caught by full byte comparison and reverted before landing the final version.

sound_tables.c pret-standards cleanup: applied the same treatment as game_tables.c to the sound engine's data tables. gSongTable (221 entries) now references gSongHeader_000..gSongHeader_220 (already-named structs in src/song_headers.c) via designated initializers instead of raw AT()-pinned addresses. gSoundPlayerTable (9 entries) references &gSoundPlayer0..8 and gSoundPlayer0Tracks..8Tracks, the latter newly aliased in asm/iwram_symbols.s (each player's IWRAM track-pool base, .set against the existing 0x0300xxxx addresses -- zero risk, same absolute-symbol technique as every other asm alias this session). gSoundExtendedCommandTable (12 entries) and all 443 combined gVoiceGroupMain/Secondary/Effects (SoundToneData) entries had their handler/wave fields discovered to store THUMB-bit-set addresses; subtracting 1 and cross-referencing decompiled.json found the extended-command handlers were all 11/11 already-decompiled real functions (CallRuntimeHandler, SoundTrackReadWavePointer, SoundTrackReadToneType/Attack/Decay/Sustain/Release/PseudoEchoVolume/Length/ToneLength/PanSweep), and 112 of the 118 distinct `wave` addresses across the voice groups matched real extracted PCM samples in sound/samples/manifest.json, now named gWave_ADDR via new .global labels inserted into the pre-existing asm/sound_samples.s .incbin blocks (which had no symbols at all before this). The remaining 6 unmatched wave addresses are honestly named gCgbWaveform_ADDR placeholders (added to asm/game_table_handlers.s) rather than guessed. SoundToneData.wave is polymorphic -- for PSG entries (type 1) it holds a small integer (0-3), not an address -- so values were classified by magnitude (>= 0x08000000) before any renaming was attempted, and small values were left as plain integer literals. Verified with make compare after every table (song table, player table, extended command table, then each voice group individually per the requested 20-30-entries-at-a-time cadence).

THUMB-bit rule made general and tooled: docs/PRET_STANDARDS.md section 5 now documents four required safeguards before applying the rule (verify the real address falls in a code region with a real name, always cast to u32 before adding the bit rather than relying on raw `void *` arithmetic, watch for double-indirection pointer-to-pointer tables, and consult a new audit tool) rather than treating it as a one-off game_tables.c discovery. Wrote tools/audit_thumb_ptrs.py, which scans every src/*.c for raw 0x08XXXXXX literals, checks address-1 against decompiled.json, and reports per-file rename candidates. Running it across the whole matching source tree (independent of the game_tables.c/sound_tables.c work, which had already been done by manual inspection) found one more real hit outside either file: src/task_constructors.c passed three raw THUMB-bit-set callback addresses directly to CreateTask that were already-decompiled functions elsewhere (VramFillTask in script_tasks.c, ScriptSpriteResetTask/ScriptSpriteResetAllTask in script_sprite.c) -- these were renamed to `(void *)((u32)Name + 1)` with matching extern declarations added. Verified with make compare.

Register-forcing cleanup, register-order hypothesis retested: tried the "reverse allocation order" idea from docs/PRET_STANDARDS.md section 5a against sprite_affine_matrix.c's SpriteAffineWriteNormal (the three-function Write cluster's representative case) -- stripped every TARGET_REGISTER hint and MATCH_RW/MATCH_OUT/MATCH_IN3 fence, kept the same declaration order, wrote plain natural C. Result: `arm-none-eabi-ld: ROM image is not 16 MB: a fragment changed size` -- a real content mismatch, not just a declaration-order fixable case. Reverted and confirmed still byte-exact. This closes out the cluster as genuinely necessary (not merely assumed by pattern-matching as the earlier round's notes said) -- all three Write functions and the sprite_affine_slots.c/sprite_transform.c fence-backed functions can be considered audited-and-confirmed rather than audited-by-inference.

pret-standards cleanup, remaining audit scope closed out: applied the same raw-hex-elimination treatment to every file flagged in the earlier full-codebase audit. script_native.c named its two shared literal-pool strings (sText_Empty, sText_PercentD) via #define alias, since (as with game_tables.c) the addresses sit in a misdecoded-as-code ASCII region shared with unrelated code. item.c's two consumable-name-table bases were already correctly named; standardized its raw `__attribute__((section(...)))` uses to the shared AT() macro instead, matching every other file. kmp_loader.c was already correctly named (no change needed). map_field.c, map_native.c, and scene_native.c each named their one raw ".KMP"/".NCD" extension-string address (sText_KmpExtension x2, sText_NcdExtension), verified against the actual ROM bytes at each address before naming rather than guessed. script_resources.c's two builtin-function-table registrations were discovered, by inspecting the raw bytes, to be exact type-punned reuses of two already-known tables: 0x081ACB7C is a 24-entry {name, handler} table of core VM/engine commands (dummy/Pad/Wait/GetVar family/CrtFade family/Bgm-Se playback, verified entry-by-entry against ROM bytes) not yet reconstructed as a matching C array so aliased as gScriptEngineFunctions rather than retyped, and 0x081AFEA4 is literally gScriptNativeCommands from game_tables.c reinterpreted through ScriptResourceEntry's compatible layout, re-declared extern under its real name; its third raw address (0x081AC698, a 12-byte zero-filled fallback record) is now sScriptResourceDefaultValue. sound_m4a.c's one remaining raw address followed the established THUMB-bit pattern: subtracting 1 landed on an existing sub_080783DC label already present in asm/code/code_0780C0.s, confirming it as a genuine not-yet-decompiled function rather than data, so it is now `(void *)((u32)sub_080783DC + 1)` with a matching extern declaration. Also fixed the sound_idle_wait.c/sound_tasks.c bonus-scope item flagged during the sound_tables.c work: all nine raw `(struct SoundPlayer *)0x03005F30`/`0x03005FB0` casts across both files now use `&gSoundPlayer0`/`&gSoundPlayer1`. tools/audit_thumb_ptrs.py now reports 0 remaining candidates across all of src/*.c. Verified with make compare after every file; full 140-test suite and audit_provenance.py (1588 compiled C / 1598 total mapped ranges, unchanged) both re-run clean at the end of this pass. Also retested (not just re-affirmed by pattern) the hardest remaining register-forcing cluster -- see the "register-order hypothesis retested" entry above -- confirming sprite_affine_matrix.c's fence-backed Write functions are genuinely necessary, not declaration-order artifacts. [SUPERSEDED: this held only for removing a function's hints all at once. Tested one hint at a time, sprite_affine_matrix.c gave up 24 of them byte-exact; see the REGISTER-HINT CORRECTION at the end of this file.]

ScriptNativeBackgroundSet decompiled to matching C (0x08012264, 148 bytes, the "BgSet" native command in gScriptNativeCommands): previously existed only as a read-only C sketch wrapped in `#ifdef NONMATCHING` inside src/map_native.c -- a preprocessor guard that is never actually defined anywhere in the build, meaning that C was pure documentation and the real ROM bytes were still linked from the untouched raw disassembly in asm/code/code_0100C0.s. Unwrapping it and testing surfaced two real gaps rather than the sketch being correct as written: the switch statement's `case 1:` (which does the same thing as `default:`) gets merged away by agbcc's switch lowering, so the compiled comparison chain pivots on value 2 instead of matching the ROM's literal, doubly-redundant `cmp r0,#1` / `cmp r0,#1` sequence -- fixed by rewriting the switch as an explicit if/goto chain that names each case's comparison directly, which does not get merged. Second, agbcc hoists each case's `ldr r5, =address` load above its branch when the assignment lives in the same compound `if (...) { assign; goto } ` block (a pure load has no ordering dependency the compiler is obligated to respect), while the ROM only loads the address after the branch is taken -- fixed by giving each case its own separate goto-labeled block instead of an inline compound statement, which stops the compiler from being able to hoist across the branch. Both fixes are examples of the section 5a "reverse register order" and "ldr/lsls scheduling" quirks applying to control flow shaping as well as register allocation -- restructuring equivalent C into a form closer to the compiler's actual lowering, not adding compiler hacks. Once byte-exact, the corresponding raw asm block in asm/code/code_0100C0.s was deleted (replaced with the standard "is decompiled as X(); see src/decompiled.json" marker matching every neighboring already-matched function) and src/decompiled.json gained its entry; game_tables.c's gScriptNativeCommands[42] handler reference was updated from the sub_08012264 placeholder to the real name. audit_provenance.py now reports 1589 compiled C / 1599 total mapped ranges (+1 from the prior 1588/1598), and the THUMB-bit audit tool (tools/audit_thumb_ptrs.py) also gained one fewer real unknown handler in gScriptNativeCommands (3 of the original 4 unknowns remain: PmbDeckMake/DeckMake/ShuffleDeckCopy, all large multi-way jump-table dispatchers in the same family, not yet attempted -- flagged in an automated small-function sweep as still unresolved and likely to hit the same jump-table-resists-matching class documented for the sub_08003AE4 interpolation cluster).

Technique note -- fixing the ldr-hoisting mismatch (worked example: ScriptNativeBackgroundSet, 0x08012264): when a case assigns a pool constant and then jumps to shared code, e.g. `if (x == N) { ptr = ADDRESS; goto shared; }`, agbcc treats the load as unconditional and safe to hoist above the comparison, since the load itself has no side effects it must order after the branch. The ROM only performs the load once the branch is actually taken. Fix: give the assignment its own goto-labeled block instead of a compound if-body --
    if (x == N) goto caseN;
    ...
caseN:
    ptr = ADDRESS;
    goto shared;
-- which removes the load from being reachable in the same basic block as the comparison, so the compiler can no longer hoist it across the branch. Confirmed by objdump: the compound-if form put `ldr r5, [pc, N]` immediately after each `cmp`/before the `beq`; the goto-labeled form moved it to the branch target, byte-matching the ROM. Try this before reaching for `nonmatching` whenever a mismatch is confined to a load appearing one branch too early.

ScriptNativeShuffleDeckCopy decompiled to matching C (0x080128C8, 132 bytes, the "ShuffleDeckCopy" native command in gScriptNativeCommands): the raw asm in asm/code/code_0100C0.s recursive-descent-disassembled this function's own 22-entry jump table as if it were code, producing a spurious `sub_0801290C` label and a long run of nonsense `cmp r1,#70`/`lsrs r1,r0,#32` instruction pairs -- the same "ASCII/data misdecoded as code" failure mode already documented for game_tables.c's string pool, but here hitting a jump table instead of strings. Extracting the raw ROM bytes directly (bypassing objdump's instruction-level view entirely) and parsing them as a flat array of 22 little-endian words resolved it: the table's true base is one word after its label (`ldr r1,_080128E0` loads a pooled constant that itself holds the address 0x080128E4, not 0x080128E0 -- an easy trap when skimming disassembly, since the pool word and the table's first real entry sit at consecutive addresses and both look identical). Once correctly parsed, the table has exactly two distinct targets: a shared handler that forwards args[0..2] to sub_080087EC (not yet decompiled), taken for original deck-type values {1,3,4,6,7,8,9,22}, and a no-op default for every other value in 1..22. This compiled to a plain `switch` with case-fallthrough (matching the existing ScriptNativeSelectLayer style already in src/mapping.c) and matched byte-for-byte on the first attempt -- no register hints, no restructuring, no ldr-hoisting workaround needed here, unlike ScriptNativeBackgroundSet. Verified with make compare; audit_provenance.py now reports 1590 compiled C / 1600 total mapped ranges (+1). Two siblings in the same jump-table family remain undecompiled and are the next candidates: DeckMake (sub_080127F8, 22-way) and PmbDeckMake (sub_080129F4, 22-way) -- both immediately adjacent to this function in ROM and very likely to have the same base-address-off-by-one trap in their raw asm, so extract their table bytes directly rather than trusting the .s file's own labels when attempting them.

ScriptNativeDeckMake decompiled to matching C (0x080127F8, 208 bytes, the "DeckMake" native command in gScriptNativeCommands, immediately adjacent to ShuffleDeckCopy): same jump-table-base-off-by-one trap as ShuffleDeckCopy (the `ldr r1,_08012814` pool word holds 0x08012818, one word past the label, and the true table starts there), and the same case set {1,3,4,6,7,8,9,22} forwarding to a shared handler -- but here the handler body is substantial: it copies a 20-entry s16 table from args[1..20] into offset+14 of whatever sub_08055F4C(mode) returns (both functions still unnamed sub_ addresses; no other call sites exist yet to hint at real names), then flags one bit per raw VM argument in a bitset living at GAME_ROOT+0x26F8 via the already-decompiled BitSet (0807A190) -- GAME_ROOT being this exact file's own pre-existing `#define GAME_ROOT (*(u8 **)0x03003FDC)`. One new symbol had to be aliased: sub_08001EB4 (the buffer-copy call) turned out to be a mid-function entry point inside the already-disassembled-but-differently-labeled sub_08001E70 in asm/code/code_0000C0.s -- no `.global` or even local label existed at that exact address before this, since nothing had branched there until now. Added via the same zero-risk `.global`/`.set` absolute alias technique in asm/game_table_handlers.s used throughout this session, rather than inserting a label into the live instruction stream. The values-copy loop needed one register hint to match: agbcc's register allocator put the cached `mode` value (args[0], read once before the loop and reused once after) in r4 and the loop counter in r3, while the ROM has them swapped (r3=mode, r4=counter) -- declaration-order reshuffling alone did not change this (tried caching mode into its own named local first, no effect), so `register s32 mode asm("r3")` was added, verified necessary the same way as always (build, check size, full make compare). Hinting the loop counter itself was tried too and made things worse -- it defeated the compiler's automatic loop-strength-reduction (collapsing per-iteration `args[i+1]`/`values[i]` addressing into plain pointer increments), replacing it with explicit index-multiply arithmetic and changing the loop's exit condition shape entirely, so the counter was left unhinted. Verified with make compare; audit_provenance.py now reports 1591 compiled C / 1601 total mapped ranges (+1). PmbDeckMake (sub_080129F4) remains as the last function in this three-part family, and per the pattern seen twice now should be checked for the same table-base trap before anything else.

PmbDeckMake attempted and left nonmatching (0x080129F4, 224 bytes): same jump-table-base trap and same case set {1,3,4,6,7,8,9,22} as DeckMake/ShuffleDeckCopy, but a more involved handler -- zeroes a 20-entry s16 array inside sub_08055F4C(mode)'s return, then validates and compacts args[1..20] through sub_080569B0() (score <= 98 to pass) into that array, flagging each accepted value's bit via BitSet. Logic is fully understood and cross-checked against the ROM byte-for-byte (see src/nonmatching/mapping_pmb_deck.c's header). What blocks the match: agbcc fuses the args[0] read with the args-pointer advance needed for a second "source" pointer into one `ldmia r4!, {r0}`, where the ROM keeps them as two separate instructions (a plain `ldrsh` now, an explicit `adds r5,r4,#4` later, once the second loop actually needs its own pointer). This fusion is driven by data dependency, not source order -- tried moving the second-pointer assignment earlier and later in the C, declaring it as its own named local, hoisting the zero-fill constant and the bitset base pointer (both of which *did* fix their own separate, real mismatches earlier in the same function), and a register hint on the mode value (the same fix that worked for DeckMake's r3/r4 swap, aimed at a different register here and had no effect on this fusion). Each fix that resolved one instruction-count mismatch shifted the compiler's own switch-generated jump table (not hand-authored -- it's regenerated from wherever this function's compiled case-handler code actually lands) enough to introduce a new mismatch elsewhere, so this was stopped rather than continued as an open-ended chain of increasingly specific hints chasing a moving target. Recorded here per docs/PRET_STANDARDS.md section 8's requirement to document *why* a function was left nonmatching. This closes out the three-function DeckMake/ShuffleDeckCopy/PmbDeckMake family: two decompiled to byte-exact matching C, one left as a fully-understood nonmatching reference.

Hidden-code audit (code misdecoded as data, the inverse of the game_tables.c/jump-table traps documented above): scanned every asm/code/*.s file for runs of 8+ consecutive raw `.4byte`/`.byte` directives (i.e. regions the original recursive-descent disassembly gave up on and emitted as inert data) whose bytes begin with a Thumb `push {..., lr}` prologue encoding. 43 such runs found across the codebase, several showing the unmistakable `mov r7,r10 / mov r6,r9 / mov r5,r8 / push {r4-r7,lr}` idiom Thumb code uses to save high registers -- a strong signal of genuine compiled prologues rather than coincidental data bytes. One was independently confirmed two ways at once: sub_080569B0 (the address PmbDeckMake's investigation above needed a `.set` alias for, since no label existed there) is exactly the first candidate this scan flagged in code_0500C0.s, sitting immediately after the already-labeled sub_08056984's real `bx r1` epilogue -- the original disassembly's reachability walk simply never found an internal branch into it, since (as far as could be told before this session) nothing else in the already-decoded portions of the ROM calls it. Disassembling its 72 bytes gives a fully coherent, self-consistent function (an 8-iteration accumulation loop over a lookup table at a literal-pool address, clamped to a max of 98 -- exactly matching how PmbDeckMake's nonmatching reference uses its return value). Attempted to give it a proper `.global`/instruction-level label in place of the `.4byte` run (converting data directives to their equivalent mnemonics is normally byte-identical and should carry zero risk) but this broke alignment across thousands of downstream lines in the same file -- GNU `as`'s own encoding choices for `bl` and `ldr rX, =literal` do not always reproduce the exact original bytes even when the semantic instruction is identical, so a hand-transcribed disassembly is not a safe drop-in replacement for the original bytes the way it would be in, say, LLVM's disassemble/reassemble round trip. Reverted immediately (`make compare` confirmed byte-exact again) and kept the existing `.set` absolute-alias technique instead, which touches zero bytes and was already in place for this address. The other 42 candidates are unverified leads, not confirmed hits -- each needs the same two-step check this one got (cross-reference for an actual caller, then disassemble and sanity-check control flow) before being trusted. Their addresses, by file:
code_0000C0.s: 0x08002158, 0x080028EC, 0x08003438, 0x0800382C, 0x080041E0, 0x08004E88, 0x080073DC
code_0080C0.s: 0x08009868, 0x0800A3B8, 0x0800A484, 0x0800A550, 0x0800DCF8
code_0100C0.s: 0x0801674C, 0x08017A74
code_0180C0.s: 0x08019818, 0x08019C08
code_0200C0.s: 0x08027BEC
code_0280C0.s: 0x0802AB44, 0x0802AD98
code_0400C0.s: 0x08042260
code_0500C0.s: 0x08051BD4, 0x08053ADC, 0x080544CC, 0x08056578, 0x0805685C, 0x080571E8
code_0580C0.s: 0x0805DBF0
code_0600C0.s: 0x08065E88
code_0680C0.s: 0x08068F4C, 0x0806B98C, 0x0806C758, 0x0806D60C, 0x0806DEC0
code_0700C0.s: 0x08075ED4, 0x08077370, 0x08077B44
code_0780C0.s: 0x0807D260, 0x0807EA90, 0x0807F1B8, 0x0807F3DC
code_0800C0.s: 0x08081554, 0x08082390
If pursuing these, alias with `.set` (never hand-splice instructions into the byte stream) and verify each with the same cross-reference-then-disassemble check before writing any C against it.
Bulk small-function decompilation, round one: RuntimeGetPointerE3C (0x08009508, 24 bytes, `u8 *gSecondaryRuntime` indexed pointer-array getter at fixed offset 0xE3C) decompiled to byte-exact matching C, following the same file's existing RuntimeGetPointerE30 precedent exactly. This required discovering and documenting a new technique: unlike every previously-decompiled function this session, this one was NOT in its own dedicated `.section .rom.ADDR, "ax"` block -- it was one of many small functions packed inline inside a single large shared section (`.rom.0000931C`, ~1KB, covering a dozen-plus unrelated functions with no internal section boundaries). Simply removing its raw asm and adding an AT()-pinned C replacement silently corrupted the ROM layout: since linker sections are concatenated by name-sort order with zero gaps, shrinking the shared section just closed the hole instead of leaving room for a same-named replacement section, shifting everything physically after it forward and breaking call sites throughout the ROM (caught only via `arm-none-eabi-nm` showing the new function at the wrong address, since `make` itself did not error). The fix: insert a new `.section .rom.<next-function-address>, "ax"` + `.syntax unified` directive immediately before whatever function originally followed in the shared section, splitting it into a correctly-shortened head and a newly-and-correctly-named tail. Zero byte risk, confirmed via `make compare`. Documented and relayed to the parallel decompilation agents working other files, since this pattern (functions living inside shared multi-function sections rather than their own dedicated ones) is common outside the handful of larger, individually-`tools/split.py`-sectioned functions handled earlier this session.

[SUPERSEDED -- see the CORRECTION a few entries below, and docs/AGBCC_CODEGEN.md: the ldrsb/ldrb reasoning in this entry is wrong.] Three adjacent candidates in the same cluster (sub_0800943C/GetField234, sub_0800945C/SetField234, sub_08009498/SetField34C -- all `gSecondaryRuntime`-indexed byte field accessors at offsets 0x234/0x34C) were attempted and reverted after failing to reproduce exact register allocation despite several C restructuring attempts (variable declaration order, expression shape, and a register hint that fixed the store's final base register but left operand order and an unrelated pool-load register choice still mismatched). The getter's mismatch looks like a genuinely different codegen path (`ldrb`+shift-sign-extend in the ROM vs `ldrsb` in every C shape tried, mirroring the documented "different agbcc pass per ROM region" possibility already noted for sound_m4a.c) rather than something a C-level rewrite can fix without forbidden techniques (forced registers used speculatively without proof, compiler flags). Reverted cleanly (confirmed via `make compare`) rather than force a fakematch or leave a broken intermediate state. `RuntimeGetPointerE3C` is the only net addition from this cluster; audit_provenance.py now reports 1592 compiled C / 1602 total mapped ranges (+1).

Bulk small-function decompilation, round two: IwramSetField3FD5 (0x08001A34, 20 bytes), IwramGetPointer2860 and IwramSetPointer2860 (0x08001B34/0x08001B4C, 24 bytes each) decompiled to byte-exact matching C, all three using the already-declared-but-previously-unused gIwramField3FD5Offset/gIwramPointer2860Offset symbols in runtime_accessors.c (evidence those names were anticipated but never actually wired up before this session). Two more instances of the shared-section extraction technique from the previous entry, including one case (IwramSetPointer2860) sitting directly before a large, clearly-hidden-code-bearing data tail (multiple `push {r4-r7,lr}`-prefixed words immediately follow its own pool constant) -- left untouched and flagged for a future hidden-code investigation pass rather than pursued now.

Technique note -- forcing a parameter into a specific register without an extra copy instruction: `register T x asm("rN") = param;` used directly as a fresh local (not renaming the parameter itself -- attempting `func(register u32 x asm("r0"))` in the parameter list is a syntax error in agbcc) reliably pins that value to the named register for the rest of the function, and because it's initialized directly from the incoming parameter (already in that register per AAPCS for the first few arguments), agbcc does not emit a redundant move. This differs from hinting a *derived* value (e.g. `register u32 addr asm("r0"); addr = base + offset;` computed mid-function) -- that consistently added an extra instruction to relocate whatever was already sitting in r0 out of the way first. The fix for IwramGetPointer2860/IwramSetPointer2860's register-order mismatch (compiler kept the running pointer total in r1, ROM has it in r0) was renaming the incoming parameter to `index0` and introducing `register u32 index asm("r0") = index0;` as the very next line, then writing the rest of the function in terms of `index` -- zero extra instructions, and the resulting instruction order matched the ROM exactly on the first attempt once the hint was placed correctly. Also confirmed empirically (not just by the existing ObjectFreeNcdResources documentation) that `(u32)externSymbolA + (u32)externSymbolB` between two different extern symbols is NOT constant-folded by agbcc into one literal -- unlike two raw hex address literals, which are -- so combining two named offset/base symbols in one expression is safe and still produces two separate pool loads, useful for matching ROM code that computes a combined base+fixed-offset before adding a variable index.

Bulk small-function decompilation, round three: GameStateGetBuffer3F38 (0x08005360, 24 bytes, `GAME_ROOT`-style pointer getter -- root's game-state base plus a fixed 0x3F38 offset) decompiled to byte-exact matching C. Reused the already-declared `gMapGenerationRootOffset` symbol (0x3FDC) and `gIwramBase`, matching this file's existing `GAME_STATE_BASE` macro pattern exactly instead of writing a raw `0x03003FDC` literal -- confirmed empirically that the raw-literal form (which worked fine for ScriptNativeDeckMake's `GAME_ROOT`-equivalent access earlier this session) does NOT reproduce this particular function's bytes, because the ROM computes the field offset (0x3F38) by reusing the *already-loaded* 0x3FDC pool register and subtracting 164 from it in place, rather than loading a third, separate 0x3F38 pool constant -- writing `0x3F38` directly in C, even split across two extern symbols added together, produces an extra unwanted pool load since the compiler has no reason to know it should reuse gMapGenerationRootOffset's value arithmetically instead of just loading the destination constant fresh. Expressing the offset explicitly as `(u32)gMapGenerationRootOffset - 164` (matching the ROM's own derivation) and placing that subtraction after the pointer dereference (matching the ROM's exact instruction order) fixed it in two iterations. This confirms, again, that agbcc reliably picks the cheapest-looking encoding for a given C shape rather than the ROM's actual original derivation, so getting a byte-exact match sometimes requires reverse-deriving the ROM's specific arithmetic path (not just its final numeric result) and writing C that walks the same path. audit_provenance.py now reports 1596 compiled C / 1606 total mapped ranges after this session's four-function bulk-decompilation pass (RuntimeGetPointerE3C, IwramSetField3FD5, IwramGetPointer2860, IwramSetPointer2860, GameStateGetBuffer3F38 -- five, not four; corrected count).

[PARTLY SUPERSEDED -- the matches in this entry stand, but its conclusion about *why* the 0x234 getter differed is wrong; see the CORRECTION below.] Bulk small-function decompilation, round four: RuntimeActorGetField358 (0x0800975C, 28 bytes, s16), RuntimeActorGetField79C and RuntimeActorSetField79C (0x08009794/0x080097B4, 32/28 bytes, s8) decompiled to byte-exact matching C on the first attempt each, using the ACTOR_LATE_GET_S16/ACTOR_LATE_GET_S8/ACTOR_LATE_SET_S8 macros already sitting unused in runtime_buffers.c. Unlike the earlier-reverted 0x234 field accessor pair, these three ROM functions already used direct `ldrsh`/`ldrsb` register-offset loads (not the older `ldrb`+shift-sign-extend idiom), so the plain `return *(s16 *)base;`/`return *(s8 *)base;` macro body matched agbcc's natural codegen without needing any register hints -- confirms the earlier hypothesis that the 0x234 getter's mismatch was a genuine different-codegen-path issue specific to that function (or its ROM region), not a flaw in the LATE macros or the general approach. Another adjacent hidden-code-bearing data tail was found and left untouched (7 words between RuntimeActorGetField358's own pool constant and RuntimeActorGetField79C, added to the same future-investigation list as the earlier one in this cluster). audit_provenance.py now reports 1599 compiled C / 1609 total mapped ranges.

CORRECTION (later, after reading the agbcc source): the diagnosis in the two entries above is wrong, and no part of this ROM was built by a different compiler pass. `ldrsb` and `ldrb`+`lsl #24`+`asr #24` are both emitted by the single `*extendqisi2_insn` pattern in gcc/thumb.md, and which one appears depends only on whether the destination register is also the base register: when they coincide the compiler cannot use `ldrsb` non-destructively and falls back to the shifts. That is a register-allocation outcome, reachable from ordinary C by controlling how long the base pointer stays live -- not evidence of a different codegen path. The sound_m4a.c premise those entries lean on is itself real but narrower than they assume: the Makefile really does build sound_m4a.c and sound_cgb_update.c with old_agbcc, and only those two files. There is no evidence of the compiler varying by ROM region, and that idea should not be used to explain away a mismatch elsewhere. The 0x234 cluster is therefore worth another attempt on those grounds. See docs/AGBCC_CODEGEN.md for the pattern table and the probes that confirm it.

Stale-duplicate cleanup: found and removed two dead `#ifdef NONMATCHING` stubs whose addresses had since been superseded by real, currently-matching implementations elsewhere -- both were never compiled (the project never defines NONMATCHING for the default build; see the Makefile's `ifdef NONMATCHING` guard around `src/nonmatching/*.c`), just stale leftovers from earlier abandoned decompilation attempts. `RuntimeGetPointerTableEntry` (src/table_accessors.c, AT("00009508")) duplicated this session's own newly-matched `RuntimeGetPointerE3C` (src/runtime_buffers.c) at the exact same address. `ScriptNativeConfigureActorSlots` (src/script_native_adapters.c, AT("00012C68")) duplicated the already-matching `ScriptNativeSetBattleParty` (src/mapping.c) -- identical logic, just a different name from an earlier pass. Removed both (plus their now-unused `extern` declarations) rather than leaving misleading dead code that could confuse a future pass into thinking those addresses were still open. A full scan of every `#ifdef NONMATCHING` block across the codebase against src/decompiled.json found no further duplicates. Verified with make compare (no behavior change, since neither stub ever compiled).

Bulk small-function pass, batch B (asm/code/code_0180C0.s, code_0200C0.s, code_0280C0.s, code_0400C0.s, code_0500C0.s, code_0580C0.s, code_0600C0.s): 33 functions decompiled and verified byte-exact (`make compare` clean after every group), 1,144 bytes of asm removed from those files. Method: enumerate `.thumb_func` blocks whose label address is not yet in src/decompiled.json, disassemble the real bytes out of baserom.gba rather than trusting the .s text (the splitter emits `bl` to unlabelled targets as raw `.2byte`/`.4byte`, which hides the instruction stream), cut the function's bytes out of the .s only after a byte-size walk over the removed lines lands exactly on the next function's address, replace them with the `@ AAAAAA..BBBBBB is decompiled as Name(); see src/decompiled.json` marker plus a `.section .rom.00BBBBBB, "ax"` split so the following code keeps its own address (the shared-section trap: removing bytes from the middle of one big section otherwise slides everything after it), and rename every `bl sub_ADDRESS` reference in asm/ and src/ to the new C name. Iterating on codegen is much faster against a single .o than a full ROM build: cpp + agbcc + as on one throwaway .c and `objdump -d` the result, then compare instruction-by-instruction with the ROM disassembly; a full `make && make compare` only as the group-level confirmation. Functions: item.c gained ItemGetField78 (056F68) and ItemGetField7C (056F7C); runtime_buffers.c gained RuntimeActorGetByteA4 (019C64) and RuntimeActorGetByte66 (019CA0); runtime_accessors.c gained GameStateGetEntry2768/2AE0/31D0 (0568B4, 056EE0, 057138), GameStateGetEncounterValue (0577E4) and GameStateGetEncounterMode (057844) -- both named from their only callers, ScriptNativeGetEncounterValue/Mode in resource_native.c -- plus GameStateSetFlag2730 (056A34), GameStateTestFlag2730 (056A60), GameStateTestFlag26F8 (056B2C), GameStateGetCurrentEntry3894 (056130) and GameStateClearEntry31D0 (057174); runtime_misc.c gained GameStateClearRecord426A (057514) and InputRepeatRearm (02AE74); new src/game_state_records.c holds the 84-byte record family at state+0x35E0 (GameStateRecordSetField4 055FE8, GameStateRecordSetField6 05601C, GameStateRecordAddField6 056050), the s16 table at +0x3894 (GameStateSetEntry3894 0560C4, GameStateGetEntry3894 0560F8), GameStateCopyRecord (056CD0), GameStateGetEntry2768Total (056984) and GameStateGetPartField64C (056DFC); new src/object_group.c holds ObjectGroupReset (029E00) and ObjectGroupSetFlag2 (029E34); new src/scene_draw_adapters.c holds SceneDraw5F874 (05F874); map_field.c gained InitializeFieldDisplay5A4 (0558A4), a third sibling of InitializeMapFieldDisplay. Five agbcc idioms did nearly all the work and are worth reusing: (1) a 16-bit parameter that the prologue sign-extends *in place* (`lsls r0,r0,#16 / asrs r0,r0,#16` with no preceding `adds rN,r0,#0`) means the parameter was declared `s16`, not `s32` cast at the top of the body -- declaring it `s16` and copying it into an `s32` local is what reproduces GameStateGetPartField64C exactly; (2) conversely, a preamble that copies an argument to a callee-saved register and only then shifts it is `v=value; v=(s16)v;` as two statements, and writing `value=(s16)value` instead costs one extra `adds`; (3) the map-generation root is reached as `iwram=gIwramBase; offset=(u32)gMapGenerationRootOffset; base=*(u8 **)(iwram+offset);` with those two locals spelled out in that order -- the expression form in the GAME_STATE_BASE macro allocates the two literals into the opposite registers, and when the same function later uses the offset again (`offset-=180`) the local form is the only way to get the `subs r1,#180` literal reuse the ROM has; (4) walking a base pointer one statement at a time (`base+=0x3894; base+=index;`) reproduces the ROM's add order, while a single compound expression reorders the adds and, worse, lets agbcc fold a nearby constant (it derived 0x64C as `subs r1,#60` off a live 1672 instead of loading the 0x64C literal the ROM uses); (5) storing a constant that the ROM loads straight into the store register (`ldr r0,=999 / strh r0`) needs the constant assigned to a local first (`stored=999; field=stored;`), since assigning the literal directly costs an extra `adds r0,r2,#0`. Trailing 0x0000 alignment padding is supplied by the established `AT("000XXXXX") const u8 NameTail[2]={0};` trick -- GNU as otherwise pads a code section with a 0x46C0 `nop` and the two bytes mismatch. One bonus hidden-code find: 0805601C is a real function (GameStateRecordSetField6) that the original recursive descent left as a `.4byte` run between two decoded neighbours; nothing in the ROM calls it (checked with a full-image BL scan), and its signature is taken from the byte-identical sibling at 08055FE8, which is the only reason it was safe to write C for it. Left mid-investigation, each blocked on register allocation rather than logic: 08055FB8 (record field-2 accumulator clamped against field 4 -- the logic is `v+=field2; field2=v; v=(s16)v; limit=field4; if (v>=(s16)field4) field2=limit;` and the only mismatch is that the ROM keeps the record pointer in r1 and the limit in r2 while agbcc emits r2/r1, unaffected by declaration order or by inlining the limit load); 08055F4C (the record lookup itself, `index=(s16)sub_08055EC8((s16)id); if (index==-1) return index; else return *(u8 **)(iwram+offset)+index*84+0x35E0;` written with a single `result` local so the early return lays out as `bne`/`b` like the ROM -- structurally identical output, index in r2 instead of r1 and the root pointer in r1 instead of r2); 08056290 and 080562C8 (the 999,999-saturating counter at state+0x38BC: 08056290 needs the root pointer re-read after a *conditional* store, which the macro form does reproduce, but with r3/r4 swapped against the ROM; 080562C8 additionally needs the value re-loaded after an *unconditional* `+=`, and agbcc forwards the stored value instead, so some other source shape is involved); 0801B7FC and 0801DADC (identical CreateTask adapters, correct size, one register-allocation difference around the shared 0xFFE0 literal) and 0801DB8C (same family, needs the 0 hoisted above the 256); 080577C0 (the encounter-value setter paired with GameStateGetEncounterValue -- correct size, value lands in r2 instead of r1); 080577A0 (digit-to-character, where agbcc merges the two branches into `+48` then `+7` while the ROM keeps two separate `adds`); and 08056F68's sibling shape generally. Also examined from the earlier hidden-code candidate list: 0x0802AB44 is genuine code (a printf-family number formatter whose entry point the disassembler missed although its body from 0802AB7C onward is decoded, with four real callers at 0802AA6C/AA82/AA98/AAAE inside the format loop at 0802AA12), 0x0802AD98 is a genuine 46-byte Shift-JIS-aware character emitter called from 0802AACA (writes a two-byte character when the high byte is 0x81..0x9F, otherwise one byte, NUL-terminates and returns the length), 0x080571E8 is a genuine 512-byte CpuCopy between state+0x33D0 and state+0x31D0 called from 0806CC06 (the mirror of the already-decompiled GameStateCopyMapBuffer at 00057218), 0x0805DBF0 (caller 0805CD14) and 0x080544CC (caller 0806726A) are genuine but large; 0x08019818, 0x08027BEC, 0x08042260, 0x08051BD4, 0x08053ADC, 0x08056578, 0x0805685C and 0x08065E88 have no caller anywhere in the image (full BL scan plus even-word and THUMB-bit pointer scans) and are deferred; 0x08019C08 does have a caller at 08019AD6 and is a small five-argument initialiser, not yet written.

Bulk small-function pass over code_0680C0.s/code_0700C0.s/code_0780C0.s/code_0800C0.s (batch C), 12 functions / 584 bytes, all verified byte-exact with `make compare` after each one: SpriteRuntimeInit (0x080804BC, 48), SpriteRuntimeSetFields8C4 (0x08080504, 56), SpriteRuntimeSetFlag800 (0x08080574, 64) and SpriteRuntimeTestFlag800 (0x080805B4, 28) in the new src/sprite_runtime_6120.c; ScriptResourceSlotFirst (0x0807F32C, 40), ScriptResourceSlotSecond (0x0807F354, 44) and ScriptResourceSetStringValue (0x0807F3DC, 92) in src/script_resources.c; ScriptResourceSetRecord224 (0x0807F1B8, 32) in src/script_resource_handles.c; FindLowestSetBit (0x080705F0, 48) in the new src/bit_scan.c; BattleRuntimeSetArena (0x08070214, 36) in src/simple_adapters.c; MapGenerationRelease (0x08070140, 60) in src/mapping.c; RuntimeStoreCurrentRecord14C (0x0806C6F8, 36) in src/runtime_core.c. Several reusable lessons came out of it. (a) The `.set alias, CSymbol` trick for keeping a removed function's `sub_08XXXXXX` name resolvable does NOT work -- GNU `as` equates to a symbol that is undefined in that translation unit and exports nothing, so the link fails with "undefined reference"; the working approach is the one the project already uses for CpuFill/CpuCopy, i.e. renaming the `bl sub_08XXXXXX` call sites in asm/code/*.s to the new C name (a pure symbol-name edit, identical encoding, zero alignment risk). SpriteRuntimeSetFields8C4 alone had 79 such call sites across 12 files. (b) agbcc's *tail merging* is what produces a single shared store at the end of an if/else: writing SpriteRuntimeSetFlag800 with explicit `flags`/`value` locals and one trailing store (the "obvious" transcription of the ROM) reproduced the logic but permanently swapped r0/r1 in both arms; the plain, natural `if (enabled) *p |= 1<<bit; else *p &= ~(1<<bit);` matched on the first try because agbcc merges the two identical `str` tails itself. Prefer the natural spelling and let the optimizer do the merging. (c) Block *order* is the most common near-miss in these guard-style accessors. ScriptResourceSlotFirst/Second emit the failure block (`movs r0,#0; b`) between the guard and the success block; a plain `if (bad) return 0; ... return ptr;` emits them the other way round. `if (ok) { ...; if (ok2) goto found; } return 0; found: ...` reproduces the ROM order exactly, and the same trick (with an extra `goto` pair) was needed for FindLowestSetBit, where the ROM additionally uses `bne X / b Y` instead of an inverted `beq` -- a tell that the source reached the not-found return through its own explicit branch rather than by falling out of an `if`. (d) `pop {r0} / bx r0` versus `pop {r1} / bx r1` in a Thumb epilogue is a reliable **return-type** signal, not noise: agbcc pops into r0 only when r0 is dead, i.e. the function returns void. ScriptResourceSetRecord224 was written returning `s32` first and mismatched on exactly those two halfwords; changing it to `void` matched. (e) The `AT("...") const u8 XxxTail[2] = {0};` idiom already used throughout src/ is needed whenever the function's own instructions end on a 2-byte-but-not-4-byte boundary AND the ROM has `0x0000` there -- otherwise agbcc's own alignment pads with `0x46C0` (`nop`) and the two bytes mismatch. It must NOT be added when the function already ends flush with its literal pool (MapGenerationRelease), or the section grows past the original size. Two functions were attempted and abandoned rather than forced, both pure register-allocation disagreements with identical logic and (for the second) identical size: SpriteRuntimeGetFields8C4 (0x0808053C, 56 bytes) -- the ROM keeps the *address* 0x03006120 in a callee-saved register and re-dereferences it before writing the third output, while agbcc common-subexpression-eliminates the load in every spelling tried (inline macro at all three uses, a named local, u16* vs s16* out-parameters); short of a `volatile` qualifier that would force three loads instead of two, nothing reproduced it. CopyBytesAdvance (0x0807E738, 52 bytes, the "copy n bytes, return source+n" helper that the 0x7E6xx-0x7F0xx resource serializers call a dozen times, `CpuCopy` above 64 bytes and an inline byte loop below) -- the ROM holds five values (preserved source r5, preserved destination r0, preserved size r4, walking source r3, counter r2 = the incoming size register), and every combination of locals tried either coalesced the preserved size with the counter (52 bytes, right size, wrong registers) or added one `adds rX, rY, #0` too many (56 bytes); the spelling that got closest is `const u8 *in = source; if (size > 64) goto useDma; if (size > 0) { do { *destination++ = *in++; } while (--size != 0); } return in; useDma: CpuCopy(destination, source, size); return source + size;`.

Hidden-code audit, batch C (the 14 candidates in code_0680C0.s/code_0700C0.s/code_0780C0.s/code_0800C0.s from the 43-candidate scan): every one was extracted from baserom.gba and disassembled rather than trusting the .s file's own view, then cross-referenced for callers. Twelve are genuine, coherent compiled functions -- correct prologues, branch targets that stay in range, proper `pop`/`bx` epilogues -- confirming the scan's premise; two are false positives in the "is this address a function *start*" sense. Decompiled, with callers or an unambiguous ABI: 0x0807F3DC is the strongest possible confirmation of the whole technique -- it already had a real C caller (ScriptResourceSetSecond in src/script_resource_handles.c) and even a pre-existing `.thumb_set sub_0807F3DC, 0x0807F3DD` alias in asm/iwram_symbols.s, yet its 92 bytes sat in code_0780C0.s as a bare 23-word `.4byte` run; it decompiled first-try to matching C as ScriptResourceSetStringValue (free the old slot allocation, HeapAlloc strlen+1, strcpy). 0x0807F1B8 had no caller at all, but its entire body is a single tail call into the already-decompiled ScriptResourceSetFirst(context+548, 0, arg), so the argument convention is unambiguous; decompiled as ScriptResourceSetRecord224. Confirmed real but deferred for want of a caller (no way to pin the signature, so no C was written): 0x08068F4C (CreateTask constructor, 0x1A80-byte task state, stores its argument into the task and calls 0x08056F04), 0x0806B98C (high-register prologue, walks an s16 array through 0x080568B4), 0x0806C758 (counts matching entries via 0x08005094 and returns a bool), 0x0806D60C and 0x0806DEC0 (near-identical siblings: CpuFill the 0x18000-byte VRAM block plus five 252-byte sub-blocks of one table, then zero a dozen fields), 0x08075ED4 (large, 68-byte frame, calls 0x0800A1C4 and the already-named RuntimeGetBattleCharacterDefinition; sits immediately after sub_08075EBC's real epilogue, the classic "nothing branches here" shape), 0x08077370 and 0x08077B44 (two more CreateTask constructors, state sizes 0xA8 and 0x338, both storing five caller arguments at task+0x54..0x6C), 0x0807D260 (fixed-point 3-axis rotation math against a sine table at 0x0807D3BC), 0x0807EA90 (hash-chain lookup over the VM context's resource table using the string compare at 0x0808280C). False positives: 0x08081554 and 0x08082390 are each exactly 4 bytes *inside* an agbcc soft-float helper whose real `push {r4, lr} / sub sp, #N` prologue is at 0x08081550 and 0x0808238C -- the `.4byte` run simply began one word late, so the scanned address matched a push encoding without being a function entry. Both bodies are compiler-runtime float comparison code (they thread through 0x08080FC4/0x080819A0 unpackers), not game C, so neither address nor its real start is worth pursuing. One bonus hit outside the candidate list turned up while working on code_0800C0.s: the 28 bytes at 0x080805B4, sitting in the literal-pool `.4byte` run after sub_08080574, are a complete `return *(u32 *)(block+0x800) & (1 << bit)` leaf -- decompiled as SpriteRuntimeTestFlag800. The lesson is that the pool-adjacent `.4byte` runs immediately *after* a labeled function are worth a second look even when they do not start with a push: a leaf function with no stack frame ends in a bare `bx lr` and never produced a push for the scan to find.

REGISTER-HINT CORRECTION (later, after tooling the audit): the two
"Register-forcing cleanup" entries above conclude that a specific list of
functions still genuinely need their register hints. That conclusion was drawn
per function -- every hint in a candidate was dropped at once, and if the ROM
stopped matching the whole set was put back. That only shows that removing
the tested set changes the current C's output. It does not prove that this
function, or any equivalent C reconstruction, needs a hint. Several functions on the "genuinely necessary"
lists turned out to need only a subset: src/nfp.c went from five hints to
three, and src/ncd_sprite.c, src/resource_native.c and
src/sprite_tile_allocator.c each shed one or more, all byte-exact.

tools/drop_register_hints.py does the per-hint version: drop one, recompile that
translation unit, keep the removal only if the machine code is unchanged, and
apply removals cumulatively so interactions are accounted for. Across the whole
build it removed 210 hints that were believed necessary or had never been
tested, taking the audit's hard-rule count from 351 to 157.

Two method points from that pass are worth repeating, because both produced
confidently wrong numbers first: compare machine code rather than the
compiler's assembly text (a removed `asm("" : "+r"(x))` fence also removes the
`.code 16` directives it emitted, so an identical instruction stream reads as a
difference), and remember that fences come in sets -- dropping one while its
partner still pins the schedule changes the instruction order, so each looks
load-bearing alone while the whole set is removable.

Shiftability cleanup, sound players: the fixed IWRAM M4A player objects are
now declared once in `include/sound.h` and used through `gSoundPlayer0` through
`gSoundPlayer8` across the idle-wait, sound-task, and scene-native code. This
replaced every raw player-address cast in those paths with the existing linker
symbols and remained byte-exact. It is a naming and linker-interface recovery,
not an assumption that player storage has become dynamically allocated.

The earlier argument-narrowing probes described specific C candidates, not
universal rules for `s16` parameters. `CreateFieldEventTask` now matches with
`s16` parameters after correcting the task-payload structure and its final
halfword offset. See the field-event constructor findings below and
`docs/AGBCC_CODEGEN.md` for the corrected explanation.

## Map queries and random pools (2026-09-15)

Four assembly ranges (432 bytes) now compile as ordinary agbcc C:

| Address | Bytes | Function | Recovered behavior |
| --- | ---: | --- | --- |
| 080036F0 | 52 | RandomPoolInitialize | Allocate or reuse storage and fill indices 0 through count minus one. |
| 08003724 | 76 | RandomPoolTake | Draw without replacement, using the last live entry to fill the removed slot. |
| 08072924 | 116 | MapCollectAttributePositions | Bounded, row-major exact-attribute search with boolean result. |
| 08072B48 | 188 | MapGenerationFindOverlappingEntry | Inclusive rectangle overlap against active current-field entries. |

The pool functions are called by the 20-entry shuffle at 08056CF8. Empty-pool
draws return element zero without advancing the RNG or decrementing the count;
the caller must still provide readable storage. This behavior is preserved,
not replaced with an invented error sentinel.

Following `AGBCC_CODEGEN.md`, candidates were compiled with the existing flags
and compared with disassembly of `baserom.gba`. In the overlap routine, assigning
the signed bound to a local before subtraction preserves the original value's
register lifetime. No forced-register declarations, assembly fences, inline
instructions, or compiler changes were introduced. Removed assembly includes
the spurious internal `sub_0800376C` label, which was only the pool draw epilogue.

Host coverage exercises allocation/reuse, draw uniqueness, first/last removal,
empty pools, search capacity and scan ordering, out-of-map attributes, mirrored
bounds, touching edges, field filtering, and entry-state narrowing.

## sub_08056CF8 fully traced; do not decompile it alone (2026-09-15)

sub_08056CF8's logic is now fully recovered by hand-decoding its one
unresolved `bl` (the raw `.short 0xf7ac` / `.word 0xb00bfd17` pair, which
is a normal ARMv4T BL followed by the epilogue's `add sp, #44` sharing the
same word -- decoding the BL alone, not the whole word, gives 0x08003770,
`HeapFreeDefault`): copy the caller's 20 halfwords onto the stack with
CpuCopy, RandomPoolInitialize a pool over them, then RandomPoolTake
repeatedly to write the shuffled values back into the *caller's own array*
in place, and HeapFreeDefault the pool. This is the DeckShuffle/
ShuffleDeckMake algorithm the random-pool functions above exist for.

It is not safe to land alone, though. It takes a real pointer argument
(the array to shuffle, read into r0 at entry and immediately copied to r7)
-- but its only C-level caller, the `REFRESH_MAP_VALUES` macro in
src/mapping.c, already calls it as `sub_08056CF8();` with *no* argument,
right after `sub_08056E3C(0, (s16)args[0], (s16)args[1]);` with no
assignment of that call's result. That only byte-matches because
sub_08056E3C returns a real pointer in r0 (confirmed: it ends with
`adds r0, r0, r4; pop {r4}; pop {r1}; bx r1`, not a bare `bx lr`) and
nothing between the two calls touches r0. The macro is almost certainly
misrepresenting one nested call, `sub_08056CF8(sub_08056E3C(0, x, y))`, as
two separate statements that happen to produce the same bytes by register-
reuse accident. sub_08056E3C itself computes a pointer through gSecondaryRuntime's own 1672-byte
actor stride plus a second table at the game root's +0x423C (not yet
named), so decompiling it is its own task.

Whoever picks this up: decompile sub_08056E3C first, confirm what it
actually returns, then land both together as one nested call and re-verify
the whole ScriptNativeRefreshMapValuesA/B expansion with a full
`make compare` (not just an isolated snippet) before trusting either
alone -- the macro is currently committed, matching code, and changing its
statement structure carries real risk of a silent mismatch a narrow test
would miss.

## Field-event constructor: corrected layout and matching C

`CreateFieldEventTask` (0x08061EA8, 120 bytes) is now reconstructed in
`src/map_field.c`. It allocates a 160-byte payload after the 32-byte task
header, binds an object pointer, and stores four signed coordinates plus one
additional signed value whose purpose is not yet known. The payload layout
is documented in `include/map_events.h`.

The old nonmatching structure misplaced the last halfword at task +0xB4;
the ROM stores it at +0xB2. Separating header and payload, using signed
16-bit parameters, and fixing that offset reproduces the original bytes
without pins, barriers, or a compiler change. Both agbcc snapshots matched
the isolated candidate. This supersedes the earlier argument-narrowing
explanation: the original reference layout was also wrong.
## Script frame temporary pools (0x0807EC58)

`ScriptFrameReleasePools` releases two arrays of `{count, data}` records. The
first owns one allocation per active record. The second owns a pointer array
and each non-null allocation referenced by that array. It then clears the
frame's interpreter work state and restores `field084` to the sum of the words
at offsets 0x28 and 0x2C. The function is a byte-exact clean-C match. Its
inner loop requires the element index to be initialized before the moving
pointer is loaded; this is source ordering, not register pinning.

## Engine-root linker symbols

`gScriptContext`/`gScriptBytecodeRoot` name compatible script views stored at
one root slot. `gSpriteEngineState` and `gSpriteRuntime` name the renderer
state and the runtime block it caches. `gMapGenerationRoot` names the existing
main-runtime root. Their linker aliases replace raw IWRAM-address casts in
matching C paths. The Japanese ROM remained byte-identical after the change;
`COMPILER_HINT_CLEANUP.md` records the typed views and host-test handling.

## SpriteUiInitialize (0x08019C08)

The 44 bytes after `sub_08019BD4` were hidden in an eleven-word raw-data run
despite a real caller at 0x08019AD6. The caller establishes a 12-byte state
at its runtime +0x70 field, passes a resource name in r2, and invokes this
five-argument initializer. `SpriteUiInitialize` now records the confirmed
layout: it activates the state, initializes the two byte arguments, sets a
-48 vertical baseline and one active slot, then resolves the supplied name
through `SpriteResourceFindGroup(1, name)`.

The direct structured C form in `src/sprite_ui.c` is instruction-for-
instruction identical to the ROM with the regular agbcc compiler: no register
pinning, inline assembly, or compiler-specific workaround was needed. The
raw run in `asm/code/code_0180C0.s` was removed, its symbol and manifest entry
were added, and `make compare` confirmed the final full-ROM SHA-1.

`SpriteUiSetupDefault` at 0x08019C78 configures the same state for an actor
part. It obtains the actor record, selects the state at its confirmed +0x64
offset, marks it as kind 1, and derives its vertical offset from byte 0x62 of
the default \u00c4RM definition (ID 85), scaled by 60. The straightforward typed
C implementation exactly reproduces all 40 ROM bytes, including the signed
byte multiplication sequence.

## Text encoder helpers (0x0802AD78 and 0x0802AD98)

Two functions in `code_0280C0.s` had been split incorrectly: the analyzer
placed a function label four bytes into the first function, hiding its
prologue in raw words. `CopyTextWithoutTerminator` copies source characters
up to, but not including, the NUL terminator and returns the number of bytes
written. `WriteEngineCharacter` writes one ordinary character or the two
bytes of an engine multibyte character, then appends a NUL terminator. Its
wrapped lead-byte test covers the inclusive `0x81..0x9F` range.

Both are ordinary C in `src/font.c`. The copy routine keeps explicit named
`copy` and `check` labels because this is the natural control-flow shape that
preserves the ROM's separate unsigned load for the stored byte and signed
load for the terminator test. It uses no register constraint or inline
assembly. The compiled objects match 32 and 48 original bytes respectively;
the latter includes its two-byte zero padding.

## Actor-part field 0x80 clear (0x08019818)

`RuntimeClearActorPartField80IfArmFlag20` accepts an actor, part, and signed
ARM ID. It checks bit 5 of byte 0x74 in that ARM definition, then zeroes the
36-byte field at the actor-part record's +0x80 offset when the bit is set. It
returns whether it performed that clear. The C spells the flag test as a
shift-to-sign comparison because that is the compiler's compact expression
of the exact bit test in the ROM; the offsets and shift each have named
constants.

This extraction was also a shared-section case: the raw function was inside
the existing `.rom.000180C0` assembly section. Removing it without a new
`.rom.00019850` boundary shifted the later code and the full comparison
reported 991 changed bytes. Splitting the retained assembly at the next
function's original address restored all placement. The final full ROM
comparison is byte-identical.

## Battle-mode adapters

The three raw wrappers adjacent to `BattleMode4222` and `BattleMode4223`
only differ by the mode passed as their fifth argument to the common
`sub_08042288` implementation. They are now the ordinary typed
`DEFINE_MODE_ADAPTER` calls `BattleMode4221`, `BattleMode4224`, and
`BattleMode4225`. The existing macro emits the original 20-byte wrapper form
for modes 1, 4, and 5 exactly. The assembly after the removed mode-4 and
mode-5 wrappers is explicitly restarted at `.rom.00042288` so its original
address cannot slide.

The same typed wrapper shape also recovered `BattleMode3C04` (0x0803C0AC),
`BattleMode41A1` and `BattleMode41A5` (0x080419F0 and 0x08041A40), and
`BattleMode4AC3` (0x0804AC64). Each has an already-decompiled neighbouring
mode that calls the same implementation with an otherwise identical ABI.
Their C bodies name that implementation and pass only the fixed mode;
`make compare` verified all four wrappers against the complete ROM. The
0x08041A40 extraction also adds an explicit retained-assembly section at
0x08041A54, preserving the following shared implementation's placement.

## Sound IRQ and early-IWRAM control accessors

`SoundGetIrqMode` (0x08001AD8) is the signed-byte reader for the same
`gSoundIrqModeOffset` field that `SoundIrqService` checks before choosing its
DMA/mixer path. Keeping `gIwramBase` and the named offset as separate locals
is ordinary C and preserves the ROM's two literal loads, addition, signed
byte conversion, and literal-pool layout exactly.

Five adjacent raw leaf routines are now named IWRAM control accessors:
`IwramEnableField2870` (0x08001BB0), `IwramClearField2870` (0x08001BC4),
`IwramGetField2871` (0x08001BD8), `IwramGetField2870` (0x08001BEC), and
`IwramSetField2870` (0x08001C00). They respectively set, clear, read, read,
and write the two neighbouring byte fields at fixed IWRAM offsets 0x2870 and
0x2871. The field-level names deliberately retain those offsets because no
caller yet establishes their gameplay role. Retained assembly restarts at
0x08001C14 after their removal, so the following routine remains fixed at
its original address. All six routines were first checked against their
individual objects, then with a complete byte-identical ROM comparison.

## Runtime task constructor at 0x08069DB4

`sub_08069DB4` is now documented in
[`src/nonmatching/runtime_task_69db4.c`](../src/nonmatching/runtime_task_69db4.c).
It allocates a 0x7038-byte main-task-manager task for `sub_08069E00`, stores
its supplied object at task-relative +0x11CC, sets that object's +20
halfword to 20, and records object +0xA90 at task-relative +0x11D0. The
worker remains assembly, so these are deliberately offset-based names rather
than guessed structure fields.

The readable C is behaviorally faithful but remains outside the matching
build: agbcc's ordinary common-subexpression elimination folds the two task
addresses together and reuses the argument instead of performing the ROM's
intervening reload through +0x11CC. This is recorded as a dependency for the
worker's eventual decompilation, not worked around with volatile access,
register pinning, or inline assembly.

## Primary-runtime flag accessors

`RuntimeTestFlag` (0x08004D90) reads a selected bit mask from byte +3 of the
allocation stored in the fixed IWRAM pointer `gPrimaryRuntime`. Declaring its
public argument full-width and narrowing it into a local reproduces the ROM's
argument copy and register allocation without compiler hints. Its caller-side
adapter, `RuntimeTestFlagU8` (0x08005094), narrows both the argument and return
value and also compiles byte-exactly.

Naming the fixed 0x03004014 slot also removed that raw address from the two
existing matching accessors that use it. The symbol is now defined beside the
other IWRAM roots in `asm/iwram_symbols.s` and declared in
`include/runtime_state.h`. Both newly recovered functions replace raw
assembly, with retained assembly restarted at 0x08004DA8 and 0x080050A8.

## Map coordinate task constructor

`CreateMapCoordinateTask` (0x0806C7AC) creates a 16-byte main-manager task
for `sub_0806C7EC`, stores its signed tile coordinates at task offsets +32
and +36, and adds one pending script task. Its script-native caller now uses
the named function through `include/task_constructors.h` rather than an
address-derived placeholder declaration.

The original function's in-place signed extension is reproduced by declaring
the coordinates as `s16` parameters and copying them into `s32` locals before
task creation. This is ordinary C and produces the exact ROM instructions;
the earlier `#ifdef NONMATCHING` wrapper and its duplicated assembly have been
replaced by the matching implementation.

`RuntimeAreFirstFlagsSet` (0x0806C758), immediately before the coordinate
constructor, is also matching C now. It tests each primary-runtime flag below
a caller-supplied count and returns whether all of them are set. The ROM uses
16.16 counters for both the scanned index and number of set flags; the C keeps
that representation explicit through `FIXED_16_16_ONE` rather than replacing
it with superficially equivalent integer counters that compile differently.

The constructor's `MapCoordinateTask` worker (0x0806C7EC) is also fully
decoded. Stage 0 maps five command modes onto field-event parameters at the
fixed field coordinates (204, 92), stage 1 waits for the child event's
completion word, and stage 2 reports the signed result, balances the script's
pending-task count, and finishes. The two identical mode-0 and mode-1 case
bodies remain separately written because that is the natural switch form that
reproduces the ROM's two distinct jump-table destinations. The task header's
previously anonymous +14 halfword is now named `EngineTask.stage` while
preserving the established 32-byte ABI.

## Message-window graphics loader

`DialogueLoadWindowGraphics` (0x08011718) configures KMP viewport slot 3 for
the ornate message window stored as `MWA.KMP`. It always points the viewport
at BG character block 3 and passes its first argument through as the KMP plane.
When its second argument is nonzero it loads both the palette and tile members;
when zero it reconfigures the viewport using graphics already in VRAM.

The previously anonymous string at 0x08086D78 is now the linker symbol
`gMessageWindowMapResourceName`. The C ternary for the load flags naturally
reproduces the ROM's branch and stack argument stores, including its literal
pool and two-byte alignment tail. All assembly callers now reference the named
function, and the complete 52-byte raw assembly body has been removed.

`DialogueCreatePromptTask` (0x08011A08) is the corresponding dialogue-prompt
constructor. It creates a 56-byte main-manager task, initializes the embedded
52-byte NCD sprite, selects the `CURSOR` group from sprite resource zero, sets
its initial container/group/animation/frame tuple, and adds one pending script
task. The resource name at 0x08086D80 is now
`gDialogueCursorResourceName`. The constructor's entire 88-byte body is
matching C; `DialogueCommandPrompt` calls it by name while the prompt worker at
0x08011A60 remains assembly.

The prompt worker was decoded far enough to establish its behavior. Stage zero
waits for A, clears 3072 bytes of message-window tiles with pattern
`0x11111111`, reloads the MWA background, releases the cursor allocation, and
advances to stage 16. Stage 16 balances the pending-script count, reports -1,
and finishes. Other stages position the cursor 212 pixels right and 36 pixels
below window record 3 before advancing and queueing its NCD animation. A
straightforward C implementation produces the same 148 bytes of operations but
orders its basic blocks differently, so the worker remains assembly rather
than using control-flow tricks. This also exposed and corrected the parameter
names of `ScheduleVramFillTask`: its payload is destination, byte count, then
fill pattern, matching the eventual `CpuFill` call.
