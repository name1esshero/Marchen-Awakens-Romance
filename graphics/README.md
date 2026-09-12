# Graphics source workspace

Open [the graphics index](index.html). The old `graphics/ncd` directory is no
longer used or present. NCD files exist only as generated build output under
`build/graphics/ncd`.

## Sprite sources

The character, effect, and UI categories share the same structure:

| Directory | Contents |
|---|---|
| `battle/characters/` | CHR.NCD character graphics and animations |
| `battle/effects/` | EFFECT.NCD battle effects |
| `ui/` | SYSTEM.NCD interface, item art and portrait resources |

Inside each category:

| Path | What to edit |
|---|---|
| `frames/*.png` | Assembled frames; open them through the animation browser |
| `cells/RESOURCE/tile_NNNNN.png` | Individual, unflipped sprite-cell images, grouped by original resource name |
| `palettes/NNN.pal` | Original 16-color palette banks, in JASC format |
| `source/groups.json` | Named resources and their animation ranges |
| `source/animations.json` | Frame sequences, expressed as first frame and count |
| `source/frames.json` | Cell ranges and durations for every animation frame |
| `source/cells.json` | Cell positions, dimensions, flips, palette/tile indices and unknown fields |
| `source/images.json` | Cell image paths and their tile-pool placement |
| `source/container.json` | Signature, reserved fields, palette/tile counts and fixed ROM allocation |

Use indexed PNGs with indices 0–15 and preserve their dimensions. Change
colors in the linked `.pal` files; embedded PNG palettes are viewing aids.
Cell tiles may be reused with other palette banks or flips.

The gallery links each frame to its component cell images and these tables.
Cell images retain pixels hidden beneath other cells; flattening a frame alone
would discard that information. All tile bytes are supplied by the individual
images, with exact coverage and no overlapping primary image ranges.

There are 12,062 primary cell images: 5,694 character, 2,842 effect, and 3,526
UI images. They cover all 32,265 cell records through tile reuse. All 9,252
animation-frame records remain present, with 6,627 assembled PNG editing views.
Shared frame paths preserve independent animation durations and sequence entries.

### Building and previewing

```
make build/graphics/ncd/CHR.ncd
make build/graphics/ncd/EFFECT.ncd
make build/graphics/ncd/SYSTEM.ncd
make sprite-previews
make compare
```

Each container has its own Make rule and dependencies. `tools/sprite_sources.py`
constructs it from a zeroed buffer, serializes the JSON tables, calculates their
16-byte alignment and derived header counts, encodes palette files, and packs
cell pixels into tiles. It never reads an original NCD, a whole-pool tile sheet,
or the base ROM during compilation.

Assembled-frame, portrait and icon edits are applied to the reconstructed cells.
An unchanged view does not mask cell edits. Pixel hashes identify edited views;
they do not contain replacement pixels or recover original bytes. Conflicting
changes to shared pixels fail. If editing a cell and one of its assembled views
would disagree, choose one view for that edit. Use cell images to edit hidden
pixels or to change artwork that cannot be represented by an assembled frame.

`make sprite-previews` refreshes unchanged views from current cell/palette/table
sources and preserves authored frame edits. Playback FPS remains a viewing
control; exact runtime affine/blending behavior and timing are not reproduced.
Changing source table counts or image dimensions can exceed the fixed ROM
allocation and is rejected. Changing values within the existing layout is
supported and deliberately changes ROM bytes.

`tools/sprite_sources.py extract` is an explicit ROM extraction operation.
It recreates tables from the original ROM and is not an ordinary build step.

## Other graphics

- `portraits` and `icons`: convenient named editing views of SYSTEM sprite cells.
- `backgrounds`: 77 mapped backgrounds with 101 editable image layers and
  JSON map layouts, plus a browser for all 104 KCG/TCG resources.
- `tilesets`: 27 resources awaiting map profiles, named after their archive members;
  their authoritative palettes live under `palettes`. Old size-based sprite
  labels and tilemap guesses have been corrected.
- `tilemaps/nfp`: editable named TSC screenblocks.
- `fonts`: editable font image, original NFT header source, and glyph indexes.

HTML, contact sheets, and report previews are derived browsing material.
The obsolete `reports/graphics/legacy` images and streams have been removed.
`reports/graphics/retired-scan.json` retains their classification evidence.
Run `python3 tools/audit_graphics_sources.py` to verify every active PNG has a
known source or preview role; the inventory distinguishes source images, extra layers, unused tiles and previews.
See [the mapped-background guide](backgrounds/README.md) for image editing.

### Retired RLE scan output

`assets_rle/` and its build manifest have been removed. The 68 heuristic hits
were not verified independent graphics assets. Two former image candidates
were inside already decoded CHR.NCD and MAP26_5A.KMP data. The only linked
candidate, ROM `0x1B0720..0x1B2BB6`, is now preserved honestly as unclassified
bytes in `data/data_1B0720.bin`; its purpose and format remain unresolved.
It is not presented as an editable image or as a proven compressed asset.

`reports/graphics/rle-retirement.json` retains the candidate offsets, sizes,
original stream hashes and archive ownership. `tools/extract_rle.py baserom.gba`
now writes only `reports/graphics/rle-candidates.json`. Decoding successfully
does not establish an asset boundary, palette or runtime consumer. The BIOS
RLE codec remains available in `tools/rle.py` for future verified uses.

### Sprite cell coordinates

NCD cell table X/Y values are centers. The compositor subtracts half the cell
width/height, as the engine does, before placing pixels. Frame PNGs and viewer
origins use those corrected bounds. The build uses the same bounds to map edits
back to cell tile bytes; do not manually shift cell centers to compensate for
an older preview. Unchanged corrected PNGs rebuild the original NCD bytes.

The sprite animation viewers now offer **Japanese / English overrides** artwork selection. English mode shows the 206 translated UI frames and nine battle-effect frames where overrides exist, retaining original artwork elsewhere. Credit frames use their English-specific layout dimensions and origins. This viewer does not simulate runtime affine effects or blending.
