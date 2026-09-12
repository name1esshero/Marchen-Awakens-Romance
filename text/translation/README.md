# Translation sources and dialogue layout

`english.json` maps exact Japanese strings to reviewed English wording.
`scripts.json` overrides these per SPC filename for context-sensitive dialogue.
`tools/translate_comments.py` applies comments and synchronizes legacy script
views. Comments do not change Japanese ROM text. Old `{kanji}` romanization
is not a translation and is not accepted as reviewed English.

## Annotation completion

The remaining 1,055 named-script records have been translated. The current
334-script extraction now has 5,562 reviewed translation records, 3,073 ASCII
literals, five formatting-only records, and zero pending translations. This
batch includes 114 numbered portrait-debug labels as well as dialogue,
exploration, shop, unlock, and opening messages. Names remain transliterations
unless their spelling was already established in the project glossary.

An explicit empty string in `scripts.json` produces a bare `// EN:` comment.
This is used for the two standalone Japanese object-particle `を` records in
DITEMCOM, which contribute no English text. It is distinct from a missing
mapping and compiles to an empty, NUL-terminated English row. It does not insert
an explanatory placeholder into the game. The audit reports these two entries
as `empty_english_comments` within its reviewed translation count.

The English runtime table currently accepts 3,089 exact source-row mappings.
109 ambiguous or unsupported keys remain excluded, with Japanese fallback;
annotations being complete does not remove the hook's context and printer
limitations. All 5,562 annotations pass font and word-width validation; four
need multiple pages. Exhaustive ROM text discovery and emulator verification
remain outstanding.

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
python3 tools/english_layout.py --pages "A longer message can continue onto another page."
```

`english_font.json` explicitly maps English characters to double-byte font
codes. The helper validates every mapped slot against the editable font PNG,
wraps at word boundaries, and emits separate NUL-terminated rows. Unmapped
characters and words longer than 21 glyphs are errors. The single-page preview
rejects overflow; `--pages` previews every page, and the English ROM uses all
wrapped rows (up to 255 per mapping).
It never truncates text or inserts unverified script opcodes. Repeated ASCII
spaces are normalized during word wrapping. The fullwidth Latin glyphs avoid
the reader interpreting the letters C and T as commands.

The helper also supplies the optional English ROM described below. Complete
localization still needs decoded script references/control flow,
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
any source row is missing or ambiguous, the entire message uses its original
Japanese rows. Longer translations use an English parent task that feeds each
page to the original renderer and waits for a fresh A press between pages.
It holds one extra VM pending-operation count until the final page finishes,
so the script cannot advance during a page break. Final-page dismissal still
uses the script's original prompt command. Inter-page waits currently have no
animated cursor; the page remains visible until A is pressed.

The VRAM queue clears the old page before the next printer task starts. Mode 1
preserves the reserved first row and its shadow. Colors
and delay controls carry across pages, and temporary allocation failures retry
without skipping text. All task state lives in allocated RAM payloads; the ROM
extension does not rely on an unallocated BSS section. Original fixed-size row
buffers and signed cursors are respected. Resource names and script operands
stay untouched.

This is an initial, partial language build. Context-specific
lookups, other text printers, and a full emulator playthrough are unfinished.
Host tests exercise the actual C lookup, row bounds, fallback, page scheduling,
style carryover, allocation retries and completion counters. Mapping output is
checked against every accepted key. They do not prove every scene is
translated or every script string reaches this printer.

The native show/prompt/finish commands and finish-task helpers now match in
`src/dialogue_commands.c`. The prompt command creates the `CURSOR` resource
and input-wait task at 08011A08; the full drawing and input state machines
still remain assembly. These commands do not establish complete VM coverage.

The VM pending-counter helpers at 0807E420/0807E448 and the original queued
VRAM fill callback at 0800380C now match as readable C in `src/script_tasks.c`.
The VM pointer is at 0300611C, its execution-state pointer is at +0x0C, and
that state's pending counter is at +0x220. The task manager allocates a cleared
32-byte header followed by the requested payload; callback/result fields are
at +0x14/+0x18. `FinishTask` marks a task for removal by its scheduler.

The betrayal in `EV_T3KIL.SPC` and its aftermath in `G_EV062.SPC` now have
English comments for all extracted dialogue, including private ÄRM/heart codes
and inline-color text. Translations are scoped to each scene. Inline-color
records still fall back to Japanese in the English ROM until translated
formatting markup is supported; annotation coverage is not runtime coverage.

`END2.SPC` now has reviewed English comments for all extracted dialogue. Five
blank/color-only records across the named scripts use `FORMAT:` annotations;
they count separately from translated dialogue, ASCII resource labels, and
pending text. The audit rejects a `FORMAT:` annotation that conceals text.
