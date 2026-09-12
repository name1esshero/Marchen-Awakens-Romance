/* Primitive bytecode reads, VM stack operations, and core script commands. */
#include "script_bytecode.h"

#ifndef AT
#define AT(x) __attribute__((section(".rom." x)))
#endif

AT("0007F518") u32 ScriptReadU8(u32 offset)
{
    return gScriptBytecodeVm->bytecode[offset];
}

AT("0007F52C") u32 ScriptReadU16(u32 offset)
{
    u8 *p = gScriptBytecodeVm->bytecode + offset;
    return p[0] + (p[1] << 8);
}

AT("0007F548") u32 ScriptReadU32(u32 offset)
{
    u8 *p = gScriptBytecodeVm->bytecode + offset;
    return p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
}

AT("0007F570") u32 ScriptReadNextU8(void)
{
    u32 value = ScriptReadU8(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter++;
    return value;
}

AT("0007F598") u32 ScriptReadNextU16(void)
{
    u32 value = ScriptReadU16(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter += 2;
    return value;
}

AT("0007F5C0") u32 ScriptReadNextU32(void)
{
    u32 value = ScriptReadU32(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter += 4;
    return value;
}

AT("0007F5E8") void ScriptPushU32(u32 value)
{
    struct ScriptBytecodeVm *vm = gScriptBytecodeVm;
    vm->stackPointer -= 4;
    *(u32 *)(vm->bytecode + vm->stackPointer) = value;
}

AT("0007F608") u32 ScriptPopU32(void)
{
    struct ScriptBytecodeVm *vm = gScriptBytecodeVm;
    u32 value = *(u32 *)(vm->bytecode + vm->stackPointer);
    vm->stackPointer += 4;
    return value;
}

AT("0007F6E4") s32 ScriptCmdJump(void)
{
    gScriptBytecodeVm->programCounter = ScriptReadNextU32();
    return 1;
}

AT("0007F700") s32 ScriptCmdJumpIfZero(void)
{
    u32 operand = ScriptReadNextU8();
    u32 destination = ScriptReadNextU32();
    if (*sub_0807F624(operand) == 0)
        gScriptBytecodeVm->programCounter = destination;
    return 1;
}

AT("0007F730") s32 ScriptCmdCall(void)
{
    u32 destination = ScriptReadNextU32();
    ScriptPushU32(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter = destination;
    return 1;
}

AT("0007F75C") s32 ScriptCmdReturn(void)
{
    gScriptBytecodeVm->programCounter = ScriptPopU32();
    return 1;
}

AT("0007F778") s32 ScriptCmdSwitch(void)
{
    u32 operand = ScriptReadNextU8();
    s32 count = ScriptReadNextU8();
    s32 value = *sub_0807F624(operand);
    s32 i;
    for (i = 0; i < count; i++) {
        u32 candidate = ScriptReadNextU32();
        u32 destination = ScriptReadNextU32();
        if (candidate == value) {
            gScriptBytecodeVm->programCounter = destination;
            break;
        }
    }
    return 1;
}

AT("0007F7C0") s32 ScriptCmdCopy(void)
{
    u32 destination = ScriptReadNextU8();
    u32 source = ScriptReadNextU8();
    *sub_0807F624(destination) = *sub_0807F624(source);
    return 1;
}

AT("0007F7E8") s32 ScriptCmdSetImmediate(void)
{
    u32 destination = ScriptReadNextU8();
    u32 value = ScriptReadNextU32();
    *sub_0807F624(destination) = value;
    return 1;
}
AT("0007F7E8") const u8 ScriptCmdSetImmediateTail[2] = {0, 0};

AT("0007F808") s32 ScriptCmdSetBytecodeAddress(void)
{
    u32 destination = ScriptReadNextU8();
    u32 offset = ScriptReadNextU32();
    *sub_0807F624(destination) = (u32)(gScriptBytecodeVm->bytecode + offset);
    return 1;
}

AT("0007F838") s32 ScriptCmdPushOperand(void)
{
    u32 operand = ScriptReadNextU8();
    ScriptPushU32(*sub_0807F624(operand));
    return 1;
}
AT("0007F838") const u8 ScriptCmdPushOperandTail[2] = {0, 0};

AT("0007F850") s32 ScriptCmdPushImmediate(void)
{
    ScriptPushU32(ScriptReadNextU32());
    return 1;
}

AT("0007F860") s32 ScriptCmdPopOperand(void)
{
    u32 operand = ScriptReadNextU8();
    *sub_0807F624(operand) = ScriptPopU32();
    return 1;
}
AT("0007F860") const u8 ScriptCmdPopOperandTail[2] = {0, 0};

#define BINARY_COMMAND(address, name, operation) \
AT(address) s32 name(void) { \
    u32 left = ScriptReadNextU8(); \
    u32 right = ScriptReadNextU8(); \
    s32 *destination = sub_0807F624(left); \
    *destination operation *sub_0807F624(right); \
    return 1; \
}

#define IMMEDIATE_COMMAND(address, name, operation) \
AT(address) s32 name(void) { \
    u32 operand = ScriptReadNextU8(); \
    u32 immediate = ScriptReadNextU32(); \
    s32 *destination = sub_0807F624(operand); \
    *destination operation immediate; \
    return 1; \
}

BINARY_COMMAND("0007F87C", ScriptCmdAdd, +=)
IMMEDIATE_COMMAND("0007F8A8", ScriptCmdAddImmediate, +=)
AT("0007F8A8") const u8 ScriptCmdAddImmediateTail[2] = {0, 0};
BINARY_COMMAND("0007F8CC", ScriptCmdSubtract, -=)
IMMEDIATE_COMMAND("0007F8F8", ScriptCmdSubtractImmediate, -=)
AT("0007F8F8") const u8 ScriptCmdSubtractImmediateTail[2] = {0, 0};
BINARY_COMMAND("0007F91C", ScriptCmdMultiply, *=)
IMMEDIATE_COMMAND("0007F948", ScriptCmdMultiplyImmediate, *=)
AT("0007F948") const u8 ScriptCmdMultiplyImmediateTail[2] = {0, 0};

AT("0007FA44") s32 ScriptCmdIncrement(void) { u32 o=ScriptReadNextU8(); (*sub_0807F624(o))++; return 1; }
AT("0007FA44") const u8 ScriptCmdIncrementTail[2] = {0, 0};
AT("0007FA5C") s32 ScriptCmdDecrement(void) { u32 o=ScriptReadNextU8(); (*sub_0807F624(o))--; return 1; }
AT("0007FA5C") const u8 ScriptCmdDecrementTail[2] = {0, 0};
AT("0007FA74") s32 ScriptCmdNegate(void) { u32 o=ScriptReadNextU8(); s32 *v=sub_0807F624(o); *v = -*v; return 1; }
AT("0007FA74") const u8 ScriptCmdNegateTail[2] = {0, 0};

BINARY_COMMAND("0007FA8C", ScriptCmdBitAnd, &=)
IMMEDIATE_COMMAND("0007FAB8", ScriptCmdBitAndImmediate, &=)
AT("0007FAB8") const u8 ScriptCmdBitAndImmediateTail[2] = {0, 0};
BINARY_COMMAND("0007FADC", ScriptCmdBitOr, |=)
IMMEDIATE_COMMAND("0007FB08", ScriptCmdBitOrImmediate, |=)
AT("0007FB08") const u8 ScriptCmdBitOrImmediateTail[2] = {0, 0};
BINARY_COMMAND("0007FB2C", ScriptCmdBitXor, ^=)
