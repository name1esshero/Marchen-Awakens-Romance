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

#define AT(x) __attribute__((section(".rom." x)))
extern void CpuFill(void *, u32, u32);

/* The dialogue reader accepts this wider lead-byte range, including private
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

AT("0007ADE4")
void ClearFont(struct Font *font)
{
    CpuFill(font, 4, 0);
}
AT("0007ADE4") const u8 ClearFontTail[2] = {0, 0};

/* Rows are rounded to eight pixels before multiplying by bit depth. */
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

/* Decode one engine character. The private lead-byte range extends beyond
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
