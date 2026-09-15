/* Raw field setters and guarded flag changes for 64-byte display objects.
 * Bits 2, 4 and 0x400 have confirmed storage/guard behavior; their rendering
 * meanings and the units of fields +24/+26/+34/+36 remain unresolved.
 * A zero two-byte alignment tail belongs to the original +28388 range. */
#include "object.h"
#include "rom_section.h"

extern s32 SpriteResourceFindGroup(u32 resource, const char *name);
extern struct SpriteResourceLevel1 *SpriteResourceGetLevel1(u32 resource, u32 index0, u32 offset);
extern void CpuCopy(void *dest, const void *src, u32 n);

/** Set +26 and clear +24. Units and gameplay meaning unresolved. */
AT("00028380")
void ObjectSetField26(struct Object *p,u16 value)
{
    p->unk_24=0;
    p->unk_26=value;
}

/** Set both +34 and +36 together. Likely a position pair, given
 * ObjectSetField34() and ObjectSetField36() set the same fields individually. */
AT("00028388")
void ObjectSetFields34And36(struct Object *p,u16 x,u16 y)
{
    p->unk_34=x;
    p->unk_36=y;
}

AT("00028388") const u8 ObjectSetFields34And36Tail[2]={0,0};
/** Set +36 alone. See ObjectSetFields34And36(). */
AT("00028390")
void ObjectSetField36(struct Object *p,u16 value)
{
    p->unk_36=value;
}

/** Set +34 alone. See ObjectSetFields34And36(). */
AT("00028394")
void ObjectSetField34(struct Object *p,u16 value)
{
    p->unk_34=value;
}

/**
 * @brief Change an active object's sprite resource/group/offset, resolve the
 * new level-1 resource record, and flag the object for a display refresh.
 * A parameter of -1 leaves that field unchanged.
 * @param object Object to update; no-op unless it is active.
 * @param resource New resource index, or -1 to keep the current one.
 * @param group New group index, or -1 to keep the current one.
 * @param offset New offset, or -1 to keep the current one.
 * @param arg4 New value for +14; meaning not yet recovered.
 * @return Nothing.
 */
AT("0002814C")
void ObjectSetResourceGroup(struct Object *object, s32 resource, s32 group, s32 offset, u32 arg4)
{
    struct SpriteResourceLevel1 *level1;
    if (object->flags & OBJECT_ACTIVE)
    {
        object->flags = (object->flags & 0xFFEF) | 0x4000;
        if (resource != -1) object->unk_08 = resource;
        if (group != -1) object->unk_0C = group;
        if (offset != -1) object->unk_10 = offset;
        object->unk_14 = arg4;
        object->unk_24 = 0;
        level1 = SpriteResourceGetLevel1(object->unk_08, object->unk_0C, object->unk_10);
        object->unk_18 = level1->unknown04;
    }
}

/** ObjectSetResourceGroup(), looking the group up by name instead of index.
 * @return Nothing. */
AT("00028118")
void ObjectSetResourceGroupByName(struct Object *object, s32 resource, const char *name, s32 offset, u32 arg4)
{
    s32 group = SpriteResourceFindGroup(resource, name);
    ObjectSetResourceGroup(object, resource, group, offset, arg4);
}
AT("00028118") const u8 ObjectSetResourceGroupByNameTail[2] = {0, 0};

/** Overwrite an active object's 64-byte record from a template, preserving
 * only its own `record` pointer, then remember the template and flag the
 * object for a display refresh.
 * @param object Object to overwrite; no-op unless it is active.
 * @param template Source record to copy from; also stored at +04.
 * @return Nothing. */
AT("000281A4")
void ObjectCopyFieldsFromTemplate(struct Object *object, const void *template)
{
    if (object->flags & OBJECT_ACTIVE)
    {
        void *record = object->record;
        CpuCopy(object, template, 64);
        object->record = record;
        object->unk_04 = (void *)template;
        object->flags = (object->flags & 0xFDFF) | 0x4000;
    }
}

/** Set or clear flag bit 4 on an active object; a no-op otherwise. Rendering
 * meaning not yet recovered. */
AT("000282F8")
void ObjectSetFlag4(struct Object *p,u16 enabled)
{
    if (p->flags & 0x8000)
    {
        if (enabled)p->flags|=4;
        else p->flags &= ~4;
    }
}

/** Set or clear flag bit 0x400 on an active object; a no-op otherwise.
 * Rendering meaning not yet recovered. */
AT("00028324")
void ObjectSetFlag400(struct Object *p,u16 enabled)
{
    if (p->flags & 0x8000)
    {
        if (enabled)p->flags|=0x400;
        else p->flags &= ~0x400;
    }
}

/** Set or clear flag bit 2 on an active object; a no-op otherwise. Rendering
 * meaning not yet recovered. */
AT("00028354")
void ObjectSetFlag2(struct Object *p,u16 enabled)
{
    if (p->flags & 0x8000)
    {
        if (enabled)p->flags|=2;
        else p->flags &= ~2;
    }
}

/** @return The active object's record data, 8 bytes into its `record`
 * block; NULL if the object is not active. */
AT("000288DC") void *ObjectGetActiveRecordData(struct Object *object)
{
 if (object->flags&OBJECT_ACTIVE)
  return (u8 *)object->record+8;
 return 0;
}

/** Toggle state bit 0 on a live object. Enabling it also clears the pending
 * position/state fields, clears bit 4, and sets bit 14. */
AT("000282B4") void ObjectSetState1(struct Object *object, u16 enabled)
{
    if (object->flags & OBJECT_ACTIVE)
    {
        if (enabled)
        {
            object->unk_24 = 0;
            object->unk_14 = 0;
            object->flags = (object->flags & 0xFFEF) | 0x4001;
        }
        else
        {
            object->flags &= 0xFFFE;
        }
    }
}
