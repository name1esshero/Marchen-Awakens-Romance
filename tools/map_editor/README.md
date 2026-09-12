# MAR map editor

Run `make map-editor`, or `py tools/map_editor/server.py` on Windows.
The server opens its full editor URL in your default browser automatically.
If browser launch is unavailable, open the printed localhost URL manually.
Use `--no-browser` to disable automatic launch.
The URL carries a random per-session token. The server binds only to 127.0.0.1;
it serves the editor UI and validated project APIs, not arbitrary files.
Stop it with Ctrl+C. A different port is available with
`python3 tools/map_editor/server.py --port 8766`.

## Editing and building

Select a map and plane, choose a tile and palette bank, and paint on the map.
Horizontal/vertical flips, layer visibility, zoom, a grid, right-click picking,
and stroke-level undo/redo are available. Select **Raw attributes** to paint
numeric u8/u16 attributes. Their collision/event meanings are not fully decoded.

The **Event calls** panel selects the same-name SPC when present, or any other
named script. Filter native calls by name and edit supported signed integer
arguments. Save writes `maps/editable/NAME.KMP.json` and, if selected,
`maps/events/NAME.SPC.json`. These are tracked source inputs. The original KMP
and SPC framing files remain the baseline; unknown fields are preserved.

Run `make` or `make english` after saving. An intentionally edited ROM should
differ from baserom. Without overrides, the default Japanese build must still
match every byte. Removing an override restores its original values on the next
build; directory dependencies detect additions and deletions as well as edits.
The English build includes the same map/event edits and its existing dialogue
translation hook; this editor does not implement a separate English map format.

Changes to the loaded map, tile art, palette, text, or event source revision
cause a stale save to fail. Invalid references and scripts that no longer fit
their compressed allocation fail before either file is written. Each file is
replaced atomically; ordinary write failures roll both files back. A machine
crash between two file replacements is not a multi-file transaction guarantee.

## Decoded formats and limits

KMP header evidence comes from the recovered viewport/attribute code at
08002604, 08003104, 08003160 and 08003178; see `include/kmp.h` and `src/kmp.c`.
Dimensions at +14/+18 count 8×8 tiles. Four plane offsets at +9C select row-major
u16 words: bits 0–9 tile index, bit 10 horizontal flip, bit 11 vertical flip,
bits 12–15 palette bank. Attribute offset +AC uses u8 or u16 entries according
to +BA. Tile and palette resource names are at +1C/+5C; palette destination
and count are at +B0/+B4. The editor uses the PNG/layout compiler for artwork.
It preserves map dimensions, plane offsets, the header, and all unknown bytes.

All 47 MAP*.KMP members load. MAP27_A references tile 1023 outside its decoded
484-tile resource. Those cells are shown in pink as unresolved, not asserted to
be blank. Existing unresolved words can be preserved or replaced with valid
tiles; the editor refuses to introduce additional unresolved references. Layer
order, blending, scrolling, and affine effects are not emulator-verified.

All 334 named SPC FUNC tables decode (33,064 native-call references).
FUNC records contain a NUL-terminated name and u32 CODE references ending in
zero. Each reference points to 29/u32 argument count followed by 80/u32 native
call. The editor allows the 13,643 calls whose integer pushes are contiguous
and follow the observed 31/0F-u32 statement prefix. Other expressions remain
read-only; these counts are from unedited source scripts, not runtime execution.
The NVAR chunk observed in G_EV023 uses a length including its eight-byte
header; FUNC and CODE use payload lengths. NVAR content is preserved read-only.

The native registry links SprSet to 08011F40 (delegating to 08010E44), HitSet to
08012208, and FldSet to 080122F8. Most sprite properties remain undecoded.
The hit-region helpers and collision test now compile as byte-matching C in
`src/hit_region.c`; their layouts are documented in `include/hit_region.h`.

### Hit regions

The engine has sixteen 16-byte records at engine state +0x1090. Each stores
an active word, signed 16-bit x/y/width/height, and a mode word. The actor's
input rectangle instead stores left/top/right/bottom corner offsets. The
collision test at 08018C4C translates those corners by the actor position,
then visits regions in index order and returns the first matching index + 1.
Zero means no hit. Mode 0 tests overlap, mode 1 strict containment; touching
edges does not count. Other mode values never hit. Negative widths and heights
are preserved by the original implementation, not normalized.

| Native call | Verified behavior |
| --- | --- |
| `HitInit(id, x, y, width, height)` | Activates the region, stores the rectangle, and resets mode to 0. Handler 080121C4 calls 08011530. |
| `HitHitRect(id, x, y, width, height)` | Activates and replaces the rectangle, preserving mode. Handler 08012244 calls 08011654. It does not perform a hit test. |
| `HitFree(id)` | Disables one region; -1 disables all sixteen. Geometry and mode remain stored. Handler 080121E4. |
| `HitSet(id, property, value)` | Updates active regions only: 54=x, 55=y, 56=width, 57=height, 61=mode. Other property values are unsupported. Handler 08012208 calls 08011554. |

