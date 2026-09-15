/* Small views into the secondary runtime allocation at IWRAM 03004020.
 *
 * Most of this file is one-line accessors named for the byte offset they read,
 * because that offset is all the ROM tells us; a name like RuntimeGetFieldEE8
 * means "the field at +0xEE8" and nothing more until a call site explains it.
 * Rename them as their meaning is recovered rather than inventing one here.
 *
 * Three strides run through the whole file and are worth knowing before
 * reading any of it:
 *
 *   +0x0000..        singleton fields and buffers hanging off the allocation
 *                    base, addressed by a bare constant offset.
 *   index * 1672     one per-actor record. The actor index scales by the
 *                    record size, then the field offset is added.
 *   part  * 104      one per-part sub-record inside an actor record, so a
 *                    part accessor is actor*1672 + part*104 + field.
 *
 * The ACTOR_*, PART_* and ACTOR_LATE_* macro families below exist so those
 * three arithmetic shapes are written once each. A macro's expansion is also
 * load-bearing for matching: the statement order in the body is what makes
 * agbcc emit the ROM's instruction order, so do not "simplify" one into a
 * single expression without re-checking `make compare`. */
#include "runtime_buffers.h"
#include "runtime_state.h"
#include "rom_section.h"

/** @return The secondary runtime's +0xEB8 buffer. */
AT("000075CC") void *RuntimeGetBufferEB8(void) { return gSecondaryRuntime+0xEB8; }
/** @return The secondary runtime's +0xED0 buffer. */
AT("000075E0") void *RuntimeGetBufferED0(void) { return gSecondaryRuntime+0xED0; }
/** @return The secondary runtime's +0xEEA buffer. */
AT("000075F0") void *RuntimeGetBufferEEA(void) { return gSecondaryRuntime+0xEEA; }
/** @return The secondary runtime's +0xEE8 byte field, sign-extended. */
AT("00007620") s32 RuntimeGetFieldEE8(void) { return *(s8 *)(gSecondaryRuntime+0xEE8); }
/** Set the secondary runtime's +0xEE8 byte field. */
AT("00007638") void RuntimeSetFieldEE8(s32 v) { *(s8 *)(gSecondaryRuntime+0xEE8)=v; }
/** @return The secondary runtime's +0xEE9 byte field, sign-extended. */
AT("0000764C") s32 RuntimeGetFieldEE9(void) { return *(s8 *)(gSecondaryRuntime+0xEE9); }
/** Set the secondary runtime's +0xEE9 byte field. */
AT("00007664") void RuntimeSetFieldEE9(s32 v) { *(s8 *)(gSecondaryRuntime+0xEE9)=v; }

/** @return The secondary runtime's +0xE9C u16 field. */
AT("00009048") u32 RuntimeGetFieldE9C(void) { return *(u16 *)(gSecondaryRuntime+0xE9C); }
/** Set the secondary runtime's +0xEB2 byte field. */
AT("0000905C") void RuntimeSetFieldEB2(u32 v) { *(u8 *)(gSecondaryRuntime+0xEB2)=v; }
/** @return The secondary runtime's +0xEB2 byte field. */
AT("00009070") u32 RuntimeGetFieldEB2(void) { return *(u8 *)(gSecondaryRuntime+0xEB2); }
/** Set the secondary runtime's +0xEB3 byte field. */
AT("00009084") void RuntimeSetFieldEB3(u32 v) { *(u8 *)(gSecondaryRuntime+0xEB3)=v; }
/** @return The secondary runtime's +0xEB3 byte field. */
AT("00009098") u32 RuntimeGetFieldEB3(void) { return *(u8 *)(gSecondaryRuntime+0xEB3); }

