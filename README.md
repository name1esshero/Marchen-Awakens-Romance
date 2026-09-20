# MAR — Knockin' on Heaven's Door

This project is a work-in-progress decompilation of the Japanese Game Boy
Advance game. It builds the Japanese ROM byte for byte from checked-in source
and also provides an optional English build, editable graphics and sound, and a
visual map editor.

The original ROM is not included. A local `baserom.gba` is optional for normal
builds and required only for byte comparison and extraction.

## Requirements

- Python 3
- GNU Make
- `arm-none-eabi` binutils and preprocessor
- [agbcc](https://github.com/pret/agbcc)

On Debian, Ubuntu, or WSL, install the ARM tools with:

```sh
sudo apt-get install build-essential python3 binutils-arm-none-eabi gcc-arm-none-eabi
```

Build and install agbcc into this project once:

```sh
git clone https://github.com/pret/agbcc /tmp/agbcc
cd /tmp/agbcc
./build.sh
./install.sh /path/to/Marchen-Awakens-Romance
```

The installer creates `tools/agbcc`, which is a local ignored dependency. The
headers under this project's `include/` directory are project source and remain
tracked.

## Build the game

Run these commands from the project root:

| Command | Result |
|---|---|
| `make -j4` | Build the byte-matching Japanese ROM as `mar.gba` and print ROM/RAM usage |
| `make -j4 english` | Build the translated ROM as `mar_english.gba` |
| `make compare` | Build `mar.gba` and compare every byte with `baserom.gba` |
| `make test` | Run the complete host-side test suite |
| `make test-english` | Run the English translation and runtime tests |
| `make ci` | Build Japanese and English ROMs and run the tests used by GitHub Actions |
| `make check-modern` | Check C sources with modern GCC without using its output in the ROM |
| `make clean` | Remove generated builds and ROM files |

A matching Japanese reference ROM has SHA-1
`5ed178bfbdf459867d64e5b91a9d9c72654e4051` and is 16,777,216 bytes. Name it
`baserom.gba` and place it in the project root. The build does not patch or copy
this file; `make compare` uses it only as the reference.

The English build is selected with the `english` target. You do not need to set
`ENGLISH` manually:

```sh
make -j4 english
```

## Use the map editor

![Map editor loading a field, changing pages, and previewing a moving event](docs/media/map-editor.gif)

Start the editor with:

```sh
make map-editor
```

On Windows, this also works from the project directory:

```powershell
py tools/map_editor/server.py
```

The server opens the editor in the default browser. If it cannot open a browser,
use the localhost URL printed in the terminal. Use `--no-browser` to prevent
automatic opening or `--port 8766` to choose another port.

Basic workflow:

1. Select a map from the catalog on the left.
2. Use the **Map**, **Collision**, **Connections**, and **Events** tabs to edit or
   inspect that part of the map.
3. Scroll over the viewport to zoom, use **Fit whole map** to see everything,
   and drag to pan.
4. In **Events**, select a decoded script and press **Play** to preview supported
   sprite movement.
5. Press **Save sources** to write editable source overrides.
6. Run `make -j4` or `make -j4 english` to compile the saved changes.

The editor saves source files and never edits a ROM directly. Some runtime
expressions, branches, dynamic positions, new-event creation, and warp behavior
remain under development. See the [complete map editor guide](tools/map_editor/README.md)
and [editor roadmap](docs/map-editor-roadmap.md).

## Edit graphics

Editable graphics are under `graphics/`. Important locations include:

| Content | Source location |
|---|---|
| Character frames and cells | `graphics/battle/characters/` |
| Battle effects | `graphics/battle/effects/` |
| UI and item artwork | `graphics/ui/` |
| Portraits and icons | `graphics/portraits/`, `graphics/icons/` |
| Backgrounds and layouts | `graphics/backgrounds/` |
| Tilesets and palettes | `graphics/tilesets/`, `graphics/palettes/` |
| Font | `graphics/fonts/font.png` |

Use indexed PNGs and preserve their palette indices and dimensions. English-only
artwork uses an `_en.png` suffix and is included only by `make english`.
Generated `.lz`, `.4bpp`, NCD, and other intermediate files belong under
`build/` and are removed by `make clean`.

Useful graphics commands:

| Command | Purpose |
|---|---|
| `make graphics` | Rebuild staged graphics from editable sources |
| `make sprite-previews` | Refresh sprite frames and animation galleries |
| `make graphics-audit` | Validate graphics sources and classifications |
| `make font-preview` | Create `build/font-preview.png` |
| `make galleries` | Regenerate local graphics, sound, and text browsers |

See the [graphics editing guide](graphics/README.md) for cell sharing, palettes,
animation tables, and background layouts.

## Edit text and scripts

- Shared English translations: `text/translation/english.json`
- Scene-specific translations: `text/translation/scripts.json`
- Extracted script text and comments: `text/nfp/*.SPC.txt`
- ÄRM and item text: `text/arm_definitions.txt` and `text/item_definitions.txt`
- Font character mapping: `charmap.txt` and
  `text/translation/english_font.json`
- Custom script sources: `scripts/source/*.json`

Run these after editing:

| Command | Purpose |
|---|---|
| `python3 tools/translate_comments.py` | Apply translation mappings to script comments |
| `python3 tools/english_layout.py --audit` | Check English glyphs and textbox widths |
| `make script-sources` | Compile custom JSON scripts into SPC files |
| `make script-catalog` | Rebuild the script/map catalog |
| `make map-audit` | Rebuild the catalog and audit map sprite references |
| `make test-english` | Test English mappings, layout, and runtime behavior |

English text is wrapped at word boundaries and split into button-advanced pages
when needed. See the [translation guide](text/translation/README.md) and
[script language reference](docs/script-language.md).

## Edit sound

Editable driver-referenced samples live in `sound/samples/*.wav`, with format,
loop, and address metadata in `sound/samples/manifest.json`. Ordinary builds
rebuild these samples automatically.

```sh
python3 tools/sound_assets.py build
```

See the [sound source guide](sound/README.md) and
[sound engine notes](docs/sound-engine.md).

## Other project tools

| Command | Purpose |
|---|---|
| `make readability-audit` | Report unexplained magic numbers in C sources |
| `make pret-audit` | Audit mechanically enforceable PRET source standards |
| `make site` | Stage galleries and documentation under `build/site` and `build/wiki` |
| `make docs-fetch` | Restore missing published gallery/report outputs |
| `make snapshot` | Create a checksummed recovery archive under `backups/` |
| `make extract` | Re-extract and regenerate sources from `baserom.gba` |

`make extract` is a full recovery operation and can replace extracted assets.
Use normal build targets for day-to-day editing.

## Project layout

| Path | Contents |
|---|---|
| `src/` | Matching readable C used by the ROM build |
| `src/nonmatching/` | Readable C candidates that are not linked into the matching build |
| `asm/` | Assembly that has not yet been replaced by matching C |
| `include/` | Shared C headers and recovered structures |
| `graphics/` | Editable artwork, palettes, layouts, and animation data |
| `sound/` | Editable samples and sound metadata |
| `text/` | Japanese text, English mappings, and font mappings |
| `maps/` | Named map sources and editor overrides |
| `scripts/` | Named and custom script sources |
| `tools/` | Build, extraction, audit, and editor programs |
| `tests/` | Host-side regression tests |
| `docs/` | Format references and detailed technical findings |
| `build/` | Disposable generated output |

More technical documentation is available in
[docs/decompilation-notes.md](docs/decompilation-notes.md),
[docs/PRET_AUDIT.md](docs/PRET_AUDIT.md),
[docs/map-and-script-runtime.md](docs/map-and-script-runtime.md),
[docs/game-flags.md](docs/game-flags.md),
[docs/save-format.md](docs/save-format.md), and
[docs/ram-layout.md](docs/ram-layout.md).
