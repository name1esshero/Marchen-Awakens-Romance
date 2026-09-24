#ifndef CAMERA_H
#define CAMERA_H

#include "gba/types.h"

/* The field renderer stores camera coordinates as signed 16.16 values.
 * The subpixel halfword is followed by the signed whole-pixel halfword. */
struct CameraCoordinate
{
    s16 fractional;
    s16 pixel;
};

/* Offset from gIwramBase to the adjacent X/Y camera coordinate pair. */
extern u8 gIwramBase[];
extern u8 gFieldCameraOffset[];

#endif
