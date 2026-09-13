@ Named locations in IWRAM used by matching C sources.
.global gIwramBase
.set gIwramBase, 0x03000000
.global gMapGenerationRootOffset
.set gMapGenerationRootOffset, 0x00003FDC
.global gMapStateOffset
.set gMapStateOffset, 0x000032F4
.global gSecondaryRuntime
.set gSecondaryRuntime, 0x03004020
.global gRuntimeObjectTable
.set gRuntimeObjectTable, 0x03004024
.global gIwramField3FD5Offset
.set gIwramField3FD5Offset, 0x00003FD5
.global gSoundIrqModeOffset
.set gSoundIrqModeOffset, 0x00003FD6
.global gIwramPointer2860Offset
.set gIwramPointer2860Offset, 0x00002860
.global gIwramField0810Offset
.set gIwramField0810Offset, 0x00000810
.global gBattleFieldRectXOffset
.set gBattleFieldRectXOffset, 0x00003BE4
.global gBattleFieldRectYOffset
.set gBattleFieldRectYOffset, 0x00003BE8
.global gBattleFieldRectWidthOffset
.set gBattleFieldRectWidthOffset, 0x00003BEC
.global gBattleFieldRectHeightOffset
.set gBattleFieldRectHeightOffset, 0x00003BF0
.global gIwramBaseRectX
.set gIwramBaseRectX, 0x03000000
.global gIwramBaseRectY
.set gIwramBaseRectY, 0x03000000
.global gIwramBaseRectWidth
.set gIwramBaseRectWidth, 0x03000000
.global gIwramBaseRectHeight
.set gIwramBaseRectHeight, 0x03000000

@ Read-only definition table embedded in the monolithic 1B0000 data block.
@ Keep this as an absolute symbol: labelling it must not split or duplicate the
@ byte-exact incbin that also supplies the editable Japanese text tables.
.global gItemDefinitions
.set gItemDefinitions, 0x081B096C

.global gSoundPlayerTable
.set gSoundPlayerTable, 0x0808B144

.global gSongTable
.set gSongTable, 0x0808B1B0
.global _ctype_
.set _ctype_, 0x081AC7A4

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
.global ScriptResourceInternName
.thumb_set ScriptResourceInternName, 0x0807EA8D
.global _08001632
.set _08001632, 0x08001632
.global _0800163A
.set _0800163A, 0x0800163A
.global _call_via_r1
.thumb_set _call_via_r1, 0x08080BC5
.global _call_via_r3
.thumb_set _call_via_r3, 0x08080BCD
@ libgcc and newlib helpers called by the source-recovered C library.  These
@ routines remain in generated assembly for now; binding their standard names
@ lets the original newlib sources link against their exact ROM implementations.
.global __adddf3
.thumb_set __adddf3, 0x08081CE5
.global __subdf3
.thumb_set __subdf3, 0x08081D15
.global __muldf3
.thumb_set __muldf3, 0x08081D4D
.global __divdf3
.thumb_set __divdf3, 0x08081FF5
.global __eqdf2
.thumb_set __eqdf2, 0x080822A9
.global __nedf2
.thumb_set __nedf2, 0x080822F5
.global __gtdf2
.thumb_set __gtdf2, 0x08082341
.global __ltdf2
.thumb_set __ltdf2, 0x080823D9
.global __ledf2
.thumb_set __ledf2, 0x08082425
.global __floatsidf
.thumb_set __floatsidf, 0x08082471
.global __fixdfsi
.thumb_set __fixdfsi, 0x080824ED
.global __negdf2
.thumb_set __negdf2, 0x08082561
.global __divsi3
.thumb_set __divsi3, 0x08080BFD
.global __modsi3
.thumb_set __modsi3, 0x08080C95
.global __udivsi3
.thumb_set __udivsi3, 0x08080DD5
.global __umodsi3
.thumb_set __umodsi3, 0x08080E4D
.global _call_via_r8
.thumb_set _call_via_r8, 0x08080BE1
.global __malloc_lock
.thumb_set __malloc_lock, 0x080859C5
.global __malloc_unlock
.thumb_set __malloc_unlock, 0x080859C9
.global _sbrk_r
.thumb_set _sbrk_r, 0x080862E1
.global _write_r
.thumb_set _write_r, 0x08086835
.global _close_r
.thumb_set _close_r, 0x080868BD
.global _fstat_r
.thumb_set _fstat_r, 0x080868F5
.global isatty
.thumb_set isatty, 0x08086941
.global _lseek_r
.thumb_set _lseek_r, 0x08086949
.global _read_r
.thumb_set _read_r, 0x08086979
.global _impure_ptr
.set _impure_ptr, 0x08F2AFEC
.global __errno
.thumb_set __errno, 0x080868E9
.global end
.set end, 0x03006128
.global sub_0807F3DC
.thumb_set sub_0807F3DC, 0x0807F3DD

@ Labels inside source-compiled newlib constant tables. Remaining assembly uses
@ PC-relative ADR/loads to these exact addresses, so keep stable aliases while
@ the bytes themselves come from the C objects.
.global _081AC738
.set _081AC738, 0x081AC738
.global _081AC754
.set _081AC754, 0x081AC754
.global _081AC768
.set _081AC768, 0x081AC768
.global _081AC770
.set _081AC770, 0x081AC770
.global _081AC78C
.set _081AC78C, 0x081AC78C
.global _081AC8B0
.set _081AC8B0, 0x081AC8B0
.global _081AC8CC
.set _081AC8CC, 0x081AC8CC
.global _081AC8DC
.set _081AC8DC, 0x081AC8DC
.global sub_081AC8EC
.set sub_081AC8EC, 0x081AC8EC
.global _081AC924
.set _081AC924, 0x081AC924
.global _081AC940
.set _081AC940, 0x081AC940
.global _081AC95C
.set _081AC95C, 0x081AC95C
.global _081AC978
.set _081AC978, 0x081AC978
.global _081AC982
.set _081AC982, 0x081AC982
.global _081AC994
.set _081AC994, 0x081AC994
