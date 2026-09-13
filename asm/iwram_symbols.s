@ Named locations in IWRAM used by matching C sources.
.global gIwramBase
.set gIwramBase, 0x03000000
.global gMapGenerationRootOffset
.set gMapGenerationRootOffset, 0x00003FDC
.global gSecondaryRuntime
.set gSecondaryRuntime, 0x03004020
.global gRuntimeObjectTable
.set gRuntimeObjectTable, 0x03004024
.global gIwramField3FD5Offset
.set gIwramField3FD5Offset, 0x00003FD5
.global gIwramPointer2860Offset
.set gIwramPointer2860Offset, 0x00002860
.global gIwramField0810Offset
.set gIwramField0810Offset, 0x00000810

@ Read-only definition table embedded in the monolithic 1B0000 data block.
@ Keep this as an absolute symbol: labelling it must not split or duplicate the
@ byte-exact incbin that also supplies the editable Japanese text tables.
.global gItemDefinitions
.set gItemDefinitions, 0x081B096C

.global gSoundPlayerTable
.set gSoundPlayerTable, 0x0808B144

.global gSongTable
.set gSongTable, 0x0808B1B0

@ Absolute aliases retained for misidentified ADR-like words in an opaque
@ table at 0800A03C. These targets fall inside newly compiled C ranges.
.global _0800A1E0
.set _0800A1E0, 0x0800A1E0
.global _0800A21C
.set _0800A21C, 0x0800A21C
.global _0800A258
.set _0800A258, 0x0800A258
.global _0800A294
.set _0800A294, 0x0800A294

@ Stable aliases used by compiled C and generated call sites. Keeping them in
@ this non-generated file prevents a code-split refresh from discarding them.
.global NfpFindEntryIndex
.thumb_set NfpFindEntryIndex, 0x0807ACC5
.global _0807D3F6
.set _0807D3F6, 0x0807D3F6
.global _call_via_r1
.thumb_set _call_via_r1, 0x08080BC5
.global _call_via_r3
.thumb_set _call_via_r3, 0x08080BCD
.global _localeconv_r
.thumb_set _localeconv_r, 0x0808522D
.global _08085268_void
.thumb_set _08085268_void, 0x08085269
