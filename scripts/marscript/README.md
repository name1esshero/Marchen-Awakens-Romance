# Editable script overrides

Full script rewrites via marscript (see `docs/marscript-language.md`), one
level above `maps/events/`'s narrow integer-only argument editor: a
`<NAME>.marscript` file here (matching a `scripts/nfp/manifest.json` entry's
`name`) replaces that script's *entire* logic for `mar_english.gba`, not
just its literal arguments -- write it by hand, or start from
`python3 tools/marscript.py decompile scripts/nfp/<NAME>.bin <NAME>.marscript`.

`tools/marscript_rom_build.py` compiles an override and, if it still fits
the script's original archive slot, places it there unchanged in size; if
it grew past that slot, the original bytes stay in place (so nothing else
in the ROM shifts) and the new content is placed in the English build's ROM
expansion region instead, via `tools/split_scripts_english.py` and
`tools/resource_catalog.py`'s `build-english` mode.

This directory, and everything it drives, is used for `mar_english.gba`
only. `mar.gba` never reads it and is unaffected by anything here, by
design: it has no expansion region to grow into, and its whole purpose is
to match `baserom.gba` byte for byte (PRET_STANDARDS.md's Golden Rule).
Removing an override here restores that script to its original bytes.
These `.marscript` files are source inputs, not generated reports.
