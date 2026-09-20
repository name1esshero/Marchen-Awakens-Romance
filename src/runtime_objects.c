/* Views of the secondary runtime's actor records and its 4-by-4 object table. */
#include "runtime_objects.h"
#include "battle_character.h"
#include "list.h"
#include "ncd.h"
#include "runtime_misc.h"
#include "runtime_state.h"
#include "rom_section.h"
extern u8 *gRuntimeObjectTable[];
#define OBJECT_TABLE gRuntimeObjectTable
#define RUNTIME_OBJECT(group,slot) OBJECT_TABLE[(group)*4+(slot)]

#define ACTOR_RECORD_SIZE 1672
#define ACTOR_RECORD_HEADER_OFFSET 0x120
#define ACTOR_PART_RECORD_SIZE 168
#define ACTOR_PART_RECORDS_OFFSET 0x23C
#define RUNTIME_LISTS_OFFSET 0x80
#define RUNTIME_LIST_STRIDE 12
#define RUNTIME_LIST_POINTERS_OFFSET 0xB0

/** Copy an object's secondary character hit box into caller-owned storage. */
AT("0000A724")
void RuntimeObjectCopySecondaryHitBounds(u32 group, u32 slot,
                                         struct HitBounds *destination)
{
    u8 *object = RUNTIME_OBJECT(group, slot);
    const struct BattleCharacterDefinition *definition =
        RuntimeGetBattleCharacterDefinition(*(s16 *)(object + 88));
    const struct HitBounds *bounds = &definition->bounds[1];

    destination->left = bounds->left;
    destination->top = bounds->top;
    destination->right = bounds->right;
    destination->bottom = bounds->bottom;
}

/** Release the sprite allocation owned by every entry in a runtime list.
 * The list links occupy the first eight bytes of each entry, immediately
 * followed by its NCD sprite.  The caller resets the list separately. */
AT("00008BE4") void RuntimeReleaseListSpriteAllocations(u32 slot)
{
    u8 **root = &gSecondaryRuntime;
    u8 *base = *root;
    u32 listOffset;
    ListNode *node;

    listOffset = slot * RUNTIME_LIST_STRIDE;
    base += RUNTIME_LISTS_OFFSET;
    base += listOffset;
    node = ((List *)base)->head;

    while (node != NULL) {
        NcdRuntimeSpriteReleaseAllocation((struct NcdSprite *)(node + 1));
        node = node->next;
    }
}

/** Reset one of the secondary runtime's four list slots and its owner. */
AT("00008A44") void RuntimeResetListSlot(u32 slot)
{
    u8 **root = &gSecondaryRuntime;
    u32 listOffset = slot * RUNTIME_LIST_STRIDE;
    u8 *base;

    listOffset += RUNTIME_LISTS_OFFSET;
    ListInit((List *)(*root + listOffset));
    base = *root;
    slot *= sizeof(void *);
    base += RUNTIME_LIST_POINTERS_OFFSET;
    base += slot;
    *(void **)base = NULL;
}

/* This expanded address calculation is shared by the paired actor-part
 * accessors below. Its statement order reproduces the original agbcc code. */
#define ACTOR_PART_RECORD_SETUP() \
    u8 **root = &gSecondaryRuntime; \
    u32 actorOffset = actor * ACTOR_RECORD_SIZE; \
    u8 *record; \
    u32 partOffset; \
    actorOffset += ACTOR_RECORD_HEADER_OFFSET; \
    record = *root + actorOffset; \
    partOffset = part * ACTOR_PART_RECORD_SIZE; \
    partOffset += ACTOR_PART_RECORDS_OFFSET; \
    record += partOffset

/** @return The secondary runtime's +0xEA0 pointer field. See
 * RuntimeSetPointerEA0(). */
AT("0000A0E8") void *RuntimeGetPointerEA0(void) { return *(void **)(gSecondaryRuntime+0xEA0); }
/** Set the secondary runtime's +0xEA0 pointer field. */
AT("0000A0FC") void RuntimeSetPointerEA0(void *v) { *(void **)(gSecondaryRuntime+0xEA0)=v; }
/** Set the secondary runtime's three adjacent byte fields at +0xE48,
 * +0xE49, and +0xE4A in one call. See the individual per-field
 * accessors below. */
AT("0000A110") void RuntimeSetFieldsE48ToE4A(s32 a,s32 b,s32 c)
{
 gSecondaryRuntime[0xE48]=a;
 gSecondaryRuntime[0xE49]=b;
 gSecondaryRuntime[0xE4A]=c;
}
/** Set the secondary runtime's +0xE48 byte field. */
AT("0000A140") void RuntimeSetFieldE48(s32 v) { gSecondaryRuntime[0xE48]=v; }
/** @return The secondary runtime's +0xE48 byte field, sign-extended. */
AT("0000A154") s32 RuntimeGetFieldE48(void) { return *(s8 *)(gSecondaryRuntime+0xE48); }
/** Set the secondary runtime's +0xE49 byte field. */
AT("0000A16C") void RuntimeSetFieldE49(s32 v) { gSecondaryRuntime[0xE49]=v; }
/** @return The secondary runtime's +0xE49 byte field, sign-extended. */
AT("0000A180") s32 RuntimeGetFieldE49(void) { return *(s8 *)(gSecondaryRuntime+0xE49); }
/** Set the secondary runtime's +0xE4A byte field. */
AT("0000A198") void RuntimeSetFieldE4A(s32 v) { gSecondaryRuntime[0xE4A]=v; }
/** @return The secondary runtime's +0xE4A byte field, sign-extended. */
AT("0000A1AC") s32 RuntimeGetFieldE4A(void) { return *(s8 *)(gSecondaryRuntime+0xE4A); }
/** @return One of an actor's per-part records (168-byte stride, based at
 * +0x23C within the actor's 1672-byte block). See RuntimeGetActorPartRecord()
 * for the other, 104-byte-stride table on the same actor. */
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

