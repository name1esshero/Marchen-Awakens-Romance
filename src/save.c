#include "save.h"
#include "runtime_leaf.h"
#include "sram.h"
#include "game_state.h"
#include "byte_utils.h"

#include "rom_section.h"
#define SRAM_BASE ((u8 *)0x0E000000)
#define SAVE_HEADER_SIZE 44
#define SAVE_FORMAT_VERSION 0x3F828F5C

extern void CpuFill(void *destination, u32 size, u32 value);
extern s32 strcmp(const char *left, const char *right);
extern s32 sub_080815C0(u32 left, u32 right);
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];

struct SaveMagic {
    u32 words[4];
};

extern const struct SaveMagic gSaveMagic;

/** Stamp a save block's magic, format version, and generation counter,
 * recompute its payload and header CRCs, and write it to SRAM synchronously.
 * @return The SRAM write's status, narrowed to s16. */
AT("0006E54C") s32 WriteSaveBlock(struct SaveBlock *save)
{
    const struct SaveMagic *magic = &gSaveMagic;
    u32 blockSize = SAVE_BLOCK_SIZE;
    u32 zero;

    *(struct SaveMagic *)save->magic = *magic;
    save->format = SAVE_FORMAT_VERSION;
    save->generation++;
    zero = 0;
    save->payloadCrc = zero;
    save->payloadCrc = CalculateSaveCrc32(save->payload, SAVE_PAYLOAD_SIZE);
    save->reserved28 = zero;
    save->headerCrc = zero;
    save->headerCrc = CalculateSaveCrc32(save, SAVE_HEADER_SIZE);
    return (s16)(s32)WriteSramFast((const u8 *)save, SRAM_BASE, blockSize);
}

/** Read a save block from SRAM synchronously and validate it, clearing just
 * the payload on a damaged payload or the whole block on a damaged header.
 * Records the outcome via GameStateSetField42C0().
 * @return Always 0. */
AT("0006E5AC") s32 LoadSaveBlock(struct SaveBlock *save)
{
    u32 blockSize = SAVE_BLOCK_SIZE;
    s32 status;

    ReadSramFast(SRAM_BASE, (u8 *)save, blockSize);
    status = (s16)ValidateSaveBlock(save);
    if (status != 0) {
        if (status > 0) {
            GameStateSetField42C0(1);
            CpuFill(save->payload, SAVE_PAYLOAD_SIZE, 0);
        } else {
            GameStateSetField42C0(-1);
            CpuFill(save, blockSize, 0);
        }
    }
    return 0;
}
AT("0006E5AC") const u8 LoadSaveBlockTail[2] = {0};

/** Check a save block's header CRC, magic, format version, and payload CRC,
 * restoring the block's CRC/reserved fields before returning either way.
 * @return 0 if fully valid; -1 on a bad header CRC; -2 on bad magic; 1 if
 * only the format version differs; 2 if only the payload CRC is bad. */
AT("0006E438") s32 ValidateSaveBlock(struct SaveBlock *save)
{
    s32 result = 0;
    u32 headerCrc = save->headerCrc;
    u32 payloadCrc = save->payloadCrc;
    u32 reserved = save->reserved28;

    save->headerCrc = 0;
    if (Crc32Difference(save, SAVE_HEADER_SIZE, headerCrc) != 0) {
        result = -1;
    } else if (strcmp(save->magic, (const char *)&gSaveMagic) != 0) {
        result = -2;
    } else {
        if (sub_080815C0(save->format, SAVE_FORMAT_VERSION) != 0)
            result = 1;
        save->payloadCrc = 0;
        if (Crc32Difference(save->payload, SAVE_PAYLOAD_SIZE, payloadCrc) != 0)
            result = 2;
    }

    save->headerCrc = headerCrc;
    save->payloadCrc = payloadCrc;
    save->reserved28 = reserved;
    return result;
}

/** Read size bytes from SRAM synchronously and immediately signal
 * completion.
 * @return Always 0. */
