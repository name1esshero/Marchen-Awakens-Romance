/* Optional localization bridge. This file is excluded from the matching ROM.
 * Lookup uses the exact bytes passed to the recovered dialogue constructor,
 * so it works for both compressed scripts in RAM and constant ROM strings.
 * Ambiguous translations and messages needing pagination retain Japanese.
 * No VM offsets, resource names, or script command arguments are rewritten. */
#include "dialogue.h"
#include "english.h"

extern void *DialogueStartOriginal(s32, s32, const char **, s32 *);

static s32 CompareRow(const char *left, const char *right)
{
    u32 i;
    for (i = 0; i < 161; i++)
    {
        u32 a = (u8)left[i];
        u32 b = (u8)right[i];
        if (a != b) return (s32)a - (s32)b;
        if (!a) return 0;
    }
    return 1; /* Original row buffer is only 161 bytes. */
}

static const struct EnglishRowMapping *FindRow(const char *source)
{
    u32 low = 0, high = gEnglishRowCount;
    while (low < high)
    {
        u32 middle = low + ((high - low) >> 1);
        s32 comparison = CompareRow(source, gEnglishRows[middle].japanese);
        if (!comparison) return &gEnglishRows[middle];
        if (comparison < 0) high = middle;
        else low = middle + 1;
    }
    return 0;
}

s32 EnglishTranslateRows(s32 mode, s32 count, const char **source, const char **output)
{
    s32 i, j, used = 0;
    s32 capacity;
    if (mode != 0 && mode != 1) return 0;
    capacity = 3 - mode;
    if (count < 1 || count > capacity || !source || !output) return 0;
    for (i = 0; i < count; i++)
    {
        const struct EnglishRowMapping *entry;
        if (!source[i]) return 0;
        entry = FindRow(source[i]);
        if (!entry || !entry->count || used + entry->count > capacity) return 0;
        for (j = 0; j < entry->count; j++) output[used++] = entry->rows[j];
    }
    return used;
}

__attribute__((section(".english.entry")))
void *EnglishDialogueStart(s32 mode, s32 count, const char **rows, s32 *result)
{
    const char *translated[3];
    s32 translatedCount = EnglishTranslateRows(mode, count, rows, translated);
    if (translatedCount)
        return DialogueStartOriginal(mode, translatedCount, translated, result);
    return DialogueStartOriginal(mode, count, rows, result);
}
