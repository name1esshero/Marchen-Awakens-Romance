#include "game_tables.h"
#include "script.h"

#include "rom_section.h"

/* Not-yet-decompiled function-pointer table targets; see
 * asm/game_table_handlers.s. */
extern void sub_0800080C(void);
extern void sub_080008B4(void);
extern void sub_08000978(void);
extern void sub_08000A3C(void);
extern void sub_08000B00(void);
extern void sub_08000BAC(void);
extern void sub_08000C70(void);
extern void sub_08000D34(void);
extern void sub_08015269(void);
extern void sub_08023B99(void);
extern void sub_08023FA5(void);
extern void sub_08024701(void);
extern void sub_080256F9(void);
extern void sub_08026141(void);
extern void sub_08026699(void);
extern void sub_08026DC1(void);
extern void sub_080272FD(void);
extern void sub_08027895(void);
extern void sub_0802AE95(void);
extern void sub_0802AEA9(void);
extern void sub_0802BB0D(void);
extern void sub_0802C15D(void);
extern void sub_0802C171(void);
extern void sub_0802C185(void);
extern void sub_0802C585(void);
extern void sub_0802CCE9(void);
extern void sub_0802D0F1(void);
extern void sub_0802D109(void);
extern void sub_0802D121(void);
extern void sub_0802D139(void);
extern void sub_0802D151(void);
extern void sub_0802D169(void);
extern void sub_0802D181(void);
extern void sub_0802D199(void);
extern void sub_0802D1B1(void);
extern void sub_0802D1C9(void);
extern void sub_0802D1E1(void);
extern void sub_0802D1F9(void);
extern void sub_0802D211(void);
extern void sub_0802D229(void);
extern void sub_0802D241(void);
extern void sub_0802D259(void);
extern void sub_0802D271(void);
extern void sub_0802D289(void);
extern void sub_0802D2A1(void);
extern void sub_0802D2B9(void);
extern void sub_0802D2D1(void);
extern void sub_0802D2E9(void);
extern void sub_0802D301(void);
extern void sub_0802D319(void);
extern void sub_0802D331(void);
extern void sub_0802D349(void);
extern void sub_0802D361(void);
extern void sub_0802D379(void);
extern void sub_0802D391(void);
extern void sub_0802D3A9(void);
extern void sub_0802D8D1(void);
extern void sub_0802DB7D(void);
extern void sub_0802DB91(void);
extern void sub_0802DBA5(void);
extern void sub_0802DBB9(void);
extern void sub_0802DBCD(void);
extern void sub_0802E371(void);
extern void sub_0802E385(void);
extern void sub_0802E399(void);
extern void sub_0802E3AD(void);
extern void sub_0802E3C1(void);
extern void sub_0802ECF9(void);
extern void sub_0802ED11(void);
extern void sub_0802ED29(void);
extern void sub_0802ED41(void);
extern void sub_0802F355(void);
extern void sub_0802F369(void);
extern void sub_0802F37D(void);
extern void sub_0802F391(void);
extern void sub_0802FC79(void);
extern void sub_0802FC8D(void);
extern void sub_080301BD(void);
extern void sub_080308C9(void);
extern void sub_08030BC5(void);
extern void sub_08030E49(void);
extern void sub_08031109(void);
extern void sub_0803111D(void);
extern void sub_08031131(void);
extern void sub_08031145(void);
extern void sub_08031C35(void);
extern void sub_080322A5(void);
extern void sub_080322B9(void);
extern void sub_08032F15(void);
extern void sub_08032F29(void);
extern void sub_08032F3D(void);
extern void sub_08032F51(void);
extern void sub_08033CD9(void);
extern void sub_08033CED(void);
extern void sub_08034689(void);
extern void sub_0803469D(void);
extern void sub_08035005(void);
extern void sub_080355D5(void);
extern void sub_08035B35(void);
extern void sub_08036319(void);
extern void sub_08036C75(void);
extern void sub_08037845(void);
extern void sub_08038015(void);
extern void sub_08038B35(void);
extern void sub_08038B49(void);
extern void sub_080397A1(void);
extern void sub_08039F29(void);
extern void sub_08039F3D(void);
extern void sub_0803A9C5(void);
extern void sub_0803B7E5(void);
extern void sub_0803B7F9(void);
extern void sub_0803C071(void);
extern void sub_0803C085(void);
extern void sub_0803C099(void);
extern void sub_0803C0C1(void);
extern void sub_0803CCF1(void);
extern void sub_0803D505(void);
extern void sub_0803DE59(void);
extern void sub_0803E239(void);
extern void sub_0803E5E1(void);
extern void sub_0803EC35(void);
extern void sub_0803EF65(void);
extern void sub_0803F515(void);
extern void sub_0803FF15(void);
extern void sub_08040785(void);
extern void sub_08040799(void);
extern void sub_080410C1(void);
extern void sub_080414B5(void);
extern void sub_080414C9(void);
extern void sub_080414DD(void);
extern void sub_080414F1(void);
extern void sub_08041A05(void);
extern void sub_08041A19(void);
extern void sub_08041A2D(void);
extern void sub_08042239(void);
extern void sub_0804224D(void);
extern void sub_080427AD(void);
extern void sub_08042D95(void);
extern void sub_0804370D(void);
extern void sub_08043721(void);
extern void sub_08043735(void);
extern void sub_08043F9D(void);
extern void sub_0804441D(void);
extern void sub_080446E1(void);
extern void sub_080446F5(void);
extern void sub_08044709(void);
extern void sub_0804471D(void);
extern void sub_08044D4D(void);
extern void sub_080450B9(void);
extern void sub_080450CD(void);
extern void sub_08045765(void);
extern void sub_08045D85(void);
extern void sub_08046409(void);
extern void sub_08046819(void);
extern void sub_0804682D(void);
extern void sub_08046841(void);
extern void sub_08046FFD(void);
extern void sub_08047619(void);
extern void sub_08047FED(void);
extern void sub_080483B1(void);
extern void sub_080483C5(void);
extern void sub_080486D5(void);
extern void sub_08048D31(void);
extern void sub_0804915D(void);
extern void sub_08049171(void);
extern void sub_080496FD(void);
extern void sub_08049C49(void);
extern void sub_0804A3D9(void);
extern void sub_0804A3F1(void);
extern void sub_0804A409(void);
extern void sub_0804A421(void);
extern void sub_0804A451(void);
extern void sub_0804A469(void);
extern void sub_0804A481(void);
extern void sub_0804A499(void);
extern void sub_0804A4B1(void);
extern void sub_0804A4C9(void);
extern void sub_0804A4E1(void);
extern void sub_0804A4F9(void);
extern void sub_0804A511(void);
extern void sub_0804A529(void);
extern void sub_0804A541(void);
extern void sub_0804A559(void);
extern void sub_0804A571(void);
extern void sub_0804A589(void);
extern void sub_0804A5A1(void);
extern void sub_0804A5B9(void);
extern void sub_0804A5D1(void);
extern void sub_0804A5E9(void);
extern void sub_0804A601(void);
extern void sub_0804A619(void);
extern void sub_0804A631(void);
extern void sub_0804A649(void);
extern void sub_0804A661(void);
extern void sub_0804A679(void);
extern void sub_0804A691(void);
extern void sub_0804A6A9(void);
extern void sub_0804A6C1(void);
extern void sub_0804A6D9(void);
extern void sub_0804A6F1(void);
extern void sub_0804A709(void);
extern void sub_0804AC29(void);
extern void sub_0804AC3D(void);
extern void sub_0804AC51(void);
extern void sub_0804AC79(void);
extern void sub_0804AF99(void);
extern void sub_0804B549(void);
extern void sub_0804C259(void);
extern void sub_0804CB45(void);
extern void sub_0804CEDD(void);
extern void sub_0804D405(void);
extern void sub_0804DCD1(void);
extern void sub_0804E641(void);
extern void sub_0804E655(void);
extern void sub_0804EC29(void);
extern void sub_0804EC41(void);
extern void sub_0804EC59(void);
extern void sub_0804EC71(void);
extern void sub_0804EC89(void);
extern void sub_0804ECA1(void);
extern void sub_0804ECB9(void);
extern void sub_0804ECD1(void);
extern void sub_0804ECE9(void);
extern void sub_0804ED01(void);
extern void sub_0804ED19(void);
extern void sub_0804ED31(void);
extern void sub_0804ED49(void);
extern void sub_0804ED61(void);
extern void sub_0804ED79(void);
extern void sub_0804ED91(void);
extern void sub_0804EDA9(void);
extern void sub_0804EDC1(void);
extern void sub_0804EDD9(void);
extern void sub_0804EDF1(void);
extern void sub_0804EE09(void);
extern void sub_0804EE21(void);
extern void sub_0804EE39(void);
extern void sub_0804EE51(void);
extern void sub_0804EE69(void);
extern void sub_0804EE81(void);
extern void sub_0804EE99(void);
extern void sub_0804EEB1(void);
extern void sub_0804EEC9(void);
extern void sub_0804EEE1(void);
extern void sub_0804EEF9(void);
extern void sub_0804EF11(void);
extern void sub_0804EF29(void);
extern void sub_0804EF41(void);
extern void sub_0804EF59(void);
extern void sub_0804F409(void);
extern void sub_0804F41D(void);
extern void sub_0804F431(void);
extern void sub_0804FBA1(void);
extern void sub_08050359(void);
extern void sub_080507FD(void);
extern void sub_08050811(void);
extern void sub_08050825(void);
extern void sub_08051035(void);
extern void sub_08051649(void);
extern void sub_080516DD(void);
extern void sub_08072C15(void);
extern void sub_08073A15(void);
extern void sub_08073CFD(void);
extern void sub_08074345(void);
extern void sub_08074BC1(void);
extern void sub_08075299(void);
extern void sub_0807571D(void);