AT("0006E720") s32 ReadSaveBytes(void *destination, u32 unused, u32 size,
                                  s32 *completion)
{
    ReadSramFast(SRAM_BASE, destination, size);
    if (completion)
        *completion = 1;
    return 0;
}

/** ReadSaveBytes() with its unused second argument fixed to 0.
 * @return Always 0. */
AT("0006E4D4") s32 ReadSaveBytesAndSignal(void *destination, u32 size,
                                           s32 *completion)
{
    return ReadSaveBytes(destination, 0, size, completion);
}

/** Create a task that writes size bytes of save to SRAM via SaveWriteTask().
 * @param mode Stored at task state +0; passed through unnamed.
 * @return The new task, or NULL if creation fails. */
AT("0006E610") struct EngineTask *CreateSaveTask(u32 mode,
                                                  struct SaveBlock *save,
                                                  u32 size,
                                                  s32 *completion)
{
    struct EngineTask *task;
    u8 *work;
    struct SaveBlock *localSave;
    u32 localSize;
    u16 localMode;

    localSave = save;
    localSize = size;
    localMode = mode;
    task = CreateTask(&gMainTaskManager, SaveWriteTask, 1,
                      (u32 *)completion, 24);
    if (task == 0)
        return 0;
    work = (u8 *)task + 32;
    *(u16 *)work = localMode;
    *(struct SaveBlock **)(work + 4) = localSave;
    *(u32 *)(work + 12) = localSize;
    return task;
}
AT("0006E610") const u8 CreateSaveTaskTail[2] = {0};

/** Create a task that asynchronously reads and validates a complete save
 * block via SaveBlockLoadTask().
 * @return The new task, or NULL if creation fails. */
AT("0006E150") struct EngineTask *CreateSaveBlockLoadTask(
    struct SaveBlock *save, u32 *completion)
{
    struct EngineTask *task = CreateTask(&gMainTaskManager,
        SaveBlockLoadTask, 1, completion, 16);

    if (task == 0)
        return 0;
    *(struct SaveBlock **)((u8 *)task + 44) = save;
    return task;
}
AT("0006E150") const u8 CreateSaveBlockLoadTaskTail[2] = {0};

/** Read and validate a complete save without blocking the caller.  A damaged
 * payload keeps its header and advances recoveryCount; a damaged header clears
 * the whole block. */
AT("0006E184") void SaveBlockLoadTask(struct EngineTask *task)
{
    struct EngineTask *localTask = task;
    u8 *work = (u8 *)localTask + 32;
    struct SaveBlock *save =
        *(struct SaveBlock **)(work + 12);

    switch (*(u16 *)((u8 *)localTask + 14)) {
    case 0:
        *(u16 *)((u8 *)localTask + 14) = 2;
        break;
    case 1:
        break;
    case 2:
        ReadSaveBytesAndSignal(*(struct SaveBlock **)(work + 12),
                               SAVE_BLOCK_SIZE, (s32 *)(work + 8));
        *(u16 *)((u8 *)localTask + 14) = 3;
    case 3: {
        s32 result = *(s32 *)(work + 8);
        if (result == 0)
            break;
        *(s32 *)(work + 4) = result;
        if (result == -1) {
            GameStateSetField42C0(-2);
            *(u16 *)((u8 *)localTask + 14) = 5;
            break;
        }
        *(u16 *)((u8 *)localTask + 14) = 4;
    }
    case 4: {
        s32 validation = ValidateSaveBlock(
            *(struct SaveBlock **)(work + 12));
        if (validation != 0) {
            if (validation > 0) {
                GameStateSetField42C0(1);
                CpuFill((*(struct SaveBlock **)(work + 12))->payload,
                        SAVE_PAYLOAD_SIZE, 0);
                save->recoveryCount++;
            } else {
                GameStateSetField42C0(-1);
                CpuFill(*(struct SaveBlock **)(work + 12),
                        SAVE_BLOCK_SIZE, 0);
            }
            *(s32 *)(work + 4) = -1;
        } else {
            *(s32 *)(work + 4) = 1;
        }
    }
    case 5:
        *(u16 *)((u8 *)localTask + 14) = 6;
        break;
    case 6: {
        u32 *completion = localTask->completion;
        if (completion != 0)
            *completion = *(s32 *)(work + 4);
        FinishTask(localTask);
        break;
    }
    }
}

