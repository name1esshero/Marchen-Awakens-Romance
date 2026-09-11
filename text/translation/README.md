# Translation sources and dialogue layout

`english.json` maps exact Japanese strings to reviewed English wording.
`scripts.json` overrides these per SPC filename for context-sensitive dialogue.
`tools/translate_comments.py` applies comments and synchronizes legacy script
views. Comments do not change Japanese ROM text. Old `{kanji}` romanization
is not a translation and is not accepted as reviewed English.

## Actual printer findings

The three-row dialogue task is created at 08011790. Its reader at 08011AF4
accepts double-byte characters, consumes `Cxxxx`/`cxxxx` as ink/shadow colors,
and consumes `Txxxx`/`txxxx` as a delay. Ordinary ASCII is skipped, including
ASCII spaces. A NUL advances to the next row; an ASCII newline does not.
The glyph cursor is advanced by the drawing task at 08011870, not the reader.
The constructor uses unbounded copies into three 161-byte row buffers.

The renderer at 08011688 uses a 192-pixel row stride. Dialogue advances by
nine pixels per eight-pixel glyph, with a one-pixel shadow offset. A conservative
limit is 21 glyphs per row. A name occupying the first row leaves two body rows.
The old reconstruction at 080322CC was incorrectly named a text printer: it
creates an object command task and receives an argument block, not a string.
The four-hex-digit helper now matches in `src/dialogue.c`. The actual reader
is linked as byte-matching C in `src/dialogue.c`; its constructor is in
`src/dialogue_start.c`.

## English preparation helper

```sh
python3 tools/english_layout.py "Ginta, let's go!"
python3 tools/english_layout.py --audit
python3 tools/english_layout.py --rows 2 "We must save everyone."
```

`english_font.json` explicitly maps English characters to double-byte font
codes. The helper validates every mapped slot against the editable font PNG,
wraps at word boundaries, and emits separate NUL-terminated rows. Unmapped
characters, overlong words, and messages needing another page are errors.
It never truncates text or inserts unverified script opcodes. Repeated ASCII
spaces are normalized during word wrapping. The fullwidth Latin glyphs avoid
the reader interpreting the letters C and T as commands.

The helper also supplies the optional English ROM described below. Complete
localization still needs decoded script references/control flow, pagination,
other UI printer profiles, and in-game testing. The 8,640 extracted SPC records are
framing candidates, not proof of exhaustive text discovery. Item descriptions
outside SPC files and false positives in old raw scans must be accounted for
separately. See `reports/text/audit.json` for current annotation counts.

## Optional English dialogue ROM

Run `make -j4 english` to build `mar_english.gba` (32 MB including ROM
expansion). `make compare` still builds the original matching Japanese ROM.
`ENGLISH=1` is defined when compiling the localization constructor and runtime;
the explicit target selects the separate linker inputs. Merely defining a
preprocessor macro on the Japanese build does not select the extension.

`tools/build_english.py` compiles exact Japanese byte strings and reviewed
`// EN:` comments into a sorted C table. It validates each English character
against the extracted font, wraps at 21 glyphs, preserves leading C/T controls,
and rejects embedded controls that lack translated markup. Conflicting
translations of the same byte string are excluded until script identity is
available at the hook. Edit the scoped dictionary in `scripts.json`, then run
`python3 tools/translate_comments.py`; never hand-edit generated mappings.

`src/english/dialogue_runtime.c` intercepts the real constructor at 08011790.
Mode 0 permits three rows; mode 1 reserves the first row and permits two. If
any source row is missing or the combined English rows exceed that capacity,
the entire message uses its original Japanese rows. It never truncates a
translation to make it fit. Resource names and script operands stay untouched.

This is an initial, partial language build. Pagination, context-specific
lookups, other text printers, and a full emulator playthrough are unfinished.
Host tests exercise the actual C lookup, row bounds, fallback and constructor
arguments against every accepted mapping. They do not prove every scene is
translated or every script string reaches this printer.
