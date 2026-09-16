#ifndef OBJECT_H
#define OBJECT_H

#include "gba/types.h"
#include "sprite_engine.h"

/* Display objects, managed by the routines around 0x08028000.
 *
 * Recovered from the initialiser at 0x08028008 and the family of accessors
 * that follow it. The flag setters begin by testing bit 15 of
 * the halfword at +0x28, so that bit gates whether an object is live; these setters
 * leave inactive objects unchanged. The raw field setters do not test it.
 *
 * Two allocations are involved. The caller owns a 64-byte struct, which the
 * initialiser clears, and the manager allocates a separate 72-byte record
 * from the heap and stores its pointer at +0x00.
 *
 * Only the fields the initialiser writes are identified. The rest are left
 * as raw bytes rather than guessed at. */

#define OBJECT_STRUCT_SIZE  64      /* the caller's struct, cleared on init */
#define OBJECT_RECORD_SIZE  72      /* the heap record the manager allocates */
#define OBJECT_RECORD_SPRITE_OFFSET 8

/* Bits in the flags halfword at +0x28. */
#define OBJECT_ACTIVE 0x8000        /* gates the flag setters */
#define OBJECT_MULTIPLE_RECORDS 0x0200 /* record points to unk_18 entries */

struct Object
{
    void *record;               /* 0x00: the 72-byte heap record */
    void *unk_04;                /* 0x04: the last template pointer given to
                                 *       ObjectCopyFieldsFromTemplate */
    s32 unk_08;                  /* 0x08: SpriteResourceGetLevel1's "resource" arg */
    s32 unk_0C;                  /* 0x0C: SpriteResourceGetLevel1's "index0" arg,
                                 *       usually a group index from
                                 *       SpriteResourceFindGroup */
    s32 unk_10;                  /* 0x10: SpriteResourceGetLevel1's "offset" arg */
    u32 unk_14;                 /* cleared when state bit 0 is enabled */
    u32 unk_18;                  /* 0x18: cached from level1->unknown04 by
                                 *       ObjectSetResourceGroup; read back as a
                                 *       signed loop count of 72-byte record
                                 *       entries by the free routines */
    u8 filler_1C[4];
    u16 unk_20;                 /* 0x20: initialised to 0x100, which is 1.0
                                 *       read as 8.8 fixed point */
    u8 filler_22[2];
    u16 unk_24;                 /* reset by ObjectSetField26 and by
                                 *       ObjectSetResourceGroup */
    u16 unk_26;
    u16 flags;                  /* 0x28: OBJECT_ACTIVE and others */
    u8 filler_2A[2];
    u16 unk_2C;                 /* 0x2C: cleared by one of the setters */
    u8 filler_2E[6];
    u16 unk_34;
    u16 unk_36;                 /* 0x36: written by the setter at 0x08028388 */
    u8 filler_38[8];
};

void *ObjectGetActiveRecordData(struct Object *object);
void ObjectSetState1(struct Object *object, u16 enabled);

/* Family around 0x08028118, decompiled together because they share the
 * struct fields discovered above.
 *
 * ObjectSetResourceGroup stores whichever of resource/group/offset the
 * caller passes something other than -1 for, always stores arg4 into
 * unk_14 and resets unk_24 to 0, and always re-derives unk_18 from
 * SpriteResourceGetLevel1 using the (possibly just-updated) fields at
 * +0x08/+0x0C/+0x10. ObjectSetResourceGroupByName is a thin wrapper that
 * looks the group index up by name first. Both are no-ops on an inactive
 * object. */
void ObjectSetResourceGroup(struct Object *object, s32 resource, s32 group, s32 offset, u32 arg4);
void ObjectSetResourceGroupByName(struct Object *object, s32 resource, const char *name, s32 offset, u32 arg4);

/* Bulk-overwrites the 64-byte object from a caller-supplied template, but
 * preserves the manager-owned record pointer (+0x00) and then stores the
 * template pointer itself into +0x04. A no-op on an inactive object. */
void ObjectCopyFieldsFromTemplate(struct Object *object, const void *template);

void ObjectFreeNcdResources(struct Object *object);
void ObjectFreeAuxiliaryResources(struct Object *object);

#endif /* OBJECT_H */
