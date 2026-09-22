/* Dispatcher over the still-assembly affine-OAM matrix writers in
 * src/nonmatching/sprite_affine_matrix.c (SpriteAffineWriteNormal,
 * SpriteAffineWriteMirrored, SpriteAffineWriteAlternateAxis). Those three
 * remain named assembly; this caller matches independently of them. */
#include "gba/types.h"

extern void SpriteAffineWriteNormal(s32 index, s32 angle, s32 scaleX, s32 scaleY);
extern void SpriteAffineWriteMirrored(s32 index, s32 angle, s32 scaleX, s32 scaleY);
extern void SpriteAffineWriteAlternateAxis(s32 index, s32 angle, s32 scaleX, s32 scaleY);

#include "rom_section.h"

/** Select an affine-OAM matrix writer by mode: 0 normal, 1 mirrored across
 * X, 2 opposite Y handedness, 3 normal rotated a half turn (+2048 of 4096
 * angle units). Any other mode does nothing. */
AT("0007CDE0") void SpriteAffineWriteDispatch(s32 index, s32 mode, s32 angle,
                                              s32 scaleX, s32 scaleY)
{
    s32 a;
    s32 x;
    s32 y;

    a = (s16)angle;
    x = (s16)scaleX;
    y = (s16)scaleY;

    switch (mode)
    {
    case 0:
        SpriteAffineWriteNormal(index, a, x, y);
        break;
    case 1:
        SpriteAffineWriteMirrored(index, a, x, y);
        break;
    case 2:
        SpriteAffineWriteAlternateAxis(index, a, x, y);
        break;
    case 3:
        SpriteAffineWriteNormal(index, (s16)(a + 2048), x, y);
        break;
    }
}
AT("0007CDE0") const u8 SpriteAffineWriteDispatchTail[2] = {0, 0};
