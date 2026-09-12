#ifndef ENGLISH_H
#define ENGLISH_H
#include "gba/types.h"
struct EnglishRowMapping
{
    const char *japanese;
    const char *const *rows;
    u8 count;
};
extern const struct EnglishRowMapping gEnglishRows[];
extern const u32 gEnglishRowCount;
/* Return zero if any row is missing/ambiguous or cannot fit this message. */
s32 EnglishTranslateRows(s32 mode, s32 count, const char **source, const char **output);
#endif
