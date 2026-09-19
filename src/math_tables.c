/* Fixed-point trigonometry and OAM geometry tables used by the renderers.
 * Editable numeric values live in data/math_tables.json; the generated
 * initializers are disposable build products. */
#include "math_tables.h"

#include "rom_section.h"

/* Masks retaining zero through eight packed 4bpp pixels in a 32-bit row. */
AT("001ACB58") const u32 gTileRemainderMasks[9] = {
    0x00000000,
    0x0000000F,
    0x000000FF,
    0x00000FFF,
    0x0000FFFF,
    0x000FFFFF,
    0x00FFFFFF,
    0x0FFFFFFF,
    0xFFFFFFFF
};

AT("00F28410") const u8 gOamAttributeMasks[16] = {
#include "../build/generated/oam_attribute_masks.inc"
};

AT("00F28420") const u8 gOamHalfDimensions[32] = {
#include "../build/generated/oam_half_dimensions.inc"
};

AT("00F28440") const u8 gOamDimensions[32] = {
#include "../build/generated/oam_dimensions.inc"
};

/** The original table contains two identical 256-entry periods. */
AT("00F28460") const s16 gSineTable8_8[512] = {
#include "../build/generated/sine_8_8.inc"
};

AT("00F28860") const s16 gSineTable14[4096] = {
#include "../build/generated/sine_2_14.inc"
};
