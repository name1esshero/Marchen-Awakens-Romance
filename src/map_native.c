/* Map/hit native-call adapters, verified against the native registry.
 * The engine supplies valid argument arrays; these original handlers do not
 * validate count. They never write the result slot. Return values below are
 * retained verbatim, without assuming 0x7FFF means an ordinary success code.
 */
#include "kmp.h"
#include "hit_region.h"
#include "runtime_misc.h"
#define AT(x) __attribute__((section(".rom." x)))
/* Native arguments occupy four bytes on GBA. String and integer arguments
 * share the same VM slots; only FldSet reads a string here. */
union MapArgument {s32 integer; const char *string;};
extern char *strcpy(char *,const char *);
extern char *strcat(char *,const char *);
extern char *strupr(char *);
extern void sub_08003178(const char *,void *,s32,s32,s32,s32,s32);

/* BgSet loads one KMP plane into the BG character block selected by the
 * viewport index, then renders that viewport at a 16.16 pixel position. */
#ifdef NONMATCHING
AT("00012264") s32 ScriptNativeBackgroundSet(u32 count,const union MapArgument *args,s32 *result)
{
    char resource[16];
    void *tileDestination;
    struct KmpViewport *view;
    strcpy(resource,args[3].string);
    strupr(resource);
    switch (args[0].integer) {
    case 1:goto defaultVram;
    case 2:tileDestination=(void *)0x06008000;break;
    case 3:tileDestination=(void *)0x0600C000;break;
    default:goto defaultVram;
    }
    goto selectedVram;
defaultVram:
    tileDestination=(void *)0x06000000;
selectedVram:
    strcpy(resource,args[3].string);
    strcat(resource,(const char *)0x08086D88);
    sub_08003178(resource,tileDestination,args[0].integer,0,
                 args[1].integer,args[2].integer,3);
    view=(struct KmpViewport *)((u8 *)0x03003BC4+args[0].integer*0xFC);
    KmpRenderViewport(view,args[4].integer<<16,args[5].integer<<16);
    return 0x7FFF;
}
#endif

AT("000122F8") s32 ScriptNativeFieldSet(u32 count,const union MapArgument *args,s32 *result)
{
    KmpLoadField(args[0].string, (s16)args[1].integer, (s16)args[2].integer);
    return 0x7FFF;
}

extern void *HeapAlloc(void *,u32);
#ifdef NONMATCHING
AT("00012318") s32 ScriptNativeFieldGet(u32 count,const s32 *args,s32 *result)
{
    u32 offset=0x3FDC;
    u8 *base=(u8 *)0x03000000;
    u8 *root=*(u8 **)(base+offset);
    void *heap=**(void ***)(root+offset-0xAC);
    char *name=HeapAlloc(heap,18);
    GameStateCopyString12F4(name);
    *result=(s32)name;
    return 1;
}
#endif
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
