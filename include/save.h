#ifndef SAVE_H
#define SAVE_H

#include "gba/types.h"
#include "task_manager.h"

#define SAVE_PAYLOAD_SIZE 0x3F00
#define SAVE_BLOCK_SIZE   0x3F2C

struct SaveBlock {
    char magic[16];
    u32 format;
    u32 generation;
    u32 recoveryCount; /* incremented when a bad payload is cleared */
    u32 reserved1C;
    u32 headerCrc;
    u32 payloadCrc;
    u32 reserved28;
    u8 payload[SAVE_PAYLOAD_SIZE];
};

s32 WriteSaveBlock(struct SaveBlock *save);
s32 LoadSaveBlock(struct SaveBlock *save);
s32 ValidateSaveBlock(struct SaveBlock *save);
s32 ReadSaveBytes(void *destination, u32 unused, u32 size, s32 *completion);
s32 ReadSaveBytesAndSignal(void *destination, u32 size, s32 *completion);
void *CreateSaveWriteTask(struct SaveBlock *save, u32 size, s32 *completion);
struct EngineTask *CreateSaveTask(u32 mode, struct SaveBlock *save, u32 size,
                                  s32 *completion);
struct EngineTask *CreateSaveBlockLoadTask(struct SaveBlock *save,
                                           u32 *completion);
void SaveBlockLoadTask(struct EngineTask *task);
struct EngineTask *CreateSaveBlockWriteTask(struct SaveBlock *save,
                                            u32 *completion);
void SaveWriteTask(struct EngineTask *task);
void SaveHeaderWriteTask(struct EngineTask *task);
void SaveBlockPrepareAndWriteTask(struct EngineTask *task);
struct EngineTask *CreateSaveHeaderWriteTask(struct SaveBlock *save, u32 size,
                                                 s32 *completion);

#endif
