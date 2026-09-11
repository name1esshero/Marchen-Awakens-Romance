#ifndef ENTITY_H
#define ENTITY_H

#include "gba/types.h"

/* Legacy name: these accessors actually address a KMP background viewport.
 * 08003178 loads its map, tiles and palettes; 08002604 initializes it.
 * See kmp.h for the recovered header and initial viewport fields.
 *
 * The four early accessors and the render override at 0x08003348/0x08003350
 * are represented here. The override's use is recovered from 0x08003358;
 * fields not yet modeled remain reserved bytes. This is a partial layout,
 * not a claim about the object's full allocation size. */
struct Entity
{
    u8 filler_00[0x20];
    u32 unk_20;
    u32 unk_24;
    u32 unk_28;
    u32 unk_2C;
    u8 filler_30[0xC4];
    const void *renderOverrideData; /* 0xF4: alternate descriptor, same layout as field 0 */
    u32 renderOverride;             /* 0xF8: nonzero selects alternate render data */
};

u32 GetEntityField20(struct Entity *entity);
u32 GetEntityField24(struct Entity *entity);
u32 GetEntityField28(struct Entity *entity);
u32 GetEntityField2C(struct Entity *entity);
void SetEntityRenderOverride(struct Entity *entity, u32 enabled);
u32 GetEntityRenderOverride(struct Entity *entity);

#endif /* ENTITY_H */
