# Build provenance and matching limits

The original ROM and rebuilt ROM are compared as complete byte arrays by
`tools/compare.py`. SHA-1 is printed as a convenience; equality is not decided
by a checksum shortcut. The final ROM is produced from linked objects with
`objcopy`, without a post-link original-ROM overlay.

## Checks performed

- Forced `make -B -j4 all`, including the agbcc compile steps. Python access to
  the original ROM was blocked during every build subprocess. The result
  matched the original.
- Changed a C field read, an assembled frame pixel, a primary cell pixel,
  a JSON frame-duration entry, a font pixel, and a looping WAV sample. The final
  ROM changed at exactly seven expected byte offsets (the audio change also
  updates its interpolation guard). See `provenance.json` for the offsets.
  The comparator returned exit status 1.
- All six source files are restored byte-for-byte by the test's `finally`
  block, followed by a rebuild and full comparison. See the recorded result
  in `provenance.json`.
- These checks run with `graphics/ncd` absent. The NCD outputs are compiled
  from individual cell images, frame editing views, palettes and JSON tables.
- Checked every range declared in `src/decompiled.json` against its linked
  object provider, ROM address, and section size. The 82 declared ranges
  comprise 72 compiled-C ranges and 10 BIOS inline-assembly wrappers. A range
  can contain multiple contiguous functions and alignment bytes.

See [provenance.json](provenance.json), [mutation-compare.log](mutation-compare.log),
and [linked-providers.json](linked-providers.json). Reproduce the active test
with `python3 tools/exercise_build.py`; run the read-only provider audit with
`python3 tools/audit_provenance.py` after a matching build.

The latest clean rebuild is recorded separately in [clean-rebuild.json](clean-rebuild.json).
It removes all generated build outputs, then builds with the retired graphics
folder absent and Python reads of the original ROM blocked.

## What a matching build does not prove

This remains an incomplete decompilation. Extracted binary data, compressed
streams, assembly, and compiler-generated C are different kinds of sources:

- `tools/emit.py` has a historical `force_literal` input. It emits preserved
  instruction halfwords when mnemonic assembly did not reproduce their
  encodings. The current count of `.inst.n` directives is recorded in `linked-providers.json`.
  Those are preserved assembly bytes, not compiler-generated C. Data directives
  and unresolved instruction/data classification also remain.
- BIOS SWI wrappers use explicit inline assembly and are reported separately
  from ordinary C.
- All 104 graphics streams are generated from editable image/layout sources.
  `tests/test_lz77.py` checks every encoded stream against its original ROM
  span. No graphics `.lz` source or original-ROM build read is needed.
  Unchanged named scripts still retain their compressed framing; their codec
  reconstruction is a separate task.

- NCD containers now compile entirely from individual cell PNGs, palette files
  and readable JSON tables. Unknown fields remain explicitly labeled numeric
  metadata. NFT and script containers still retain unedited container bytes.
  Undecoded data blobs elsewhere are still build inputs. This audit does not claim
  that these bytes have been decompiled or semantically understood.

Missing required PNGs or frame/icon manifests now fail rather than silently
keeping original content. Readable C under `src/nonmatching/` remains excluded
from the matching build and is not counted as compiled matching code.

Mapped-image compilation has an additional end-to-end proof in
[mapped-image-proof.json](mapped-image-proof.json): it checks an actual image
pixel and an independent tilemap flip after a clean build, then restores both
sources and the matching ROM.

The optional `make english` target links `mar_english.gba` from the original
objects, an explicitly sized constructor bridge, and agbcc-compiled C/text in
ROM expansion. It does not patch `mar.gba`. Japanese bytes outside
011790..011870 remain identical; the default `make compare` stays byte-exact.
Runtime lookup, word wrapping, A-button pagination, allocation retries, style
carryover and completion counters are host-tested. A full emulator playthrough
is still needed; other printers and context-specific lookups are unfinished.

`decompilation-progress.json` counts real linked C separately from BIOS assembly.
Its byte percentage uses the code/data region minus known PCM, not a guessed
number of functions or a claim that every byte in that region is executable.

The matching graphics encoder was recovered by comparing token decisions:
VRAM streams exclude distance-one matches. The previous `vram_safe` option
was a no-op. Nearest-first longest matches with minimum distance two reproduce
all 104 streams. The optimized prefix search is checked against brute force.
All 344 loose source `.lz` files are gone; only generated build outputs and
historical backups contain them. `retired-graphics-lz.json` records each removed
stream's hash and image-derived byte comparison; `retired-script-lz.json`
records the separate duplicate-script cleanup.

The mapped-image proof also tests rejection of an oversized edit. Its accepted
fixture changes the first OP_05_A image pixel and flips map cell 2 (tile 4),
which fits the existing 16,339-byte allocation with the matching encoder.

The VM continuation batch adds ten matching C ranges (364 bytes): shutdown,
result accessors, parent-frame lookup, pending-task accessors, per-update budget
accessors, the bounded update loop and the native return command. The recovered
loop blocks on pending operations, accumulates dispatch work up to its budget,
and preserves nonpositive termination codes. Host tests cover blocking, budget
changes during dispatch, unsigned counter underflow and root-frame unwinding.
Two old function labels (0807E416 and 0807F1B4) were actually literal-pool data
inside these newly recovered functions.

The frame-management batch adds five matching ranges (300 bytes) in
`src/script_frames.c`. `include/script_vm.h` records partial, named VM/frame
layouts; the agbcc comparison verifies their GBA offsets. Frame teardown restores
the parent before freeing storage, uses the VM heap for its tables, and frees
an owned resource through heap zero. Host tests distinguish owned/borrowed/null
resources, rejected flag indices, and work-batch versus pending-aware dispatch.
