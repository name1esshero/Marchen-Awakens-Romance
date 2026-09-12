/* Raw field setters and guarded flag changes for 64-byte display objects.
 * Bits 2, 4 and 0x400 have confirmed storage/guard behavior; their rendering
 * meanings and the units of fields +24/+26/+34/+36 remain unresolved.
 * A zero two-byte alignment tail belongs to the original +28388 range. */
#include "object.h"
#define AT(x) __attribute__((section(".rom." x)))

AT("00028380")
void ObjectSetField26(struct Object *p,u16 value)
{
    p->unk_24=0;
    p->unk_26=value;
}

AT("00028388")
void ObjectSetFields34And36(struct Object *p,u16 x,u16 y)
{
    p->unk_34=x;
    p->unk_36=y;
}

AT("00028388") const u8 ObjectSetFields34And36Tail[2]={0,0};
AT("00028390")
void ObjectSetField36(struct Object *p,u16 value)
{
    p->unk_36=value;
}

AT("00028394")
void ObjectSetField34(struct Object *p,u16 value)
{
    p->unk_34=value;
}

AT("000282F8")
void ObjectSetFlag4(struct Object *p,u16 enabled)
{
    if (p->flags & 0x8000)
    {
        if (enabled)p->flags|=4;
        else p->flags &= ~4;
    }
}

AT("00028324")
void ObjectSetFlag400(struct Object *p,u16 enabled)
{
    if (p->flags & 0x8000)
    {
        if (enabled)p->flags|=0x400;
        else p->flags &= ~0x400;
    }
}

AT("00028354")
void ObjectSetFlag2(struct Object *p,u16 enabled)
{
    if (p->flags & 0x8000)
    {
        if (enabled)p->flags|=2;
        else p->flags &= ~2;
    }
}

AT("000288DC") void *ObjectGetActiveRecordData(struct Object *object)
{
 if (object->flags&OBJECT_ACTIVE)
  return (u8 *)object->record+8;
 return 0;
}
