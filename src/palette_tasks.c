/* Palette transition task construction. */
#include "gba/types.h"

#include "heap.h"
#include "rom_section.h"
#include "task_manager.h"

#define PALETTE_BANK_SHIFT 4
#define PALETTE_BLEND_ONE (1 << 24)

/** State consumed by PaletteInterpolationTask after the standard task header. */
struct PaletteInterpolationTaskState
{
    s16 remainingFrames;
    u16 paletteOffset;
    s32 blend;
    s32 blendStep;
    const void *target;
    const void *source;
};

/** State consumed by PaletteSequenceTask after the standard task header. */
struct PaletteSequenceTaskState
{
    s16 frameCount;
    u16 unknown02;
    u16 *firstColors;
    u16 *secondColors;
    void *target;
    s16 colorCount;
    u8 unknown12[18];
};

extern void PaletteInterpolationTask(struct EngineTask *task);
extern void PaletteSequenceTask(struct EngineTask *task);
extern void CpuCopy(void *destination, const void *source, u32 size);

/**
 * @brief Start a fixed-duration transition between two palette buffers.
 *
 * The task begins at full 8.24 fixed-point blend and advances toward zero.
 * A zero duration is normalized to one frame so the signed division remains
 * defined. The bank and color selectors are packed in the format consumed by
 * the task callback.
 *
 * @return The new task, or NULL when allocation fails.
 */
AT("0000382C")
struct EngineTask *CreatePaletteInterpolationTask(
    s32 frameCountArg, const void *source, const void *target,
    u32 paletteIndex, u32 paletteBank, s32 *completion)
{
    struct EngineTask *task;
    struct PaletteInterpolationTaskState *state;
    s32 frameCount = (s16)frameCountArg;

    task = CreateTask(&gMainTaskManager, PaletteInterpolationTask, 3,
                      (u32 *)completion, sizeof(*state));
    if (task == NULL)
        return NULL;

    if (frameCount == 0)
        frameCount = 1;

    state = (struct PaletteInterpolationTaskState *)(task + 1);
    state->remainingFrames = frameCount;
    state->target = target;
    state->source = source;
    state->paletteOffset = (paletteBank << PALETTE_BANK_SHIFT) | paletteIndex;
    state->blend = PALETTE_BLEND_ONE;
    state->blendStep = -PALETTE_BLEND_ONE / state->remainingFrames;
    return task;
}

/**
 * @brief Clone two color arrays and start their palette sequence task.
 *
 * The task owns both copies, allowing the caller's arrays to be temporary.
 * A zero frame count is normalized to one before the callback begins.
 *
 * @return The new task, or NULL when task allocation fails.
 */
AT("000041E0")
struct EngineTask *CreatePaletteSequenceTask(
    s32 frameCountArg, const u16 *firstColors, const u16 *secondColors,
    void *target, s32 colorCount, s32 *completion)
{
    struct EngineTask *task;
    struct PaletteSequenceTaskState *state;
    s32 frameCount = (s16)frameCountArg;
    u32 copySize;

    task = CreateTask(&gMainTaskManager, PaletteSequenceTask, 3,
                      (u32 *)completion, sizeof(*state));
    if (task == NULL)
        return NULL;

    if (frameCount == 0)
        frameCount = 1;

    state = (struct PaletteSequenceTaskState *)(task + 1);
    state->frameCount = frameCount;
    state->colorCount = colorCount;
    copySize = colorCount * sizeof(u16);
    state->firstColors = HeapAlloc(NULL, copySize);
    CpuCopy(state->firstColors, firstColors, copySize);
    state->secondColors = HeapAlloc(NULL, copySize);
    CpuCopy(state->secondColors, secondColors, copySize);
    state->target = target;
    return task;
}
AT("000041E0") const u8 CreatePaletteSequenceTaskTail[2] = {0, 0};
