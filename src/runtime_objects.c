/* Views of the secondary runtime's actor records and its 4-by-4 object table. */
#include "runtime_objects.h"
#include "runtime_state.h"
#include "rom_section.h"
extern u8 *gRuntimeObjectTable[];
#define OBJECT_TABLE gRuntimeObjectTable
#define RUNTIME_OBJECT(group,slot) OBJECT_TABLE[(group)*4+(slot)]

AT("0000A0E8") void *RuntimeGetPointerEA0(void) { return *(void **)(gSecondaryRuntime+0xEA0); }
AT("0000A0FC") void RuntimeSetPointerEA0(void *v) { *(void **)(gSecondaryRuntime+0xEA0)=v; }
AT("0000A110") void RuntimeSetFieldsE48ToE4A(s32 a,s32 b,s32 c)
{
 gSecondaryRuntime[0xE48]=a;
 gSecondaryRuntime[0xE49]=b;
 gSecondaryRuntime[0xE4A]=c;
}
AT("0000A140") void RuntimeSetFieldE48(s32 v) { gSecondaryRuntime[0xE48]=v; }
AT("0000A154") s32 RuntimeGetFieldE48(void) { return *(s8 *)(gSecondaryRuntime+0xE48); }
AT("0000A16C") void RuntimeSetFieldE49(s32 v) { gSecondaryRuntime[0xE49]=v; }
AT("0000A180") s32 RuntimeGetFieldE49(void) { return *(s8 *)(gSecondaryRuntime+0xE49); }
AT("0000A198") void RuntimeSetFieldE4A(s32 v) { gSecondaryRuntime[0xE4A]=v; }
AT("0000A1AC") s32 RuntimeGetFieldE4A(void) { return *(s8 *)(gSecondaryRuntime+0xE4A); }
AT("0000A1C4") void *RuntimeGetActorRecord(u32 actor,u32 part)
{
 u8 **root=&gSecondaryRuntime;
 u32 actorOffset=actor*1672;
 u8 *base;
 u32 partOffset;
 actorOffset+=0x120;
 base=*root+actorOffset;
 partOffset=part*168;
 partOffset+=0x23C;
 return base+partOffset;
}
/* The same actor record's other per-part table: stride 104 rather than 168,
 * based at +0x4DC. Callers treat each entry as a party slot. */
AT("000083B8") void *RuntimeGetActorPartRecord(u32 actor,u32 part)
{
 u8 **root=&gSecondaryRuntime;
 u32 actorOffset=actor*1672;
 u8 *base;
 u32 partOffset;
 actorOffset+=0x120;
 base=*root+actorOffset;
 partOffset=part*104;
 partOffset+=0x4DC;
 return base+partOffset;
}

#define GET_S8(address,name,field) \
 AT(address) s32 name(u32 group,u32 slot) { u8 **table=OBJECT_TABLE; return *(s8 *)(table[group*4+slot]+(field)); }
#define SET_FLAG(address,name,field) \
 AT(address) void name(u32 group,u32 slot,s32 enabled) { u8 **table=OBJECT_TABLE; u8 *object=table[group*4+slot]; object[field]=!!enabled; }

GET_S8("0000A1E8",RuntimeObjectGetField00,0x00)
GET_S8("0000A200",RuntimeObjectGetField01,0x01)
GET_S8("0000A218",RuntimeObjectGetField02,0x02)
GET_S8("0000A230",RuntimeObjectGetField03,0x03)
GET_S8("0000A248",RuntimeObjectGetField04,0x04)
GET_S8("0000A260",RuntimeObjectGetField05,0x05)
GET_S8("0000A278",RuntimeObjectGetField0A,0x0A)
GET_S8("0000A290",RuntimeObjectGetField09,0x09)
SET_FLAG("0000A2A8",RuntimeObjectSetFlag00,0x00)
SET_FLAG("0000A2C4",RuntimeObjectSetFlag01,0x01)
SET_FLAG("0000A2E0",RuntimeObjectSetFlag02,0x02)
SET_FLAG("0000A2FC",RuntimeObjectSetFlag03,0x03)
SET_FLAG("0000A318",RuntimeObjectSetFlag04,0x04)
SET_FLAG("0000A334",RuntimeObjectSetFlag05,0x05)
SET_FLAG("0000A350",RuntimeObjectSetFlag0A,0x0A)
SET_FLAG("0000A36C",RuntimeObjectSetFlag09,0x09)
AT("0000A388") s32 RuntimeObjectGetField48(u32 group,u32 slot)
{
 u8 **table=OBJECT_TABLE;
 return *(s16 *)(table[group*4+slot]+0x48);
}

