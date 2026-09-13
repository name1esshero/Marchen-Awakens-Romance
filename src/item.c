/* Item-table accessors. IDs are deliberately narrowed to signed 16 bits,
 * as in the original callers. No range checks are added. */
#include "item.h"

#define AT(x) __attribute__((section(".rom." x)))

/* Consumables use one-based IDs.  Their printable fields have the same
 * 0x50-byte stride as ItemDefinition, but the lookup bases point at a blank
 * ID-zero name/description immediately before gItemDefinitions. */
#define CONSUMABLE_NAME_BASE        ((const char *)0x081BE7EC)
#define CONSUMABLE_DESCRIPTION_BASE ((const char *)0x081BE80E)
#define CONSUMABLE_RECORD_SIZE      0x50

__attribute__((section(".rom.00056464")))
const struct ArmDefinition *ItemGetDefinition(s32 id)
{
    return &gArmDefinitions[(s16)id];
}

#ifndef ENGLISH
__attribute__((section(".rom.00056474")))
const char *ItemGetName(s32 id)
{
    return (const char *)gArmDefinitions[(s16)id].name;
}
#endif

#ifndef ENGLISH
__attribute__((section(".rom.00056484")))
const char *ItemGetDescription(s32 id)
{
    return (const char *)gArmDefinitions[(s16)id].description;
}
#endif

__attribute__((section(".rom.00056494")))
s32 ItemGetField63(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field63;
}

__attribute__((section(".rom.000564AC")))
s32 ItemGetField64(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field64;
}

__attribute__((section(".rom.000564C4")))
s32 ItemGetField65(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].element;
}

__attribute__((section(".rom.000564DC")))
s32 ItemGetField5C(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field5C;
}

__attribute__((section(".rom.000564F0")))
s32 ItemGetField5E(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field5E;
}

__attribute__((section(".rom.00056504")))
s32 ItemGetField60(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field60;
}

__attribute__((section(".rom.00056518")))
s32 ItemGetField59(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].type;
}

__attribute__((section(".rom.00056530")))
s32 ItemGetField5A(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field5A;
}

__attribute__((section(".rom.00056548")))
s32 ItemGetField62(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field62;
}

__attribute__((section(".rom.00056560")))
s32 ItemGetField58(s32 id)
{
    id = (s16)id;
    return gArmDefinitions[id].field58;
}

AT("00057108")
const char *ConsumableGetName(s32 id)
{
    return CONSUMABLE_NAME_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
}

AT("00057120")
const char *ConsumableGetDescription(s32 id)
{
    return CONSUMABLE_DESCRIPTION_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
}

/* Some resource-loading paths use a distinct entry point with the same
 * lookup semantics.  Keep it named separately because callers may be patched
 * independently by the English build. */
AT("0005715C")
const char *ConsumableGetResourceName(s32 id)
{
    return CONSUMABLE_NAME_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
}