/** Set the secondary runtime's +0xEAE byte field. */
AT("000098F8") void RuntimeSetFieldEAE(s32 v) { *(s8 *)(gSecondaryRuntime+0xEAE)=v; }
/** @return The secondary runtime's +0xEAE byte field, sign-extended. */
AT("0000990C") s32 RuntimeGetFieldEAE(void) { return *(s8 *)(gSecondaryRuntime+0xEAE); }
/** Set the secondary runtime's +0xEAF byte field. */
AT("00009924") void RuntimeSetFieldEAF(s32 v) { *(s8 *)(gSecondaryRuntime+0xEAF)=v; }
/** @return The secondary runtime's +0xEAF byte field, sign-extended. */
AT("00009938") s32 RuntimeGetFieldEAF(void) { return *(s8 *)(gSecondaryRuntime+0xEAF); }
/** Set the secondary runtime's +0xEB0 byte field. */
AT("00009950") void RuntimeSetFieldEB0(s32 v) { *(s8 *)(gSecondaryRuntime+0xEB0)=v; }
/** @return The secondary runtime's +0xEB0 byte field, sign-extended. */
AT("00009964") s32 RuntimeGetFieldEB0(void) { return *(s8 *)(gSecondaryRuntime+0xEB0); }

/** Set the secondary runtime's +0xEB1 byte field. */
AT("00009CC4") void RuntimeSetFieldEB1(s32 v) { *(s8 *)(gSecondaryRuntime+0xEB1)=v; }
/** @return The secondary runtime's +0xEB1 byte field, sign-extended. */
AT("00009CD8") s32 RuntimeGetFieldEB1(void) { return *(s8 *)(gSecondaryRuntime+0xEB1); }
/** Set the secondary runtime's +0xEB4 u16 field. */
AT("00009CF0") void RuntimeSetFieldEB4(s32 v) { *(u16 *)(gSecondaryRuntime+0xEB4)=v; }
/** @return The secondary runtime's +0xEB4 field, read as a signed
 * halfword. */
AT("00009D04") s32 RuntimeGetFieldEB4(void) { return *(s16 *)(gSecondaryRuntime+0xEB4); }
/** Set the secondary runtime's +0xEB6 byte field. */
AT("00009D1C") void RuntimeSetFieldEB6(s32 v) { *(s8 *)(gSecondaryRuntime+0xEB6)=v; }
/** @return The secondary runtime's +0xEB6 byte field, sign-extended. */
AT("00009D30") s32 RuntimeGetFieldEB6(void) { return *(s8 *)(gSecondaryRuntime+0xEB6); }
/** Set the secondary runtime's +0xEB7 byte field. */
AT("00009D48") void RuntimeSetFieldEB7(s32 v) { *(s8 *)(gSecondaryRuntime+0xEB7)=v; }
/** @return The secondary runtime's +0xEB7 byte field, sign-extended. */
AT("00009D5C") s32 RuntimeGetFieldEB7(void) { return *(s8 *)(gSecondaryRuntime+0xEB7); }

/** Set an actor record's +0x7A0 byte field. */
AT("00009D9C") void RuntimeRecordSetByte7A0(u32 index,s32 value)
{
 u8 *record=gSecondaryRuntime;
 record += index*1672;
 record += 0x7A0;
 *record=value;
}

/* Each actor owns a 104-byte array of component state.  These byte fields
 * control component visibility/state and are intentionally signed. */
#define PART_SET_S8(address,name,field) AT(address) void name(u32 actor,u32 part,s32 value) { u8 *base=gSecondaryRuntime; part*=104; actor*=1672; part+=actor; base+=part; base+=(field); *(s8 *)base=value; }
#define PART_GET_S8(address,name,field) AT(address) s32 name(u32 actor,u32 part) { u8 *base=gSecondaryRuntime; part*=104; actor*=1672; part+=actor; base+=part; base+=(field); return *(s8 *)base; }

PART_SET_S8("00009B24",RuntimePartSetField656,0x656)
PART_GET_S8("00009B4C",RuntimePartGetField656,0x656)
PART_SET_S8("00009B70",RuntimePartSetField652,0x652)
PART_GET_S8("00009B98",RuntimePartGetField652,0x652)
PART_SET_S8("00009BBC",RuntimePartSetField651,0x651)
PART_GET_S8("00009BE4",RuntimePartGetField651,0x651)

