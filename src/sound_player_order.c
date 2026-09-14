#include "sound.h"

#include "rom_section.h"

/** Dynamic effects can borrow these six M4A players in priority order. */
AT("001ACC44") const s16 gDynamicSoundPlayerOrder[6] = {1, 2, 3, 6, 7, 8};
