/* Readable equivalent of the lookup chain ending at 0807BB88.
 * Not instruction-matched: the original resolves a registered container
 * through global state; these helpers expose the file-relative operations.
 * Dimensions use attr0 bits 14..15 (shape) and attr1 bits 14..15 (size).
 * The 32,265 records in CHR/EFFECT/SYSTEM.NCD all satisfy the 4bpp layout.
 */
#include "ncd.h"

const struct NcdCell *GetNcdFrameCell(const struct NcdHeader *data,
                                    u32 group, u32 animation, u32 frame, u32 cell)
{
    const u8 *base = (const u8 *)data;
    const struct NcdGroup *groups = (const void *)(base + data->groupsOffset);
    const struct NcdAnimation *animations = (const void *)(base + data->animationsOffset);
    const struct NcdFrame *frames = (const void *)(base + data->framesOffset);
    const struct NcdCell *cells = (const void *)(base + data->cellsOffset);
    const struct NcdAnimation *selected = &animations[groups[group].firstAnimation + animation];
    return &cells[frames[selected->firstFrame + frame].firstCell + cell];
}

const u8 *GetNcdCellTiles(const struct NcdHeader *data, const struct NcdCell *cell)
{
    return (const u8 *)data + data->tilesOffset + cell->tileIndex * 32;
}

const u16 *GetNcdCellPalette(const struct NcdHeader *data, const struct NcdCell *cell)
{
    return (const u16 *)((const u8 *)data + data->palettesOffset + cell->paletteIndex * 32);
}