/** Set a part record's +0xA4 byte field. See RuntimeActorGetByteA4(). */
AT("00019C50") void RuntimeActorSetByteA4(u32 actor,u32 part,u32 value)
{
 u8 *record;
 extern u8 *RuntimeGetActorRecord(u32 actor,u32 part);
 record=RuntimeGetActorRecord(actor,part);
 record[0xA4]=value;
}
AT("00019C50") const u8 RuntimeActorSetByteA4Tail[2]={0};

/** Add two values into the secondary runtime's +0xE50/+0xE54 u32 fields
 * (RuntimeGetBufferE50() and its unnamed neighbor). */
AT("00008684") void RuntimeAddOffsets(u32 first,u32 second)
{
 u8 *base=gSecondaryRuntime;
 *(u32 *)(base+0xE50)+=first;
 *(u32 *)(base+0xE54)+=second;
}

extern u32 SpriteResourceFindGroup(u32 type,const void *name);
/** Initialize a part record's header: set two enable flags, resolve its
 * sprite resource group by name, and store an initial value. */
AT("00019C34") void RuntimePartInitHeader(u8 *record,const void *resourceName,u32 value)
{
 u32 one=1;
 record[18]=one;
 record[19]=one;
 *(u16 *)(record+26)=one;
 *(u16 *)(record+28)=SpriteResourceFindGroup(1,resourceName);
 *(u16 *)(record+16)=value;
}

/** Per-actor animation and position state.  The explicit arithmetic order is
 * retained because it reproduces the original agbcc instruction schedule. */
AT("00009DB8") s32 RuntimeActorGetField7A0(u32 actor) { u8 *base=gSecondaryRuntime; actor*=1672; base+=actor; base+=0x7A0; return *(s8 *)base; }
AT("00009DD4") void RuntimeActorSetField7A2(u32 actor,s32 value) { u8 *base=gSecondaryRuntime; actor*=1672; base+=actor; base+=0x7A2; *(s16 *)base=value; }
AT("00009E5C") void RuntimeActorSetField352(u32 actor,s32 value) { u8 *base=gSecondaryRuntime; actor*=1672; base+=actor; base+=0x352; *(s16 *)base=value; }
AT("00009E78") s32 RuntimeActorGetField352(u32 actor) { u8 *base=gSecondaryRuntime; actor*=1672; base+=actor; base+=0x352; return *(s16 *)base; }
AT("00009E98") s32 RuntimeActorGetField350(u32 actor) { u8 *base=gSecondaryRuntime; actor*=1672; base+=actor; base+=0x350; return *(s8 *)base; }

AT("00009D74") void RuntimeActorSetActiveVector(u32 actor,s32 first,s32 second,s32 third)
{
 u8 **root=&gSecondaryRuntime;
 u32 stride=1672;
 actor*=stride;
 actor+=(u32)*root;
 actor+=0x7A0;
 *(u8 *)actor=1;
 *(s16 *)(actor+2)=first;
 *(s16 *)(actor+4)=second;
 *(s16 *)(actor+6)=third;
}
/** @return An actor record's +0x7A2 field, read as a signed halfword. See
 * RuntimeActorSetField7A2(). */
AT("00009DF0") s32 RuntimeActorGetField7A2(u32 actor)
{
 u8 *base=gSecondaryRuntime;
 u32 stride=1672;
 actor*=stride;
 base+=actor;
 base+=0x7A2;
 return *(s16 *)base;
}
/** Set an actor record's +0x7A4 and +0x7A6 halfword fields together. See
 * RuntimeActorSetActiveVector() for the full three-field vector these are
 * part of. */
AT("00009E10") void RuntimeActorSetFields7A4_7A6(u32 actor,s32 first,s32 second)
{
 u8 **root=&gSecondaryRuntime;
 u32 stride=1672;
 actor*=stride;
 actor+=(u32)*root;
 actor+=0x7A0;
 *(s16 *)(actor+4)=first;
 *(s16 *)(actor+6)=second;
}
/** Read an actor record's +0x7A4 and +0x7A6 halfword fields together. See
 * RuntimeActorSetFields7A4_7A6(). */
