/* Script bytecode opcode dispatch, decoded from the original 256-entry table. */
#include "script.h"
#include "script_bytecode.h"

#include "rom_section.h"
#define INVALID ScriptCmdFail

extern s32 ScriptCmdConcatStrings(void);
extern s32 ScriptCmdCallNative(void);

/* Runtime archive and resource names. This whole ASCII block was previously
 * misidentified as Thumb instructions in code_0800C0.s. */
AT("00086A14") const char gDefaultArchiveName[8] = "MAR.NFP";
AT("00086A1C") const char gFontResourceName[12] = "FONT.NFT";
AT("00086A28") const char gSystemNcdResourceName[12] = "SYSTEM.NCD";
AT("00086A34") const char gEffectNcdResourceName[12] = "EFFECT.NCD";
AT("00086A40") const char gCharacterNcdResourceName[8] = "CHR.NCD";
AT("00086A48") const char gSystemMountName[8] = "SYSTEM";
AT("00086A50") const char gMainMountName[4] = "MAR";
AT("00086A54") const char gMainArchiveName[8] = "MAR.NFP";
AT("00086A5C") const char gMapArchiveKmpExtension[8] = ".KMP";
AT("00086A64") const char gSceneArchiveName[8] = "MAR.NFP";
AT("00086A6C") const char gSceneNcdExtension[8] = ".NCD";

/* Names for the engine-provided native commands. */
AT("00086A74") const char gScriptEngineNameBgmPlaySync[12] = "BgmPlaySync";
AT("00086A80") const char gScriptEngineNameBgmVol[8] = "BgmVol";
AT("00086A88") const char gScriptEngineNameBgmFadeSync[12] = "BgmFadeSync";
AT("00086A94") const char gScriptEngineNameBgmFade[8] = "BgmFade";
AT("00086A9C") const char gScriptEngineNameBgmStopAll[12] = "BgmStopAll";
AT("00086AA8") const char gScriptEngineNameBgmStop[8] = "BgmStop";
AT("00086AB0") const char gScriptEngineNameBgmPlay[8] = "BgmPlay";
AT("00086AB8") const char gScriptEngineNameSePlay[8] = "SePlay";
AT("00086AC0") const char gScriptEngineNameGetCrtRgb[12] = "GetCrtRgb";
AT("00086ACC") const char gScriptEngineNameSetCrtRgb[12] = "SetCrtRgb";
AT("00086AD8") const char gScriptEngineNameGetCrtFade[12] = "GetCrtFade";
AT("00086AE4") const char gScriptEngineNameSetCrtFade[12] = "SetCrtFade";
AT("00086AF0") const char gScriptEngineNameCrtFadeSync[12] = "CrtFadeSync";
AT("00086AFC") const char gScriptEngineNameCrtFade[8] = "CrtFade";
AT("00086B04") const char gScriptEngineNameGetAobNo[12] = "GetAobNo";
AT("00086B10") const char gScriptEngineNameGetNcdNo[12] = "GetNcdNo";
AT("00086B1C") const char gScriptEngineNameAddVar[8] = "AddVar";
AT("00086B24") const char gScriptEngineNameSetVar[8] = "SetVar";
AT("00086B2C") const char gScriptEngineNameGetVar[8] = "GetVar";
AT("00086B34") const char gScriptEngineNameSetBool[8] = "SetBool";
AT("00086B3C") const char gScriptEngineNameGetBool[8] = "GetBool";
AT("00086B44") const char gScriptEngineNameWait[8] = "Wait";
AT("00086B4C") const char gScriptEngineNamePad[4] = "Pad";
AT("00086B50") const char gScriptEngineNameDummy[8] = "dummy";