The editor labels these arguments and offers **Preview selected hit rectangle**
for literal HitInit/HitHitRect calls. It projects that individual call onto the
current map, including signed 16-bit truncation, and follows edits and undo.
It does not execute the script, combine branches, or claim that a region is
currently active. Selecting another script can project unrelated geometry.

The registry's `HitGet` handler at 08012230 calls the sprite-property getter
at 08011174, which looks up a sprite through 080106C8. It does not call the
separate hit-region getter at 080115CC. The editor therefore does not label
HitGet as reading these region fields merely because of its name; that original
routing needs further investigation.

There is no event insertion/deletion, control-flow editing, or graphical warp
placement yet. Script call order and same-name associations do not establish
which calls execute in a particular scene. This remains a partial source
editor, with no emulator-verified event simulation.

## Verification

`python3 -m unittest discover -s tests -p test_map_editor.py` checks every map,
every script table, preservation of nonedited bytes, conflict detection,
compression overflow, two-file rollback, and authenticated HTTP save/reload.
HTTP tests require localhost socket access.
`python3 -m unittest discover -s tests -p test_hit_region.py` checks the C
helpers, edge-touch behavior, containment, first-hit ordering, and 500
deterministic randomized comparisons against a rectangle oracle.

`python3 tools/exercise_map_editor.py` performs a reversible real-ROM proof:
a map flip, raw attribute and SprSet immediate are saved through the editor
model and rebuilt. It checks the exact resulting map/script spans, removes the
overrides, and checks that Make detects the removal and restores byte equality.
Build subprocesses cannot read baserom. The proof refuses to overwrite existing
MAP01_A overrides and must run without concurrent source editing/building.
Results go to ignored `reports/build/map-editor-proof.json`.

For an optional real-browser interaction check, start
`python3 tests/map_editor_browser_server.py`, then run
`node tests/map_editor_browser.cjs build/map-editor-browser.json` in another
terminal with Node.js, `playwright-core`, and its Chromium browser installed.
`MAR_PLAYWRIGHT_MODULE` can point to a separately installed module. The server
uses temporary copies of two maps and a separate hit-region script, and the browser driver refuses a session
not marked as that isolated fixture. The browser also checks hit-rectangle
labels, rendered pixels, live edits, and undo. Ctrl+C removes the fixture. Screenshots
and results go to ignored `reports/maps/`. No browser package is required to
run the editor itself.

## Recording the README demonstration

The tracked `docs/media/map-editor.gif` consists of actual browser screenshots
of MAP01_A loading and scrolling in both directions. No replacement UI or
generated artwork is used. Initial browser network latency is set to 800 ms
to make the real loading state visible, then removed before scrolling.
To regenerate it, start the isolated browser fixture
described above, then run:

```sh
node tools/map_editor/record_demo.cjs build/map-editor-browser.json
python3 tools/map_editor/make_demo_gif.py
```

Recording requires Playwright with Chromium; GIF encoding requires Pillow.
These are optional documentation tools, not editor or ROM build dependencies.
Raw PNG frames and their capture manifest stay in ignored
`build/map-editor-demo/`; only the resulting GIF belongs in documentation.

The viewport dispatch at 08002630 is also recovered as byte-matching
`KmpRenderViewport` in `src/kmp.c`. It chooses 08002650 for mode zero and
08002798 otherwise, forwarding the original fixed-point coordinates. The
rendering paths themselves remain in assembly.

### Field loading

`FldSet` at 080122F8 delegates to the matching C `KmpLoadField` at 080032B8
in `src/map_field.c`. Its arguments are a resource basename and two signed
16-bit pixel coordinates. It saves the basename through 08006A1C, appends
`.KMP`, uppercases the result, and loads plane 0 into background/viewport 0
and plane 1 into background/viewport 1. Both share tile VRAM at 06000000.
The first load uses flags 3 (palette and tiles); the second uses flags 0,
reusing those resources. The coordinates are multiplied by 65536 and passed
to both viewport render calls. This is viewport positioning; actor spawning
and event transitions still need separate tracing.

The original filename buffer is sixteen bytes and uses unchecked copies;
source edits must keep the basename plus `.KMP` and NUL within that limit.
The slot stride is 0xFC bytes, with slots 0 and 1 at 03003BC4 and 03003CC0.
`KmpSetClip` (08003280) writes the four existing clip fields, and `KmpResetClip`
(08003294) restores zero origins and the KMP's tile dimensions. These also
compile to the original bytes. `tests/test_map_field.py` checks the loader's
call order, names, resource flags, slot stride, signed coordinate extremes,
and clip reset behavior using the actual C with mocked hardware calls.
