/* KMP resource loading: named map, palette and tile members become two
 * independently scrollable viewport layers. */
#include "kmp.h"
#include "nfp.h"
#include "runtime_leaf.h"
#include "runtime_misc.h"

#include "rom_section.h"
extern const char gMainArchiveName[];
#define MAIN_ARCHIVE_NAME gMainArchiveName
#define KMP_VIEWPORTS ((struct KmpViewport *)(gIwramBase + 0x3BC4))
#define KMP_SCREEN_BUFFERS ((u16 *)(gIwramBase + 0x0860))

extern u8 gIwramBase[];
extern void IwramSetPointer2860(s32 background, u16 *screenBuffer);

/** KCG members on this path have a GBA LZ header: the upper 24 bits of its
 * first word are the decoded byte count. The loader reserves sixteen extra
 * bytes for its working/alignment area. */
AT("00003410")
u32 KmpGetCompressedTileAllocationSize(const char *mapResource)
{
    const struct KmpHeader *map = NfpOpenByName(MAIN_ARCHIVE_NAME, mapResource);
    const u32 *tiles = NfpOpenByName(MAIN_ARCHIVE_NAME, map->tileResource);
    return (*tiles >> 8) + 16;
}

/**
 * @brief Load selected palette and tile members and configure a KMP viewport.
 * @param name KMP member name in the main archive.
 * @param tileDestination VRAM destination for the KCG tile data.
 * @param slot Viewport and screen-buffer slot.
 * @param plane Tilemap plane selected from the KMP header.
 * @param paletteOffset Additional destination palette-bank offset.
 * @param tileOffset Additional tile-index offset.
 * @param flags Combination of KMP_LOAD_PALETTE and KMP_LOAD_TILES.
 */
AT("00003178")
void KmpLoadResource(const char *name, void *tileDestination, s32 slot,
                     s32 plane,
                     s32 paletteOffset, s32 tileOffset, s32 flags)
{
    const void *resource;
    const struct KmpHeader *data;
    struct KmpViewport *view;
    u16 *screenBuffer;
    u8 *viewportBase;
    u8 *iwramBase;
    u8 *slotBase;
    u32 viewportOffset;
    u32 size;

    data = NfpOpenByName(MAIN_ARCHIVE_NAME, name);
    if (flags & 1)
    {
        resource = NfpOpenByName(MAIN_ARCHIVE_NAME, data->paletteResource);
        CpuCopy((u8 *)RuntimeGetBlock6120(0, 0)
                    + (data->paletteBaseBank + paletteOffset) * 32,
                resource, data->paletteBankCount * 32);
    }

    if (flags & 2)
    {
        resource = NfpOpenByName(MAIN_ARCHIVE_NAME, data->tileResource);
        size = NfpGetEntrySizeByName(MAIN_ARCHIVE_NAME, data->tileResource);
        if (data->compressedTiles)
            Lz77UnCompVramSwapped(tileDestination, resource);
        else
            CpuCopy(tileDestination, resource, size);
    }

    /* Each viewport occupies 0xFC bytes; each tilemap screen buffer is
     * 0x800 bytes. Keep the common IWRAM base visible so these addresses
     * describe the actual runtime layout instead of unrelated literals. */
    viewportOffset = ((slot << 6) - slot) << 2;
    viewportBase = (u8 *)KMP_VIEWPORTS;
    view = (struct KmpViewport *)(viewportBase + viewportOffset);
    screenBuffer = KMP_SCREEN_BUFFERS + slot * 0x400;
    KmpInitViewport(view, data, screenBuffer, slot, plane, 0);
    IwramSetPointer2860(slot, screenBuffer);

    iwramBase = viewportBase - 0x3BC4;
    slotBase = iwramBase + viewportOffset;
    *(u16 *)(slotBase + 0x3BD0) = paletteOffset;
    *(u16 *)(slotBase + 0x3BD2) = tileOffset;
}
