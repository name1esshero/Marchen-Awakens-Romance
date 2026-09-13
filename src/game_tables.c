#include "game_tables.h"
#include "script.h"

#define AT(x) __attribute__((section(".rom." x)))

/* Fixed 16-halfword blocks copied into battle runtime work areas. */
AT("001ACC50") const u16 gBattleRuntimePresetA[16] = {
    0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};
AT("001ACC70") const u16 gBattleRuntimePresetB[16] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

/* Friend ARM definition index -> ownership bit. 444 is the invalid sentinel. */
AT("001ACC90") const s16 gFriendArmOwnershipBits[8] = {
    1, 2, 12, 444, 23, 444, 21, 1,
};
AT("001ACCA0") const u16 gBattlePartyDefaults[8] = {
    1, 3, 4, 6, 9, 8, 7, 22,
};

/* Battle action/effect dispatch slots. Repeated fallback entries are retained. */
AT("001ACCB0") void *const gBattleActionHandlers[444] = {
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x0803DE59,
    (void *)0x0803E239, (void *)0x0803E5E1, (void *)0x0803EC35, (void *)0x0803EF65,
    (void *)0x0803F515, (void *)0x0803FF15, (void *)0x08040785, (void *)0x08040799,
    (void *)0x08015269, (void *)0x08075299, (void *)0x08075299, (void *)0x0804CB45,
    (void *)0x0804CEDD, (void *)0x0804D405, (void *)0x0804DCD1, (void *)0x0804E641,
    (void *)0x0804E655, (void *)0x080256F9, (void *)0x080256F9, (void *)0x08015269,
    (void *)0x08031109, (void *)0x0803111D, (void *)0x08031131, (void *)0x08031145,
    (void *)0x0804F409, (void *)0x0804F41D, (void *)0x0804F431, (void *)0x0804FBA1,
    (void *)0x08050359, (void *)0x08031C35, (void *)0x080322A5, (void *)0x080322B9,
    (void *)0x08015269, (void *)0x080507FD, (void *)0x08050811, (void *)0x08050825,
    (void *)0x08051035, (void *)0x0802D8D1, (void *)0x08051649, (void *)0x080410C1,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x0802FC79,
    (void *)0x0802FC8D, (void *)0x080516DD, (void *)0x0803C071, (void *)0x0803C085,
    (void *)0x0803C099, (void *)0x0803C0C1, (void *)0x08015269, (void *)0x08015269,
    (void *)0x0804AC29, (void *)0x0804AC3D, (void *)0x0804AC51, (void *)0x0804AC79,
    (void *)0x080414B5, (void *)0x080414C9, (void *)0x080414DD, (void *)0x080414F1,
    (void *)0x08041A05, (void *)0x08041A19, (void *)0x08041A2D, (void *)0x08042239,
    (void *)0x0804224D, (void *)0x08026699, (void *)0x0804A3D9, (void *)0x0804A3F1,
    (void *)0x0804A409, (void *)0x0804A421, (void *)0x080427AD, (void *)0x08026DC1,
    (void *)0x08042D95, (void *)0x080301BD, (void *)0x0803B7E5, (void *)0x0803B7F9,
    (void *)0x08027895, (void *)0x0804370D, (void *)0x08043721, (void *)0x08043735,
    (void *)0x08043F9D, (void *)0x0804441D, (void *)0x0804EC29, (void *)0x0804ECD1,
    (void *)0x0804ED79, (void *)0x0804EE21, (void *)0x0804EEC9, (void *)0x0804EC41,
    (void *)0x0804ECE9, (void *)0x0804ED91, (void *)0x0804EE39, (void *)0x0804EEE1,
    (void *)0x0804EC59, (void *)0x0804ED01, (void *)0x0804EDA9, (void *)0x0804EE51,
    (void *)0x0804EEF9, (void *)0x0804EC71, (void *)0x0804ED19, (void *)0x0804EDC1,
    (void *)0x0804EE69, (void *)0x0804EF11, (void *)0x0804EC89, (void *)0x0804ED31,
    (void *)0x0804EDD9, (void *)0x0804EE81, (void *)0x0804EF29, (void *)0x0804ECA1,
    (void *)0x0804ED49, (void *)0x0804EDF1, (void *)0x0804EE99, (void *)0x0804EF41,
    (void *)0x0804ECB9, (void *)0x0804ED61, (void *)0x0804EE09, (void *)0x0804EEB1,
    (void *)0x0804EF59, (void *)0x0804A451, (void *)0x0804A469, (void *)0x0804A481,
    (void *)0x0804A499, (void *)0x0804A4B1, (void *)0x0804A4C9, (void *)0x0804A4E1,
    (void *)0x0804A4F9, (void *)0x0804A511, (void *)0x0804A529, (void *)0x0804A541,
    (void *)0x0804A559, (void *)0x0804A571, (void *)0x0804A589, (void *)0x0804A5A1,
    (void *)0x0804A5B9, (void *)0x0804A5D1, (void *)0x0804A5E9, (void *)0x0804A601,
    (void *)0x0804A619, (void *)0x0804A631, (void *)0x0804A649, (void *)0x0804A661,
    (void *)0x0804A679, (void *)0x0804A691, (void *)0x0804A6A9, (void *)0x0804A6C1,
    (void *)0x0804A6D9, (void *)0x0804A6F1, (void *)0x0804A709, (void *)0x0802D0F1,
    (void *)0x0802D109, (void *)0x0802D121, (void *)0x0802D139, (void *)0x0802D151,
    (void *)0x0802D169, (void *)0x0802D181, (void *)0x0802D199, (void *)0x0802D1B1,
    (void *)0x0802D1C9, (void *)0x0802D1E1, (void *)0x0802D1F9, (void *)0x0802D211,
    (void *)0x0802D229, (void *)0x0802D241, (void *)0x0802D259, (void *)0x0802D271,
    (void *)0x0802D289, (void *)0x0802D2A1, (void *)0x0802D2B9, (void *)0x0802D2D1,
    (void *)0x0802D2E9, (void *)0x0802D301, (void *)0x0802D319, (void *)0x0802D331,
    (void *)0x0802D349, (void *)0x0802D361, (void *)0x0802D379, (void *)0x0802D391,
    (void *)0x0802D3A9, (void *)0x08073A15, (void *)0x08073A15, (void *)0x08073A15,
    (void *)0x0803CCF1, (void *)0x0803D505, (void *)0x0802DB7D, (void *)0x0802DB91,
    (void *)0x0802DBA5, (void *)0x0802DBB9, (void *)0x0802DBCD, (void *)0x0802ECF9,
    (void *)0x0802ED11, (void *)0x0802ED29, (void *)0x0802ED41, (void *)0x0802E371,
    (void *)0x0802E385, (void *)0x0802E399, (void *)0x0802E3AD, (void *)0x0802E3C1,
    (void *)0x0802F355, (void *)0x0802F369, (void *)0x0802F37D, (void *)0x0802F391,
    (void *)0x08032F15, (void *)0x08032F29, (void *)0x08032F3D, (void *)0x08032F51,
    (void *)0x08033CD9, (void *)0x08033CED, (void *)0x08034689, (void *)0x0803469D,
    (void *)0x08073CFD, (void *)0x08074345, (void *)0x08074BC1, (void *)0x08035005,
    (void *)0x080355D5, (void *)0x08035B35, (void *)0x08036319, (void *)0x08036C75,
    (void *)0x0807571D, (void *)0x0804AF99, (void *)0x0804B549, (void *)0x0804C259,
    (void *)0x08037845, (void *)0x08038015, (void *)0x08038B35, (void *)0x08038B49,
    (void *)0x080397A1, (void *)0x08039F29, (void *)0x08039F3D, (void *)0x0803A9C5,
    (void *)0x080446E1, (void *)0x080446F5, (void *)0x08044709, (void *)0x0804471D,
    (void *)0x08044D4D, (void *)0x080450B9, (void *)0x080450CD, (void *)0x08045765,
    (void *)0x08045D85, (void *)0x08046409, (void *)0x0802C15D, (void *)0x0802C171,
    (void *)0x0802C185, (void *)0x0802C585, (void *)0x0802CCE9, (void *)0x08046819,
    (void *)0x0804682D, (void *)0x08046841, (void *)0x08046FFD, (void *)0x08047619,
    (void *)0x08047FED, (void *)0x080483B1, (void *)0x080483C5, (void *)0x080486D5,
    (void *)0x080272FD, (void *)0x08048D31, (void *)0x0804915D, (void *)0x08049171,
    (void *)0x080496FD, (void *)0x08049C49, (void *)0x080308C9, (void *)0x08030BC5,
    (void *)0x08030E49, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08015269, (void *)0x08015269, (void *)0x08015269, (void *)0x08015269,
    (void *)0x08023B99, (void *)0x08023FA5, (void *)0x08023FA5, (void *)0x08024701,
    (void *)0x08026141, (void *)0x0802AE95, (void *)0x0802AEA9, (void *)0x0802BB0D,
    (void *)0x08072C15, (void *)0x08031109, (void *)0x0803111D, (void *)0x080507FD,
    (void *)0x08050811, (void *)0x08050825, (void *)0x08051035, (void *)0x080410C1,
    (void *)0x0803C071, (void *)0x0803C085, (void *)0x0804A3D9, (void *)0x0804A3F1,
    (void *)0x0804A409, (void *)0x0804EC29, (void *)0x0804ECD1, (void *)0x0804ED79,
    (void *)0x0804EE21, (void *)0x0804EEC9, (void *)0x0804EC41, (void *)0x0804ECE9,
    (void *)0x0804ED91, (void *)0x0804EE39, (void *)0x0804EEE1, (void *)0x0804EC59,
    (void *)0x0804ED01, (void *)0x0804EDA9, (void *)0x0804EE51, (void *)0x0804EEF9,
    (void *)0x0804EC71, (void *)0x0804ED19, (void *)0x0804EDC1, (void *)0x0804EE69,
    (void *)0x0804EF11, (void *)0x0804EC89, (void *)0x0804ED31, (void *)0x0804EDD9,
    (void *)0x0804EE81, (void *)0x0804EF29, (void *)0x0804ECA1, (void *)0x0804ED49,
    (void *)0x0804EDF1, (void *)0x0804EE99, (void *)0x0804EF41, (void *)0x0804ECB9,
    (void *)0x0804ED61, (void *)0x0804EE09, (void *)0x0804EEB1, (void *)0x0804EF59,
    (void *)0x0804A451, (void *)0x0804A469, (void *)0x0804A481, (void *)0x0804A499,
    (void *)0x0804A4C9, (void *)0x0804A4E1, (void *)0x0804A4F9, (void *)0x0804A511,
    (void *)0x0804A541, (void *)0x0804A559, (void *)0x0804A571, (void *)0x0804A589,
    (void *)0x0804A5B9, (void *)0x0804A5D1, (void *)0x0804A5E9, (void *)0x0804A601,
    (void *)0x0804A631, (void *)0x0804A649, (void *)0x0804A661, (void *)0x0804A679,
    (void *)0x0804A6A9, (void *)0x0804A6C1, (void *)0x0804A6D9, (void *)0x0804A6F1,
    (void *)0x0802D0F1, (void *)0x0802D109, (void *)0x0802D121, (void *)0x0802D139,
    (void *)0x0802D169, (void *)0x0802D181, (void *)0x0802D199, (void *)0x0802D1B1,
    (void *)0x0802D1E1, (void *)0x0802D1F9, (void *)0x0802D211, (void *)0x0802D229,
    (void *)0x0802D259, (void *)0x0802D271, (void *)0x0802D289, (void *)0x0802D2A1,
    (void *)0x0802D2D1, (void *)0x0802D2E9, (void *)0x0802D301, (void *)0x0802D319,
    (void *)0x0802D349, (void *)0x0802D361, (void *)0x0802D379, (void *)0x0802D391,
    (void *)0x0802ECF9, (void *)0x0802ED11, (void *)0x0802ED29, (void *)0x0802E371,
    (void *)0x0802E385, (void *)0x0802E399, (void *)0x0802F355, (void *)0x0802F369,
    (void *)0x08032F15, (void *)0x08032F29, (void *)0x08033CD9, (void *)0x08033CED,
    (void *)0x08073CFD, (void *)0x08074345, (void *)0x080355D5, (void *)0x08035B35,
    (void *)0x0807571D, (void *)0x0804AF99, (void *)0x08037845, (void *)0x08038015,
    (void *)0x080397A1, (void *)0x08039F29, (void *)0x080446E1, (void *)0x080446F5,
    (void *)0x08044709, (void *)0x080450B9, (void *)0x080450CD, (void *)0x08045765,
    (void *)0x0802C15D, (void *)0x0802C171, (void *)0x0802C185, (void *)0x08046819,
    (void *)0x0804682D, (void *)0x08046841, (void *)0x08047FED, (void *)0x080483B1,
    (void *)0x080483C5, (void *)0x08048D31, (void *)0x0804915D, (void *)0x08049171,
};