/** Store two integer coordinates as 16.16 values in an actor-part record. */
AT("0000A3B8")
void RuntimeActorPartSetFixed18And1C(u32 actor, u32 part, s32 first, s32 second)
{
    ACTOR_PART_RECORD_SETUP();

    *(u32 *)(record + 0x18) = first << 16;
    *(u32 *)(record + 0x1C) = second << 16;
}

/** Store two integer coordinates as 16.16 values in the record's second
 * coordinate pair. */
AT("0000A484")
void RuntimeActorPartSetFixed20And24(u32 actor, u32 part, s32 first, s32 second)
{
    ACTOR_PART_RECORD_SETUP();

    *(u32 *)(record + 0x20) = first << 16;
    *(u32 *)(record + 0x24) = second << 16;
}

/** Store the raw 32-bit values at actor-part offsets +0x18 and +0x1C. */
AT("0000A550")
void RuntimeActorPartSetWords18And1C(u32 actor, u32 part, u32 first, u32 second)
{
    ACTOR_PART_RECORD_SETUP();

    *(u32 *)(record + 0x18) = first;
    *(u32 *)(record + 0x1C) = second;
}

/** Read the raw 32-bit values at actor-part offsets +0x18 and +0x1C. */
AT("0000A580")
void RuntimeActorPartGetWords18And1C(u32 actor, u32 part, u32 *first, u32 *second)
{
    ACTOR_PART_RECORD_SETUP();

    *first = *(u32 *)(record + 0x18);
    *second = *(u32 *)(record + 0x1C);
}

/** Store the raw 32-bit values at actor-part offsets +0x20 and +0x24. */
AT("0000A604")
void RuntimeActorPartSetWords20And24(u32 actor, u32 part, u32 first, u32 second)
{
    ACTOR_PART_RECORD_SETUP();

    *(u32 *)(record + 0x20) = first;
    *(u32 *)(record + 0x24) = second;
}

/** Read the raw 32-bit values at actor-part offsets +0x20 and +0x24. */
AT("0000A634")
void RuntimeActorPartGetWords20And24(u32 actor, u32 part, u32 *first, u32 *second)
{
    ACTOR_PART_RECORD_SETUP();

    *first = *(u32 *)(record + 0x20);
    *second = *(u32 *)(record + 0x24);
}
/** The same actor record's other per-part table: stride 104 rather than 168,
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
/** @return A runtime object's +0x48 s16 field. See
 * RuntimeObjectSetField48(). */
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
/** Set a runtime object's +0x18 field from an integer, storing it as a
 * 16.16 fixed-point value (shifted left 16 bits). Paired with
 * RuntimeObjectGetWord18()'s raw fixed-point reader. */
AT("0000A424") void RuntimeObjectSetFixed18(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x18)=value<<16; }
GET_S16("0000A43C",RuntimeObjectGetField1A,0x1A)
/** Set a runtime object's +0x1C field from an integer, storing it as a
 * 16.16 fixed-point value (shifted left 16 bits). Paired with
 * RuntimeObjectGetWord1C()'s raw fixed-point reader. */
AT("0000A454") void RuntimeObjectSetFixed1C(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x1C)=value<<16; }
GET_S16("0000A46C",RuntimeObjectGetField1E,0x1E)
/** Set a runtime object's +0x20 field from an integer, storing it as a
 * 16.16 fixed-point value (shifted left 16 bits). Paired with
 * RuntimeObjectGetWord20()'s raw fixed-point reader. */
AT("0000A4F0") void RuntimeObjectSetFixed20(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(u32 *)(object+0x20)=value<<16; }
GET_S16("0000A508",RuntimeObjectGetField22,0x22)
/** Set a runtime object's +0x24 field from an integer, storing it as a
 * 16.16 fixed-point value (shifted left 16 bits). Paired with
 * RuntimeObjectGetWord24()'s raw fixed-point reader. */
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
/** Set a runtime object's +0x36 byte field. See RuntimeObjectGetField36(). */
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
/** Set a runtime object's +0x39 byte field. See RuntimeObjectGetField39(). */
AT("0000A82C") void RuntimeObjectSetField39(u32 group,u32 slot,s32 value) { OBJECT_PTR(); *(s8 *)(object+0x39)=value; }
SET_S16("000097D0",RuntimeObjectSetField4A,0x4A)
GET_S16("000097E8",RuntimeObjectGetField4A,0x4A)
SET_S16("00009800",RuntimeObjectSetField4C,0x4C)
GET_S16("00009818",RuntimeObjectGetField4C,0x4C)
