#ifndef SCRIPT_BYTECODE_H
#define SCRIPT_BYTECODE_H

#include "gba/types.h"

struct ScriptBytecodeVm {
    u8 unknown00[0x30];
    u8 *bytecode;
    u8 unknown34[0x10];
    u32 programCounter;
    u8 unknown48[0x3C];
    u32 stackPointer;
};

struct ScriptBytecodeContext {
    u8 unknown00[12];
    struct ScriptBytecodeVm *vm;
};

struct ScriptBytecodeRoot {
    u8 unknown00[12];
    struct ScriptBytecodeContext *context;
};

#define gScriptBytecodeRoot (*(struct ScriptBytecodeRoot **)0x0300611C)
#define gScriptBytecodeVm (gScriptBytecodeRoot->context->vm)

u32 ScriptReadU8(u32 offset);
u32 ScriptReadU16(u32 offset);
u32 ScriptReadU32(u32 offset);
u32 ScriptReadNextU8(void);
u32 ScriptReadNextU16(void);
u32 ScriptReadNextU32(void);
void ScriptPushU32(u32 value);
u32 ScriptPopU32(void);
s32 *sub_0807F624(u32 operand);

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

#endif