/* Decimal resource names "00" through "47" and generated-effect names. */
AT("001AD3A0") const char *const gTwoDigitResourceNames[48] = {
    (const char *)0x08086C30, (const char *)0x08086C2C, (const char *)0x08086C28, (const char *)0x08086C24,
    (const char *)0x08086C20, (const char *)0x08086C1C, (const char *)0x08086C18, (const char *)0x08086C14,
    (const char *)0x08086C10, (const char *)0x08086C0C, (const char *)0x08086C08, (const char *)0x08086C04,
    (const char *)0x08086C00, (const char *)0x08086BFC, (const char *)0x08086BF8, (const char *)0x08086BF4,
    (const char *)0x08086BF0, (const char *)0x08086BEC, (const char *)0x08086BE8, (const char *)0x08086BE4,
    (const char *)0x08086BE0, (const char *)0x08086BDC, (const char *)0x08086BD8, (const char *)0x08086BD4,
    (const char *)0x08086BD0, (const char *)0x08086BCC, (const char *)0x08086BC8, (const char *)0x08086BC4,
    (const char *)0x08086BC0, (const char *)0x08086BBC, (const char *)0x08086BB8, (const char *)0x08086BB4,
    (const char *)0x08086BB0, (const char *)0x08086BAC, (const char *)0x08086BA8, (const char *)0x08086BA4,
    (const char *)0x08086BA0, (const char *)0x08086B9C, (const char *)0x08086B98, (const char *)0x08086B94,
    (const char *)0x08086B90, (const char *)0x08086B8C, (const char *)0x08086B88, (const char *)0x08086B84,
    (const char *)0x08086B80, (const char *)0x08086B7C, (const char *)0x08086B78, (const char *)0x08086B74,
};
AT("001AD460") const char *const gGeneratedEffectNames[13] = {
    (const char *)0x08086D24, (const char *)0x08086D18, (const char *)0x08086D0C, (const char *)0x08086D00,
    (const char *)0x08086CF4, (const char *)0x08086CE8, (const char *)0x08086CDC, (const char *)0x08086CD0,
    (const char *)0x08086CC4, (const char *)0x08086CB8, (const char *)0x08086CAC, (const char *)0x08086CA0,
    (const char *)0x08086C94,
};
AT("001AD494") const u16 gResourceSlotIndices[4] = {
    0, 1, 2, 3,
};
AT("001AD49C") const u16 gResourceSlotMasks[4] = {
    256, 512, 1024, 2048,
};