/* Fixed 16-halfword blocks copied into battle runtime work areas. */
AT("001ACC50") const u16 gBattleRuntimePresetA[BATTLE_PRESET_SIZE] = {
    0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};
AT("001ACC70") const u16 gBattleRuntimePresetB[BATTLE_PRESET_SIZE] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

/* Friend ARM definition index -> ownership bit. */
AT("001ACC90") const s16 gFriendArmOwnershipBits[8] = {
    1, 2, 12, FRIEND_ARM_NO_OWNERSHIP_BIT, 23, FRIEND_ARM_NO_OWNERSHIP_BIT, 21, 1,
};
AT("001ACCA0") const u16 gBattlePartyDefaults[8] = {
    1, 3, 4, 6, 9, 8, 7, 22,
};

/* Battle action/effect dispatch slots. Repeated fallback entries are
 * retained. Handlers are not yet individually decompiled; each name is
 * a plain alias for its ROM address, declared in asm/game_table_handlers.s. */
AT("001ACCB0") void *const gBattleActionHandlers[BATTLE_ACTION_HANDLER_COUNT] = {
    [0] = (void *)sub_08015269, [1] = (void *)sub_08015269, [2] = (void *)sub_08015269, [3] = (void *)sub_0803DE59,
    [4] = (void *)sub_0803E239, [5] = (void *)sub_0803E5E1, [6] = (void *)sub_0803EC35, [7] = (void *)sub_0803EF65,
    [8] = (void *)sub_0803F515, [9] = (void *)sub_0803FF15, [10] = (void *)sub_08040785, [11] = (void *)sub_08040799,
    [12] = (void *)sub_08015269, [13] = (void *)sub_08075299, [14] = (void *)sub_08075299, [15] = (void *)sub_0804CB45,
    [16] = (void *)sub_0804CEDD, [17] = (void *)sub_0804D405, [18] = (void *)sub_0804DCD1, [19] = (void *)sub_0804E641,
    [20] = (void *)sub_0804E655, [21] = (void *)sub_080256F9, [22] = (void *)sub_080256F9, [23] = (void *)sub_08015269,
    [24] = (void *)sub_08031109, [25] = (void *)sub_0803111D, [26] = (void *)sub_08031131, [27] = (void *)sub_08031145,
    [28] = (void *)sub_0804F409, [29] = (void *)sub_0804F41D, [30] = (void *)sub_0804F431, [31] = (void *)sub_0804FBA1,
    [32] = (void *)sub_08050359, [33] = (void *)sub_08031C35, [34] = (void *)sub_080322A5, [35] = (void *)sub_080322B9,
    [36] = (void *)sub_08015269, [37] = (void *)sub_080507FD, [38] = (void *)sub_08050811, [39] = (void *)sub_08050825,
    [40] = (void *)sub_08051035, [41] = (void *)sub_0802D8D1, [42] = (void *)sub_08051649, [43] = (void *)sub_080410C1,
    [44] = (void *)sub_08015269, [45] = (void *)sub_08015269, [46] = (void *)sub_08015269, [47] = (void *)sub_0802FC79,
    [48] = (void *)sub_0802FC8D, [49] = (void *)sub_080516DD, [50] = (void *)sub_0803C071, [51] = (void *)sub_0803C085,
    [52] = (void *)sub_0803C099, [53] = (void *)sub_0803C0C1, [54] = (void *)sub_08015269, [55] = (void *)sub_08015269,
    [56] = (void *)sub_0804AC29, [57] = (void *)sub_0804AC3D, [58] = (void *)sub_0804AC51, [59] = (void *)sub_0804AC79,
    [60] = (void *)sub_080414B5, [61] = (void *)sub_080414C9, [62] = (void *)sub_080414DD, [63] = (void *)sub_080414F1,
    [64] = (void *)sub_08041A05, [65] = (void *)sub_08041A19, [66] = (void *)sub_08041A2D, [67] = (void *)sub_08042239,
    [68] = (void *)sub_0804224D, [69] = (void *)sub_08026699, [70] = (void *)sub_0804A3D9, [71] = (void *)sub_0804A3F1,
    [72] = (void *)sub_0804A409, [73] = (void *)sub_0804A421, [74] = (void *)sub_080427AD, [75] = (void *)sub_08026DC1,
    [76] = (void *)sub_08042D95, [77] = (void *)sub_080301BD, [78] = (void *)sub_0803B7E5, [79] = (void *)sub_0803B7F9,
    [80] = (void *)sub_08027895, [81] = (void *)sub_0804370D, [82] = (void *)sub_08043721, [83] = (void *)sub_08043735,
    [84] = (void *)sub_08043F9D, [85] = (void *)sub_0804441D, [86] = (void *)sub_0804EC29, [87] = (void *)sub_0804ECD1,
    [88] = (void *)sub_0804ED79, [89] = (void *)sub_0804EE21, [90] = (void *)sub_0804EEC9, [91] = (void *)sub_0804EC41,
    [92] = (void *)sub_0804ECE9, [93] = (void *)sub_0804ED91, [94] = (void *)sub_0804EE39, [95] = (void *)sub_0804EEE1,
    [96] = (void *)sub_0804EC59, [97] = (void *)sub_0804ED01, [98] = (void *)sub_0804EDA9, [99] = (void *)sub_0804EE51,
    [100] = (void *)sub_0804EEF9, [101] = (void *)sub_0804EC71, [102] = (void *)sub_0804ED19, [103] = (void *)sub_0804EDC1,
    [104] = (void *)sub_0804EE69, [105] = (void *)sub_0804EF11, [106] = (void *)sub_0804EC89, [107] = (void *)sub_0804ED31,
    [108] = (void *)sub_0804EDD9, [109] = (void *)sub_0804EE81, [110] = (void *)sub_0804EF29, [111] = (void *)sub_0804ECA1,
    [112] = (void *)sub_0804ED49, [113] = (void *)sub_0804EDF1, [114] = (void *)sub_0804EE99, [115] = (void *)sub_0804EF41,
    [116] = (void *)sub_0804ECB9, [117] = (void *)sub_0804ED61, [118] = (void *)sub_0804EE09, [119] = (void *)sub_0804EEB1,
    [120] = (void *)sub_0804EF59, [121] = (void *)sub_0804A451, [122] = (void *)sub_0804A469, [123] = (void *)sub_0804A481,
    [124] = (void *)sub_0804A499, [125] = (void *)sub_0804A4B1, [126] = (void *)sub_0804A4C9, [127] = (void *)sub_0804A4E1,
    [128] = (void *)sub_0804A4F9, [129] = (void *)sub_0804A511, [130] = (void *)sub_0804A529, [131] = (void *)sub_0804A541,
    [132] = (void *)sub_0804A559, [133] = (void *)sub_0804A571, [134] = (void *)sub_0804A589, [135] = (void *)sub_0804A5A1,
    [136] = (void *)sub_0804A5B9, [137] = (void *)sub_0804A5D1, [138] = (void *)sub_0804A5E9, [139] = (void *)sub_0804A601,
    [140] = (void *)sub_0804A619, [141] = (void *)sub_0804A631, [142] = (void *)sub_0804A649, [143] = (void *)sub_0804A661,
    [144] = (void *)sub_0804A679, [145] = (void *)sub_0804A691, [146] = (void *)sub_0804A6A9, [147] = (void *)sub_0804A6C1,
    [148] = (void *)sub_0804A6D9, [149] = (void *)sub_0804A6F1, [150] = (void *)sub_0804A709, [151] = (void *)sub_0802D0F1,
    [152] = (void *)sub_0802D109, [153] = (void *)sub_0802D121, [154] = (void *)sub_0802D139, [155] = (void *)sub_0802D151,
    [156] = (void *)sub_0802D169, [157] = (void *)sub_0802D181, [158] = (void *)sub_0802D199, [159] = (void *)sub_0802D1B1,
    [160] = (void *)sub_0802D1C9, [161] = (void *)sub_0802D1E1, [162] = (void *)sub_0802D1F9, [163] = (void *)sub_0802D211,
    [164] = (void *)sub_0802D229, [165] = (void *)sub_0802D241, [166] = (void *)sub_0802D259, [167] = (void *)sub_0802D271,
    [168] = (void *)sub_0802D289, [169] = (void *)sub_0802D2A1, [170] = (void *)sub_0802D2B9, [171] = (void *)sub_0802D2D1,
    [172] = (void *)sub_0802D2E9, [173] = (void *)sub_0802D301, [174] = (void *)sub_0802D319, [175] = (void *)sub_0802D331,
    [176] = (void *)sub_0802D349, [177] = (void *)sub_0802D361, [178] = (void *)sub_0802D379, [179] = (void *)sub_0802D391,
    [180] = (void *)sub_0802D3A9, [181] = (void *)sub_08073A15, [182] = (void *)sub_08073A15, [183] = (void *)sub_08073A15,
    [184] = (void *)sub_0803CCF1, [185] = (void *)sub_0803D505, [186] = (void *)sub_0802DB7D, [187] = (void *)sub_0802DB91,
    [188] = (void *)sub_0802DBA5, [189] = (void *)sub_0802DBB9, [190] = (void *)sub_0802DBCD, [191] = (void *)sub_0802ECF9,
    [192] = (void *)sub_0802ED11, [193] = (void *)sub_0802ED29, [194] = (void *)sub_0802ED41, [195] = (void *)sub_0802E371,
    [196] = (void *)sub_0802E385, [197] = (void *)sub_0802E399, [198] = (void *)sub_0802E3AD, [199] = (void *)sub_0802E3C1,
    [200] = (void *)sub_0802F355, [201] = (void *)sub_0802F369, [202] = (void *)sub_0802F37D, [203] = (void *)sub_0802F391,
    [204] = (void *)sub_08032F15, [205] = (void *)sub_08032F29, [206] = (void *)sub_08032F3D, [207] = (void *)sub_08032F51,
    [208] = (void *)sub_08033CD9, [209] = (void *)sub_08033CED, [210] = (void *)sub_08034689, [211] = (void *)sub_0803469D,
    [212] = (void *)sub_08073CFD, [213] = (void *)sub_08074345, [214] = (void *)sub_08074BC1, [215] = (void *)sub_08035005,
    [216] = (void *)sub_080355D5, [217] = (void *)sub_08035B35, [218] = (void *)sub_08036319, [219] = (void *)sub_08036C75,
    [220] = (void *)sub_0807571D, [221] = (void *)sub_0804AF99, [222] = (void *)sub_0804B549, [223] = (void *)sub_0804C259,
    [224] = (void *)sub_08037845, [225] = (void *)sub_08038015, [226] = (void *)sub_08038B35, [227] = (void *)sub_08038B49,
    [228] = (void *)sub_080397A1, [229] = (void *)sub_08039F29, [230] = (void *)sub_08039F3D, [231] = (void *)sub_0803A9C5,
    [232] = (void *)sub_080446E1, [233] = (void *)sub_080446F5, [234] = (void *)sub_08044709, [235] = (void *)sub_0804471D,
    [236] = (void *)sub_08044D4D, [237] = (void *)sub_080450B9, [238] = (void *)sub_080450CD, [239] = (void *)sub_08045765,
    [240] = (void *)sub_08045D85, [241] = (void *)sub_08046409, [242] = (void *)sub_0802C15D, [243] = (void *)sub_0802C171,
    [244] = (void *)sub_0802C185, [245] = (void *)sub_0802C585, [246] = (void *)sub_0802CCE9, [247] = (void *)sub_08046819,
    [248] = (void *)sub_0804682D, [249] = (void *)sub_08046841, [250] = (void *)sub_08046FFD, [251] = (void *)sub_08047619,
    [252] = (void *)sub_08047FED, [253] = (void *)sub_080483B1, [254] = (void *)sub_080483C5, [255] = (void *)sub_080486D5,
    [256] = (void *)sub_080272FD, [257] = (void *)sub_08048D31, [258] = (void *)sub_0804915D, [259] = (void *)sub_08049171,
    [260] = (void *)sub_080496FD, [261] = (void *)sub_08049C49, [262] = (void *)sub_080308C9, [263] = (void *)sub_08030BC5,
    [264] = (void *)sub_08030E49, [265] = (void *)sub_08015269, [266] = (void *)sub_08015269, [267] = (void *)sub_08015269,
    [268] = (void *)sub_08015269, [269] = (void *)sub_08015269, [270] = (void *)sub_08015269, [271] = (void *)sub_08015269,
    [272] = (void *)sub_08015269, [273] = (void *)sub_08015269, [274] = (void *)sub_08015269, [275] = (void *)sub_08015269,
    [276] = (void *)sub_08015269, [277] = (void *)sub_08015269, [278] = (void *)sub_08015269, [279] = (void *)sub_08015269,
    [280] = (void *)sub_08015269, [281] = (void *)sub_08015269, [282] = (void *)sub_08015269, [283] = (void *)sub_08015269,
    [284] = (void *)sub_08015269, [285] = (void *)sub_08015269, [286] = (void *)sub_08015269, [287] = (void *)sub_08015269,
    [288] = (void *)sub_08015269, [289] = (void *)sub_08015269, [290] = (void *)sub_08015269, [291] = (void *)sub_08015269,
    [292] = (void *)sub_08015269, [293] = (void *)sub_08015269, [294] = (void *)sub_08015269, [295] = (void *)sub_08015269,
    [296] = (void *)sub_08015269, [297] = (void *)sub_08015269, [298] = (void *)sub_08015269, [299] = (void *)sub_08015269,
    [300] = (void *)sub_08023B99, [301] = (void *)sub_08023FA5, [302] = (void *)sub_08023FA5, [303] = (void *)sub_08024701,
    [304] = (void *)sub_08026141, [305] = (void *)sub_0802AE95, [306] = (void *)sub_0802AEA9, [307] = (void *)sub_0802BB0D,
    [308] = (void *)sub_08072C15, [309] = (void *)sub_08031109, [310] = (void *)sub_0803111D, [311] = (void *)sub_080507FD,
    [312] = (void *)sub_08050811, [313] = (void *)sub_08050825, [314] = (void *)sub_08051035, [315] = (void *)sub_080410C1,
    [316] = (void *)sub_0803C071, [317] = (void *)sub_0803C085, [318] = (void *)sub_0804A3D9, [319] = (void *)sub_0804A3F1,
    [320] = (void *)sub_0804A409, [321] = (void *)sub_0804EC29, [322] = (void *)sub_0804ECD1, [323] = (void *)sub_0804ED79,
    [324] = (void *)sub_0804EE21, [325] = (void *)sub_0804EEC9, [326] = (void *)sub_0804EC41, [327] = (void *)sub_0804ECE9,
    [328] = (void *)sub_0804ED91, [329] = (void *)sub_0804EE39, [330] = (void *)sub_0804EEE1, [331] = (void *)sub_0804EC59,
    [332] = (void *)sub_0804ED01, [333] = (void *)sub_0804EDA9, [334] = (void *)sub_0804EE51, [335] = (void *)sub_0804EEF9,
    [336] = (void *)sub_0804EC71, [337] = (void *)sub_0804ED19, [338] = (void *)sub_0804EDC1, [339] = (void *)sub_0804EE69,
    [340] = (void *)sub_0804EF11, [341] = (void *)sub_0804EC89, [342] = (void *)sub_0804ED31, [343] = (void *)sub_0804EDD9,
    [344] = (void *)sub_0804EE81, [345] = (void *)sub_0804EF29, [346] = (void *)sub_0804ECA1, [347] = (void *)sub_0804ED49,
    [348] = (void *)sub_0804EDF1, [349] = (void *)sub_0804EE99, [350] = (void *)sub_0804EF41, [351] = (void *)sub_0804ECB9,
    [352] = (void *)sub_0804ED61, [353] = (void *)sub_0804EE09, [354] = (void *)sub_0804EEB1, [355] = (void *)sub_0804EF59,
    [356] = (void *)sub_0804A451, [357] = (void *)sub_0804A469, [358] = (void *)sub_0804A481, [359] = (void *)sub_0804A499,
    [360] = (void *)sub_0804A4C9, [361] = (void *)sub_0804A4E1, [362] = (void *)sub_0804A4F9, [363] = (void *)sub_0804A511,
    [364] = (void *)sub_0804A541, [365] = (void *)sub_0804A559, [366] = (void *)sub_0804A571, [367] = (void *)sub_0804A589,
    [368] = (void *)sub_0804A5B9, [369] = (void *)sub_0804A5D1, [370] = (void *)sub_0804A5E9, [371] = (void *)sub_0804A601,
    [372] = (void *)sub_0804A631, [373] = (void *)sub_0804A649, [374] = (void *)sub_0804A661, [375] = (void *)sub_0804A679,
    [376] = (void *)sub_0804A6A9, [377] = (void *)sub_0804A6C1, [378] = (void *)sub_0804A6D9, [379] = (void *)sub_0804A6F1,
    [380] = (void *)sub_0802D0F1, [381] = (void *)sub_0802D109, [382] = (void *)sub_0802D121, [383] = (void *)sub_0802D139,
    [384] = (void *)sub_0802D169, [385] = (void *)sub_0802D181, [386] = (void *)sub_0802D199, [387] = (void *)sub_0802D1B1,
    [388] = (void *)sub_0802D1E1, [389] = (void *)sub_0802D1F9, [390] = (void *)sub_0802D211, [391] = (void *)sub_0802D229,
    [392] = (void *)sub_0802D259, [393] = (void *)sub_0802D271, [394] = (void *)sub_0802D289, [395] = (void *)sub_0802D2A1,
    [396] = (void *)sub_0802D2D1, [397] = (void *)sub_0802D2E9, [398] = (void *)sub_0802D301, [399] = (void *)sub_0802D319,
    [400] = (void *)sub_0802D349, [401] = (void *)sub_0802D361, [402] = (void *)sub_0802D379, [403] = (void *)sub_0802D391,
    [404] = (void *)sub_0802ECF9, [405] = (void *)sub_0802ED11, [406] = (void *)sub_0802ED29, [407] = (void *)sub_0802E371,
    [408] = (void *)sub_0802E385, [409] = (void *)sub_0802E399, [410] = (void *)sub_0802F355, [411] = (void *)sub_0802F369,
    [412] = (void *)sub_08032F15, [413] = (void *)sub_08032F29, [414] = (void *)sub_08033CD9, [415] = (void *)sub_08033CED,
    [416] = (void *)sub_08073CFD, [417] = (void *)sub_08074345, [418] = (void *)sub_080355D5, [419] = (void *)sub_08035B35,
    [420] = (void *)sub_0807571D, [421] = (void *)sub_0804AF99, [422] = (void *)sub_08037845, [423] = (void *)sub_08038015,
    [424] = (void *)sub_080397A1, [425] = (void *)sub_08039F29, [426] = (void *)sub_080446E1, [427] = (void *)sub_080446F5,
    [428] = (void *)sub_08044709, [429] = (void *)sub_080450B9, [430] = (void *)sub_080450CD, [431] = (void *)sub_08045765,
    [432] = (void *)sub_0802C15D, [433] = (void *)sub_0802C171, [434] = (void *)sub_0802C185, [435] = (void *)sub_08046819,
    [436] = (void *)sub_0804682D, [437] = (void *)sub_08046841, [438] = (void *)sub_08047FED, [439] = (void *)sub_080483B1,
    [440] = (void *)sub_080483C5, [441] = (void *)sub_08048D31, [442] = (void *)sub_0804915D, [443] = (void *)sub_08049171,
};