/** Create a task that prepares and writes a complete save block
 * asynchronously via SaveBlockPrepareAndWriteTask().
 * @return The new task, or NULL if creation fails. */
AT("0006E264") struct EngineTask *CreateSaveBlockWriteTask(
    struct SaveBlock *save, u32 *completion)
{
    struct EngineTask *task = CreateTask(&gMainTaskManager,
        SaveBlockPrepareAndWriteTask, 1, completion, 16);

    if (task == 0)
        return 0;
    *(struct SaveBlock **)((u8 *)task + 44) = save;
    return task;
}
AT("0006E264") const u8 CreateSaveBlockWriteTaskTail[2] = {0};

/** Prepare every field in a full save block, then delegate the physical SRAM
 * transfer to SaveWriteTask and forward its completion status. */
AT("0006E298") void SaveBlockPrepareAndWriteTask(struct EngineTask *task)
{
    u8 *work = (u8 *)task + 32;

    switch (*(u16 *)((u8 *)task + 14)) {
    case 0: {
        const struct SaveMagic *magic = &gSaveMagic;
        u32 zero;

        *(struct SaveMagic *)(*(struct SaveBlock **)(work + 12))->magic =
            *magic;
        (*(struct SaveBlock **)(work + 12))->format = SAVE_FORMAT_VERSION;
        (*(struct SaveBlock **)(work + 12))->generation++;
        zero = 0;
        (*(struct SaveBlock **)(work + 12))->payloadCrc = zero;
        (*(struct SaveBlock **)(work + 12))->payloadCrc =
            (u8)BufferXor((*(struct SaveBlock **)(work + 12))->payload,
                          SAVE_PAYLOAD_SIZE);
        (*(struct SaveBlock **)(work + 12))->reserved28 = zero;
        (*(struct SaveBlock **)(work + 12))->headerCrc = zero;
        (*(struct SaveBlock **)(work + 12))->headerCrc =
            (u8)BufferXor(*(const u8 **)(work + 12), SAVE_HEADER_SIZE);
        *(u16 *)((u8 *)task + 14) = 1;
        break;
    }
    case 1:
        *(u16 *)((u8 *)task + 14) = 2;
        break;
    case 2:
        CreateSaveWriteTask(*(struct SaveBlock **)(work + 12),
                            SAVE_BLOCK_SIZE, (s32 *)(work + 8));
        *(u16 *)((u8 *)task + 14) = 3;
        /* Observe the child completion slot on the creation frame too. */
    case 3: {
        s32 childResult = *(s32 *)(work + 8);
        if (childResult == 0)
            break;
        *(s32 *)(work + 4) = childResult;
        *(u16 *)((u8 *)task + 14) = 4;
        /* State four advances immediately to the final notification state. */
    }
    case 4:
        *(u16 *)((u8 *)task + 14) = 5;
        break;
    case 5:
        if (task->completion)
            *task->completion = *(s32 *)(work + 4);
        FinishTask(task);
        break;
    }
}
AT("0006E298") const u8 SaveBlockPrepareAndWriteTaskTail[2] = {0};

/** Create a task that writes and verifies just the save header
 * asynchronously via SaveHeaderWriteTask().
 * @return The new task, or NULL if creation fails. */
AT("0006E360") struct EngineTask *CreateSaveHeaderWriteTask(
    struct SaveBlock *save, u32 size, s32 *completion)
{
    struct EngineTask *task = CreateTask(
        &gMainTaskManager, SaveHeaderWriteTask, 1,
        (u32 *)completion, 20);
    u8 *work;

    if (!task)
        return 0;
    work = (u8 *)task + 32;
    *(struct SaveBlock **)(work + 4) = save;
    *(u32 *)(work + 8) = size;
    return task;
}
AT("0006E360") const u8 CreateSaveHeaderWriteTaskTail[2] = {0};

