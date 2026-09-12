# English UI artwork

`make english` selects sibling `_en.png` frame/cell sources. The regular build
ignores these overrides. Both languages compile their SYSTEM containers from
PNG tiles, palettes, and readable metadata; no original container is patched in.

Nineteen title-menu frames, 1878–1892 and 1898–1901, have English artwork.
The inventory in `english.json` covers 206 user-reported Japanese frames.
New Game/Continue options reuse foreground words over shared background cells.
Batch 3 localizes those foreground layers while retaining the background tiles.

## Editing rules

- Preserve each original PNG's dimensions, indexed palette, and transparency.
  Staff credits 1700–1781 instead use the explicit English dimensions and cell
  layouts in `source/english/credits_layouts.json`.
- Palette index 0 is transparent. Letter strokes and outlines must use opaque
  indices, even when they are dark. A PNG alpha channel cannot make GBA index 0
  opaque. Do not paint a checkerboard into an image to represent transparency.
- Inspect lettering on contrasting backgrounds. Matching palettes alone cannot
  detect accidental holes in letter strokes.
- Keep all frames of an animation consistent. Shared tile edits must agree;
  the source compiler rejects conflicting visible frame edits.
- PNGs are the editable sources; rebuilding does not regenerate the lettering.

The Story Mode variants retain the original ring pixels in columns 0–31 and
208–239. Tests cover those columns, palette/alpha preservation, and the repaired
upper M stroke. The English link audit recompiles the source artwork independently
and checks the linked bytes; this does not replace checking the running game.

## Artwork provenance

The built-in image generation tool supplied the English lettering. Its final
prompt requested exact `STORY MODE` text, continuous opaque silver-white upper
fill and orange lower fill, dark outlines, the original italic pixel style,
and a solid magenta background for unambiguous keying. It explicitly prohibited
checkerboard patterns and missing ink inside letter strokes.

The selected lettering was cropped, keyed, converted to the original opaque
palette indices, and placed into the original 240×32 frames. The rings came
from the original PNGs, not the generated image. An earlier checkerboard-based
draft had a missing M fill and was replaced; it is not the current source.

Additional built-in generation prompts requested exact `ARM TRADE`, `BATTLE MODE`,
and `DUNGEON MODE` lettering with the same opaque fills and no ornaments.
Original end ornaments were retained; inactive and disabled versions use the
existing white and gray palette colors.

## Batch 2: 20 small-label frames

Frames 1820–1839 are localized (18 PNGs; two Edward frames share existing
canonical sources). Names use the extracted font's Latin glyphs and the
existing right alignment. Selected tabs retain their blue gradient and opaque
black shadow. Buy/Sell stay within the first 32 pixels because their final tile
area is shared. No pixels change between transparent and opaque in this batch.

`text_labels.json` records source paths, frame aliases, English text and styles.
`python3 tools/ui_text.py` validates these labels using the recovered font and
English character mapping. `--write` authors missing PNGs; `--replace` is an
explicit request to overwrite existing variants. Ordinary ROM builds read the
PNGs and never rerender authored labels. Both PNGs and recipes are editable.

## Batch 3: layered title options and prompts

Twenty more frames are localized: 1686, 1687, 1783, 1801, 1804, 1805,
1807–1810, 1893–1897, and 1902–1906. The ARM prompt says to **enter** a recipe,
not choose one. はつどう means activate; its compact button label is **Use**.

The title options say NEW / CONTINUE and NEW RUN / CONTINUE RUN. Sixteen
English cell PNGs preserve the original shared words and leave all background
cells unchanged. `layered_labels.json` documents which frames share each word.
Edit those `_en.png` foreground cells, then run
`python3 tools/ui_layered_text.py --refresh-frames` to update their assembled
frame previews. The compiler rejects edits that disagree between layers and
previews. Neither this refresh nor ordinary builds change Japanese assets.

The built-in image tool supplied the bold title lettering: NEW and CONTINUE
from the earlier title drafts, and a new RUN word. The RUN prompt requested
bold italic pixel letters with opaque silver/orange strokes, dark outline, and
a solid magenta key background, without ornaments or checkerboard. Conversion
preserved the original indexed palettes; the original background sprites are
composited separately. The other ten labels use the extracted game font.
`python3 tools/ui_text.py --batch 3` validates their recipes. As before, writing
or replacing authored PNGs requires explicit flags.