#define OBJECT_PTR() u8 **table=OBJECT_TABLE; u8 *object=table[group*4+slot]
#define SET_S16(address,name,field) AT(address) void name(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(s16 *)(object+(field))=value; }
#define GET_S16(address,name,field) AT(address) s32 name(u32 group,u32 slot) { u8 **table=OBJECT_TABLE; return *(s16 *)(table[group*4+slot]+(field)); }
#define SET_U32(address,name,field) AT(address) void name(u32 group,u32 slot,u32 value) { OBJECT_PTR(); *(u32 *)(object+(field))=value; }
#define GET_U32(address,name,field) AT(address) u32 name(u32 group,u32 slot) { u8 **table=OBJECT_TABLE; return *(u32 *)(table[group*4+slot]+(field)); }

SET_S16("0000A3A0",RuntimeObjectSetField48,0x48)
AT("0000A424") void RuntimeObjectSetFixed18(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x18)=value<<16; }
GET_S16("0000A43C",RuntimeObjectGetField1A,0x1A)
AT("0000A454") void RuntimeObjectSetFixed1C(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x1C)=value<<16; }
GET_S16("0000A46C",RuntimeObjectGetField1E,0x1E)
AT("0000A4F0") void RuntimeObjectSetFixed20(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x20)=value<<16; }
GET_S16("0000A508",RuntimeObjectGetField22,0x22)
AT("0000A520") void RuntimeObjectSetFixed24(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x24)=value<<16; }
GET_S16("0000A538",RuntimeObjectGetField26,0x26)
SET_U32("0000A5B4",RuntimeObjectSetWord18,0x18)
GET_U32("0000A5C8",RuntimeObjectGetWord18,0x18)
SET_U32("0000A5DC",RuntimeObjectSetWord1C,0x1C)
GET_U32("0000A5F0",RuntimeObjectGetWord1C,0x1C)
SET_U32("0000A668",RuntimeObjectSetWord20,0x20)
GET_U32("0000A67C",RuntimeObjectGetWord20,0x20)
SET_U32("0000A690",RuntimeObjectSetWord24,0x24)
GET_U32("0000A6A4",RuntimeObjectGetWord24,0x24)
AT("0000A6B8") void RuntimeObjectSetField36(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(s8 *)(object+0x36)=value; }
GET_S8("0000A6D0",RuntimeObjectGetField36,0x36)
GET_S16("0000A75C",RuntimeObjectGetField3A,0x3A)
SET_S16("0000A774",RuntimeObjectSetField3A,0x3A)
GET_S16("0000A788",RuntimeObjectGetField3C,0x3C)
SET_S16("0000A7A0",RuntimeObjectSetField3C,0x3C)
GET_S16("0000A7B4",RuntimeObjectGetField3E,0x3E)
SET_S16("0000A7CC",RuntimeObjectSetField3E,0x3E)
GET_S16("0000A7E0",RuntimeObjectGetField40,0x40)
SET_S16("0000A7F8",RuntimeObjectSetField40,0x40)
GET_S8("0000A810",RuntimeObjectGetField39,0x39)
AT("0000A82C") void RuntimeObjectSetField39(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(s8 *)(object+0x39)=value; }
SET_S16("000097D0",RuntimeObjectSetField4A,0x4A)
GET_S16("000097E8",RuntimeObjectGetField4A,0x4A)
SET_S16("00009800",RuntimeObjectSetField4C,0x4C)
GET_S16("00009818",RuntimeObjectGetField4C,0x4C)
