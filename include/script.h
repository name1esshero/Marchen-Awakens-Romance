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

/* Opcodes identified so far.
 *
 * OP_PUSH_STRING is followed by a 16-bit length and that many bytes of
 * Shift-JIS text, the terminator included. A valid length and terminator are necessary but not sufficient to prove
 * instruction boundaries. The current scanner finds 8,640 round-tripping
 * records across 334 SPCs; complete control-flow coverage is not yet proven.
 *
 * Native command index 0x22 resolves to the object-task wrapper 080322A4,
 * not the actual message printer at 08011790. This does not establish
 * that every 0x22 byte is a print opcode or follows every string. */
#define OP_PUSH_STRING  0x10
#define NATIVE_OBJECT_COMMAND_22 0x22

/* Native command table at 0x081ACCB0. A scan counted 444 pointer-shaped
 * words; that is not proof of the table boundary or bytecode opcode count. */
#define gScriptOpcodeHandlers ((void **)0x081ACCB0)
#define SCRIPT_SCANNED_POINTER_COUNT 444

#endif /* SCRIPT_H */
