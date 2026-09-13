#include "english.h"
#include "item.h"

/* Menu renderers request definition names directly and never pass them through
 * DialogueStart. This English-only helper preserves the original signed ID
 * narrowing, then uses the same reviewed exact-string mapping as dialogue. */
const char *EnglishItemGetName(s32 id)
{
    const char *japanese = gItemDefinitions[(s16)id].name;
    return EnglishTranslateSingle(japanese);
}

const char *EnglishItemGetDescription(s32 id)
{
    const char *japanese = gItemDefinitions[(s16)id].description;
    return EnglishTranslateSingle(japanese);
}
