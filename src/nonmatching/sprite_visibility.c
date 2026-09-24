/* Readable model of sub_0800E518, retained outside the matching build.
 * Its source-level behavior is clear, but the natural C shape tried so far
 * emits a different instruction sequence and shifts other fragments when
 * substituted into the original split assembly object.
 */
#include "camera.h"

enum
{
    SPRITE_CULL_MARGIN = 64,
    FIELD_SCREEN_WIDTH = 240,
    FIELD_SCREEN_HEIGHT = 160
};

/** Return zero when the signed pixel origin is inside the expanded viewport,
 * or -1 when callers should skip constructing its render object. */
s32 SpriteVisibilityCheckObserved(s32 x, s32 y)
{
    struct CameraCoordinate *camera;
    s32 cameraX;
    s32 cameraY;
    s32 cameraRelativeX;
    s32 cameraRelativeY;

    x = (s16)x;
    y = (s16)y;
    /* PRET_PTR_INT_OK: operation=add the absolute IWRAM-relative camera offset to the IWRAM base; evidence=ROM code loads 0x03000000 and 0x3BD4, adds them, then derives Y from the same base at +4; typed=the offset symbol is a linker absolute, not an object pointer. */
    camera = (struct CameraCoordinate *)(gIwramBase
             + (u32)gFieldCameraOffset);
    cameraX = camera[0].pixel;
    cameraY = camera[1].pixel;

    cameraRelativeX = x - cameraX;
    if (cameraRelativeX > FIELD_SCREEN_WIDTH + SPRITE_CULL_MARGIN - 1)
        return -1;
    if (cameraRelativeX <= -SPRITE_CULL_MARGIN)
        return -1;

    cameraRelativeY = y - cameraY;
    if (cameraRelativeY > FIELD_SCREEN_HEIGHT + SPRITE_CULL_MARGIN - 1)
        return -1;
    if (cameraRelativeY <= -SPRITE_CULL_MARGIN)
        return -1;

    return 0;
}
