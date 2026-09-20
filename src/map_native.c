/* Map/hit native-call adapters, verified against the native registry.
 * The engine supplies valid argument arrays; these original handlers do not
 * validate count. They never write the result slot. Return values below are
 * retained verbatim, without assuming 0x7FFF means an ordinary success code.
 */
#include "kmp.h"
#include "gba/io_reg.h"
#include "hit_region.h"
#include "runtime_misc.h"
#include "rom_section.h"
/* Native arguments occupy four bytes on GBA. String and integer arguments
 * share the same VM slots; only FldSet reads a string here. */
union MapArgument {s32 integer; const char *string;};
extern const char gScriptKmpExtension[];
extern char *strcpy(char *,const char *);
extern char *strcat(char *,const char *);
extern char *strupr(char *);
#define sText_KmpExtension gScriptKmpExtension

/** BgSet loads one KMP plane into the BG character block selected by the
 * viewport index, then renders that viewport at a 16.16 pixel position. */
AT("00012264") s32 ScriptNativeBackgroundSet(u32 count,const union MapArgument *args,s32 *result)
{
    char resource[16];
    void *tileDestination;
    struct KmpViewport *view;
    strcpy(resource,args[3].string);
    strupr(resource);
    if(args[0].integer==1)goto defaultVram;
    if(args[0].integer<=1)goto defaultVram;
    if(args[0].integer==2)goto case2Vram;
    if(args[0].integer==3)goto case3Vram;
    goto defaultVram;
case2Vram:
    tileDestination=BG_CHAR_ADDR(2);
    goto selectedVram;
case3Vram:
    tileDestination=BG_CHAR_ADDR(3);
    goto selectedVram;
defaultVram:
    tileDestination=BG_CHAR_ADDR(0);
selectedVram:
    strcpy(resource,args[3].string);
    strcat(resource,sText_KmpExtension);
    KmpLoadResource(resource,tileDestination,args[0].integer,0,
                 args[1].integer,args[2].integer,3);
    view=&gKmpViewports[args[0].integer];
    KmpRenderViewport(view,args[4].integer<<KMP_FIXED_SHIFT,
                      args[5].integer<<KMP_FIXED_SHIFT);
    return 0x7FFF;
}

/** Execute the field set native script command. */
AT("000122F8") s32 ScriptNativeFieldSet(u32 count,const union MapArgument *args,s32 *result)
{
    KmpLoadField(args[0].string, (s16)args[1].integer, (s16)args[2].integer);
    return 0x7FFF;
}

extern void *HeapAlloc(void *,u32);
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

/** Return a heap-owned copy of the current field basename to the script VM.
 * The allocation comes from the game state's default heap; the caller owns
 * the returned 18-byte buffer. */
AT("00012318") s32 ScriptNativeFieldGet(u32 count,const s32 *args,s32 *result)
{
    u8 *base;
    u32 offset;
    u8 *root;
    void *heap;
    char *name;

    base = gIwramBase;
    offset = (u32)gMapGenerationRootOffset;
    root = *(u8 **)(base + offset);
    offset -= 0xAC;
    heap = **(void ***)(root + offset);
    name = HeapAlloc(heap,18);
    GameStateCopyString12F4(name);
    *result=(s32)name;
    return 1;
}
/** Execute the hit init native script command. */
AT("000121C4") s32 ScriptNativeHitInit(u32 count,const s32 *args,s32 *result)
{
    HitRegionInit(args[0], args[1], args[2], args[3], args[4]);
    return 1;
}
/** Execute the hit rect native script command. */
AT("00012244") s32 ScriptNativeHitRect(u32 count,const s32 *args,s32 *result)
{
    HitRegionSetRect(args[0], args[1], args[2], args[3], args[4]);
    return 1;
}
/** Execute the hit free native script command. */
AT("000121E4") s32 ScriptNativeHitFree(u32 count,const s32 *args,s32 *result)
{
    if (args[0] != -1)
        HitRegionDisable(args[0]);
    else
        HitRegionDisableAll();
    return 0x7FFF;
}
