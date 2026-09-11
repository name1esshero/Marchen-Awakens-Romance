/* Complete reading of 0807AE7C, not yet instruction-matched.
 * This maps engine character codes to FONT.NFT slots, not Unicode values.
 * tools/font.py mirrors this mapping and tests/test_font.py compares all
 * 65,536 inputs against an interpreter running the original ROM routine.
 * Several numeric ranges contain unused slots; do not compact them.
 */
#include "font.h"

u32 FontCharacterToGlyph(u16 code)
{
    if (code <= 0xFF)
        return code + 0x1E0;
    if ((code >= 0x81B0 && code <= 0x81BA) ||
        (code >= 0x8140 && code <= 0x81FF))
        return code - 0x8140;
    if (code >= 0x839F && code <= 0x83D6)
        return code - 0x81C0;
    if (code >= 0x8440 && code <= 0x8491)
        return code - 0x8200;
    if (code >= 0x8740 && code <= 0x879C)
        return code - 0x82C0;
    if (code >= 0x8240 && code <= 0x84BF)
    {
        if (code <= 0x829A || (code >= 0x829F && code <= 0x82F1))
            return code - 0x8180;
        if (code >= 0x8340 && code <= 0x83FF)
            return code - 0x81C0;
        return 0; /* Original behavior for the gaps in this range. */
    }
    if (code >= 0x8840 && code <= 0x988F)
        return ((code >> 8) - 0x88) * 192 + (code & 0xFF) + 0x500;
    if (code >= 0x9890 && code <= 0x9FFF)
        return ((code >> 8) - 0x98) * 192 + (code & 0xFF) + 0x1100;
    if (code >= 0xE040 && code <= 0xEAAF)
        return ((code >> 8) - 0xE0) * 192 + (code & 0xFF) + 0x1700;
    return 0xFFFF;
}

/* 0807AE10 redirects two private codes before selecting a glyph, then
 * substitutes the filled-square character for unsupported codes. */
const u8 *GetFontGlyph(const struct Font *font, u16 code)
{
    u32 index, stride;
    const struct FontData *data = font->data;
    if (code == 0xF056)
        code = 0x81FA;
    else if (code == 0xF040)
        code = 0x81F9;
    index = FontCharacterToGlyph(code);
    if (index == 0xFFFF)
        index = FontCharacterToGlyph(0x81A1);
    stride = ((data->width + 7) >> 3) * data->bpp * data->height;
    return (const u8 *)data + data->glyphOffset + index * stride;
}