## Batch 4: 20 prompts and location labels

Frames 1508–1509, 1561–1564, 1802–1803, 1806, 1811–1819, and 1840–1841
are localized. Short hints use `Deck: drop`, `Deck: add`, `See`, and `Link` to
fit existing tile allocations. Shared START/SELECT sprites and the common
`Deck:` prefix retain consistent pixels. Link-status text fits the existing
message boxes, including the three-line cable-error message.

Location labels are Ruins, Jack's House, Ice Castle, and Pazu Town; Pazu follows
the script translation spelling. Ice Castle retains the original F marker and
the entire reserved area from column 56 onward. A generated draft was too
cramped there, so its final narrow lettering uses intact game-font columns
with integer vertical scaling, blue fill, and an opaque white outline.

The built-in image tool supplied the other three location-label edits. Prompts
requested each exact English name, cyan/blue gradient pixel lettering, an
opaque white outline, a solid magenta key background, and no floor markers or
extra artwork. Final indexed PNGs preserve the original dimensions and palettes.
Seventeen font-based recipes are in `text_labels.json` (`--batch 4`).

The Link Battle source uses palette index 15 for its Japanese lettering,
unlike the index-14 Buy/Sell labels. Its recipe records that distinction so
reauthoring clears the Japanese text correctly. A regression check covers the
residual-ink problem as well as the preserved icons, borders, and F marker.

## Batch 5: forty native-font labels

Frames 0537–0538, 0700–0706, 0790–0794, 1510, 1565–1578,
1581–1590 and 1782 use the recovered Latin glyphs. The authoring recipes in
`text_labels.json` specify opaque ink, original palette colors, and bounded
text regions. Native glyphs retain whole pixel columns; location names use
integer vertical scaling and the original blue/white palette. Gate titles
retain their yellow/red palette, and pause-menu labels retain red/white.

Popup options keep their original row positions and cursor margins. Long
options use short labels: Recipe for ARM recipe, Drop for Discard, and No save
for Don't save. The quantity header (0700) reads Owned. Vestri Underground Lake
uses Vestri Lake on screen. Gate floor labels omit the repeated word Gate to
fit while preserving the original BF markers and their placement.

Perika, Vestri and Alter prefixes share tiles between maps; their English
regions agree exactly. All five related gate floor labels are included together
because they share the final Japanese character tile. Their BF tiles remain
unchanged. The source compiler checks shared-tile conflicts, and the English
ROM audit checks for visible changes outside authored variants.

Batch 5 brought the inventory to 119 authored frames out of 206.

## Batch 6: remaining 87 reported frames

All 206 reported frames have authored English variants. The final batch covers
0539–0541, 1685, 1698 and 1700–1781. Thirteen credit-name readings are provisional,
as explicitly requested by the user; `credits_translation.json` and the main
inventory identify each one. The original Japanese names remain alongside the
readings. The game-specific GameFAQs credit list and linked staff profiles
support the other romanizations; these are not claims of an official English
localization. More text-bearing graphics may exist outside the reported set.

The material panel preserves its original counter columns, borders, separator
and transparency mask. Generated background reconstruction was used only under
old bright text pixels; all other purple texture pixels remain original. The
selected imagegen source was `exec-216e7d71-a8ab-486e-95ec-357a699bbf25.png`.
The editable textless authoring base is `source/english/1685_background.png`.
Owned sits inside the cyan tab at rows 4–11; the old lettering and shadow are
cleared at rows 3–11. A test protects this placement and surrounding pixels.

Staff credits use native-font PNGs plus English-only OAM layouts. Their Japanese
cells share surnames and leave gaps between names, so editing flat PNGs inside
the original cell coverage cannot fit full Latin names reliably. The English
layouts retain the same 82 frame records and 253 cell records, and allocate
1,200 of the existing 1,644 isolated credit tiles (29944–31587). Long names wrap
at word boundaries across two rows. No added palette, archive resizing, original
ROM reads or binary fallback is used. All 82 assembled renders are checked
against the source PNGs during compilation.

The English link audit permits only the declared credit position/shape bytes,
tile indices and derived cell-tile reference count in addition to artwork and
the existing dialogue bridge. Every palette and all other metadata must remain
identical. Japanese builds ignore the English layouts. The new credits and
artwork still need an in-game review; static audits do not establish playback.