AT("001AC6A8") const char gScriptBuiltinNameResurn[8] = "resurn";
AT("001AC6B0") const char gScriptBuiltinNameExit[8] = "exit";
AT("001AC6B8") const char gScriptBuiltinNameCall[8] = "call";
AT("001AC6C0") const char gScriptBuiltinNameExec[8] = "exec";
AT("001AC6C8") const char gScriptBuiltinNameChain[8] = "chain";
AT("001AC6D0") const char gScriptBuiltinNameStrmid[8] = "strmid";
AT("001AC6D8") const char gScriptBuiltinNameStrright[12] = "strright";
AT("001AC6E4") const char gScriptBuiltinNameStrleft[8] = "strleft";
AT("001AC6EC") const char gScriptBuiltinNameStrlen[8] = "strlen";
AT("001AC6F4") const char gScriptBuiltinNameStrcmp[8] = "strcmp";
AT("001AC6FC") const char gScriptBuiltinNameVal[4] = "val";
AT("001AC700") const char gScriptBuiltinNameStr[4] = "str";
AT("001AC704") const char gScriptBuiltinNameSrand[8] = "srand";
AT("001AC70C") const char gScriptBuiltinNameRand[8] = "rand";
AT("001AC714") const char gScriptBuiltinNameMin[4] = "min";
AT("001AC718") const char gScriptBuiltinNameMax[4] = "max";
AT("001AC71C") const char gScriptBuiltinNameChr[4] = "chr";
AT("001AC720") const char gScriptBuiltinNameAsc[4] = "asc";
AT("001AC724") const char gScriptBuiltinNameAbs[4] = "abs";

/* Empty resource fallback followed by the integer formatting string used by
 * the native string-conversion command. The empty string begins at byte 8 of
 * the fallback record. */
AT("001AC698") const u8 gScriptResourceDefaultValue[12] = {0};
AT("001AC6A4") const char gScriptDecimalFormat[4] = "%d";

extern s32 ScriptNativeAbs(void);
extern s32 ScriptNativeCharacterCode(void);
extern s32 ScriptNativeCharacterString(void);
extern s32 ScriptNativeMax(void);
extern s32 ScriptNativeMin(void);
extern s32 ScriptNativeRandomRange(void);
extern s32 ScriptNativeSeedRandom(void);
extern s32 ScriptNativeIntegerString(void);
extern s32 ScriptNativeParseInteger(void);
extern s32 ScriptNativeCompareStrings(void);
extern s32 ScriptNativeStringLength(void);
extern s32 ScriptNativeLeft(void);
extern s32 ScriptNativeRight(void);
extern s32 ScriptNativeSubstring(void);
extern s32 ScriptNativeChain(void);
extern s32 ScriptNativeExec(void);
extern s32 ScriptNativeCall(void);
extern s32 ScriptNativeResurn(void);
extern s32 ScriptNativeExit(void);

#define THUMB_NATIVE(function) ((u32)(function) + 1)

/** Core engine commands registered in every script VM. The order is the
 * original ROM order; registration stops at the null sentinel. */