/** Write and verify the 44-byte save header asynchronously.  Two deliberately
 * separate wait states give the child SRAM task a frame to publish its result
 * before this parent reports completion. */
AT("0006E39C") void SaveHeaderWriteTask(struct EngineTask *task)
{
    u8 *work = (u8 *)task + 32;
    struct SaveBlock *save;
    u8 *iwram = gIwramBase;
    u32 rootOffset = (u32)gMapGenerationRootOffset;

    save = *(struct SaveBlock **)(iwram + rootOffset);

    switch (*(u16 *)((u8 *)task + 14)) {
    case 0:
        save->reserved28 = 0;
        save->headerCrc = 0;
        save->headerCrc = (u8)BufferXor((const u8 *)save, SAVE_HEADER_SIZE);
        *(u16 *)((u8 *)task + 14) = 1;
        break;
    case 1:
        *(u16 *)((u8 *)task + 14) = 2;
        break;
    case 2:
        CreateSaveWriteTask(save, SAVE_HEADER_SIZE, (s32 *)(work + 16));
        *(u16 *)((u8 *)task + 14) = 3;
        /* Fall through and observe the child completion slot immediately. */
    case 3: {
        s32 childResult = *(s32 *)(work + 16);
        if (childResult == 0)
            break;
        *(s32 *)(work + 12) = childResult;
        *(u16 *)((u8 *)task + 14) = 4;
        /* State four is a deliberate one-step transition to state five. */
    }
    case 4:
        *(u16 *)((u8 *)task + 14) = 5;
        break;
    case 5:
        if (task->completion)
            *task->completion = *(s32 *)(work + 12);
        FinishTask(task);
        break;
    }
}

/** Recompute a save's CRCs, write it to SRAM, then verify the write and
 * retry up to 15 times before reporting failure.
 * @return Nothing. */
AT("0006E650") void SaveWriteTask(struct EngineTask *task)
{
    u8 *taskBytes = (u8 *)task;
    u8 *work = taskBytes + 32;
    u16 state = *(u16 *)((u8 *)task + 14);
    u32 writingState = 0x100;

    switch (state) {
    case 0: {
        (*(struct SaveBlock **)(work + 4))->payloadCrc = state;
        (*(struct SaveBlock **)(work + 4))->payloadCrc =
            (u8)BufferXor((*(struct SaveBlock **)(work + 4))->payload,
                          SAVE_PAYLOAD_SIZE);
        (*(struct SaveBlock **)(work + 4))->reserved28 = state;
        (*(struct SaveBlock **)(work + 4))->headerCrc = state;
        (*(struct SaveBlock **)(work + 4))->headerCrc =
            (u8)BufferXor(*(const u8 **)(work + 4), 44);
        WriteSram(*(const u8 **)(work + 4), SRAM_BASE,
                  *(u32 *)(work + 12));
        *(u16 *)((u8 *)task + 14) = writingState;
        break;
    }
    case 0x100: {
        SramVerifyFunc *verify = &VerifySramFast;
        struct SaveBlock *save = *(struct SaveBlock **)(work + 4);
        u8 *mismatch = (*verify)((const u8 *)save, SRAM_BASE,
                                 *(u32 *)(work + 12));
        if (mismatch != 0) {
            s32 attempts = *(s32 *)(work + 20) + 1;
            *(u32 *)(work + 20) = attempts;
            if (attempts > 15) {
                u32 failedState = 0x2000;
                *(u16 *)((u8 *)task + 14) = failedState;
            }
        } else {
            u32 finishedState;
            *(u32 *)(work + 16) = 0;
            *(u32 *)(work + 20) = 0;
            GameStateSetField42BC(128);
            finishedState = 0x1000;
            *(u16 *)((u8 *)task + 14) = finishedState;
        }
        break;
    }
    case 0x1000:
        if (task->completion)
            *task->completion = 1;
        FinishTask(task);
        break;
    case 0x2000:
        if (task->completion)
            *task->completion = -1;
        FinishTask(task);
        break;
    }
}
