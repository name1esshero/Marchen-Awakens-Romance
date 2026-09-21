#include "english.h"
#include "item.h"

extern const char gConsumableNoneDescription[];

#define CONSUMABLE_NAME_BASE        ((const char *)&gConsumableNoneText)
#define CONSUMABLE_DESCRIPTION_BASE gConsumableNoneDescription
#define CONSUMABLE_RECORD_SIZE      0x50

/* Menu renderers request definition names directly and never pass them through
 * DialogueStart. This English-only helper preserves the original signed ID
 * narrowing, then uses the same reviewed exact-string mapping as dialogue. */
const char *EnglishItemGetName(s32 id)
{
    const char *japanese = (const char *)gArmDefinitions[(s16)id].name;
    return EnglishTranslateSingle(japanese);
}

const char *EnglishItemGetDescription(s32 id)
{
    const char *japanese = (const char *)gArmDefinitions[(s16)id].description;
    return EnglishTranslateSingle(japanese);
}

const char *EnglishConsumableGetName(s32 id)
{
    const char *japanese =
        CONSUMABLE_NAME_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
    return EnglishTranslateSingle(japanese);
}

const char *EnglishConsumableGetDescription(s32 id)
{
    const char *japanese =
        CONSUMABLE_DESCRIPTION_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
    return EnglishTranslateSingle(japanese);
}

const char *EnglishConsumableGetResourceName(s32 id)
{
    const char *japanese =
        CONSUMABLE_NAME_BASE + (s16)id * CONSUMABLE_RECORD_SIZE;
    return EnglishTranslateSingle(japanese);
}