/* Decimal resource names "00" through "47", named in place at their fixed
 * ROM addresses rather than copied, since the strings are shared with other
 * unrelated data in the same pool. */
#define gTwoDigitName00 ((const char *)0x08086C30)
#define gTwoDigitName01 ((const char *)0x08086C2C)
#define gTwoDigitName02 ((const char *)0x08086C28)
#define gTwoDigitName03 ((const char *)0x08086C24)
#define gTwoDigitName04 ((const char *)0x08086C20)
#define gTwoDigitName05 ((const char *)0x08086C1C)
#define gTwoDigitName06 ((const char *)0x08086C18)
#define gTwoDigitName07 ((const char *)0x08086C14)
#define gTwoDigitName08 ((const char *)0x08086C10)
#define gTwoDigitName09 ((const char *)0x08086C0C)
#define gTwoDigitName10 ((const char *)0x08086C08)
#define gTwoDigitName11 ((const char *)0x08086C04)
#define gTwoDigitName12 ((const char *)0x08086C00)
#define gTwoDigitName13 ((const char *)0x08086BFC)
#define gTwoDigitName14 ((const char *)0x08086BF8)
#define gTwoDigitName15 ((const char *)0x08086BF4)
#define gTwoDigitName16 ((const char *)0x08086BF0)
#define gTwoDigitName17 ((const char *)0x08086BEC)
#define gTwoDigitName18 ((const char *)0x08086BE8)
#define gTwoDigitName19 ((const char *)0x08086BE4)
#define gTwoDigitName20 ((const char *)0x08086BE0)
#define gTwoDigitName21 ((const char *)0x08086BDC)
#define gTwoDigitName22 ((const char *)0x08086BD8)
#define gTwoDigitName23 ((const char *)0x08086BD4)
#define gTwoDigitName24 ((const char *)0x08086BD0)
#define gTwoDigitName25 ((const char *)0x08086BCC)
#define gTwoDigitName26 ((const char *)0x08086BC8)
#define gTwoDigitName27 ((const char *)0x08086BC4)
#define gTwoDigitName28 ((const char *)0x08086BC0)
#define gTwoDigitName29 ((const char *)0x08086BBC)
#define gTwoDigitName30 ((const char *)0x08086BB8)
#define gTwoDigitName31 ((const char *)0x08086BB4)
#define gTwoDigitName32 ((const char *)0x08086BB0)
#define gTwoDigitName33 ((const char *)0x08086BAC)
#define gTwoDigitName34 ((const char *)0x08086BA8)
#define gTwoDigitName35 ((const char *)0x08086BA4)
#define gTwoDigitName36 ((const char *)0x08086BA0)
#define gTwoDigitName37 ((const char *)0x08086B9C)
#define gTwoDigitName38 ((const char *)0x08086B98)
#define gTwoDigitName39 ((const char *)0x08086B94)
#define gTwoDigitName40 ((const char *)0x08086B90)
#define gTwoDigitName41 ((const char *)0x08086B8C)
#define gTwoDigitName42 ((const char *)0x08086B88)
#define gTwoDigitName43 ((const char *)0x08086B84)
#define gTwoDigitName44 ((const char *)0x08086B80)
#define gTwoDigitName45 ((const char *)0x08086B7C)
#define gTwoDigitName46 ((const char *)0x08086B78)
#define gTwoDigitName47 ((const char *)0x08086B74)

