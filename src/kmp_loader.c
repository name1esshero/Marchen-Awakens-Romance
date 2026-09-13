/* KMP resource loading: named map, palette and tile members become two
 * independently scrollable viewport layers. */
#include "kmp.h"
#include "nfp.h"
#include "runtime_leaf.h"
#include "runtime_misc.h"

#define AT(x) __attribute__((section(".rom." x)))
#define MAIN_ARCHIVE_NAME ((const char *)0x08086A54)
#define KMP_VIEWPORTS ((struct KmpViewport *)0x03003BC4)
#define KMP_SCREEN_BUFFERS ((u16 *)0x03000860)

extern u32 sub_0807AD4C(const char *archive, const char *member);
extern void sub_08001B4C(s32 background, u16 *screenBuffer);

#ifdef NONMATCHING
AT("00003178")
void KmpLoadResource(const char *name, void *tileDestination, s32 slot, s32 plane,
                     s32 paletteOffset, s32 tileOffset, s32 flags)
{
    const struct KmpHeader *data;
    struct KmpViewport *view;
    u16 *screenBuffer;

    data = NfpOpenByName(MAIN_ARCHIVE_NAME, name);
    if (flags & 1)
    {
        const void *palette = NfpOpenByName(MAIN_ARCHIVE_NAME,
                                            data->paletteResource);
        void *destination = RuntimeGetBlock6120(0, 0);
        destination = (u8 *)destination
                    + (data->paletteBaseBank + paletteOffset) * 32;
        CpuCopy(destination, palette, data->paletteBankCount * 32);
    }

    if (flags & 2)
    {
        const void *tiles = NfpOpenByName(MAIN_ARCHIVE_NAME,
                                          data->tileResource);
        u32 size = sub_0807AD4C(MAIN_ARCHIVE_NAME, data->tileResource);
        if (data->compressedTiles)
            Lz77UnCompVramSwapped(tileDestination, tiles);
        else
            CpuCopy(tileDestination, tiles, size);
    }

    view = (struct KmpViewport *)((u8 *)KMP_VIEWPORTS
                                + (((slot << 6) - slot) << 2));
    screenBuffer = KMP_SCREEN_BUFFERS + slot * 0x400;
    KmpInitViewport(view, data, screenBuffer, slot, plane, 0);
    sub_08001B4C(slot, screenBuffer);
    view->paletteBankOffset = paletteOffset;
    view->tileIndexOffset = tileOffset;
}
#endif
