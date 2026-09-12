/* Native VM scalar/string helpers. They return 1 on success and write to the
 * result slot. Allocation failure returns -1 without changing that slot.
 * Character conversion follows the engine's private double-byte encoding,
 * including F0xx codes; it is not Unicode or general-purpose Shift-JIS.
 * The empty fallback at 081AC6A0 and "%d" at 081AC6A4 are verified ROM data.
 * Original behavior is retained: min/max read args[0] even for count zero,
 * abs(INT_MIN) keeps 0x80000000, and character strings truncate to two bytes.
 */
#include "script_vm.h"
#define AT(x) __attribute__((section(".rom." x)))
#define VM (*(struct ScriptContext **)0x0300611C)
extern void *HeapAlloc(void *,u32);
AT("000800FC") s32 ScriptNativeAbs(u32 count,const s32 *args,s32 *result)
{
 s32 value=args[0];
 if(value<0)value=(s32)(0u-(u32)value);
 *result=value;
 return 1;
}
AT("000800FC") const u8 ScriptNativeAbsTail[2]={0,0};
AT("00080110") s32 ScriptNativeCharacterCode(u32 count,const u8 **args,u32 *result)
{
 const u8 *text=args[0];
 u32 first;
 if(!text)text=(const u8 *)0x081AC6A0;
 first=text[0];
 if(first-128<=31 || first>223) *result=text[1]+(first<<8);
 else *result=first;
 return 1;
}
AT("00080140") s32 ScriptNativeCharacterString(u32 count,const u32 *args,u8 **result)
{
 u8 *text=HeapAlloc(VM->state->heap,4);
 u32 value;
 if(!text)return -1;
 value=args[0];
 if(value>255){text[0]=value>>8;text[1]=value;text[2]=0;}
 else {text[0]=value;text[1]=0;}
 *result=text;
 return 1;
}
AT("00080140") const u8 ScriptNativeCharacterStringTail[2]={0,0};
AT("00080188") s32 ScriptNativeMax(u32 count,const s32 *args,s32 *result)
{
 u32 i; s32 value=args[0];
 for(i=1;i<count;i++) if(args[i]>value)value=args[i];
 *result=value;return 1;
}
AT("00080188") const u8 ScriptNativeMaxTail[2]={0,0};
AT("000801B4") s32 ScriptNativeMin(u32 count,const s32 *args,s32 *result)
{
 u32 i; s32 value=args[0];
 for(i=1;i<count;i++) if(args[i]<value)value=args[i];
 *result=value;return 1;
}
AT("000801B4") const u8 ScriptNativeMinTail[2]={0,0};
extern s32 siprintf(char *,const char *,...);
AT("00080214") s32 ScriptNativeIntegerString(u32 count,const s32 *args,char **result)
{
 char *text=HeapAlloc(VM->state->heap,16);
 s32 status;
 if(text){s32 value=args[0];siprintf(text,(const char *)0x081AC6A4,value);*result=text;status=1;}
 else status=-1;
 return status;
}
AT("00080214") const u8 ScriptNativeIntegerStringTail[2]={0,0};
