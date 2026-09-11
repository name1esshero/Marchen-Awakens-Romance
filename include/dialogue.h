#ifndef DIALOGUE_H
#define DIALOGUE_H
#include "gba/types.h"

/* Payload follows the 0x20-byte task header. Three rows hold engine bytes,
 * not Unicode characters. byteOffset and drawnBytes are signed bytes. */
struct DialogueState
{
    u8 rows[3][161];             /* 000..1E2 */
    s8 byteOffset;               /* 1E3 */
    s8 drawnBytes;               /* 1E4 */
    s8 row;                      /* 1E5 */
    u16 lengths[3];
    u8 ink;                      /* 1EC */
    u8 shadow;                   /* 1ED */
    u16 delay;                   /* 1EE */
    u16 countdown;               /* 1F0 */
    u16 glyph;                   /* 1F2 */
    s8 mode;                     /* 1F4: 0 = three rows, 1 = rows 1..2 */
};

const u8 *DialogueReadHex4(const u8 *, u32 *);
u16 DialogueNextGlyph(struct DialogueState *);
void *DialogueStart(s32 mode, s32 count, const char **rows, s32 *result);
#endif
