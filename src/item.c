/* Item-table accessors. IDs are deliberately narrowed to signed 16 bits,
 * as in the original callers. No range checks are added. */
#include "item.h"

__attribute__((section(".rom.00056464")))
const struct ItemDefinition *ItemGetDefinition(s32 id)
{
    return &gItemDefinitions[(s16)id];
}

#ifndef ENGLISH
__attribute__((section(".rom.00056474")))
const char *ItemGetName(s32 id)
{
    return gItemDefinitions[(s16)id].name;
}
#endif

__attribute__((section(".rom.00056484")))
const char *ItemGetDescription(s32 id)
{
    return gItemDefinitions[(s16)id].description;
}

__attribute__((section(".rom.00056494")))
s32 ItemGetField63(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field63;
}

__attribute__((section(".rom.000564AC")))
s32 ItemGetField64(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field64;
}

__attribute__((section(".rom.000564C4")))
s32 ItemGetField65(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field65;
}

__attribute__((section(".rom.000564DC")))
s32 ItemGetField5C(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field5C;
}

__attribute__((section(".rom.000564F0")))
s32 ItemGetField5E(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field5E;
}

__attribute__((section(".rom.00056504")))
s32 ItemGetField60(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field60;
}

__attribute__((section(".rom.00056518")))
s32 ItemGetField59(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field59;
}

__attribute__((section(".rom.00056530")))
s32 ItemGetField5A(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field5A;
}

__attribute__((section(".rom.00056548")))
s32 ItemGetField62(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field62;
}

__attribute__((section(".rom.00056560")))
s32 ItemGetField58(s32 id)
{
    id = (s16)id;
    return gItemDefinitions[id].field58;
}