AT("00009E34") void RuntimeActorGetFields7A4_7A6(u32 actor,u16 *first,u16 *second)
{
 u8 **root=&gSecondaryRuntime;
 u32 stride=1672;
 actor*=stride;
 actor+=(u32)*root;
 actor+=0x7A0;
 *first=*(u16 *)(actor+4);
 *second=*(u16 *)(actor+6);
}
/** @return The address of an actor record's +0x354 field. See
 * RuntimeActorGetField354()/RuntimeActorSetField354() for the typed
 * accessors over the same field. */
AT("00009EEC") void *RuntimeActorGetField354Address(u32 actor)
{
 u8 **root=&gSecondaryRuntime;
 u32 stride=1672;
 actor*=stride;
 actor+=(u32)*root;
 actor+=0x354;
 return (void *)actor;
}

extern u8 *RuntimeGetActorPartRecord(u32 actor,u32 part);
extern s16 *sub_080099E0(u32 actor,u32 group);

/** Count occurrences of value in the five-element signed lookup returned for
 * this actor/group pair. */
AT("000099B8") u32 RuntimeCountMatchingValues(u32 actor,u32 group,s32 value)
{
 u32 count=0;
 s16 *values=sub_080099E0(actor,group);
 s32 remaining=4;
 do {
  if (*values==value)
   count++;
  remaining--;
  values++;
 } while (remaining>=0);
 return count;
}

/** @return How many of an actor's four part records have both their +0 and
 * +8 fields nonzero. */
AT("00009C94") u32 RuntimeCountReadyParts(u32 actor)
{
 u32 count=0;
 s32 part=0;
 do {
  u8 *record=RuntimeGetActorPartRecord(actor,part);
  if (*(u32 *)record && *(u32 *)(record+8))
   count++;
  part++;
 } while (part<=2);
 return count;
}
AT("00009C94") const u8 RuntimeCountReadyPartsTail[2]={0};

/** Clear the +14 byte field of all four of an actor's part records. */
AT("0000AEF0") void RuntimeActorClearField0E(u32 actor)
{
 s32 part=0;
 do {
  u8 *record;
  extern u8 *RuntimeGetActorRecord(u32 actor,u32 part);
  record=RuntimeGetActorRecord(actor,part);
  record[14]=0;
  part++;
 } while (part<=3);
}
AT("0000AEF0") const u8 RuntimeActorClearField0ETail[2]={0};

/** @return Whether any of an actor's four part records has both its +0
 * byte and +74 halfword nonzero. */
AT("00009830") u32 RuntimeActorHasReadyPart(u32 actor)
{
 s32 part=0;
 do {
  u8 *record;
  extern u8 *RuntimeGetActorRecord(u32 actor,u32 part);
  record=RuntimeGetActorRecord(actor,part);
  if (*(s8 *)record && *(s16 *)(record+74))
   return 1;
  part++;
 } while (part<=3);
 return 0;
}
AT("00009830") const u8 RuntimeActorHasReadyPartTail[2]={0};

#define ACTOR_GET_S8(address,name,field) AT(address) s32 name(u32 index) { u8 *base=gSecondaryRuntime; index*=1672; base+=(field); base+=index; return *(s8 *)base; }
#define ACTOR_SET_S8(address,name,field) AT(address) void name(u32 index,s32 value) { u8 *base=gSecondaryRuntime; index*=1672; base+=(field); base+=index; *(s8 *)base=value; }
#define ACTOR_GET_S16(address,name,field) AT(address) s32 name(u32 index) { u8 *base=gSecondaryRuntime; index*=1672; base+=(field); base+=index; return *(s16 *)base; }
#define ACTOR_SET_S16(address,name,field) AT(address) void name(u32 index,s32 value) { u8 *base=gSecondaryRuntime; index*=1672; base+=(field); base+=index; *(s16 *)base=value; }
#define ACTOR_GET_U32(address,name,field) AT(address) u32 name(u32 index) { u8 *base=gSecondaryRuntime; index*=1672; base+=(field); base+=index; return *(u32 *)base; }
#define ACTOR_SET_U32(address,name,field) AT(address) void name(u32 index,u32 value) { u8 *base=gSecondaryRuntime; index*=1672; base+=(field); base+=index; *(u32 *)base=value; }
#define ACTOR_LATE_GET_S8(address,name,field) AT(address) s32 name(u32 index) { u8 *base=gSecondaryRuntime; index*=1672; base+=index; base+=(field); return *(s8 *)base; }
#define ACTOR_LATE_GET_S16(address,name,field) AT(address) s32 name(u32 index) { u8 *base=gSecondaryRuntime; index*=1672; base+=index; base+=(field); return *(s16 *)base; }
#define ACTOR_LATE_SET_S16(address,name,field) AT(address) void name(u32 index,s32 value) { u8 *base=gSecondaryRuntime; index*=1672; base+=index; base+=(field); *(s16 *)base=value; }
#define ACTOR_LATE_SET_S8(address,name,field) AT(address) void name(u32 index,s32 value) { u8 *base=gSecondaryRuntime; index*=1672; base+=index; base+=(field); *(s8 *)base=value; }

