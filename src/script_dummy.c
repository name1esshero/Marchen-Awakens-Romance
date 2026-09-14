#include "gba/types.h"

#include "rom_section.h"

/** Native-command-table placeholder selected by the script name "dummy". */
AT("00005B2C") s32 ScriptNativeDummy(void)
{
    return 0x7FFF;
}
