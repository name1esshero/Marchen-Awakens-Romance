/* Script-native wrappers for the display-object command task.
 *
 * Both entry points forward the four VM arguments and select a small mode
 * value through the fifth ABI argument. The task constructor and state
 * machine remain in assembly while their request layout is decoded.
 */
#include "gba/types.h"

#include "runtime_state.h"
#include "rom_section.h"

extern void *CreateTask(void *state, void *callback, u32 priority,
                        s32 *result, u32 payloadSize);
extern void ObjectCommandTaskMain(void *task);

struct ObjectCommandRequest
{
    u8 unknown00[0xA0];
    u32 window;
    u32 line;
    u32 firstWindow;
    u8 unknownAC[0x26];
    s16 mode;
    u8 unknownD4[4];
    const s32 *arguments;
    s32 childResults[1];
};

/** Create a display-object command task for a window/line pair.
 * @param window Selects the task manager together with line.
 * @param line Sub-slot within the window's task managers.
 * @param arguments VM arguments passed through to the task.
 * @param result Set to -1 if creation fails.
 * @param mode Command mode (narrowed to a signed halfword).
 * @return The new task's request record, or NULL if creation fails. */
AT("000322CC")
void *StartObjectCommandTask(u32 window, u32 line, const s32 *arguments,
                             s32 *result, s32 mode)
{
    struct ObjectCommandRequest *request;
    s32 commandMode = (s16)mode;
    s32 i;

    request = CreateTask(gSecondaryRuntime + (window << 5) + (line << 4),
                         ObjectCommandTaskMain, 0, result, 192);
    if (request == 0)
    {
        if (result != 0)
            *result = -1;
        return 0;
    }

    request->window = window;
    request->line = line;
    request->firstWindow = window == 0;
    request->arguments = arguments;
    request->mode = commandMode;
    for (i = 0; i < 1; i++)
        request->childResults[i] = -1;
    return request;
}
AT("000322CC") const u8 StartObjectCommandTaskTail[2] = {0, 0};

/** StartObjectCommandTask() with mode fixed to 2. */
AT("000322A4")
void *ScriptCommand22(u32 window, u32 line, const s32 *arguments, s32 *result)
{
    return StartObjectCommandTask(window, line, arguments, result, 2);
}

/** StartObjectCommandTask() with mode fixed to 3. */
AT("000322B8")
void *ScriptCommandMode3(u32 window, u32 line, const s32 *arguments, s32 *result)
{
    return StartObjectCommandTask(window, line, arguments, result, 3);
}
