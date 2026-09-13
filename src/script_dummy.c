#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))

/* Native-command-table placeholder selected by the script name "dummy". */
AT("00005B2C") s32 ScriptNativeDummy(void)
{
    return 0x7FFF;
}