/** @return An indexed pointer from the secondary runtime's +0xE30 table. */
AT("00009304") u32 RuntimeGetPointerE30(u32 index) { u8 *base=gSecondaryRuntime; u32 offset=index*4; base+=0xE30; return *(u32 *)(base+offset); }
ACTOR_SET_U32("00009630",RuntimeActorSetField33C,0x33C)
ACTOR_GET_U32("0000964C",RuntimeActorGetField33C,0x33C)
ACTOR_SET_U32("00009668",RuntimeActorSetField340,0x340)
ACTOR_GET_U32("00009684",RuntimeActorGetField340,0x340)
ACTOR_SET_U32("000096A0",RuntimeActorSetField224,0x224)
ACTOR_GET_U32("000096BC",RuntimeActorGetField224,0x224)
ACTOR_SET_U32("000096D8",RuntimeActorSetField228,0x228)
ACTOR_GET_U32("000096F4",RuntimeActorGetField228,0x228)
/** @return Whether a bit is set in the secondary runtime's +0xC0 flag
 * byte. */
AT("00009710") u32 RuntimeTestFlagC0(u32 bit) { return gSecondaryRuntime[0xC0] & (1u<<bit); }
ACTOR_SET_U32("00009EB4",RuntimeActorSetField354,0x354)
ACTOR_GET_U32("00009ED0",RuntimeActorGetField354,0x354)
/** @return An indexed pointer from the secondary runtime's +0xE3C table. */
AT("00009508") u32 RuntimeGetPointerE3C(u32 index) { u8 *base=gSecondaryRuntime; u32 offset=index*4; base+=0xE3C; return *(u32 *)(base+offset); }
ACTOR_LATE_GET_S16("0000975C",RuntimeActorGetField358,0x358)
ACTOR_LATE_GET_S8("00009794",RuntimeActorGetField79C,0x79C)
ACTOR_LATE_SET_S8("000097B4",RuntimeActorSetField79C,0x79C)

/** @return A part record's +0xA4 byte field, sign-extended. See
 * RuntimeActorSetByteA4(). */
AT("00019C64") s32 RuntimeActorGetByteA4(u32 actor,u32 part)
{
 u8 *record;
 extern u8 *RuntimeGetActorRecord(u32 actor,u32 part);
 record=RuntimeGetActorRecord(actor,part);
 return (s8)record[0xA4];
}
AT("00019C64") const u8 RuntimeActorGetByteA4Tail[2]={0};

/** @return Byte +2 of the part record's sub-record at +100 (the same
 * sub-record the setters at 019C78 also use). */
AT("00019CA0") u32 RuntimeActorGetByte66(u32 actor,u32 part)
{
 u8 *record;
 extern u8 *RuntimeGetActorRecord(u32 actor,u32 part);
 record=RuntimeGetActorRecord(actor,part);
 record+=100; /* the per-actor sub-record the setters at 019C78 also use */
 return record[2];
}
AT("00019CA0") const u8 RuntimeActorGetByte66Tail[2]={0};
