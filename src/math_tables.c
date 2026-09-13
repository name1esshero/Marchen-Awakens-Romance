/* Fixed-point trigonometry and OAM geometry tables used by the renderers.
 * Editable numeric values live in data/math_tables.json; the generated
 * initializers are disposable build products. */
#include "math_tables.h"

#define AT(x) __attribute__((section(".rom." x)))

AT("00F28410") const u8 gOamAttributeMasks[16] = {
#include "../build/generated/oam_attribute_masks.inc"
};

AT("00F28420") const u8 gOamHalfDimensions[32] = {
#include "../build/generated/oam_half_dimensions.inc"
};

AT("00F28440") const u8 gOamDimensions[32] = {
#include "../build/generated/oam_dimensions.inc"
};

/* The original table contains two identical 256-entry periods. */
AT("00F28460") const s16 gSineTable8_8[512] = {
#include "../build/generated/sine_8_8.inc"
};

AT("00F28860") const s16 gSineTable14[4096] = {
#include "../build/generated/sine_2_14.inc"
};
