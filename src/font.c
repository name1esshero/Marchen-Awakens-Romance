#include "font.h"

/* 0807ADC4 initializes this handle, then calls the setter. The caller at
 * 080017xx obtains data with GetNamedResource("MAR.NFP", "FONT.NFT"). */
__attribute__((section(".rom.0007ADE0"), used))
void SetFontData(struct Font *font, const struct FontData *data)
{
    font->data = data;
}
void sub_0807ADE0(struct Font *, const struct FontData *)
    __attribute__((alias("SetFontData")));

#include "rom_section.h"
extern void CpuFill(void *, u32, u32);

/** The dialogue reader accepts this wider lead-byte range, including private
 * font codes outside standard Shift-JIS. */
AT("000025E4")
u32 IsEngineDoubleByte(const u8 *text)
{
    if ((u8)(text[0] + 0x80) <= 0x1F || text[0] > 0xDF)
        return 1;
    return 0;
}
AT("000025E4") const u8 IsEngineDoubleByteTail[2] = {0, 0};

AT("0007ADC4")
void InitFont(struct Font *font, const struct FontData *data)
{
    CpuFill(font, 4, 0);
    SetFontData(font, data);
}

/**
 * @brief Clear the font object's active data pointer.
 * @param font Font object to reset.
 */
AT("0007ADE4")
void ClearFont(struct Font *font)
{
    CpuFill(font, 4, 0);
}
AT("0007ADE4") const u8 ClearFontTail[2] = {0, 0};

/** Rows are rounded to eight pixels before multiplying by bit depth. */
AT("0007ADF4")
u32 FontGlyphStride(const struct Font *font)
{
    const struct FontData *data = font->data;
    u32 stride = (data->width + 7) >> 3;
    stride = data->bpp * stride;
    return data->height * stride;
}
AT("0007ADF4") const u8 FontGlyphStrideTail[2] = {0, 0};

AT("0007AE08")
const u16 *FontPalette(const struct Font *font)
{
    const struct FontData *data = font->data;
    return (const u16 *)((const u8 *)data + data->paletteOffset);
}

/** Convert the game's 16-bit character encoding to a FONT.NFT glyph index.
 * The arithmetic is deliberately expressed as wrapped 16-bit deltas: these
 * are sparse Shift-JIS and private-use ranges, rather than Unicode ranges.
 * Gaps in the kana block map to glyph zero as they did in the original. */
AT("0007AE7C")
u32 FontCharacterToGlyph(u32 input)
{
    u32 raw = input << 16;
    u16 code = (u16)input;
    u32 result = 0;

    if ((code & 0xFF00) == 0) {
        result = code + 0x1E0;
    } else {
        u16 delta = code - 0x81B0;
        if (delta <= 0xA) {
            result = code - 0x8140;
        } else {
            delta = code - 0x8140;
            if (delta <= 0xBF)
                result = code - 0x8140;
            else if ((u16)(code - 0x839F) <= 0x37)
                result = code - 0x81C0;
            else if ((u16)(code - 0x8440) <= 0x51)
                result = code - 0x8200;
            else if ((u16)(code - 0x8740) <= 0x5C)
                result = code - 0x82C0;
            else if ((u16)(code - 0x8240) <= 0x27F) {
                if ((u16)(code - 0x8240) <= 0x5A ||
                    (u16)(code - 0x829F) <= 0x52)
                    result = code - 0x8180;
                else if ((u16)(code - 0x8340) <= 0xBF)
                    result = code - 0x81C0;
            } else if ((u16)(code - 0x8840) <= 0x104F) {
                result = ((raw >> 24) - 0x88) * 192 +
                         (code & 0xFF) + 0x500;
            } else if ((u16)(code - 0x9890) <= 0x521F) {
                if ((u16)(code - 0x9890) <= 0x76F)
                    result = ((raw >> 24) - 0x98) * 192 +
                             (code & 0xFF) + 0x1100;
                else if ((u16)(code - 0xE040) <= 0xA6F)
                    result = ((code >> 8) - 0xE0) * 192 +
                             (code & 0xFF) + 0x1700;
                else
                    result = 0xFFFF;
            } else {
                result = 0xFFFF;
            }
        }
    }
    return result;
}

/** Resolve the two control-font aliases, substitute the game's missing-glyph
 * box for unsupported codes, then locate the packed bitmap in FONT.NFT. */
AT("0007AE10")
const u8 *GetFontGlyph(const struct Font *font, u32 input)
{
    const struct Font *handle = font;
    u16 code = input;
    u32 glyph;

    if (code == 0xF056)
        glyph = FontCharacterToGlyph(0x81FA);
    else if (code == 0xF040)
        glyph = FontCharacterToGlyph(0x81F9);
    else {
        glyph = FontCharacterToGlyph(code);
        if (glyph == 0xFFFF)
            glyph = FontCharacterToGlyph(0x81A1);
    }

    {
        const struct FontData *data = handle->data;
        const u8 *result = (const u8 *)data + data->glyphOffset;
        u32 stride = ((data->width * data->bpp) + 7) >> 3;
        stride *= data->height;
        return result + glyph * stride;
    }
}

/** Decode one engine character. The private lead-byte range extends beyond
 * standard Shift-JIS. The high and low bytes occupy disjoint bits, so their
 * sum is the combined code; callers advance the cursor themselves. */
AT("000025BC")
u16 ReadEngineCharacter(const u8 *text)
{
    u16 value;
    if ((u8)(text[0] + 0x80) <= 0x1F || text[0] > 0xDF)
        value = (text[0] << 8) + text[1];
    else
        value = text[0];
    return value;
}
