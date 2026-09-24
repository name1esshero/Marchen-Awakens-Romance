/* Readable C model of sub_08002650, the regular KMP viewport renderer.
 *
 * This is analysis code only. The ROM still uses the original assembly; the
 * separate roles of its two coordinate writeback pairs are not yet known.
 */
#include "kmp.h"

/**
 * Rebuild the regular renderer's 32 by 32 circular screen buffer.
 *
 * Assembly evidence at 08002650..08002794:
 * - input fixed-point positions become tile origins with an arithmetic >> 19;
 * - source coverage is clipped to the KMP width and height;
 * - each source tile is written to destination (x & 31, y & 31);
 * - each u16 entry receives (paletteBankOffset << 12) | tileIndexOffset;
 * - the whole 0x800-byte destination is cleared before clipped writes;
 * - both viewport coordinate pairs at +0x10/+0x14 and +0x18/+0x1c receive
 *   the input positions at the end.
 *
 * The arithmetic right shift for negative positions matches the observed
 * ARM `asrs`; agbcc targets the ARM/GBA behavior here.
 */
void KmpRenderRegularViewportObserved(struct KmpViewport *view,
                                      s32 xFixed, s32 yFixed)
{
    const struct KmpHeader *header = view->data;
    const u8 *headerBytes = (const u8 *)header;
    const u16 *source;
    u16 *screen = view->screenBuffer;
    s32 xTile = xFixed >> 19;
    s32 yTile = yFixed >> 19;
    s32 sourceLeft = xTile < 0 ? 0 : xTile;
    s32 sourceTop = yTile < 0 ? 0 : yTile;
    s32 sourceRight = xTile + KMP_VIEWPORT_SIDE;
    s32 sourceBottom = yTile + KMP_VIEWPORT_SIDE;
    s32 x, y;
    u16 entryOffset = (u16)(((u32)view->paletteBankOffset << 12)
                          | view->tileIndexOffset);

    if (sourceRight > (s32)header->widthTiles)
        sourceRight = header->widthTiles;
    if (sourceBottom > (s32)header->heightTiles)
        sourceBottom = header->heightTiles;

    /* CpuFill receives destination, byte count 0x800, and zero fill value. */
    for (y = 0; y < KMP_VIEWPORT_TILE_COUNT; y++)
        screen[y] = 0;

    source = (const u16 *)(headerBytes
             + header->planeOffsets[(s8)view->plane]);
    source += sourceTop * header->widthTiles + sourceLeft;

    for (y = sourceTop; y < sourceBottom; y++)
    {
        for (x = sourceLeft; x < sourceRight; x++)
        {
            u32 screenIndex = ((u32)y & (KMP_VIEWPORT_SIDE - 1))
                            * KMP_VIEWPORT_SIDE
                            + ((u32)x & (KMP_VIEWPORT_SIDE - 1));
            screen[screenIndex] = (u16)(*source + entryOffset);
            source++;
        }
        source += header->widthTiles - (sourceRight - sourceLeft);
    }

    /* Preserve the observed duplicate writeback without assigning meanings
     * to the four viewport words before their other uses are traced. */
    view->requestedXFixed = xFixed;
    view->requestedYFixed = yFixed;
    view->extentOrPosition.rendered.renderedXFixed = xFixed;
    view->extentOrPosition.rendered.renderedYFixed = yFixed;
}
