/* Native command 0x22 and its object-task constructor.
 * Earlier notes incorrectly called this the text printer. Its task body
 * manages display objects; the real three-line message printer starts at
 * 08011790 and advances glyphs through 08011AF4. The precise gameplay role
 * of this object command still needs decoding. */

#include "gba/types.h"
#include "script.h"

/* The object table the object-command system allocates its state from. */
#define gObjectHeap (*(u8 **)0x03004020)

extern void *CreateTask(void *state, void *func, u32 unused, s32 *result, u32 stackSize);
extern void ObjectCommandTaskMain(void *task);           /* sub_0803235C */

/* Distinguishes the two entry points that share StartObjectCommandTask. Both create object tasks;
 * the value is stored in the task and selects how the text is presented. */
#define OBJECT_COMMAND_MODE_2   2     /* from opcode 0x22 */
#define OBJECT_COMMAND_MODE_3 3     /* from the second entry point at 0x080322B8 */

struct ObjectCommandRequest
{
    u8 filler_00[0xA0];
    u32 window;             /* 0xA0 */
    u32 line;               /* 0xA4 */
    u32 is_first_window;    /* 0xA8: set when window index is zero */
    u8 filler_AC[0x26];
    s16 mode;             /* 0xD2 */
    u8 filler_D4[4];
    const s32 *arguments;   /* 0xD8: native command arguments, not a string */
    s32 childResult;        /* 0xDC: one result slot, initialized to -1 */
};

/* Set up a native object command task and retain its argument block.
 *
 * Returns NULL if no task could be allocated; the caller's result word is set
 * to -1 in that case so the script can tell the command could not start.
 */
void *StartObjectCommandTask(u32 window, u32 line, const s32 *arguments, s32 *result,
                    s32 unk)
{
    struct ObjectCommandRequest *req;

    req = CreateTask(gObjectHeap + (window << 5) + (line << 4),
                     ObjectCommandTaskMain, 0, result, 192);
    if (req == NULL)
    {
        if (result != NULL)
            *result = -1;
        return NULL;
    }

    req->window = window;
    req->line = line;
    req->is_first_window = (window == 0);
    req->arguments = arguments;
    req->mode = (s16)unk;

    /* The loop in the assembly executes once, at +DC. */
    req->childResult = -1;

    return req;
}

/* gScriptOpcodeHandlers[0x22]. Native object-command entry; the VM passes an argument block. */
void ScriptCommand22(u32 window, u32 line, const s32 *arguments, s32 *result)
{
    StartObjectCommandTask(window, line, arguments, result, OBJECT_COMMAND_MODE_2);
}

/* A second entry point at 0x080322B8, identical but for the mode it selects. */
void ScriptCommand223(u32 window, u32 line, const s32 *arguments, s32 *result)
{
    StartObjectCommandTask(window, line, arguments, result, OBJECT_COMMAND_MODE_3);
}