/* Cumulative character-growth thresholds; values flatten at the level cap. */
AT("001AD4A4") const u32 gCharacterGrowthThresholds[100] = {
    0, 0, 10, 30, 60, 100, 160, 240,
    340, 460, 600, 770, 970, 1200, 1460, 1750,
    2070, 2420, 2800, 3210, 3650, 4150, 4670, 5210,
    5770, 6350, 6950, 7570, 8210, 8870, 9550, 10250,
    10970, 11710, 12470, 13250, 14050, 14870, 15710, 16570,
    17450, 18350, 19270, 20210, 21170, 22170, 23190, 24230,
    25290, 26390, 27540, 28740, 29990, 31290, 32640, 34040,
    35490, 36990, 38490, 39990, 41490, 42990, 44490, 45990,
    47490, 48990, 50490, 51990, 53490, 54990, 56490, 57990,
    59490, 60990, 62490, 63990, 65490, 66990, 68490, 69990,
    71490, 72990, 74490, 75990, 77490, 78990, 80490, 81990,
    83490, 84990, 86490, 87990, 89490, 90990, 92490, 93990,
    95490, 96990, 98490, 99999,
};

/* Eight engine startup/state handlers used by the pre-script dispatcher. */
AT("001AFE84") void *const gEngineStartupHandlers[8] = {
    (void *)0x0800080C, (void *)0x080008B4, (void *)0x08000978, (void *)0x08000A3C,
    (void *)0x08000B00, (void *)0x08000BAC, (void *)0x08000C70, (void *)0x08000D34,
};

