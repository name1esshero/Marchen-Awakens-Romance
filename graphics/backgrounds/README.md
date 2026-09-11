# Editable mapped backgrounds

Open [the image browser](index.html). Eighty-two resources now have 116
editable image layers using KMP maps or affine TSC frames.
For example, [OP_05_A.KCG.png](OP_05_A.KCG.png) is the complete 240×160 picture
of Ginta and Babbo. OP_06_A is 240×256; other resources use their own map sizes.

Edit these PNGs directly. The build converts the assembled pixels back into
the tile indices, horizontal/vertical flips, and palette banks expected by the
map. For KMP resources, the JSON beside each primary image records dimensions and
each plane's u16 map entries. Bits 0–9 select a tile, bits 10/11 flip it, and bits 12–15 select
its palette bank. Additional nonempty planes have `.layer1.png` etc. names;
these are separate layers, not a claim about their final on-screen priority.

The mapped images replace the old tile-sheet PNGs as build inputs. Small
`unused/` strips preserve tiles not referenced by the rendered planes; deleting
one fails the build. The image compiler reconstructs every tile from the PNGs,
without reading the original ROM, compressed tile stream, or a hidden tile sheet.
Baseline hashes only distinguish edited copies of shared tiles; hashes cannot
recover pixel content. All LZ77 streams are generated from reconstructed pixels by the matching
VRAM-safe encoder. No original compressed source is used. Edited streams must
fit their fixed ROM spans.

A tile can appear more than once. Editing one occurrence changes that tile
wherever the game uses it. Incompatible edits to separate copies are rejected;
giving those copies independent artwork would require allocating more tiles
and changing the map. Keep each cell's pixels within its assigned palette bank.
Edit the linked `graphics/palettes/*.pal` file to change the game's colors.

Preview the compiled primary layer, including propagated shared-tile edits:

```sh
python3 tools/mapped_images.py preview --asset OP_05_A.KCG --output /tmp/OP_05_A.png
make -j4 compare
```

Map JSON edits are written into the corresponding KMP planes during the build.
The raw KMP source retains unknown header and attribute data; this is a partial
format reconstruction, not a complete scene engine. Width/height changes are
rejected because they require reallocating the map's fixed spans.

## Evidence and remaining cases

The renderer at 08002650 selects a plane through KMP+0x9C and uses dimensions
at +0x14/+0x18. The loader at 08003178 reads the KCG/KCL names at +0x1C/+0x5C
and palette destination/count at +0xB0/+0xB4. The recovered header is in
`include/kmp.h`; viewport initialization and attribute addressing now match
in `src/kmp.c`.

The original [KMP migration report](../../reports/graphics/mapped-image-migration.json)
listed 27 exceptions. The [affine migration](../../reports/graphics/affine-image-migration.json)
resolves five, leaving 22 resources needing another map profile. These include regular TSC cases,
missing same-name KMPs, and unresolved tile or palette references. OP_07_A
references tile 1023 outside its own tile resource and remains unresolved.
Those resources retain their tile-sheet sources rather than guessed pictures.

`tools/exercise_mapped_images.py` checks a clean build, an actual PNG edit and
an independent map flip in the final ROM, then restores both sources.
Run it alone after the unit tests have finished.

## Affine TSC frames

NE01, NE13, NE33, SK02 and SK03 now build from 15 individual mapped frames.
Their JSON uses `format: affine_tsc_u8`: each entry is a byte tile index,
without flip or palette-bank bits. Each frame names its separate TSC source.
Pixels remain 8bpp indices, including the original palette placement; colors
come from the named TCL `.pal` file. Shared tile edit rules apply across frames.
Unused tiles remain in the narrow strips so the tile payload rebuilds exactly.

NE01's loader at 080411A4 confirms BG2CNT 188A, a 128×128 affine map, and a
96-byte palette copy to color 192. Other four resources use complete square
byte-index planes: 128×128 for NE13/NE33/SK02 and 256×256 for SK03. Those
shapes are inferred from their plane sizes and valid references. The earlier
claim that every TSC was a 2048-byte screenblock was incorrect.
