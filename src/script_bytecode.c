/* Primitive bytecode reads, VM stack operations, and core script commands. */
#include "script_bytecode.h"

#ifndef AT
#include "rom_section.h"
#endif

extern s32 __divsi3(s32 dividend, s32 divisor);
extern s32 __modsi3(s32 dividend, s32 divisor);
extern s32 ScriptResourceSelectValueSlot(void *record, s32 selector);
extern void HeapFree(void *heap, void *block);

/** @return A pointer to the VM register an encoded operand refers to.
 * Operand bit 7 selects the currently executing frame's live registers;
 * otherwise the operand indexes the root context's own frame directly. */
AT("0007F624") s32 *ScriptResolveOperand(u32 operand)
{
    s32 *result;
    if (operand & 0x80)
    {
        struct ScriptBytecodeVm *vm = gScriptBytecodeVm;
        result = (s32 *)vm->frame.live.registers[operand & -129];
    }
    else
    {
        struct ScriptBytecodeContext *context = gScriptBytecodeRoot->context;
        result = &context->vm->frame.live.registers[operand];
    }
    return result;
}

/** @return The byte at offset in the active VM's bytecode. */
AT("0007F518") u32 ScriptReadU8(u32 offset)
{
    return gScriptBytecodeVm->bytecode[offset];
}

/** @return The little-endian halfword at offset in the active VM's
 * bytecode. */
AT("0007F52C") u32 ScriptReadU16(u32 offset)
{
    u8 *p = gScriptBytecodeVm->bytecode + offset;
    return p[0] + (p[1] << 8);
}

/** @return The little-endian word at offset in the active VM's bytecode. */
AT("0007F548") u32 ScriptReadU32(u32 offset)
{
    u8 *p = gScriptBytecodeVm->bytecode + offset;
    return p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
}

/** Read a byte at the program counter and advance it by 1.
 * @return The byte read. */