AT("001ACB7C")
const struct ScriptResourceEntry gScriptEngineFunctions[] = {
    { .name = gScriptEngineNameDummy,
      .value = THUMB_NATIVE(ScriptNativeDummy) },
    { .name = gScriptEngineNamePad,
      .value = THUMB_NATIVE(ScriptNativeStartTask05378) },
    { .name = gScriptEngineNameWait,
      .value = THUMB_NATIVE(ScriptNativeStartTask053E4) },
    { .name = gScriptEngineNameGetBool,
      .value = THUMB_NATIVE(ScriptNativeTestGameFlag) },
    { .name = gScriptEngineNameSetBool,
      .value = THUMB_NATIVE(ScriptNativeSetGameValue) },
    { .name = gScriptEngineNameGetVar,
      .value = THUMB_NATIVE(ScriptNativeGetStatePointer) },
    { .name = gScriptEngineNameSetVar,
      .value = THUMB_NATIVE(ScriptNativeSetStatePointer) },
    { .name = gScriptEngineNameAddVar,
      .value = THUMB_NATIVE(ScriptNativeAdvanceStatePointer) },
    { .name = gScriptEngineNameGetNcdNo,
      .value = THUMB_NATIVE(ScriptNativeLookupResource) },
    { .name = gScriptEngineNameGetAobNo,
      .value = THUMB_NATIVE(ScriptNativeFindNamedResource) },
    { .name = gScriptEngineNameCrtFade,
      .value = THUMB_NATIVE(ScriptNativeStartTask05530) },
    { .name = gScriptEngineNameCrtFadeSync,
      .value = THUMB_NATIVE(ScriptNativeSetCrtFade) },
    { .name = gScriptEngineNameSetCrtFade,
      .value = THUMB_NATIVE(ScriptNativeGetCrtFade) },
    { .name = gScriptEngineNameGetCrtFade,
      .value = THUMB_NATIVE(ScriptNativeGetSpriteRuntime) },
    { .name = gScriptEngineNameSetCrtRgb,
      .value = THUMB_NATIVE(ScriptNativeSetRuntimeCoordinate) },
    { .name = gScriptEngineNameGetCrtRgb,
      .value = THUMB_NATIVE(ScriptNativeGetRuntimeCoordinate) },
    { .name = gScriptEngineNameSePlay,
      .value = THUMB_NATIVE(ScriptNativeStartIndexedSong) },
    { .name = gScriptEngineNameBgmPlay,
      .value = THUMB_NATIVE(ScriptNativeSelectSceneValue) },
    { .name = gScriptEngineNameBgmStop,
      .value = THUMB_NATIVE(ScriptNativeStopTrackedSong) },
    { .name = gScriptEngineNameBgmStopAll,
      .value = THUMB_NATIVE(ScriptNativeResetNineChannels) },
    { .name = gScriptEngineNameBgmFade,
      .value = THUMB_NATIVE(ScriptNativeStartTask056AC) },
    { .name = gScriptEngineNameBgmFadeSync,
      .value = THUMB_NATIVE(ScriptNativeStartTask057C0) },
    { .name = gScriptEngineNameBgmVol,
      .value = THUMB_NATIVE(ScriptNativeSetSoundPlayerVolume) },
    { .name = gScriptEngineNameBgmPlaySync,
      .value = THUMB_NATIVE(ScriptNativeStartTask05848) },
    { .name = 0, .value = 0 },
};

/** Built-in expression functions exposed by name to compiled scripts.  The
 * original library misspells "resurn"; keep it for bytecode compatibility. */
AT("00F2AC60")
const struct ScriptResourceEntry gScriptBuiltinFunctions[] = {
    { gScriptBuiltinNameAbs,      (u32)ScriptNativeAbs },
    { gScriptBuiltinNameAsc,      (u32)ScriptNativeCharacterCode },
    { gScriptBuiltinNameChr,      (u32)ScriptNativeCharacterString },
    { gScriptBuiltinNameMax,      (u32)ScriptNativeMax },
    { gScriptBuiltinNameMin,      (u32)ScriptNativeMin },
    { gScriptBuiltinNameRand,     (u32)ScriptNativeRandomRange },
    { gScriptBuiltinNameSrand,    (u32)ScriptNativeSeedRandom },
    { gScriptBuiltinNameStr,      (u32)ScriptNativeIntegerString },
    { gScriptBuiltinNameVal,      (u32)ScriptNativeParseInteger },
    { gScriptBuiltinNameStrcmp,   (u32)ScriptNativeCompareStrings },
    { gScriptBuiltinNameStrlen,   (u32)ScriptNativeStringLength },
    { gScriptBuiltinNameStrleft,  (u32)ScriptNativeLeft },
    { gScriptBuiltinNameStrright, (u32)ScriptNativeRight },
    { gScriptBuiltinNameStrmid,   (u32)ScriptNativeSubstring },
    { gScriptBuiltinNameChain,    (u32)ScriptNativeChain },
    { gScriptBuiltinNameExec,     (u32)ScriptNativeExec },
    { gScriptBuiltinNameCall,     (u32)ScriptNativeCall },
    { gScriptBuiltinNameExit,     (u32)ScriptNativeExit },
    { gScriptBuiltinNameResurn,   (u32)ScriptNativeResurn },
    { 0, 0 },
};

