#ifndef SCRIPT_BYTECODE_H
#define SCRIPT_BYTECODE_H

#include "gba/types.h"

struct ScriptBytecodeVm {
    u8 unknown00[0x10];
    u16 firstNamedResourceCount;
    u16 secondNamedResourceCount;
    u8 unknown14[0x1C];
    u8 *bytecode;
    u8 unknown34[4];
    void *table38;
    void *table3C;
    u8 unknown40[4];
    u32 programCounter;
    union {
        struct {
            s32 registers[15];
            u32 stackPointer;
        } live;
        s32 words[16];
    } frame;
    u32 callbacks[8];
    u16 frameFlags;
    u16 activeCallbackFlags;
};

struct ScriptResourceNode {
    struct ScriptResourceNode *next;
    char *typedName;
    u8 value[1];
};

struct ScriptResourceSlot {
    u32 referenceCount;
    void *allocation;
};

struct ScriptPointerRecord {
    void *value;
    void *alternate;
};

struct ScriptBytecodeContext {
    void *heap;
    void *resourceHeap;
    struct ScriptResourceNode **resourceBuckets;
    struct ScriptBytecodeVm *vm;
    u16 firstNamedResourceCount;
    u16 secondNamedResourceCount;
    struct ScriptResourceSlot firstResources[32];
    struct ScriptResourceSlot secondResources[32];
    u32 stepBudget;
    u32 dispatchState;
    s32 dispatchIndex;
    u32 pendingTasks;
    struct ScriptPointerRecord result;
    struct ScriptPointerRecord scriptName;
};

struct ScriptBytecodeRoot {
    u8 unknown00[12];
    struct ScriptBytecodeContext *context;
};

/* Name/value entries installed in the bytecode VM's resource namespace.
 * Function entries store a Thumb function pointer in value. */
struct ScriptResourceEntry {
    const char *name;
    u32 value;
};

extern const struct ScriptResourceEntry gScriptBuiltinFunctions[];
extern const struct ScriptResourceEntry gScriptEngineFunctions[];
extern const u8 gScriptResourceDefaultValue[12];

extern struct ScriptBytecodeRoot *gScriptBytecodeRoot;
#define gScriptBytecodeVm (gScriptBytecodeRoot->context->vm)

u32 ScriptReadU8(u32 offset);
u32 ScriptReadU16(u32 offset);
u32 ScriptReadU32(u32 offset);
u32 ScriptReadNextU8(void);
u32 ScriptReadNextU16(void);
u32 ScriptReadNextU32(void);
void ScriptPushU32(u32 value);
u32 ScriptPopU32(void);
s32 *ScriptResolveOperand(u32 operand);
s32 ScriptPushFrameAndJump(u32 callbackIndex, u32 destination);
s32 ScriptRestoreFrame(void);
s32 ScriptResourceHash(s32 type, const char *name);
u8 *ScriptResourceFind(s32 type, const char *name);
s32 ScriptResourceSet(s32 type, const char *name, const void *value, s32 size);
s32 ScriptResourceRemove(s32 type, const char *name);
u8 *ScriptResourceTableFind(struct ScriptResourceNode **buckets, s32 type,
                            const char *name);
s32 ScriptResourceTableSet(void *heap, struct ScriptResourceNode **buckets,
                           s32 type, const char *name, const void *value,
                           s32 size);
s32 ScriptResourceTableRemove(void *heap,
                              struct ScriptResourceNode **buckets,
                              s32 type, const char *name);
s32 ScriptResourceRegisterBuiltins(
    void *heap, struct ScriptResourceNode **buckets);
s32 ScriptExecutionStateInitialize(
    void *resourceHeap, struct ScriptResourceNode **resourceBuckets,
    void *heapMemory, u32 heapSize);

s32 ScriptCmdJump(void);
s32 ScriptCmdJumpIfZero(void);
s32 ScriptCmdCall(void);
s32 ScriptCmdReturn(void);
s32 ScriptCmdSwitch(void);
s32 ScriptCmdCopy(void);
s32 ScriptCmdSetImmediate(void);
s32 ScriptCmdSetBytecodeAddress(void);
s32 ScriptCmdPushOperand(void);
s32 ScriptCmdPushImmediate(void);
s32 ScriptCmdPopOperand(void);
s32 ScriptCmdAdd(void);
s32 ScriptCmdAddImmediate(void);
s32 ScriptCmdSubtract(void);
s32 ScriptCmdSubtractImmediate(void);
s32 ScriptCmdMultiply(void);
s32 ScriptCmdMultiplyImmediate(void);
s32 ScriptCmdIncrement(void);
s32 ScriptCmdDecrement(void);
s32 ScriptCmdNegate(void);
s32 ScriptCmdBitAnd(void);
s32 ScriptCmdBitAndImmediate(void);
s32 ScriptCmdBitOr(void);
s32 ScriptCmdBitOrImmediate(void);
s32 ScriptCmdBitXor(void);
s32 ScriptCmdDivide(void);
s32 ScriptCmdDivideImmediate(void);
s32 ScriptCmdModulo(void);
s32 ScriptCmdModuloImmediate(void);
s32 ScriptCmdBitXorImmediate(void);
s32 ScriptCmdBitNot(void);
s32 ScriptCmdLogicalAnd(void);
s32 ScriptCmdLogicalAndImmediate(void);
s32 ScriptCmdLogicalOr(void);
s32 ScriptCmdLogicalOrImmediate(void);
s32 ScriptCmdLogicalNot(void);
s32 ScriptCmdSignBit(void);
s32 ScriptCmdLessThanOrEqualZero(void);
s32 ScriptCmdGreaterThanZero(void);
s32 ScriptCmdGreaterThanOrEqualZero(void);
s32 ScriptCmdEqualZero(void);
s32 ScriptCmdNotEqualZero(void);
s32 ScriptCmdSetCallback(void);
s32 ScriptCmdRunCallback(void);
s32 ScriptCmdJumpRelative(void);
s32 ScriptCmdClearFrameFlag(void);
s32 ScriptCmdSetFrameFlag(void);
s32 ScriptCmdReadContextField14(void);
s32 ScriptCmdReadContextField114(void);
s32 ScriptCmdReadTable38Field(void);
s32 ScriptCmdReadTable3CField(void);
s32 ScriptCmdConcatStrings(void);
s32 ScriptCmdFreeString(void);
s32 ScriptCmdCallNative(void);
s32 ScriptCmdRestoreFrameAndGetResult(void);
s32 ScriptCmdFail(void);

#endif
