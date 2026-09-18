#ifndef FONT_H
#define FONT_H
#include "gba/types.h"

/* FONT.NFT at ROM 0x7BA990, looked up by name through MAR.NFP.
 * Field uses: 0807ADF4 (stride), 0807AE08 (palette), 0807AE10 (glyph).
 * This ROM uses width=8, height=8, bpp=1, glyphCount=7984, stride=8.
 * Bitmap rows are stored most-significant bit first. */
struct FontData
{
    u8 signature[0x20];
    u32 width;
    u32 height;
    u32 bpp;
    u32 glyphCount;
    u32 paletteOffset;
    u32 glyphOffset;
    u32 bytesPerGlyph;
    u32 unk_3C;
};
struct Font { const struct FontData *data; };
void SetFontData(struct Font *font, const struct FontData *data);
u16 ReadEngineCharacter(const u8 *text);
u32 CopyTextWithoutTerminator(u8 *destination, const char *source);
u32 WriteEngineCharacter(u8 *text, u16 code);
u32 IsEngineDoubleByte(const u8 *text);
void InitFont(struct Font *, const struct FontData *);
void ClearFont(struct Font *);
u32 FontGlyphStride(const struct Font *);
const u16 *FontPalette(const struct Font *);
u32 FontCharacterToGlyph(u32 code);
const u8 *GetFontGlyph(const struct Font *, u32 code);
#endif