AT("00F2A860")
const ScriptOpcodeHandler gScriptOpcodeHandlers[SCRIPT_OPCODE_COUNT] = {
    /* 0x00 */
    INVALID, /* 0x00 */
    INVALID, /* 0x01 */
    INVALID, /* 0x02 */
    INVALID, /* 0x03 */
    INVALID, /* 0x04 */
    INVALID, /* 0x05 */
    INVALID, /* 0x06 */
    INVALID, /* 0x07 */
    INVALID, /* 0x08 */
    INVALID, /* 0x09 */
    INVALID, /* 0x0A */
    INVALID, /* 0x0B */
    INVALID, /* 0x0C */
    INVALID, /* 0x0D */
    INVALID, /* 0x0E */
    INVALID, /* 0x0F */
    /* 0x10 */
    ScriptCmdJumpRelative, /* 0x10 */
    ScriptCmdJump, /* 0x11 */
    ScriptCmdJumpIfZero, /* 0x12 */
    ScriptCmdCall, /* 0x13 */
    ScriptCmdReturn, /* 0x14 */
    ScriptCmdSwitch, /* 0x15 */
    INVALID, /* 0x16 */
    INVALID, /* 0x17 */
    INVALID, /* 0x18 */
    INVALID, /* 0x19 */
    INVALID, /* 0x1A */
    INVALID, /* 0x1B */
    INVALID, /* 0x1C */
    INVALID, /* 0x1D */
    INVALID, /* 0x1E */
    INVALID, /* 0x1F */
    /* 0x20 */
    ScriptCmdCopy, /* 0x20 */
    ScriptCmdSetImmediate, /* 0x21 */
    ScriptCmdSetBytecodeAddress, /* 0x22 */
    INVALID, /* 0x23 */
    INVALID, /* 0x24 */
    INVALID, /* 0x25 */
    INVALID, /* 0x26 */
    INVALID, /* 0x27 */
    ScriptCmdPushOperand, /* 0x28 */
    ScriptCmdPushImmediate, /* 0x29 */
    ScriptCmdPopOperand, /* 0x2A */
    INVALID, /* 0x2B */
    INVALID, /* 0x2C */
    INVALID, /* 0x2D */
    INVALID, /* 0x2E */
    INVALID, /* 0x2F */
    /* 0x30 */
    ScriptCmdAdd, /* 0x30 */
    ScriptCmdAddImmediate, /* 0x31 */
    ScriptCmdSubtract, /* 0x32 */
    ScriptCmdSubtractImmediate, /* 0x33 */
    ScriptCmdMultiply, /* 0x34 */
    ScriptCmdMultiplyImmediate, /* 0x35 */
    ScriptCmdDivide, /* 0x36 */
    ScriptCmdDivideImmediate, /* 0x37 */
    ScriptCmdModulo, /* 0x38 */
    ScriptCmdModuloImmediate, /* 0x39 */
    ScriptCmdIncrement, /* 0x3A */
    ScriptCmdDecrement, /* 0x3B */
    ScriptCmdNegate, /* 0x3C */
    INVALID, /* 0x3D */
    INVALID, /* 0x3E */
    INVALID, /* 0x3F */
    /* 0x40 */
    ScriptCmdBitAnd, /* 0x40 */
    ScriptCmdBitAndImmediate, /* 0x41 */
    ScriptCmdBitOr, /* 0x42 */
    ScriptCmdBitOrImmediate, /* 0x43 */
    ScriptCmdBitXor, /* 0x44 */
    ScriptCmdBitXorImmediate, /* 0x45 */
    ScriptCmdBitNot, /* 0x46 */
    INVALID, /* 0x47 */
    INVALID, /* 0x48 */
    INVALID, /* 0x49 */
    INVALID, /* 0x4A */
    INVALID, /* 0x4B */
    INVALID, /* 0x4C */
    INVALID, /* 0x4D */
    INVALID, /* 0x4E */
    INVALID, /* 0x4F */
    /* 0x50 */
    ScriptCmdLogicalAnd, /* 0x50 */
    ScriptCmdLogicalAndImmediate, /* 0x51 */
    ScriptCmdLogicalOr, /* 0x52 */
    ScriptCmdLogicalOrImmediate, /* 0x53 */
    ScriptCmdLogicalNot, /* 0x54 */
    INVALID, /* 0x55 */
    INVALID, /* 0x56 */
    INVALID, /* 0x57 */
    ScriptCmdSignBit, /* 0x58 */
    ScriptCmdLessThanOrEqualZero, /* 0x59 */
    ScriptCmdGreaterThanZero, /* 0x5A */
    ScriptCmdGreaterThanOrEqualZero, /* 0x5B */
    ScriptCmdEqualZero, /* 0x5C */
    ScriptCmdNotEqualZero, /* 0x5D */
    INVALID, /* 0x5E */
    INVALID, /* 0x5F */
    /* 0x60 */
    ScriptCmdSetCallback, /* 0x60 */
    ScriptCmdRunCallback, /* 0x61 */
    ScriptRestoreFrame, /* 0x62 */
    ScriptCmdClearFrameFlag, /* 0x63 */
    ScriptCmdSetFrameFlag, /* 0x64 */
    INVALID, /* 0x65 */
    INVALID, /* 0x66 */
    INVALID, /* 0x67 */
    INVALID, /* 0x68 */
    INVALID, /* 0x69 */
    INVALID, /* 0x6A */
    INVALID, /* 0x6B */
    INVALID, /* 0x6C */
    INVALID, /* 0x6D */
    INVALID, /* 0x6E */
    INVALID, /* 0x6F */
    /* 0x70 */
    ScriptCmdReadContextField14, /* 0x70 */
    ScriptCmdReadContextField114, /* 0x71 */
    ScriptCmdReadTable38Field, /* 0x72 */
    ScriptCmdReadTable3CField, /* 0x73 */
    INVALID, /* 0x74 */
    INVALID, /* 0x75 */
    INVALID, /* 0x76 */
    INVALID, /* 0x77 */
    ScriptCmdConcatStrings, /* 0x78 */
    ScriptCmdFreeString, /* 0x79 */
    INVALID, /* 0x7A */
    INVALID, /* 0x7B */
    INVALID, /* 0x7C */
    INVALID, /* 0x7D */
    INVALID, /* 0x7E */
    INVALID, /* 0x7F */
    /* 0x80 */
    ScriptCmdCallNative, /* 0x80 */
    INVALID, /* 0x81 */
    INVALID, /* 0x82 */
    INVALID, /* 0x83 */
    INVALID, /* 0x84 */
    INVALID, /* 0x85 */
    INVALID, /* 0x86 */
    INVALID, /* 0x87 */
    INVALID, /* 0x88 */
    INVALID, /* 0x89 */
    INVALID, /* 0x8A */
    INVALID, /* 0x8B */
    INVALID, /* 0x8C */
    INVALID, /* 0x8D */
    INVALID, /* 0x8E */
    ScriptCmdRestoreFrameAndGetResult, /* 0x8F */
    /* 0x90 */
    INVALID, /* 0x90 */
    INVALID, /* 0x91 */
    INVALID, /* 0x92 */
    INVALID, /* 0x93 */
    INVALID, /* 0x94 */
    INVALID, /* 0x95 */
    INVALID, /* 0x96 */
    INVALID, /* 0x97 */
    INVALID, /* 0x98 */
    INVALID, /* 0x99 */
    INVALID, /* 0x9A */
    INVALID, /* 0x9B */
    INVALID, /* 0x9C */
    INVALID, /* 0x9D */
    INVALID, /* 0x9E */
    INVALID, /* 0x9F */
    /* 0xA0 */
    INVALID, /* 0xA0 */
    INVALID, /* 0xA1 */
    INVALID, /* 0xA2 */
    INVALID, /* 0xA3 */
    INVALID, /* 0xA4 */
    INVALID, /* 0xA5 */
    INVALID, /* 0xA6 */
    INVALID, /* 0xA7 */
    INVALID, /* 0xA8 */
    INVALID, /* 0xA9 */
    INVALID, /* 0xAA */
    INVALID, /* 0xAB */
    INVALID, /* 0xAC */
    INVALID, /* 0xAD */
    INVALID, /* 0xAE */
    INVALID, /* 0xAF */
    /* 0xB0 */
    INVALID, /* 0xB0 */
    INVALID, /* 0xB1 */
    INVALID, /* 0xB2 */
    INVALID, /* 0xB3 */
    INVALID, /* 0xB4 */
    INVALID, /* 0xB5 */
    INVALID, /* 0xB6 */
    INVALID, /* 0xB7 */
    INVALID, /* 0xB8 */
    INVALID, /* 0xB9 */
    INVALID, /* 0xBA */
    INVALID, /* 0xBB */
    INVALID, /* 0xBC */
    INVALID, /* 0xBD */
    INVALID, /* 0xBE */
    INVALID, /* 0xBF */
    /* 0xC0 */
    INVALID, /* 0xC0 */
    INVALID, /* 0xC1 */
    INVALID, /* 0xC2 */
    INVALID, /* 0xC3 */
    INVALID, /* 0xC4 */
    INVALID, /* 0xC5 */
    INVALID, /* 0xC6 */
    INVALID, /* 0xC7 */
    INVALID, /* 0xC8 */
    INVALID, /* 0xC9 */
    INVALID, /* 0xCA */
    INVALID, /* 0xCB */
    INVALID, /* 0xCC */
    INVALID, /* 0xCD */
    INVALID, /* 0xCE */
    INVALID, /* 0xCF */
    /* 0xD0 */
    INVALID, /* 0xD0 */
    INVALID, /* 0xD1 */
    INVALID, /* 0xD2 */
    INVALID, /* 0xD3 */
    INVALID, /* 0xD4 */
    INVALID, /* 0xD5 */
    INVALID, /* 0xD6 */
    INVALID, /* 0xD7 */
    INVALID, /* 0xD8 */
    INVALID, /* 0xD9 */
    INVALID, /* 0xDA */
    INVALID, /* 0xDB */
    INVALID, /* 0xDC */
    INVALID, /* 0xDD */
    INVALID, /* 0xDE */
    INVALID, /* 0xDF */
    /* 0xE0 */
    INVALID, /* 0xE0 */
    INVALID, /* 0xE1 */
    INVALID, /* 0xE2 */
    INVALID, /* 0xE3 */
    INVALID, /* 0xE4 */
    INVALID, /* 0xE5 */
    INVALID, /* 0xE6 */
    INVALID, /* 0xE7 */
    INVALID, /* 0xE8 */
    INVALID, /* 0xE9 */
    INVALID, /* 0xEA */
    INVALID, /* 0xEB */
    INVALID, /* 0xEC */
    INVALID, /* 0xED */
    INVALID, /* 0xEE */
    INVALID, /* 0xEF */
    /* 0xF0 */
    INVALID, /* 0xF0 */
    INVALID, /* 0xF1 */
    INVALID, /* 0xF2 */
    INVALID, /* 0xF3 */
    INVALID, /* 0xF4 */
    INVALID, /* 0xF5 */
    INVALID, /* 0xF6 */
    INVALID, /* 0xF7 */
    INVALID, /* 0xF8 */
    INVALID, /* 0xF9 */
    INVALID, /* 0xFA */
    INVALID, /* 0xFB */
    INVALID, /* 0xFC */
    INVALID, /* 0xFD */
    INVALID, /* 0xFE */
    INVALID, /* 0xFF */
};

#undef INVALID
