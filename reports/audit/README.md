# Current audit status (2026-09-12)

The sections below describe historical batches; their counts and runtime limitations are not current totals.

Fresh checks confirm complete linked coverage of all 16 MiB (1,239 sections, no gaps or overlaps), original-ROM equality, 830 distinct archive payload offsets, 104 named graphics compressed round trips, and 115 PCM samples reproducing 1,139,745 sample bytes. These checks do not establish visual correctness, complete discovery, or complete sound sequencing.

Current linked C provenance is 89 ordinary C functions (3,888 owned bytes) and 10 BIOS wrappers. Ten named tile resources still lack verified assembled layouts. See `../rom/coverage.json`, `../graphics/tileset-layout-audit.json`, and `../build/decompilation-progress.json` for current evidence.

English overrides are now explicitly accounted for by the graphics coverage checker; variants must belong to a build-consumed cell/frame source and retain its palette and alpha table. Runtime pagination exists, but static audits cannot establish complete playthrough coverage.

---

# Setup corrections and remaining limits

Regenerate [setup.json](setup.json) with `python3 tools/audit_setup.py`.
The checks read the original ROM and editable sources; they do not modify
either or repair a mismatch.

The NFP directory contains 830 members. Ownership checks expose 148 of 149 old
sound guesses as overlapping named graphics/map/font data. The remaining
FE2C20 candidate contains ARM instructions and pointers and is retained as
unclassified data. The actual sound-driver references lead to 115 PCM samples,
now compiled from WAV/JSON. All 1,139,745 PCM bytes match the original.

The historical disassembly scan predicted **7,638 function starts inside these
sample spans**. These are excluded from code emission, which now includes the
built audio instead. The historical analysis is retained for traceability;
its function count must not be used as a decompilation progress metric.
Preserved Thumb literal directives fell from 10,216 to 832 during this batch.
Neither count represents newly decompiled C. Linked providers are checked
separately in `reports/build/linked-providers.json`.

The raw text scan contains 936 records: 760 outside the named archive, 160
inside SPC members, 14 inside KMP maps, and two inside NCD sprite containers.
The latter 16 are not verified text. Resource names inside SPCs are legitimate
strings but are not necessarily spoken dialogue. Named sources are authoritative
for SPC edits; old ROM-address views can duplicate them.

All old `{kanji}` placeholders in text views have been removed. Actual item
descriptions and additional script/resource strings received English wording;
unverified graphics scan hits are explicitly marked unverified. Remaining
untranslated named records retain TODO comments. This is not a claim that all
text is translated or that the framing scanner found every string.

Recovered C structures were corrected for object size/field offsets and the
native object task's argument pointer, mode conversion, result slot and task
constructor signature. The object task had incorrectly been called a text
printer. The actual dialogue reader is reconstructed separately, with explicit
ASCII-command handling and an English glyph/layout preparation tool. Full VM
decoding, English runtime integration and in-game validation remain unfinished.

Build checks now generate C header dependencies and list required WAV inputs
from the sample manifest. A forced agbcc rebuild matches with Python original-ROM
reads blocked. Deliberate edits to six source files alter exactly seven expected
ROM bytes, including a derived audio loop guard. Restoring the sources restores
the matching ROM. See `reports/build/provenance.json` for the recorded evidence.

## Follow-up source cleanup

The obsolete `reports/graphics/legacy` folder has now been removed: 621 files
and seven redundant staged compressed entries. None were referenced by ROM
assembly. Classification records remain in `reports/graphics/retired-scan.json`.
The extraction tools no longer recreate heuristic image dumps.

All 104 named KCG/TCG inputs use archive filenames. Seventy-seven now have
101 mapped image layers in `graphics/backgrounds`; 27 await other map profiles.
This corrects 23 old sprite labels and four tilemap labels based on pixel
statistics. `reports/graphics/source-coverage.json` accounts for all 19,106
active PNGs by source/view/preview role, with no missing or untracked files.
That coverage does not establish exhaustive discovery of assets outside the
known sources or resolve the internal fields of KMP maps.

A further 119 named-script records received reviewed English comments. The
character decoder, four-digit dialogue formatting parser, and pulse-counter
update now compile to their original bytes with agbcc. See the linked-provider
audit for the current 50 ranges (40 ordinary C and ten BIOS assembly wrappers).
The clean rebuild with the legacy folder absent is recorded in
`reports/build/clean-rebuild.json`.

The KMP tracing also identifies the old Entity accessors as background viewport
accessors. `include/kmp.h` documents the recovered header; `src/kmp.c` supplies
matching viewport initialization and 16-bit attribute addressing. Readable map
planes now drive image compilation and KMP rebuilding for the migrated sources.
See `graphics/backgrounds/README.md` and the mapped-image mutation proof.
