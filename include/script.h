#ifndef SCRIPT_H
#define SCRIPT_H

#include "gba/types.h"

/* The game's script format.
 *
 * Most of the dialogue is not stored as loose strings in the ROM. It is
 * compiled into script files, stored as either raw or LZ77-compressed SPC archive members:
 * Decoded member framing:
 *
 *     "SCRP" u32 total_size
 *     "CODE" u32 chunk_size
 *     <chunk_size bytes of bytecode>
 *
 * 334 named SPC files exist (240 compressed, 94 uncompressed).
 * The interpreter also dispatches native commands through a function table.
 * Native command indices must not be confused with bytecode operand tags. */

#define SCRP_MAGIC 0x50524353      /* 'SCRP' little endian */
#define CODE_MAGIC 0x45444F43      /* 'CODE' little endian */

struct ScrpHeader
{
    u32 magic;          /* 'SCRP' */
    u32 size;
    u32 chunk_magic;    /* 'CODE' */
    u32 chunk_size;
};

/* Opcodes identified from the 256-entry dispatch table at 08F2A860.
 *
 * 0x10 is a relative jump followed by a u16 distance. The compiler uses it
 * to skip inline strings/data, then opcode 0x22 obtains the embedded address.
 * A valid length and terminator are necessary but not sufficient to prove
 * that an arbitrary 0x10 byte begins one of those compiler idioms. The current
 * scanner finds 8,640 round-tripping records across 334 SPCs.
 *
 * Native command index 0x22 resolves to the object-task wrapper 080322A4,
 * not the actual message printer at 08011790. This does not establish
 * that every 0x22 byte is a print opcode or follows every string. */
#define OP_JUMP_RELATIVE 0x10
#define OP_SET_BYTECODE_ADDRESS 0x22
#define OP_PUSH_OPERAND 0x28
#define OP_PUSH_IMMEDIATE 0x29
#define OP_POP_OPERAND 0x2A
#define OP_NATIVE_CALL 0x80
#define NATIVE_OBJECT_COMMAND_22 0x22

/* Every one of the 256 opcode slots is present. Unsupported slots point to
 * ScriptCmdFail. */
#define SCRIPT_OPCODE_COUNT 256
typedef s32 (*ScriptOpcodeHandler)(void);
extern const ScriptOpcodeHandler gScriptOpcodeHandlers[SCRIPT_OPCODE_COUNT];

/* The 128 native commands are name/handler pairs at 081AFEA4. The word at
 * 081AFEA0 belongs to the preceding table. Names point into the ASCII pool
 * near 08086D00. 081ACCB0 is an unrelated battle-action pointer table. */
struct ScriptNativeCommand
{
    const char *name;
    void *handler;
};
#define SCRIPT_NATIVE_COMMAND_COUNT 128
extern const struct ScriptNativeCommand
    gScriptNativeCommands[SCRIPT_NATIVE_COMMAND_COUNT];

#endif /* SCRIPT_H */
