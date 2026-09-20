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
 * The ROM uses the same 16.16 fixed-point loop-stepping idiom as
 * CountConsumableInventoryCopies (src/runtime_accessors.c): a plain index
 * used only for addressing, carried from the previous iteration's
 * pre-increment fixed-point value (`index = previous >> 16;`), separate from
 * the fixed-point accumulator used only for the loop's termination test.
 * That idiom is reproduced exactly below, including its unsigned (`bcs`/
 * `bcc`) loop-entry and termination comparisons -- an earlier version of
 * this candidate used a signed cast that produced `bge`/`blt` instead, a
 * real divergence from the ROM's actual instructions, not merely a
 * candidate that "did not match yet."
 *
 * What remains open: the ROM loads the address of `gMapGenerationRoot` and
 * the `0x426A` constant *before* it sets up the fixed-point step and the
 * `args`-walking pointer copy. Every C shape tried here -- including
 * priming an unused reference to `gMapGenerationRoot` earlier in the
 * function, and every declaration/assignment order of the fixed-point and
 * walker locals -- has agbcc defer those two pool loads to immediately
 * before the loop's first real use of them instead, after the fixed-point
 * and walker setup. Ten restructurings were tried; all ten produced the
 * identical instruction count, both `bcs`/`bcc` branches, the walker
 * correctly copied out of the `args` parameter into its own register (not
 * merely a codegen curiosity -- the ROM's `adds r3, r5, #0` is exactly that
 * copy), and an otherwise-identical loop body; only the relative order of
 * two setup instruction pairs, and consequently which register letters
 * they land in, differs. This looks like the same class of "the compiler
 * schedules a pool load at last use, not at the point a human wrote the
 * reference" limitation as the still-unresolved "where is a pool address
 * materialised" cases noted in docs/AGBCC_CODEGEN.md, not a logic gap.
 */
s32 ScriptNativeCopyMapHalfwordsCandidate(u32 count, const s32 *args, s32 *result)
{
    s32 step;
    const s32 *walker;
    u32 index;
    s32 indexFixed;

    (void)result;
    GameStateClearRecord426A();
    if (count > MAP_HALFWORD_COPY_LIMIT)
        count = MAP_HALFWORD_COPY_LIMIT;

    index = 0;
    if (index >= count)
        goto done;

    indexFixed = 1 << 16;
    step = indexFixed;
    walker = args;
    do
    {
        *(u16 *)((u8 *)gMapGenerationRoot + MAP_HALFWORD_RECORD_OFFSET
                 + index * 2) = (u16)*walker;
        walker++;
        {
            s32 previous = indexFixed;

            indexFixed += step;
            index = previous >> 16;
        }
    } while (index < count);
done:
    return 1;
}
