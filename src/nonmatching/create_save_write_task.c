#include "save.h"

/**
 * @brief CreateSaveTask() with mode fixed to a plain SRAM write.
 * @param save Save block to write.
 * @param size Number of bytes to write.
 * @param completion Optional task completion word.
 * @return The created save task.
 *
 * The ROM preserves save in r5 and size in r4. Both available agbcc binaries
 * allocate an ordinary forwarding wrapper in the opposite order, so the exact
 * symbolic implementation remains in assembly pending source-shape recovery.
 */
void *CreateSaveWriteTask(struct SaveBlock *save, u32 size, s32 *completion)
{
    return CreateSaveTask(0, save, size, completion);
}