AT("001AD3A0") const char *const gTwoDigitResourceNames[TWO_DIGIT_RESOURCE_NAME_COUNT] = {
    [0] = gTwoDigitName00, [1] = gTwoDigitName01, [2] = gTwoDigitName02, [3] = gTwoDigitName03,
    [4] = gTwoDigitName04, [5] = gTwoDigitName05, [6] = gTwoDigitName06, [7] = gTwoDigitName07,
    [8] = gTwoDigitName08, [9] = gTwoDigitName09, [10] = gTwoDigitName10, [11] = gTwoDigitName11,
    [12] = gTwoDigitName12, [13] = gTwoDigitName13, [14] = gTwoDigitName14, [15] = gTwoDigitName15,
    [16] = gTwoDigitName16, [17] = gTwoDigitName17, [18] = gTwoDigitName18, [19] = gTwoDigitName19,
    [20] = gTwoDigitName20, [21] = gTwoDigitName21, [22] = gTwoDigitName22, [23] = gTwoDigitName23,
    [24] = gTwoDigitName24, [25] = gTwoDigitName25, [26] = gTwoDigitName26, [27] = gTwoDigitName27,
    [28] = gTwoDigitName28, [29] = gTwoDigitName29, [30] = gTwoDigitName30, [31] = gTwoDigitName31,
    [32] = gTwoDigitName32, [33] = gTwoDigitName33, [34] = gTwoDigitName34, [35] = gTwoDigitName35,
    [36] = gTwoDigitName36, [37] = gTwoDigitName37, [38] = gTwoDigitName38, [39] = gTwoDigitName39,
    [40] = gTwoDigitName40, [41] = gTwoDigitName41, [42] = gTwoDigitName42, [43] = gTwoDigitName43,
    [44] = gTwoDigitName44, [45] = gTwoDigitName45, [46] = gTwoDigitName46, [47] = gTwoDigitName47,
};

/* Names of thirteen procedurally-generated battle effects, named in place
 * for the same reason as gTwoDigitResourceNames above. */
#define gEffectNameEF_GEN01 ((const char *)0x08086D24)
#define gEffectNameEF_GEN02 ((const char *)0x08086D18)
#define gEffectNameEF_GEN03 ((const char *)0x08086D0C)
#define gEffectNameEF_GEN04 ((const char *)0x08086D00)
#define gEffectNameEF_GEN05 ((const char *)0x08086CF4)
#define gEffectNameEF_GEN06 ((const char *)0x08086CE8)
#define gEffectNameEF_GEN07 ((const char *)0x08086CDC)
#define gEffectNameEF_GEN08 ((const char *)0x08086CD0)
#define gEffectNameEF_GEN09 ((const char *)0x08086CC4)
#define gEffectNameEF_GEN10 ((const char *)0x08086CB8)
#define gEffectNameEF_GEN11 ((const char *)0x08086CAC)
#define gEffectNameEF_GEN12 ((const char *)0x08086CA0)
#define gEffectNameEF_GEN13 ((const char *)0x08086C94)

