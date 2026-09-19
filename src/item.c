/* Item-table accessors. IDs are deliberately narrowed to signed 16 bits,
 * as in the original callers. No range checks are added. */
#include "item.h"

#include "game_state.h"
#include "rom_section.h"

extern s32 GameStateGetEntry2768Total(s32 id);
extern void sub_08056A8C(s32 id, s32 mode);
extern u16 sub_080570BC(s32 id);

/* Consumables use one-based IDs.  Their printable fields have the same
 * 0x50-byte stride as ItemDefinition, but the lookup bases point at a blank
 * ID-zero name/description immediately before gItemDefinitions. */
extern const char gConsumableNoneDescription[];
#define CONSUMABLE_NAME_BASE        ((const char *)&gConsumableNoneText)
#define CONSUMABLE_DESCRIPTION_BASE gConsumableNoneDescription
#define CONSUMABLE_RECORD_SIZE      0x50

enum
{
    ARM_PURCHASE_MAX_COPIES = 98,
    ARM_INVENTORY_MODE = 1,
    CONSUMABLE_COST_OFFSET = 0x4C,
    CONSUMABLE_ADD_FAILED = 0xFFFF
};

/**
 * @brief Spend the shared resource counter to add an ARM or consumable.
 * @param id Signed 16-bit ARM or consumable identifier.
 * @param isConsumable Zero for an ARM; nonzero for a consumable.
 * @return A ResourcePurchaseResult describing the attempted purchase.
 */
AT("0005427C")
s32 TryPurchaseArmOrConsumable(s32 id, s32 isConsumable)
{
    s32 savedId;
    s32 resourceId;

    resourceId = (s16)id;
    savedId = resourceId;
    if ((s16)isConsumable == 0)
    {
        const struct ArmDefinition *definition = ItemGetDefinition(resourceId);

        if (definition->field6C > GameStateGetResourceCounter())
            return RESOURCE_PURCHASE_NOT_ENOUGH;
        if ((s16)GameStateGetEntry2768Total(resourceId) <=
            ARM_PURCHASE_MAX_COPIES)
        {
            GameStateAddResourceCounter(-definition->field6C);
            sub_08056A8C(resourceId, ARM_INVENTORY_MODE);
            return RESOURCE_PURCHASE_SUCCESS;
        }
    }
    else
    {
        const u8 *entry =
            (const u8 *)ConsumableGetResourceName(savedId);

        if (*(const u32 *)(entry + CONSUMABLE_COST_OFFSET) >
            GameStateGetResourceCounter())
            return RESOURCE_PURCHASE_NOT_ENOUGH;
        if (sub_080570BC(savedId) != CONSUMABLE_ADD_FAILED)
        {
            GameStateAddResourceCounter(
                -*(const u32 *)(entry + CONSUMABLE_COST_OFFSET));
            return RESOURCE_PURCHASE_SUCCESS;
        }
    }
    return RESOURCE_PURCHASE_FULL;
}

/** Return the arm definition selected by a signed-16-bit ID. */
AT("00056464")
const struct ArmDefinition *ItemGetDefinition(s32 id)
{
    return &gArmDefinitions[(s16)id];
}

#ifndef ENGLISH
/** Return the Japanese name for a signed-16-bit arm ID. */
AT("00056474")
const char *ItemGetName(s32 id)
{
    return (const char *)gArmDefinitions[(s16)id].name;
}
#endif

#ifndef ENGLISH
/** Return the Japanese description for a signed-16-bit arm ID. */
AT("00056484")
const char *ItemGetDescription(s32 id)
{
    return (const char *)gArmDefinitions[(s16)id].description;
}
#endif

/** Return field 0x63 from the signed-16-bit arm definition ID. */
AT("00056494")
s32 ItemGetField63(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field63;
}

/** Return field 0x64 from the signed-16-bit arm definition ID. */
AT("000564AC")
s32 ItemGetField64(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field64;
}

/** Return field 0x65 from the signed-16-bit arm definition ID. */
AT("000564C4")
s32 ItemGetField65(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].element;
}

/** Return field 0x5C from the signed-16-bit arm definition ID. */
AT("000564DC")
s32 ItemGetField5C(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field5C;
}

/** Return field 0x5E from the signed-16-bit arm definition ID. */
AT("000564F0")
s32 ItemGetField5E(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field5E;
}

/** Return field 0x60 from the signed-16-bit arm definition ID. */
AT("00056504")
s32 ItemGetField60(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field60;
}

/** Return field 0x59 from the signed-16-bit arm definition ID. */
AT("00056518")
s32 ItemGetField59(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].type;
}

/** Return field 0x5A from the signed-16-bit arm definition ID. */
AT("00056530")
s32 ItemGetField5A(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field5A;
}

/** Return field 0x62 from the signed-16-bit arm definition ID. */
AT("00056548")
s32 ItemGetField62(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field62;
}

/** Return field 0x58 from the signed-16-bit arm definition ID. */
AT("00056560")
s32 ItemGetField58(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field58;
}

/** Return a consumable name from the fixed-stride text table. */
AT("00057108")
const char *ConsumableGetName(s32 id)
{
    return CONSUMABLE_NAME_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
}

/** Return a consumable description from the fixed-stride text table. */
AT("00057120")
const char *ConsumableGetDescription(s32 id)
{
    return CONSUMABLE_DESCRIPTION_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
}

/** Some resource-loading paths use a distinct entry point with the same
 * lookup semantics.  Keep it named separately because callers may be patched
 * independently by the English build. */
AT("0005715C")
const char *ConsumableGetResourceName(s32 id)
{
    return CONSUMABLE_NAME_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
}

/** field78 is read through the address of field7C: the original source
 * computed the record address from the 0x7C offset and stepped back one
 * word, which is why the literal pool holds the field7C base. */
AT("00056F68")
u32 ItemGetField78(s32 id)
{
    const u8 *record;
    id = (s16)id;
    record = (const u8 *)&gArmDefinitions[id].field7C;
    return *(const u32 *)(record - 4);
}

AT("00056F7C")
u32 ItemGetField7C(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field7C;
}