AT("0007F570") u32 ScriptReadNextU8(void)
{
    u32 value = ScriptReadU8(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter++;
    return value;
}

/** Read a halfword at the program counter and advance it by 2.
 * @return The halfword read. */
AT("0007F598") u32 ScriptReadNextU16(void)
{
    u32 value = ScriptReadU16(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter += 2;
    return value;
}

/** Read a word at the program counter and advance it by 4.
 * @return The word read. */
AT("0007F5C0") u32 ScriptReadNextU32(void)
{
    u32 value = ScriptReadU32(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter += 4;
    return value;
}

/** Push a word onto the VM stack, which lives within the bytecode buffer
 * and grows downward. */
AT("0007F5E8") void ScriptPushU32(u32 value)
{
    struct ScriptBytecodeVm *vm = gScriptBytecodeVm;
    vm->frame.live.stackPointer -= 4;
    *(u32 *)(vm->bytecode + vm->frame.live.stackPointer) = value;
}

/** Pop a word from the VM stack. @return The popped value. */
AT("0007F608") u32 ScriptPopU32(void)
{
    struct ScriptBytecodeVm *vm = gScriptBytecodeVm;
    u32 value = *(u32 *)(vm->bytecode + vm->frame.live.stackPointer);
    vm->frame.live.stackPointer += 4;
    return value;
}

/** Opcode JUMP: unconditional jump to a bytecode-encoded address.
 * @return Always 1. */
AT("0007F6E4") s32 ScriptCmdJump(void)
{
    gScriptBytecodeVm->programCounter = ScriptReadNextU32();
    return 1;
}

/** Opcode JZ: jump to a bytecode-encoded address if an operand is zero.
 * @return Always 1. */
AT("0007F700") s32 ScriptCmdJumpIfZero(void)
{
    u32 operand = ScriptReadNextU8();
    u32 destination = ScriptReadNextU32();
    if (*ScriptResolveOperand(operand) == 0)
        gScriptBytecodeVm->programCounter = destination;
    return 1;
}

/** Opcode CALL: push the return address and jump to a bytecode-encoded
 * address. @return Always 1. */
AT("0007F730") s32 ScriptCmdCall(void)
{
    u32 destination = ScriptReadNextU32();
    ScriptPushU32(gScriptBytecodeVm->programCounter);
    gScriptBytecodeVm->programCounter = destination;
    return 1;
}

/** Opcode RET: pop a return address pushed by ScriptCmdCall() and jump to
 * it. @return Always 1. */
AT("0007F75C") s32 ScriptCmdReturn(void)
{
    gScriptBytecodeVm->programCounter = ScriptPopU32();
    return 1;
}

/** Opcode SWITCH: jump to the destination whose bytecode-encoded case value
 * matches an operand, out of a bytecode-encoded case table; falls through
 * if none match. @return Always 1. */
AT("0007F778") s32 ScriptCmdSwitch(void)
{
    u32 operand = ScriptReadNextU8();
    s32 count = ScriptReadNextU8();
    s32 value = *ScriptResolveOperand(operand);
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

/** Opcode COPY: copy one operand's value into another.
 * @return Always 1. */
AT("0007F7C0") s32 ScriptCmdCopy(void)
{
    u32 destination = ScriptReadNextU8();
    u32 source = ScriptReadNextU8();
    *ScriptResolveOperand(destination) = *ScriptResolveOperand(source);
    return 1;
}

/** Opcode SETI: store a bytecode-encoded immediate into an operand.
 * @return Always 1. */
AT("0007F7E8") s32 ScriptCmdSetImmediate(void)
{
    u32 destination = ScriptReadNextU8();
    u32 value = ScriptReadNextU32();
    *ScriptResolveOperand(destination) = value;
    return 1;
}
AT("0007F7E8") const u8 ScriptCmdSetImmediateTail[2] = {0, 0};

/** Opcode SETADDR: store a pointer into the active VM's own bytecode buffer,
 * at a bytecode-encoded offset, into an operand.
 * @return Always 1. */
AT("0007F808") s32 ScriptCmdSetBytecodeAddress(void)
{
    u32 destination = ScriptReadNextU8();
    u32 offset = ScriptReadNextU32();
    *ScriptResolveOperand(destination) = (u32)(gScriptBytecodeVm->bytecode + offset);
    return 1;
}

/** Opcode PUSH: push an operand's value onto the VM stack.
 * @return Always 1. */
AT("0007F838") s32 ScriptCmdPushOperand(void)
{
    u32 operand = ScriptReadNextU8();
    ScriptPushU32(*ScriptResolveOperand(operand));
    return 1;
}
AT("0007F838") const u8 ScriptCmdPushOperandTail[2] = {0, 0};

/** Opcode PUSHI: push a bytecode-encoded immediate onto the VM stack.
 * @return Always 1. */
AT("0007F850") s32 ScriptCmdPushImmediate(void)
{
    ScriptPushU32(ScriptReadNextU32());
    return 1;
}

/** Opcode POP: pop the VM stack into an operand.
 * @return Always 1. */
AT("0007F860") s32 ScriptCmdPopOperand(void)
{
    u32 operand = ScriptReadNextU8();
    *ScriptResolveOperand(operand) = ScriptPopU32();
    return 1;
}
AT("0007F860") const u8 ScriptCmdPopOperandTail[2] = {0, 0};

#define BINARY_COMMAND(address, name, operation) \
AT(address) s32 name(void) { \
    u32 left = ScriptReadNextU8(); \
    u32 right = ScriptReadNextU8(); \
    s32 *destination = ScriptResolveOperand(left); \
    *destination operation *ScriptResolveOperand(right); \
    return 1; \
}

#define IMMEDIATE_COMMAND(address, name, operation) \
AT(address) s32 name(void) { \
    u32 operand = ScriptReadNextU8(); \
    u32 immediate = ScriptReadNextU32(); \
    s32 *destination = ScriptResolveOperand(operand); \
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

/** Opcode INC: increment an operand in place. @return Always 1. */
AT("0007FA44") s32 ScriptCmdIncrement(void) { u32 o=ScriptReadNextU8(); (*ScriptResolveOperand(o))++; return 1; }
AT("0007FA44") const u8 ScriptCmdIncrementTail[2] = {0, 0};
/** Opcode DEC: decrement an operand in place. @return Always 1. */
AT("0007FA5C") s32 ScriptCmdDecrement(void) { u32 o=ScriptReadNextU8(); (*ScriptResolveOperand(o))--; return 1; }
AT("0007FA5C") const u8 ScriptCmdDecrementTail[2] = {0, 0};
/** Opcode NEG: negate an operand in place. @return Always 1. */
AT("0007FA74") s32 ScriptCmdNegate(void) { u32 o=ScriptReadNextU8(); s32 *v=ScriptResolveOperand(o); *v = -*v; return 1; }
AT("0007FA74") const u8 ScriptCmdNegateTail[2] = {0, 0};

BINARY_COMMAND("0007FA8C", ScriptCmdBitAnd, &=)
IMMEDIATE_COMMAND("0007FAB8", ScriptCmdBitAndImmediate, &=)
AT("0007FAB8") const u8 ScriptCmdBitAndImmediateTail[2] = {0, 0};
BINARY_COMMAND("0007FADC", ScriptCmdBitOr, |=)
IMMEDIATE_COMMAND("0007FB08", ScriptCmdBitOrImmediate, |=)
AT("0007FB08") const u8 ScriptCmdBitOrImmediateTail[2] = {0, 0};
BINARY_COMMAND("0007FB2C", ScriptCmdBitXor, ^=)

/** Opcode DIV: signed in-place division of one operand by another.
 * @return 1 on success, -1 on division by zero (the destination is left
 * unchanged). */
AT("0007F96C") s32 ScriptCmdDivide(void)
{
    u32 left=ScriptReadNextU8(), right=ScriptReadNextU8();
    s32 divisor=*ScriptResolveOperand(right);
    s32 *destination;
    if (divisor == 0) return -1;
    destination=ScriptResolveOperand(left);
    *destination=__divsi3(*destination,divisor);
    return 1;
}
AT("0007F96C") const u8 ScriptCmdDivideTail[2] = {0,0};

/** Opcode DIVI: signed in-place division of an operand by a bytecode-
 * encoded immediate.
 * @return 1 on success, -1 on division by zero (the destination is left
 * unchanged). */
AT("0007F9A4") s32 ScriptCmdDivideImmediate(void)
{
    u32 operand=ScriptReadNextU8();
    s32 divisor=ScriptReadNextU32();
    s32 *destination;
    if (divisor == 0) return -1;
    destination=ScriptResolveOperand(operand);
    *destination=__divsi3(*destination,divisor);
    return 1;
}
AT("0007F9A4") const u8 ScriptCmdDivideImmediateTail[2] = {0,0};

/** Opcode MOD: signed in-place modulo of one operand by another.
 * @return 1 on success, -1 on division by zero (the destination is left
 * unchanged). */
AT("0007F9D8") s32 ScriptCmdModulo(void)
{
    u32 left=ScriptReadNextU8(), right=ScriptReadNextU8();
    s32 divisor=*ScriptResolveOperand(right);
    s32 *destination;
    if (divisor == 0) return -1;
    destination=ScriptResolveOperand(left);
    *destination=__modsi3(*destination,divisor);
    return 1;
}
AT("0007F9D8") const u8 ScriptCmdModuloTail[2] = {0,0};

/** Opcode MODI: signed in-place modulo of an operand by a bytecode-encoded
 * immediate.
 * @return 1 on success, -1 on division by zero (the destination is left
 * unchanged). */
AT("0007FA10") s32 ScriptCmdModuloImmediate(void)
{
    u32 operand=ScriptReadNextU8();
    s32 divisor=ScriptReadNextU32();
    s32 *destination;
    if (divisor == 0) return -1;
    destination=ScriptResolveOperand(operand);
    *destination=__modsi3(*destination,divisor);
    return 1;
}
AT("0007FA10") const u8 ScriptCmdModuloImmediateTail[2] = {0,0};

IMMEDIATE_COMMAND("0007FB58", ScriptCmdBitXorImmediate, ^=)
AT("0007FB58") const u8 ScriptCmdBitXorImmediateTail[2] = {0,0};

/** Opcode NOT: bitwise-complement an operand in place. @return Always 1. */
AT("0007FB7C") s32 ScriptCmdBitNot(void) { u32 o=ScriptReadNextU8(); s32 *v=ScriptResolveOperand(o); *v=~*v; return 1; }
AT("0007FB7C") const u8 ScriptCmdBitNotTail[2] = {0,0};

/** Opcode LAND: logical AND of two operands, written back to the first.
 * @return Always 1. */
AT("0007FB94") s32 ScriptCmdLogicalAnd(void)
{
    u32 left=ScriptReadNextU8(), right=ScriptReadNextU8();
    s32 *destination=ScriptResolveOperand(left);
    s32 *source=ScriptResolveOperand(right);
    s32 result=0;
    if (*destination != 0) {
        s32 sourceValue=*source;
        result=((u32)(-sourceValue | sourceValue)) >> 31;
    }
    *destination=result;
    return 1;
}

/** Opcode LANDI: logical AND of an operand and a bytecode-encoded
 * immediate, written back to the operand. @return Always 1. */
AT("0007FBCC") s32 ScriptCmdLogicalAndImmediate(void)
{
    u32 operand=ScriptReadNextU8();
    s32 immediate=ScriptReadNextU32();
    s32 *destination=ScriptResolveOperand(operand);
    s32 result=0;
    if (*destination != 0)
        result=((u32)(-immediate | immediate)) >> 31;
    *destination=result;
    return 1;
}
AT("0007FBCC") const u8 ScriptCmdLogicalAndImmediateTail[2]={0,0};

/** Opcode LOR: logical OR of two operands, written back to the first.
 * @return Always 1. */
AT("0007FBFC") s32 ScriptCmdLogicalOr(void)
{
    u32 left=ScriptReadNextU8(), right=ScriptReadNextU8();
    s32 *destination=ScriptResolveOperand(left);
    s32 *source=ScriptResolveOperand(right);
    s32 result=0;
    if (*destination != 0 || *source != 0)
        result=1;
    *destination=result;
    return 1;
}

/** Opcode LORI: logical OR of an operand and a bytecode-encoded immediate,
 * written back to the operand. @return Always 1. */
AT("0007FC34") s32 ScriptCmdLogicalOrImmediate(void)
{
    u32 operand=ScriptReadNextU8();
    s32 immediate=ScriptReadNextU32();
    s32 *destination=ScriptResolveOperand(operand);
    s32 result=0;
    if (*destination != 0 || immediate != 0)
        result=1;
    *destination=result;
    return 1;
}
AT("0007FC34") const u8 ScriptCmdLogicalOrImmediateTail[2]={0,0};

/** Opcode LNOT: logical NOT of an operand in place. @return Always 1. */
AT("0007FC64") s32 ScriptCmdLogicalNot(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = !*value; return 1;
}
AT("0007FC64") const u8 ScriptCmdLogicalNotTail[2]={0,0};

/** Opcode SIGN: replace an operand with its sign bit (0 or 1).
 * @return Always 1. */
AT("0007FC84") s32 ScriptCmdSignBit(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = (u32)*value >> 31; return 1;
}
AT("0007FC84") const u8 ScriptCmdSignBitTail[2]={0,0};

/** Opcode LEZ: replace an operand with whether it is <= 0.
 * @return Always 1. */
AT("0007FC9C") s32 ScriptCmdLessThanOrEqualZero(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = *value <= 0; return 1;
}
AT("0007FC9C") const u8 ScriptCmdLessThanOrEqualZeroTail[2]={0,0};

/** Opcode GTZ: replace an operand with whether it is > 0.
 * @return Always 1. */
AT("0007FCBC") s32 ScriptCmdGreaterThanZero(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = *value > 0; return 1;
}
AT("0007FCBC") const u8 ScriptCmdGreaterThanZeroTail[2]={0,0};

/** Opcode GEZ: replace an operand with whether it is >= 0.
 * @return Always 1. */
AT("0007FCDC") s32 ScriptCmdGreaterThanOrEqualZero(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = *value >= 0; return 1;
}

/** Opcode EQZ: replace an operand with whether it equals 0.
 * @return Always 1. */
AT("0007FCF4") s32 ScriptCmdEqualZero(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = *value == 0; return 1;
}
AT("0007FCF4") const u8 ScriptCmdEqualZeroTail[2]={0,0};

/** Opcode NEZ: replace an operand with whether it is nonzero.
 * @return Always 1. */
AT("0007FD14") s32 ScriptCmdNotEqualZero(void)
{
    u32 operand=ScriptReadNextU8(); s32 *value=ScriptResolveOperand(operand);
    *value = *value != 0; return 1;
}
AT("0007FD14") const u8 ScriptCmdNotEqualZeroTail[2]={0,0};

/** Opcode SETCB: install a bytecode address into one of the VM's callback
 * slots. See ScriptCmdRunCallback(). @return Always 1. */
AT("0007FD30") s32 ScriptCmdSetCallback(void)
{
    u32 index=ScriptReadNextU8();
    gScriptBytecodeVm->callbacks[index]=ScriptReadNextU32();
    return 1;
}

/** Opcode RUNCB: push a nested frame and jump to a callback installed by
 * ScriptCmdSetCallback(); a no-op if that slot is empty.
 * @return Always 1. */
AT("0007FD58") s32 ScriptCmdRunCallback(void)
{
    u32 index=ScriptReadNextU8();
    u32 callback=gScriptBytecodeVm->callbacks[index];
    if (callback == 0)
        return 1;
    return ScriptPushFrameAndJump(index,callback);
}
AT("0007FD58") const u8 ScriptCmdRunCallbackTail[2]={0,0};

/** Opcode JMPR: jump forward by a bytecode-encoded halfword offset from the
 * current program counter. @return Always 1. */
AT("0007F6C4") s32 ScriptCmdJumpRelative(void)
{
    gScriptBytecodeVm->programCounter += ScriptReadNextU16();
    return 1;
}

/** Save all 16 VM registers (register 15 is the stack cursor), then the frame
 * flags and return cursor. Callback execution is therefore nestable. */
AT("0007F664") s32 ScriptPushFrameAndJump(u32 callbackIndex, u32 destination)
{
    s32 i;
    for (i=0; i<=15; i++)
        ScriptPushU32(gScriptBytecodeVm->frame.words[i]);
    ScriptPushU32(gScriptBytecodeVm->frameFlags);
    ScriptPushU32(gScriptBytecodeVm->programCounter);
    {
        struct ScriptBytecodeRoot *root = gScriptBytecodeRoot;
        root->context->vm->frameFlags |= 1;
        root->context->vm->programCounter=destination;
    }
    return 1;
}

/** Pop a frame pushed by ScriptPushFrameAndJump(): all 16 VM registers,
 * frame flags, and the return program counter, in reverse push order.
 * @return Always 1. */
AT("0007FD88") s32 ScriptRestoreFrame(void)
{
    s32 i;
    u32 cursor=ScriptPopU32();
    gScriptBytecodeVm->frameFlags=ScriptPopU32();
    for (i=15; i>=0; i--)
        gScriptBytecodeVm->frame.words[i]=ScriptPopU32();
    gScriptBytecodeVm->programCounter=cursor;
    return 1;
}

/** Opcode CLRFLAG: clear the VM's frame-flag bit 0.
 * @return Always 1. */
AT("0007FDD4") s32 ScriptCmdClearFrameFlag(void)
{
    gScriptBytecodeVm->frameFlags &= 0xFFFE;
    return 1;
}

/** Opcode SETFLAG: set the VM's frame-flag bit 0.
 * @return Always 1. */
AT("0007FDF4") s32 ScriptCmdSetFrameFlag(void)
{
    gScriptBytecodeVm->frameFlags |= 1;
    return 1;
}

/** Opcode: read one of the VM context's first-class resource value slots
 * (an 8-byte stride table based at context +20), selected by a bytecode-
 * encoded index. @return Always 1. */
AT("0007FE10") s32 ScriptCmdReadContextField14(void)
{
    u32 index=ScriptReadNextU16();
    s32 *destination=ScriptResolveOperand(ScriptReadNextU8());
    struct ScriptBytecodeRoot *root = gScriptBytecodeRoot;
    u32 offset = index * 8;
    offset += 20;
    *destination=ScriptResourceSelectValueSlot((u8 *)root->context + offset,
                              *destination);
    return 1;
}
/** Opcode: read one of the VM context's second-class resource value slots
 * (an 8-byte stride table based at context +276), selected by a bytecode-
 * encoded index. @return Always 1. */
AT("0007FE44") s32 ScriptCmdReadContextField114(void)
{
    u32 index=ScriptReadNextU16();
    s32 *destination=ScriptResolveOperand(ScriptReadNextU8());
    struct ScriptBytecodeRoot *root = gScriptBytecodeRoot;
    u32 offset = index * 8;
    offset += 276;
    *destination=ScriptResourceSelectValueSlot((u8 *)root->context + offset,
                              *destination);
    return 1;
}
/** Opcode: read one of the active VM's table38 resource value slots (an
 * 8-byte stride table), selected by a bytecode-encoded index.
 * @return Always 1. */
AT("0007FE7C") s32 ScriptCmdReadTable38Field(void)
{
    u32 index=ScriptReadNextU8();
    s32 *destination=ScriptResolveOperand(ScriptReadNextU8());
    *destination=ScriptResourceSelectValueSlot((u8 *)gScriptBytecodeVm->table38 + index*8,
                              *destination);
    return 1;
}

/** Opcode: read one of the active VM's table3C resource value slots (an
 * 8-byte stride table), selected by a bytecode-encoded index.
 * @return Always 1. */
AT("0007FEB0") s32 ScriptCmdReadTable3CField(void)
{
    u32 index=ScriptReadNextU8();
    s32 *destination=ScriptResolveOperand(ScriptReadNextU8());
    *destination=ScriptResourceSelectValueSlot((u8 *)gScriptBytecodeVm->table3C + index*8,
                              *destination);
    return 1;
}

/** Opcode: free an operand's heap-allocated string, if any, and clear it
 * to 0. @return Always 1. */
AT("0007FFA0") s32 ScriptCmdFreeString(void)
{
    s32 *value=ScriptResolveOperand(ScriptReadNextU8());
    if (*value != 0) {
        HeapFree(gScriptBytecodeRoot->context->heap,(void *)*value);
        *value=0;
    }
    return 1;
}
extern void ScriptPopFrame(void);
/** Opcode: pop the current script frame, then return the VM context's
 * result word (context +532) as the VM's own dispatch status. */
AT("0008004C") s32 ScriptCmdRestoreFrameAndGetResult(void)
{
    ScriptPopFrame();
    return *(s32 *)((u8 *)gScriptBytecodeRoot->context + 532);
}

/** Used by unimplemented/invalid opcode slots to stop script execution. */
AT("00080068") s32 ScriptCmdFail(void)
{
    return -1;
}
AT("00080068") const u8 ScriptCmdFailTail[2]={0,0};
