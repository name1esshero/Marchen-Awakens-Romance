#include "game_state.h"
#include "runtime_misc.h"

#define MAP_HALFWORD_COPY_LIMIT 40
#define MAP_HALFWORD_RECORD_OFFSET 0x426A

/**
 * @brief Replace the script-configured map halfword record.
 * @param count Number of VM argument slots supplied.
 * @param args Values to narrow and copy into the record.
 * @param result Unused native-command result pointer.
 * @return One, indicating that the native command completed immediately.
 *
 * The exact ROM shifts the index before adding the record offset and retains
 * a 16.16 induction value in a different low register. Both available agbcc
 * snapshots compile this natural typed loop to a same-size near match, so the
 * symbolic implementation remains in assembly until its source shape is
 * recovered.
 */
s32 ScriptNativeCopyMapHalfwords(u32 count, const s32 *args, s32 *result)
{
    u8 * volatile *root = (u8 * volatile *)&gMapGenerationRoot;
    u16 *record;
    u32 index;

    (void)result;
    GameStateClearRecord426A();
    if (count > MAP_HALFWORD_COPY_LIMIT)
        count = MAP_HALFWORD_COPY_LIMIT;

    for (index = 0; index < count; index++) {
        record = (u16 *)(*root + MAP_HALFWORD_RECORD_OFFSET);
        record[index] = (u16)args[index];
    }
    return 1;
}
