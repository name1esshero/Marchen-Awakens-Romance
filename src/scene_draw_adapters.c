/* Scene draw entry points that only chain a fixed sequence of draw steps on
 * one scene pointer.  The steps themselves are still assembly, so they keep
 * their sub_ADDRESS placeholder names. */
#include "gba/types.h"

#include "rom_section.h"

extern void sub_0805EF14(void *scene);
extern void sub_0805F1F8(void *scene);
extern void sub_0805F6B8(void *scene);

/**
 * @brief Run the three recovered drawing stages for this scene family.
 * @param scene Scene state passed unchanged to every drawing stage.
 */
AT("0005F874") void SceneDraw5F874(void *scene)
{
    sub_0805EF14(scene);
    sub_0805F1F8(scene);
    sub_0805F6B8(scene);
}
AT("0005F874") const u8 SceneDraw5F874Tail[2] = {0};
