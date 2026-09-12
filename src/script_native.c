/* Native VM scalar/string helpers. They return 1 on success and write to the
 * result slot. Allocation failure returns -1 without changing that slot.
 * Character conversion follows the engine's private double-byte encoding,
 * including F0xx codes; it is not Unicode or general-purpose Shift-JIS.
 * The empty fallback at 081AC6A0 and "%d" at 081AC6A4 are verified ROM data.
 * Original behavior is retained: min/max read args[0] even for count zero,
 * abs(INT_MIN) keeps 0x80000000, and character strings truncate to two bytes.
 */
#include "script_vm.h"
#include "random.h"
#define AT(x) __attribute__((section(".rom." x)))
#define VM (*(struct ScriptContext **)0x0300611C)
extern void *HeapAlloc(void *,u32);
extern u32 sub_08080E4C(u32 dividend,u32 divisor);
extern s32 sub_08082640(const char *text);
extern s32 strcmp(const char *left,const char *right);
extern u32 strlen(const char *text);
extern char *strcpy(char *destination,const char *source);
extern char *strncpy(char *destination,const char *source,u32 length);
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

/* Native calls receive their arguments as an array in r1 and write their
 * scalar result through r2. The first ABI argument is the argument count. */
AT("000801E0") s32 ScriptNativeRandomRange(u32 count,const u32 *args,u32 *result)
{
 u32 low=Random();
 u32 high=Random();
 *result=sub_08080E4C(low | (high << 15),args[0]);
 return 1;
}

AT("00080204") s32 ScriptNativeSeedRandom(u32 count,const u32 *args,u32 *result)
{
 RandomSeed(args[0]);
 return 1;
}
AT("00080204") const u8 ScriptNativeSeedRandomTail[2]={0,0};
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

AT("00080250") s32 ScriptNativeParseInteger(u32 count,const char **args,s32 *result)
{
 const char *text=args[0];
 if(!text)text=(const char *)0x081AC6A0;
 *result=sub_08082640(text);
 return 1;
}
AT("00080270") s32 ScriptNativeCompareStrings(u32 count,const char **args,s32 *result)
{
 const char *left=args[0];
 const char *right;
 s32 comparison;
 if(!left)left=(const char *)0x081AC6A0;
 right=args[1];
 if(!right)right=(const char *)0x081AC6A0;
 comparison=strcmp(left,right);
 if(comparison<0)comparison=-1;
 else if(comparison>0)comparison=1;
 *result=comparison;
 return 1;
}

AT("000802A8") s32 ScriptNativeStringLength(u32 count,const char **args,u32 *result)
{
 const char *text=args[0];
 if(!text)text=(const char *)0x081AC6A0;
 *result=strlen(text);
 return 1;
}

struct ScriptSubstringArgs { const char *text; u32 start; u32 length; };

AT("00080320") s32 ScriptNativeRight(u32 count,const struct ScriptSubstringArgs *args,char **result)
{
 const char *text=args->text;
 u32 textLength,length,offset;
 char *copy;
 if(!text)text=(const char *)0x081AC6A0;
 textLength=strlen(text);
 length=args->start;
 if(length>textLength)length=textLength;
 copy=HeapAlloc(VM->state->heap,length+1);
 if(!copy)return -1;
 offset=textLength-length;
 strcpy(copy,text+offset);
 *result=copy;
 return 1;
}
AT("00080320") const u8 ScriptNativeRightTail[2]={0,0};

AT("00080380") s32 ScriptNativeSubstring(u32 count,const struct ScriptSubstringArgs *args,char **result)
{
 const char *text=args->text;
 u32 textLength,start,length;
 char *copy;
 if(!text)text=(const char *)0x081AC6A0;
 textLength=strlen(text);
 start=args->start;
 if(start>textLength)start=textLength;
 text+=start;
 textLength-=start;
 length=args->length;
 if(length>textLength)length=textLength;
 copy=HeapAlloc(VM->state->heap,length+1);
 if(!copy)return -1;
 strncpy(copy,text,length);
 copy[length]=0;
 *result=copy;
 return 1;
}
AT("00080380") const u8 ScriptNativeSubstringTail[2]={0,0};
