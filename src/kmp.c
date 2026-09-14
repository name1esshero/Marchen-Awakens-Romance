/* Recovered KMP viewport initialization and 16-bit attribute addressing. */
#include "kmp.h"
#include "rom_section.h"

/** This helper always assumes u16 attributes, as does the original.
 * 08003104 is the separate bounds-checked reader that also handles u8 data. */
AT("00003160")
u16 *KmpAttributeAddress(struct KmpViewport *view, u32 x, u32 y)
{
    const struct KmpHeader *data = view->data;
    return (u16 *)((u8 *)data + data->attributesOffset) + (x + y * data->widthTiles);
}
AT("00003160") const u8 KmpAttributeAddressTail[2] = {0, 0};

AT("00002604")
void KmpInitViewport(struct KmpViewport *view, const struct KmpHeader *data,
                     u16 *screen, u32 background, u32 plane, u32 mode)
{
    view->data = data;
    view->renderMode = mode;
    view->background = background;
    view->plane = plane;
    view->screenBuffer = screen;
    view->widthFixed = data->widthTiles << 19;
    view->heightFixed = data->heightTiles << 19;
    view->clipX = 0;
    view->clipY = 0;
    view->clipWidth = data->widthTiles;
    view->clipHeight = data->heightTiles;
}

/* Convert signed pixel coordinates to tile coordinates; return -1 outside
 * the map. The header selects byte or word attributes independently of art. */
__attribute__((section(".rom.00003104")))
s32 KmpReadAttribute(struct KmpViewport *view, s32 x, s32 y)
{
 const struct KmpHeader *data;
 s32 tileX = x >> 3;
 s32 tileY = y >> 3;
 s32 result;
 if (tileX < 0 || tileX >= (s32)view->data->widthTiles ||
     tileY < 0 || tileY >= (s32)view->data->heightTiles)
  return -1;
 data = view->data;
 if (data->wordAttributes)
  result = ((u16 *)((u8 *)data + data->attributesOffset))[tileX + data->widthTiles * tileY];
 else
  result = *((u8 *)data + data->attributesOffset + tileX + tileY * data->widthTiles);
 return result;
}
__attribute__((section(".rom.00003104"))) const u8 KmpReadAttributeTail[2]={0,0};

/* Dispatch using the signed mode byte, preserving 16.16 viewport coordinates.
 * Both render paths remain in assembly; this wrapper does not change position
 * units or infer layer blending from the mode number.
 */
extern void sub_08002650(struct KmpViewport *, s32, s32);
extern void sub_08002798(struct KmpViewport *, s32, s32);
/**
 * @brief Render a KMP viewport with its regular or affine renderer.
 * @param view Viewport configuration and destination buffer.
 * @param x Horizontal 16.16 map position.
 * @param y Vertical 16.16 map position.
 */
AT("00002630")
void KmpRenderViewport(struct KmpViewport *view, s32 x, s32 y)
{
    if ((s8)view->renderMode == 0)
        sub_08002650(view, x, y);
    else
        sub_08002798(view, x, y);
}
AT("00002630") const u8 KmpRenderViewportTail[2] = {0, 0};