/* Script-visible native command registry. Comments preserve the source names. */
AT("001AFEA4") const struct ScriptNativeCommand
    gScriptNativeCommands[SCRIPT_NATIVE_COMMAND_COUNT] = {
    {(const char *)0x0808737C, (void *)0x08011C69}, /*   0: MswStr */
    {(const char *)0x08087374, (void *)0x08011CC5}, /*   1: MswHit */
    {(const char *)0x0808736C, (void *)0x08011CD5}, /*   2: MswInit */
    {(const char *)0x08087364, (void *)0x08011CE5}, /*   3: FceInit */
    {(const char *)0x0808735C, (void *)0x08011D09}, /*   4: FceChg */
    {(const char *)0x08087354, (void *)0x08011D29}, /*   5: FceFree */
    {(const char *)0x0808734C, (void *)0x08011D51}, /*   6: FceSet */
    {(const char *)0x08087344, (void *)0x08011D79}, /*   7: FceGet */
    {(const char *)0x0808733C, (void *)0x08011D8D}, /*   8: FceMove */
    {(const char *)0x08087334, (void *)0x08011DB5}, /*   9: FceSync */
    {(const char *)0x0808732C, (void *)0x08011DD5}, /*  10: ChrInit */
    {(const char *)0x08087324, (void *)0x08011DFD}, /*  11: ChrChg */
    {(const char *)0x0808731C, (void *)0x08011E21}, /*  12: ChrFree */
    {(const char *)0x08087314, (void *)0x08011E49}, /*  13: ChrSet */
    {(const char *)0x0808730C, (void *)0x08011E71}, /*  14: ChrGet */
    {(const char *)0x08087304, (void *)0x08011E85}, /*  15: ChrMove */
    {(const char *)0x080872FC, (void *)0x08011EAD}, /*  16: ChrSync */
    {(const char *)0x080872F4, (void *)0x08011ECD}, /*  17: SprInit */
    {(const char *)0x080872EC, (void *)0x08011EF5}, /*  18: SprChg */
    {(const char *)0x080872E4, (void *)0x08011F15}, /*  19: SprFree */
    {(const char *)0x080872DC, (void *)0x08011F41}, /*  20: SprSet */
    {(const char *)0x080872D4, (void *)0x08011F69}, /*  21: SprGet */
    {(const char *)0x080872CC, (void *)0x08011F9D}, /*  22: SprMove */
    {(const char *)0x080872C4, (void *)0x08011FC5}, /*  23: SprSync */
    {(const char *)0x080872B8, (void *)0x08011F7D}, /*  24: SprHitRect */
    {(const char *)0x080872B0, (void *)0x0801200D}, /*  25: PccInit */
    {(const char *)0x080872A8, (void *)0x08012035}, /*  26: PccFree */
    {(const char *)0x080872A0, (void *)0x08012061}, /*  27: PccChg */
    {(const char *)0x08087298, (void *)0x08011FE5}, /*  28: PccSet */
    {(const char *)0x08087290, (void *)0x08012085}, /*  29: PccGet */
    {(const char *)0x08087288, (void *)0x080120A5}, /*  30: PccMove */
    {(const char *)0x08087280, (void *)0x080120D1}, /*  31: PccSync */
    {(const char *)0x08087278, (void *)0x080120F5}, /*  32: PccStat */
    {(const char *)0x08087270, (void *)0x08012141}, /*  33: ExtSet */
    {(const char *)0x08087268, (void *)0x08012169}, /*  34: ExtGet */
    {(const char *)0x08087260, (void *)0x0801217D}, /*  35: ExtMove */
    {(const char *)0x08087258, (void *)0x080121A5}, /*  36: ExtSync */
    {(const char *)0x08087250, (void *)0x080121C5}, /*  37: HitInit */
    {(const char *)0x08087248, (void *)0x08012231}, /*  38: HitGet */
    {(const char *)0x08087240, (void *)0x08012209}, /*  39: HitSet */
    {(const char *)0x08087238, (void *)0x080121E5}, /*  40: HitFree */
    {(const char *)0x0808722C, (void *)0x08012245}, /*  41: HitHitRect */
    {(const char *)0x08087224, (void *)0x08012265}, /*  42: BgSet */
    {(const char *)0x0808721C, (void *)0x080122F9}, /*  43: FldSet */
    {(const char *)0x08087214, (void *)0x08012319}, /*  44: FldGet */
    {(const char *)0x08087208, (void *)0x0801234D}, /*  45: CameraMode */
    {(const char *)0x080871FC, (void *)0x080129F5}, /*  46: PmbDeckMake */
    {(const char *)0x080871EC, (void *)0x08012AD5}, /*  47: PmbStatusInit */
    {(const char *)0x080871E0, (void *)0x08012AE5}, /*  48: PmbSetParty */
    {(const char *)0x080871D4, (void *)0x08012B65}, /*  49: PmdGetParty */
    {(const char *)0x080871C4, (void *)0x08012B81}, /*  50: PmbDefFashion */
    {(const char *)0x080871B4, (void *)0x08012B99}, /*  51: PmbSetFriend */
    {(const char *)0x080871A8, (void *)0x08012C15}, /*  52: BtlDeckMake */
    {(const char *)0x08087198, (void *)0x08012C55}, /*  53: BtlStatusInit */
    {(const char *)0x0808718C, (void *)0x08012C69}, /*  54: BtlSetParty */
    {(const char *)0x0808717C, (void *)0x08012CA5}, /*  55: BtlDeckShuffle */
    {(const char *)0x08087170, (void *)0x08012365}, /*  56: BtlFldLock */
    {(const char *)0x08087160, (void *)0x08012375}, /*  57: BtlFldUnLock */
    {(const char *)0x08087154, (void *)0x08012385}, /*  58: BtlFldRect */
    {(const char *)0x08087144, (void *)0x08012501}, /*  59: BtlGetCentorX */
    {(const char *)0x08087134, (void *)0x08012515}, /*  60: BtlGetCentorY */
    {(const char *)0x08087128, (void *)0x080123ED}, /*  61: BtlStart */
    {(const char *)0x08087120, (void *)0x080124E1}, /*  62: BtlWin */
    {(const char *)0x08087118, (void *)0x080123F9}, /*  63: BtlEnd */
    {(const char *)0x08087108, (void *)0x08012491}, /*  64: BtlFldSetInfo */
    {(const char *)0x080870F8, (void *)0x080124B1}, /*  65: BtlEscapeFlgSet */
    {(const char *)0x080870E8, (void *)0x080124C1}, /*  66: BtlSetWallView */
    {(const char *)0x080870DC, (void *)0x080124D1}, /*  67: BtlLogoPut */
    {(const char *)0x080870D0, (void *)0x08012529}, /*  68: DungGenInit */
    {(const char *)0x080870C4, (void *)0x08012545}, /*  69: DungGenFree */
    {(const char *)0x080870B8, (void *)0x08012571}, /*  70: DungGenMake */
    {(const char *)0x080870A8, (void *)0x080125AD}, /*  71: DungGenStart */
    {(const char *)0x08087098, (void *)0x08012559}, /*  72: DungGenResize */
    {(const char *)0x08087088, (void *)0x08012595}, /*  73: DungGenGetChrX */
    {(const char *)0x08087078, (void *)0x080125C1}, /*  74: DungGenGetChrY */
    {(const char *)0x08087068, (void *)0x080125D9}, /*  75: DungDispCoffer */
    {(const char *)0x08087058, (void *)0x080125F1}, /*  76: DungSetFloor */
    {(const char *)0x08087048, (void *)0x08012605}, /*  77: DungGetFloor */
    {(const char *)0x08087038, (void *)0x08012619}, /*  78: DungSetEvFlg */
    {(const char *)0x08087028, (void *)0x08012629}, /*  79: DungGetEvFlg */
    {(const char *)0x08087018, (void *)0x0801263D}, /*  80: DungGetGenNo */
    {(const char *)0x08087008, (void *)0x08012659}, /*  81: DungSetBtlOdds */
    {(const char *)0x08086FF8, (void *)0x0801266D}, /*  82: DungGetBtlOdds */
    {(const char *)0x08086FE8, (void *)0x08012681}, /*  83: DungGetBossFlg */
    {(const char *)0x08086FD8, (void *)0x080126A1}, /*  84: DungSetBossFlg */
    {(const char *)0x08086FC4, (void *)0x080126BD}, /*  85: DungGetBossTreasure */
    {(const char *)0x08086FB0, (void *)0x080126DD}, /*  86: DungSetBossTreasure */
    {(const char *)0x08086FA0, (void *)0x080126F9}, /*  87: DungGetBossBtl */
    {(const char *)0x08086F90, (void *)0x08012715}, /*  88: DungSetBossBtl */
    {(const char *)0x08086F80, (void *)0x08012731}, /*  89: DungSaveScrName */
    {(const char *)0x08086F70, (void *)0x08012761}, /*  90: DungGenEvClear */
    {(const char *)0x08086F60, (void *)0x0801276D}, /*  91: DungChangeTbl */
    {(const char *)0x08086F54, (void *)0x08012795}, /*  92: GetPlayMode */
    {(const char *)0x08086F48, (void *)0x080127A9}, /*  93: SetPlayMode */
    {(const char *)0x08086F3C, (void *)0x08012979}, /*  94: SetBabbo */
    {(const char *)0x08086F30, (void *)0x08012989}, /*  95: BgIntAttr */
    {(const char *)0x08086F20, (void *)0x08012995}, /*  96: BgSetAttrEnable */
    {(const char *)0x08086F10, (void *)0x08012995}, /*  97: BgGetAttrEnable */
    {(const char *)0x08086EFC, (void *)0x080129B9}, /*  98: BgSetAttrEnables */
    {(const char *)0x08086EF0, (void *)0x080129CD}, /*  99: GetIrqCause */
    {(const char *)0x08086EE0, (void *)0x080129E1}, /* 100: GetBtlRoomNo */
    {(const char *)0x08086ED4, (void *)0x080127F9}, /* 101: DeckMake */
    {(const char *)0x08086EC4, (void *)0x080127B9}, /* 102: ShuffleDeckMake */
    {(const char *)0x08086EB4, (void *)0x080128C9}, /* 103: ShuffleDeckCopy */
    {(const char *)0x08086EA8, (void *)0x0801294D}, /* 104: DeckShuffle */
    {(const char *)0x08086E9C, (void *)0x080124FD}, /* 105: BtlStatus */
    {(const char *)0x08086E94, (void *)0x08012CD1}, /* 106: PopMenu */
    {(const char *)0x08086E88, (void *)0x08012CE9}, /* 107: SysMenuExec */
    {(const char *)0x08086E78, (void *)0x08012D05}, /* 108: ShopListMake */
    {(const char *)0x08086E6C, (void *)0x08012D65}, /* 109: ItemInit */
    {(const char *)0x08086E64, (void *)0x08012D99}, /* 110: ItemGet */
    {(const char *)0x08086E58, (void *)0x08012DB5}, /* 111: GetHaveArm */
    {(const char *)0x08086E50, (void *)0x08012DF9}, /* 112: ArmGet */
    {(const char *)0x08086E44, (void *)0x08012E35}, /* 113: GetPewter */
    {(const char *)0x08086E38, (void *)0x08012E49}, /* 114: AddPewter */
    {(const char *)0x08086E28, (void *)0x08012E59}, /* 115: AllHpRecover */
    {(const char *)0x08086E18, (void *)0x08012E65}, /* 116: OneHpRecover */
    {(const char *)0x08086E08, (void *)0x08012D4D}, /* 117: GetSelectSpot */
    {(const char *)0x08086DF8, (void *)0x08012EA9}, /* 118: GetBoxInName */
    {(const char *)0x08086DEC, (void *)0x08012F05}, /* 119: GetDungData */
    {(const char *)0x08086DE0, (void *)0x08012F15}, /* 120: BuffRand */
    {(const char *)0x08086DD8, (void *)0x08012F39}, /* 121: SpotPut */
    {(const char *)0x08086DCC, (void *)0x08012F55}, /* 122: SioMapNoGet */
    {(const char *)0x08086DC0, (void *)0x08012F6D}, /* 123: StaffRoll */
    {(const char *)0x08086DB4, (void *)0x08012F81}, /* 124: SetDungCnt */
    {(const char *)0x08086DA8, (void *)0x08012F91}, /* 125: GetDungCnt */
    {(const char *)0x08086D9C, (void *)0x08012FA9}, /* 126: SetFhantom */
    {(const char *)0x08086D90, (void *)0x08012FB9}, /* 127: GetFhantom */
};
