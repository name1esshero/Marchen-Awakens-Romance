/* Map/hit native-call adapters, verified against the native registry.
 * The engine supplies valid argument arrays; these original handlers do not
 * validate count. They never write the result slot. Return values below are
 * retained verbatim, without assuming 0x7FFF means an ordinary success code.
 */
#include "kmp.h"
#include "hit_region.h"
#define AT(x) __attribute__((section(".rom." x)))
/* Native arguments occupy four bytes on GBA. String and integer arguments
 * share the same VM slots; only FldSet reads a string here. */
union MapArgument {s32 integer; const char *string;};
AT("000122F8") s32 ScriptNativeFieldSet(u32 count,const union MapArgument *args,s32 *result)
{
    KmpLoadField(args[0].string, (s16)args[1].integer, (s16)args[2].integer);
    return 0x7FFF;
}
AT("000121C4") s32 ScriptNativeHitInit(u32 count,const s32 *args,s32 *result)
{
    HitRegionInit(args[0], args[1], args[2], args[3], args[4]);
    return 1;
}
AT("00012244") s32 ScriptNativeHitRect(u32 count,const s32 *args,s32 *result)
{
    HitRegionSetRect(args[0], args[1], args[2], args[3], args[4]);
    return 1;
}
AT("000121E4") s32 ScriptNativeHitFree(u32 count,const s32 *args,s32 *result)
{
    if (args[0] != -1)
        HitRegionDisable(args[0]);
    else
        HitRegionDisableAll();
    return 0x7FFF;
}
