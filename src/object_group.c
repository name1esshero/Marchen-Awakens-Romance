/* Group-wide operations on the seven 64-byte display objects a battle entity
 * owns.  Three of them sit at +0x1C/+0x5C/+0x9C and are visited in descending
 * order; the remaining four follow from +0xDC with the plain object stride. */
#include "gba/types.h"

#include "rom_section.h"

extern void sub_080281E0(void *object);
extern void ObjectSetFlag2(void *object, u32 enabled);

#define OBJECT_GROUP_FIRST 0x1C
#define OBJECT_GROUP_SECOND 0x5C
#define OBJECT_GROUP_THIRD 0x9C
#define OBJECT_GROUP_TAIL 0xDC
#define OBJECT_GROUP_TAIL_LAST 3
#define OBJECT_SIZE 0x40

/**
 * @brief Release every display object owned by a battle entity.
 * @param entity Base of the entity's embedded display-object group.
 */
AT("00029E00") void ObjectGroupReset(void *entity)
{
    u8 *object = entity;
    s32 i;

    sub_080281E0(object + OBJECT_GROUP_THIRD);
    sub_080281E0(object + OBJECT_GROUP_SECOND);
    sub_080281E0(object + OBJECT_GROUP_FIRST);
    object += OBJECT_GROUP_TAIL;
    for (i = OBJECT_GROUP_TAIL_LAST; i >= 0; i--)
    {
        sub_080281E0(object);
        object += OBJECT_SIZE;
    }
}
AT("00029E00") const u8 ObjectGroupResetTail[2] = {0};

/**
 * @brief Apply object flag two to every display object owned by an entity.
 * @param entity Base of the entity's embedded display-object group.
 * @param enabled Value passed to each object's flag setter.
 */
AT("00029E34") void ObjectGroupSetFlag2(void *entity, u32 enabled)
{
    u8 *object = entity;
    u32 value;
    s32 i;

    value = (u16)enabled;
    ObjectSetFlag2(object + OBJECT_GROUP_THIRD, value);
    ObjectSetFlag2(object + OBJECT_GROUP_SECOND, value);
    ObjectSetFlag2(object + OBJECT_GROUP_FIRST, value);
    object += OBJECT_GROUP_TAIL;
    for (i = OBJECT_GROUP_TAIL_LAST; i >= 0; i--)
    {
        ObjectSetFlag2(object, value);
        object += OBJECT_SIZE;
    }
}
AT("00029E34") const u8 ObjectGroupSetFlag2Tail[2] = {0};