AT("001AD460") const char *const gGeneratedEffectNames[GENERATED_EFFECT_NAME_COUNT] = {
    [0] = gEffectNameEF_GEN01, [1] = gEffectNameEF_GEN02, [2] = gEffectNameEF_GEN03, [3] = gEffectNameEF_GEN04,
    [4] = gEffectNameEF_GEN05, [5] = gEffectNameEF_GEN06, [6] = gEffectNameEF_GEN07, [7] = gEffectNameEF_GEN08,
    [8] = gEffectNameEF_GEN09, [9] = gEffectNameEF_GEN10, [10] = gEffectNameEF_GEN11, [11] = gEffectNameEF_GEN12,
    [12] = gEffectNameEF_GEN13,
};


AT("001AD494") const u16 gResourceSlotIndices[4] = {
    0, 1, 2, 3,
};
AT("001AD49C") const u16 gResourceSlotMasks[4] = {
    1 << 8, 1 << 9, 1 << 10, 1 << 11,
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
AT("001AFE84") void *const gEngineStartupHandlers[ENGINE_STARTUP_HANDLER_COUNT] = {
    (void *)sub_0800080C, (void *)sub_080008B4, (void *)sub_08000978, (void *)sub_08000A3C,
    (void *)sub_08000B00, (void *)sub_08000BAC, (void *)sub_08000C70, (void *)sub_08000D34,
};

/* Script-visible native command registry. Names are pointers into the ASCII
 * pool near 08086D00. Handler addresses in the ROM have the thumb bit set;
 * the four still-unnamed ones are declared here by their real (even) address. */
#define gScriptNativeName_MswStr ((const char *)0x0808737C)
#define gScriptNativeName_MswHit ((const char *)0x08087374)
#define gScriptNativeName_MswInit ((const char *)0x0808736C)
#define gScriptNativeName_FceInit ((const char *)0x08087364)
#define gScriptNativeName_FceChg ((const char *)0x0808735C)
#define gScriptNativeName_FceFree ((const char *)0x08087354)
#define gScriptNativeName_FceSet ((const char *)0x0808734C)
#define gScriptNativeName_FceGet ((const char *)0x08087344)
#define gScriptNativeName_FceMove ((const char *)0x0808733C)
#define gScriptNativeName_FceSync ((const char *)0x08087334)
#define gScriptNativeName_ChrInit ((const char *)0x0808732C)
#define gScriptNativeName_ChrChg ((const char *)0x08087324)
#define gScriptNativeName_ChrFree ((const char *)0x0808731C)
#define gScriptNativeName_ChrSet ((const char *)0x08087314)
#define gScriptNativeName_ChrGet ((const char *)0x0808730C)
#define gScriptNativeName_ChrMove ((const char *)0x08087304)
#define gScriptNativeName_ChrSync ((const char *)0x080872FC)
#define gScriptNativeName_SprInit ((const char *)0x080872F4)
#define gScriptNativeName_SprChg ((const char *)0x080872EC)
#define gScriptNativeName_SprFree ((const char *)0x080872E4)
#define gScriptNativeName_SprSet ((const char *)0x080872DC)
#define gScriptNativeName_SprGet ((const char *)0x080872D4)
#define gScriptNativeName_SprMove ((const char *)0x080872CC)
#define gScriptNativeName_SprSync ((const char *)0x080872C4)
#define gScriptNativeName_SprHitRect ((const char *)0x080872B8)
#define gScriptNativeName_PccInit ((const char *)0x080872B0)
#define gScriptNativeName_PccFree ((const char *)0x080872A8)
#define gScriptNativeName_PccChg ((const char *)0x080872A0)
#define gScriptNativeName_PccSet ((const char *)0x08087298)
#define gScriptNativeName_PccGet ((const char *)0x08087290)
#define gScriptNativeName_PccMove ((const char *)0x08087288)
#define gScriptNativeName_PccSync ((const char *)0x08087280)
#define gScriptNativeName_PccStat ((const char *)0x08087278)
#define gScriptNativeName_ExtSet ((const char *)0x08087270)
#define gScriptNativeName_ExtGet ((const char *)0x08087268)
#define gScriptNativeName_ExtMove ((const char *)0x08087260)
#define gScriptNativeName_ExtSync ((const char *)0x08087258)
#define gScriptNativeName_HitInit ((const char *)0x08087250)
#define gScriptNativeName_HitGet ((const char *)0x08087248)
#define gScriptNativeName_HitSet ((const char *)0x08087240)
#define gScriptNativeName_HitFree ((const char *)0x08087238)
#define gScriptNativeName_HitHitRect ((const char *)0x0808722C)
#define gScriptNativeName_BgSet ((const char *)0x08087224)
#define gScriptNativeName_FldSet ((const char *)0x0808721C)
#define gScriptNativeName_FldGet ((const char *)0x08087214)
#define gScriptNativeName_CameraMode ((const char *)0x08087208)
#define gScriptNativeName_PmbDeckMake ((const char *)0x080871FC)
#define gScriptNativeName_PmbStatusInit ((const char *)0x080871EC)
#define gScriptNativeName_PmbSetParty ((const char *)0x080871E0)
#define gScriptNativeName_PmdGetParty ((const char *)0x080871D4)
#define gScriptNativeName_PmbDefFashion ((const char *)0x080871C4)
#define gScriptNativeName_PmbSetFriend ((const char *)0x080871B4)
#define gScriptNativeName_BtlDeckMake ((const char *)0x080871A8)
#define gScriptNativeName_BtlStatusInit ((const char *)0x08087198)
#define gScriptNativeName_BtlSetParty ((const char *)0x0808718C)
#define gScriptNativeName_BtlDeckShuffle ((const char *)0x0808717C)
#define gScriptNativeName_BtlFldLock ((const char *)0x08087170)
#define gScriptNativeName_BtlFldUnLock ((const char *)0x08087160)
#define gScriptNativeName_BtlFldRect ((const char *)0x08087154)
#define gScriptNativeName_BtlGetCentorX ((const char *)0x08087144)
#define gScriptNativeName_BtlGetCentorY ((const char *)0x08087134)
#define gScriptNativeName_BtlStart ((const char *)0x08087128)
#define gScriptNativeName_BtlWin ((const char *)0x08087120)
#define gScriptNativeName_BtlEnd ((const char *)0x08087118)
#define gScriptNativeName_BtlFldSetInfo ((const char *)0x08087108)
#define gScriptNativeName_BtlEscapeFlgSet ((const char *)0x080870F8)
#define gScriptNativeName_BtlSetWallView ((const char *)0x080870E8)
#define gScriptNativeName_BtlLogoPut ((const char *)0x080870DC)
#define gScriptNativeName_DungGenInit ((const char *)0x080870D0)
#define gScriptNativeName_DungGenFree ((const char *)0x080870C4)
#define gScriptNativeName_DungGenMake ((const char *)0x080870B8)
#define gScriptNativeName_DungGenStart ((const char *)0x080870A8)
#define gScriptNativeName_DungGenResize ((const char *)0x08087098)
#define gScriptNativeName_DungGenGetChrX ((const char *)0x08087088)
#define gScriptNativeName_DungGenGetChrY ((const char *)0x08087078)
#define gScriptNativeName_DungDispCoffer ((const char *)0x08087068)
#define gScriptNativeName_DungSetFloor ((const char *)0x08087058)
#define gScriptNativeName_DungGetFloor ((const char *)0x08087048)
#define gScriptNativeName_DungSetEvFlg ((const char *)0x08087038)
#define gScriptNativeName_DungGetEvFlg ((const char *)0x08087028)
#define gScriptNativeName_DungGetGenNo ((const char *)0x08087018)
#define gScriptNativeName_DungSetBtlOdds ((const char *)0x08087008)
#define gScriptNativeName_DungGetBtlOdds ((const char *)0x08086FF8)
#define gScriptNativeName_DungGetBossFlg ((const char *)0x08086FE8)
#define gScriptNativeName_DungSetBossFlg ((const char *)0x08086FD8)
#define gScriptNativeName_DungGetBossTreasure ((const char *)0x08086FC4)
#define gScriptNativeName_DungSetBossTreasure ((const char *)0x08086FB0)
#define gScriptNativeName_DungGetBossBtl ((const char *)0x08086FA0)
#define gScriptNativeName_DungSetBossBtl ((const char *)0x08086F90)
#define gScriptNativeName_DungSaveScrName ((const char *)0x08086F80)
#define gScriptNativeName_DungGenEvClear ((const char *)0x08086F70)
#define gScriptNativeName_DungChangeTbl ((const char *)0x08086F60)
#define gScriptNativeName_GetPlayMode ((const char *)0x08086F54)
#define gScriptNativeName_SetPlayMode ((const char *)0x08086F48)
#define gScriptNativeName_SetBabbo ((const char *)0x08086F3C)
#define gScriptNativeName_BgIntAttr ((const char *)0x08086F30)
#define gScriptNativeName_BgSetAttrEnable ((const char *)0x08086F20)
#define gScriptNativeName_BgGetAttrEnable ((const char *)0x08086F10)
#define gScriptNativeName_BgSetAttrEnables ((const char *)0x08086EFC)
#define gScriptNativeName_GetIrqCause ((const char *)0x08086EF0)
#define gScriptNativeName_GetBtlRoomNo ((const char *)0x08086EE0)
#define gScriptNativeName_DeckMake ((const char *)0x08086ED4)
#define gScriptNativeName_ShuffleDeckMake ((const char *)0x08086EC4)
#define gScriptNativeName_ShuffleDeckCopy ((const char *)0x08086EB4)
#define gScriptNativeName_DeckShuffle ((const char *)0x08086EA8)
#define gScriptNativeName_BtlStatus ((const char *)0x08086E9C)
#define gScriptNativeName_PopMenu ((const char *)0x08086E94)
#define gScriptNativeName_SysMenuExec ((const char *)0x08086E88)
#define gScriptNativeName_ShopListMake ((const char *)0x08086E78)
#define gScriptNativeName_ItemInit ((const char *)0x08086E6C)
#define gScriptNativeName_ItemGet ((const char *)0x08086E64)
#define gScriptNativeName_GetHaveArm ((const char *)0x08086E58)
#define gScriptNativeName_ArmGet ((const char *)0x08086E50)
#define gScriptNativeName_GetPewter ((const char *)0x08086E44)
#define gScriptNativeName_AddPewter ((const char *)0x08086E38)
#define gScriptNativeName_AllHpRecover ((const char *)0x08086E28)
#define gScriptNativeName_OneHpRecover ((const char *)0x08086E18)
#define gScriptNativeName_GetSelectSpot ((const char *)0x08086E08)
#define gScriptNativeName_GetBoxInName ((const char *)0x08086DF8)
#define gScriptNativeName_GetDungData ((const char *)0x08086DEC)
#define gScriptNativeName_BuffRand ((const char *)0x08086DE0)
#define gScriptNativeName_SpotPut ((const char *)0x08086DD8)
#define gScriptNativeName_SioMapNoGet ((const char *)0x08086DCC)
#define gScriptNativeName_StaffRoll ((const char *)0x08086DC0)
#define gScriptNativeName_SetDungCnt ((const char *)0x08086DB4)
#define gScriptNativeName_GetDungCnt ((const char *)0x08086DA8)
#define gScriptNativeName_SetFhantom ((const char *)0x08086D9C)
#define gScriptNativeName_GetFhantom ((const char *)0x08086D90)

extern void DialogueCommandShow(void);
extern void DialogueCommandPrompt(void);
extern void DialogueCommandFinish(void);
extern void ScriptNativeEffectAConfigure6(void);
extern void ScriptNativeEffectAConfigure5(void);
extern void ScriptNativeEffectAWait(void);
extern void ScriptNativeEffectAStart(void);
extern void ScriptNativeEffectAQuery(void);
extern void ScriptNativeEffectAStartFull(void);
extern void ScriptNativeEffectACommand(void);
extern void ScriptNativeEffectBStartFull(void);
extern void ScriptNativeEffectBConfigure(void);
extern void ScriptNativeEffectBWait(void);
extern void ScriptNativeEffectBStart(void);
extern void ScriptNativeEffectBQuery(void);
extern void ScriptNativeEffectBStartExtended(void);
extern void ScriptNativeEffectBCommand(void);
extern void ScriptNativeSpriteInit(void);
extern void ScriptNativeSpriteChange(void);
extern void ScriptNativeSpriteWait(void);
extern void ScriptNativeSpriteSet(void);
extern void ScriptNativeSpriteGet(void);
extern void ScriptNativeSpriteEffect(void);
extern void ScriptNativeSpriteCommand(void);
extern void ScriptNativeSpriteConfigure(void);
extern void ScriptNativeFieldEffectStartFull(void);
extern void ScriptNativeFieldEffectWait(void);
extern void ScriptNativeFieldEffectConfigure(void);
extern void ScriptNativeFieldEffectStart(void);
extern void ScriptNativeFieldEffectCommand(void);
extern void ScriptNativeFieldEffectStart8(void);
extern void ScriptNativeFieldEffectQuery(void);
extern void ScriptNativeSetActorName(void);
extern void ScriptNativeEffectCStart(void);
extern void ScriptNativeEffectCQuery(void);
extern void ScriptNativeEffectCStartFull(void);
extern void ScriptNativeEffectCCommand(void);
extern void ScriptNativeHitInit(void);
extern void ScriptNativeHitEffectQuery(void);
extern void ScriptNativeHitEffectStart(void);
extern void ScriptNativeHitFree(void);
extern void ScriptNativeHitRect(void);
extern void sub_08012264(void);
extern void ScriptNativeFieldSet(void);
extern void ScriptNativeFieldGet(void);
extern void ScriptNativeSetRuntimePair(void);
extern void sub_080129F4(void);
extern void ScriptNativeCall08740(void);
extern void ScriptNativeConfigureResourceSlots(void);
extern void ScriptNativeQueryResourceSlot(void);
extern void ScriptNativeSelectLayer(void);
extern void ScriptNativeSetFriendArms(void);
extern void ScriptNativeWriteMapValuesB(void);
extern void ScriptNativeCall088E0(void);
extern void ScriptNativeSetBattleParty(void);
extern void ScriptNativeRefreshMapValuesB(void);
extern void ScriptNativeBattleFieldLock(void);
extern void ScriptNativeBattleFieldUnlock(void);
extern void ScriptNativeGetBattleFieldRect(void);
extern void ScriptNativeMapGetPointer1COffset(void);
extern void ScriptNativeMapGetPointer20Offset(void);
extern void ScriptNativeCall08968(void);
extern void ScriptNativeMapResetActor(void);
extern void ScriptNativeResetFieldScene(void);
extern void ScriptNativeMapSetBoundedValue(void);
extern void ScriptNativeMapSetValue08(void);
extern void ScriptNativeMapSetValue25(void);
extern void ScriptNativeMapResetGenerator(void);
extern void ScriptNativeMapConfigure3(void);
extern void ScriptNativeMapRefresh(void);
extern void ScriptNativeMapRandomize(void);
extern void ScriptNativeMapClearDState(void);
extern void ScriptNativeMapConfigure2(void);
extern void ScriptNativeMapQueryD60(void);
extern void ScriptNativeMapQueryD84(void);
extern void ScriptNativeMapSetMode(void);
extern void ScriptNativeMapSetSeed(void);
extern void ScriptNativeMapGetSeed(void);
extern void ScriptNativeMapCall1088(void);
extern void ScriptNativeMapQuery10A4(void);
extern void ScriptNativeMapGetField618(void);
extern void ScriptNativeMapSetField10(void);
extern void ScriptNativeMapGetField10(void);
extern void ScriptNativeMapGetField624(void);
extern void ScriptNativeMapSetField624(void);
extern void ScriptNativeMapGetField626(void);
extern void ScriptNativeMapSetField626(void);
extern void ScriptNativeMapGetField628(void);
extern void ScriptNativeMapSetField628(void);
extern void ScriptNativeCopyGeneratedName(void);
extern void ScriptNativeMapFinalize(void);
extern void ScriptNativeMapSelectSlot(void);
extern void ScriptNativeGetField4258(void);
extern void ScriptNativeSetField4258(void);
extern void ScriptNativeSetField12EC(void);
extern void ScriptNativeCall067DC(void);
extern void ScriptNativeCall0680C(void);
extern void ScriptNativeCall06858(void);
extern void ScriptNativeGetField4256(void);
extern void ScriptNativeGetField60E(void);
extern void sub_080127F8(void);
extern void ScriptNativeWriteMapValues(void);
extern void sub_080128C8(void);
extern void ScriptNativeRefreshMapValuesA(void);
extern void ScriptNativeBattleStatus(void);
extern void ScriptNativeMapCoordinateCall(void);
extern void ScriptNativeSetPendingMapValue(void);
extern void ScriptNativeCopyMapHalfwords(void);
extern void ScriptNativeClearMapHalfwords(void);
extern void ScriptNativeQueryResourceId(void);
extern void ScriptNativeQueryModeResource(void);
extern void ScriptNativeSetModeResource(void);
extern void ScriptNativeQueryResourceState(void);
extern void ScriptNativeSetResourceState(void);
extern void ScriptNativeResetResourceState(void);
extern void ScriptNativeUseResource(void);
extern void ScriptNativeGetMapStatus(void);
extern void ScriptNativeGetResourceName(void);
extern void ScriptNativeResetEncounterState(void);
extern void ScriptNativeChooseRandomValue(void);
extern void ScriptNativeStartMapCoordinateEvent(void);
extern void ScriptNativeGetField42BA(void);
extern void ScriptNativeStartEncounter(void);
extern void ScriptNativeSetEncounterValue(void);
extern void ScriptNativeGetEncounterValue(void);
extern void ScriptNativeSetEncounterMode(void);
extern void ScriptNativeGetEncounterMode(void);

AT("001AFEA4") const struct ScriptNativeCommand
    gScriptNativeCommands[SCRIPT_NATIVE_COMMAND_COUNT] = {
    [0] = { .name = gScriptNativeName_MswStr, .handler = (void *)((u32)DialogueCommandShow + 1) },
    [1] = { .name = gScriptNativeName_MswHit, .handler = (void *)((u32)DialogueCommandPrompt + 1) },
    [2] = { .name = gScriptNativeName_MswInit, .handler = (void *)((u32)DialogueCommandFinish + 1) },
    [3] = { .name = gScriptNativeName_FceInit, .handler = (void *)((u32)ScriptNativeEffectAConfigure6 + 1) },
    [4] = { .name = gScriptNativeName_FceChg, .handler = (void *)((u32)ScriptNativeEffectAConfigure5 + 1) },
    [5] = { .name = gScriptNativeName_FceFree, .handler = (void *)((u32)ScriptNativeEffectAWait + 1) },
    [6] = { .name = gScriptNativeName_FceSet, .handler = (void *)((u32)ScriptNativeEffectAStart + 1) },
    [7] = { .name = gScriptNativeName_FceGet, .handler = (void *)((u32)ScriptNativeEffectAQuery + 1) },
    [8] = { .name = gScriptNativeName_FceMove, .handler = (void *)((u32)ScriptNativeEffectAStartFull + 1) },
    [9] = { .name = gScriptNativeName_FceSync, .handler = (void *)((u32)ScriptNativeEffectACommand + 1) },
    [10] = { .name = gScriptNativeName_ChrInit, .handler = (void *)((u32)ScriptNativeEffectBStartFull + 1) },
    [11] = { .name = gScriptNativeName_ChrChg, .handler = (void *)((u32)ScriptNativeEffectBConfigure + 1) },
    [12] = { .name = gScriptNativeName_ChrFree, .handler = (void *)((u32)ScriptNativeEffectBWait + 1) },
    [13] = { .name = gScriptNativeName_ChrSet, .handler = (void *)((u32)ScriptNativeEffectBStart + 1) },
    [14] = { .name = gScriptNativeName_ChrGet, .handler = (void *)((u32)ScriptNativeEffectBQuery + 1) },
    [15] = { .name = gScriptNativeName_ChrMove, .handler = (void *)((u32)ScriptNativeEffectBStartExtended + 1) },
    [16] = { .name = gScriptNativeName_ChrSync, .handler = (void *)((u32)ScriptNativeEffectBCommand + 1) },
    [17] = { .name = gScriptNativeName_SprInit, .handler = (void *)((u32)ScriptNativeSpriteInit + 1) },
    [18] = { .name = gScriptNativeName_SprChg, .handler = (void *)((u32)ScriptNativeSpriteChange + 1) },
    [19] = { .name = gScriptNativeName_SprFree, .handler = (void *)((u32)ScriptNativeSpriteWait + 1) },
    [20] = { .name = gScriptNativeName_SprSet, .handler = (void *)((u32)ScriptNativeSpriteSet + 1) },
    [21] = { .name = gScriptNativeName_SprGet, .handler = (void *)((u32)ScriptNativeSpriteGet + 1) },
    [22] = { .name = gScriptNativeName_SprMove, .handler = (void *)((u32)ScriptNativeSpriteEffect + 1) },
    [23] = { .name = gScriptNativeName_SprSync, .handler = (void *)((u32)ScriptNativeSpriteCommand + 1) },
    [24] = { .name = gScriptNativeName_SprHitRect, .handler = (void *)((u32)ScriptNativeSpriteConfigure + 1) },
    [25] = { .name = gScriptNativeName_PccInit, .handler = (void *)((u32)ScriptNativeFieldEffectStartFull + 1) },
    [26] = { .name = gScriptNativeName_PccFree, .handler = (void *)((u32)ScriptNativeFieldEffectWait + 1) },
    [27] = { .name = gScriptNativeName_PccChg, .handler = (void *)((u32)ScriptNativeFieldEffectConfigure + 1) },
    [28] = { .name = gScriptNativeName_PccSet, .handler = (void *)((u32)ScriptNativeFieldEffectStart + 1) },
    [29] = { .name = gScriptNativeName_PccGet, .handler = (void *)((u32)ScriptNativeFieldEffectCommand + 1) },
    [30] = { .name = gScriptNativeName_PccMove, .handler = (void *)((u32)ScriptNativeFieldEffectStart8 + 1) },
    [31] = { .name = gScriptNativeName_PccSync, .handler = (void *)((u32)ScriptNativeFieldEffectQuery + 1) },
    [32] = { .name = gScriptNativeName_PccStat, .handler = (void *)((u32)ScriptNativeSetActorName + 1) },
    [33] = { .name = gScriptNativeName_ExtSet, .handler = (void *)((u32)ScriptNativeEffectCStart + 1) },
    [34] = { .name = gScriptNativeName_ExtGet, .handler = (void *)((u32)ScriptNativeEffectCQuery + 1) },
    [35] = { .name = gScriptNativeName_ExtMove, .handler = (void *)((u32)ScriptNativeEffectCStartFull + 1) },
    [36] = { .name = gScriptNativeName_ExtSync, .handler = (void *)((u32)ScriptNativeEffectCCommand + 1) },
    [37] = { .name = gScriptNativeName_HitInit, .handler = (void *)((u32)ScriptNativeHitInit + 1) },
    [38] = { .name = gScriptNativeName_HitGet, .handler = (void *)((u32)ScriptNativeHitEffectQuery + 1) },
    [39] = { .name = gScriptNativeName_HitSet, .handler = (void *)((u32)ScriptNativeHitEffectStart + 1) },
    [40] = { .name = gScriptNativeName_HitFree, .handler = (void *)((u32)ScriptNativeHitFree + 1) },
    [41] = { .name = gScriptNativeName_HitHitRect, .handler = (void *)((u32)ScriptNativeHitRect + 1) },
    [42] = { .name = gScriptNativeName_BgSet, .handler = (void *)((u32)sub_08012264 + 1) },
    [43] = { .name = gScriptNativeName_FldSet, .handler = (void *)((u32)ScriptNativeFieldSet + 1) },
    [44] = { .name = gScriptNativeName_FldGet, .handler = (void *)((u32)ScriptNativeFieldGet + 1) },
    [45] = { .name = gScriptNativeName_CameraMode, .handler = (void *)((u32)ScriptNativeSetRuntimePair + 1) },
    [46] = { .name = gScriptNativeName_PmbDeckMake, .handler = (void *)((u32)sub_080129F4 + 1) },
    [47] = { .name = gScriptNativeName_PmbStatusInit, .handler = (void *)((u32)ScriptNativeCall08740 + 1) },
    [48] = { .name = gScriptNativeName_PmbSetParty, .handler = (void *)((u32)ScriptNativeConfigureResourceSlots + 1) },
    [49] = { .name = gScriptNativeName_PmdGetParty, .handler = (void *)((u32)ScriptNativeQueryResourceSlot + 1) },
    [50] = { .name = gScriptNativeName_PmbDefFashion, .handler = (void *)((u32)ScriptNativeSelectLayer + 1) },
    [51] = { .name = gScriptNativeName_PmbSetFriend, .handler = (void *)((u32)ScriptNativeSetFriendArms + 1) },
    [52] = { .name = gScriptNativeName_BtlDeckMake, .handler = (void *)((u32)ScriptNativeWriteMapValuesB + 1) },
    [53] = { .name = gScriptNativeName_BtlStatusInit, .handler = (void *)((u32)ScriptNativeCall088E0 + 1) },
    [54] = { .name = gScriptNativeName_BtlSetParty, .handler = (void *)((u32)ScriptNativeSetBattleParty + 1) },
    [55] = { .name = gScriptNativeName_BtlDeckShuffle, .handler = (void *)((u32)ScriptNativeRefreshMapValuesB + 1) },
    [56] = { .name = gScriptNativeName_BtlFldLock, .handler = (void *)((u32)ScriptNativeBattleFieldLock + 1) },
    [57] = { .name = gScriptNativeName_BtlFldUnLock, .handler = (void *)((u32)ScriptNativeBattleFieldUnlock + 1) },
    [58] = { .name = gScriptNativeName_BtlFldRect, .handler = (void *)((u32)ScriptNativeGetBattleFieldRect + 1) },
    [59] = { .name = gScriptNativeName_BtlGetCentorX, .handler = (void *)((u32)ScriptNativeMapGetPointer1COffset + 1) },
    [60] = { .name = gScriptNativeName_BtlGetCentorY, .handler = (void *)((u32)ScriptNativeMapGetPointer20Offset + 1) },
    [61] = { .name = gScriptNativeName_BtlStart, .handler = (void *)((u32)ScriptNativeCall08968 + 1) },
    [62] = { .name = gScriptNativeName_BtlWin, .handler = (void *)((u32)ScriptNativeMapResetActor + 1) },
    [63] = { .name = gScriptNativeName_BtlEnd, .handler = (void *)((u32)ScriptNativeResetFieldScene + 1) },
    [64] = { .name = gScriptNativeName_BtlFldSetInfo, .handler = (void *)((u32)ScriptNativeMapSetBoundedValue + 1) },
    [65] = { .name = gScriptNativeName_BtlEscapeFlgSet, .handler = (void *)((u32)ScriptNativeMapSetValue08 + 1) },
    [66] = { .name = gScriptNativeName_BtlSetWallView, .handler = (void *)((u32)ScriptNativeMapSetValue25 + 1) },
    [67] = { .name = gScriptNativeName_BtlLogoPut, .handler = (void *)((u32)ScriptNativeMapResetGenerator + 1) },
    [68] = { .name = gScriptNativeName_DungGenInit, .handler = (void *)((u32)ScriptNativeMapConfigure3 + 1) },
    [69] = { .name = gScriptNativeName_DungGenFree, .handler = (void *)((u32)ScriptNativeMapRefresh + 1) },
    [70] = { .name = gScriptNativeName_DungGenMake, .handler = (void *)((u32)ScriptNativeMapRandomize + 1) },
    [71] = { .name = gScriptNativeName_DungGenStart, .handler = (void *)((u32)ScriptNativeMapClearDState + 1) },
    [72] = { .name = gScriptNativeName_DungGenResize, .handler = (void *)((u32)ScriptNativeMapConfigure2 + 1) },
    [73] = { .name = gScriptNativeName_DungGenGetChrX, .handler = (void *)((u32)ScriptNativeMapQueryD60 + 1) },
    [74] = { .name = gScriptNativeName_DungGenGetChrY, .handler = (void *)((u32)ScriptNativeMapQueryD84 + 1) },
    [75] = { .name = gScriptNativeName_DungDispCoffer, .handler = (void *)((u32)ScriptNativeMapSetMode + 1) },
    [76] = { .name = gScriptNativeName_DungSetFloor, .handler = (void *)((u32)ScriptNativeMapSetSeed + 1) },
    [77] = { .name = gScriptNativeName_DungGetFloor, .handler = (void *)((u32)ScriptNativeMapGetSeed + 1) },
    [78] = { .name = gScriptNativeName_DungSetEvFlg, .handler = (void *)((u32)ScriptNativeMapCall1088 + 1) },
    [79] = { .name = gScriptNativeName_DungGetEvFlg, .handler = (void *)((u32)ScriptNativeMapQuery10A4 + 1) },
    [80] = { .name = gScriptNativeName_DungGetGenNo, .handler = (void *)((u32)ScriptNativeMapGetField618 + 1) },
    [81] = { .name = gScriptNativeName_DungSetBtlOdds, .handler = (void *)((u32)ScriptNativeMapSetField10 + 1) },
    [82] = { .name = gScriptNativeName_DungGetBtlOdds, .handler = (void *)((u32)ScriptNativeMapGetField10 + 1) },
    [83] = { .name = gScriptNativeName_DungGetBossFlg, .handler = (void *)((u32)ScriptNativeMapGetField624 + 1) },
    [84] = { .name = gScriptNativeName_DungSetBossFlg, .handler = (void *)((u32)ScriptNativeMapSetField624 + 1) },
    [85] = { .name = gScriptNativeName_DungGetBossTreasure, .handler = (void *)((u32)ScriptNativeMapGetField626 + 1) },
    [86] = { .name = gScriptNativeName_DungSetBossTreasure, .handler = (void *)((u32)ScriptNativeMapSetField626 + 1) },
    [87] = { .name = gScriptNativeName_DungGetBossBtl, .handler = (void *)((u32)ScriptNativeMapGetField628 + 1) },
    [88] = { .name = gScriptNativeName_DungSetBossBtl, .handler = (void *)((u32)ScriptNativeMapSetField628 + 1) },
    [89] = { .name = gScriptNativeName_DungSaveScrName, .handler = (void *)((u32)ScriptNativeCopyGeneratedName + 1) },
    [90] = { .name = gScriptNativeName_DungGenEvClear, .handler = (void *)((u32)ScriptNativeMapFinalize + 1) },
    [91] = { .name = gScriptNativeName_DungChangeTbl, .handler = (void *)((u32)ScriptNativeMapSelectSlot + 1) },
    [92] = { .name = gScriptNativeName_GetPlayMode, .handler = (void *)((u32)ScriptNativeGetField4258 + 1) },
    [93] = { .name = gScriptNativeName_SetPlayMode, .handler = (void *)((u32)ScriptNativeSetField4258 + 1) },
    [94] = { .name = gScriptNativeName_SetBabbo, .handler = (void *)((u32)ScriptNativeSetField12EC + 1) },
    [95] = { .name = gScriptNativeName_BgIntAttr, .handler = (void *)((u32)ScriptNativeCall067DC + 1) },
    [96] = { .name = gScriptNativeName_BgSetAttrEnable, .handler = (void *)((u32)ScriptNativeCall0680C + 1) },
    [97] = { .name = gScriptNativeName_BgGetAttrEnable, .handler = (void *)((u32)ScriptNativeCall0680C + 1) },
    [98] = { .name = gScriptNativeName_BgSetAttrEnables, .handler = (void *)((u32)ScriptNativeCall06858 + 1) },
    [99] = { .name = gScriptNativeName_GetIrqCause, .handler = (void *)((u32)ScriptNativeGetField4256 + 1) },
    [100] = { .name = gScriptNativeName_GetBtlRoomNo, .handler = (void *)((u32)ScriptNativeGetField60E + 1) },
    [101] = { .name = gScriptNativeName_DeckMake, .handler = (void *)((u32)sub_080127F8 + 1) },
    [102] = { .name = gScriptNativeName_ShuffleDeckMake, .handler = (void *)((u32)ScriptNativeWriteMapValues + 1) },
    [103] = { .name = gScriptNativeName_ShuffleDeckCopy, .handler = (void *)((u32)sub_080128C8 + 1) },
    [104] = { .name = gScriptNativeName_DeckShuffle, .handler = (void *)((u32)ScriptNativeRefreshMapValuesA + 1) },
    [105] = { .name = gScriptNativeName_BtlStatus, .handler = (void *)((u32)ScriptNativeBattleStatus + 1) },
    [106] = { .name = gScriptNativeName_PopMenu, .handler = (void *)((u32)ScriptNativeMapCoordinateCall + 1) },
    [107] = { .name = gScriptNativeName_SysMenuExec, .handler = (void *)((u32)ScriptNativeSetPendingMapValue + 1) },
    [108] = { .name = gScriptNativeName_ShopListMake, .handler = (void *)((u32)ScriptNativeCopyMapHalfwords + 1) },
    [109] = { .name = gScriptNativeName_ItemInit, .handler = (void *)((u32)ScriptNativeClearMapHalfwords + 1) },
    [110] = { .name = gScriptNativeName_ItemGet, .handler = (void *)((u32)ScriptNativeQueryResourceId + 1) },
    [111] = { .name = gScriptNativeName_GetHaveArm, .handler = (void *)((u32)ScriptNativeQueryModeResource + 1) },
    [112] = { .name = gScriptNativeName_ArmGet, .handler = (void *)((u32)ScriptNativeSetModeResource + 1) },
    [113] = { .name = gScriptNativeName_GetPewter, .handler = (void *)((u32)ScriptNativeQueryResourceState + 1) },
    [114] = { .name = gScriptNativeName_AddPewter, .handler = (void *)((u32)ScriptNativeSetResourceState + 1) },
    [115] = { .name = gScriptNativeName_AllHpRecover, .handler = (void *)((u32)ScriptNativeResetResourceState + 1) },
    [116] = { .name = gScriptNativeName_OneHpRecover, .handler = (void *)((u32)ScriptNativeUseResource + 1) },
    [117] = { .name = gScriptNativeName_GetSelectSpot, .handler = (void *)((u32)ScriptNativeGetMapStatus + 1) },
    [118] = { .name = gScriptNativeName_GetBoxInName, .handler = (void *)((u32)ScriptNativeGetResourceName + 1) },
    [119] = { .name = gScriptNativeName_GetDungData, .handler = (void *)((u32)ScriptNativeResetEncounterState + 1) },
    [120] = { .name = gScriptNativeName_BuffRand, .handler = (void *)((u32)ScriptNativeChooseRandomValue + 1) },
    [121] = { .name = gScriptNativeName_SpotPut, .handler = (void *)((u32)ScriptNativeStartMapCoordinateEvent + 1) },
    [122] = { .name = gScriptNativeName_SioMapNoGet, .handler = (void *)((u32)ScriptNativeGetField42BA + 1) },
    [123] = { .name = gScriptNativeName_StaffRoll, .handler = (void *)((u32)ScriptNativeStartEncounter + 1) },
    [124] = { .name = gScriptNativeName_SetDungCnt, .handler = (void *)((u32)ScriptNativeSetEncounterValue + 1) },
    [125] = { .name = gScriptNativeName_GetDungCnt, .handler = (void *)((u32)ScriptNativeGetEncounterValue + 1) },
    [126] = { .name = gScriptNativeName_SetFhantom, .handler = (void *)((u32)ScriptNativeSetEncounterMode + 1) },
    [127] = { .name = gScriptNativeName_GetFhantom, .handler = (void *)((u32)ScriptNativeGetEncounterMode + 1) },
};


