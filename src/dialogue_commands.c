/* Native dialogue commands and completion tasks, recovered byte-for-byte.
 * Show starts the printer and sets display-state flags on windows 2 and 3.
 * Prompt creates the CURSOR/input task at 08011A08; its full state machine is
 * still assembly. Finish reports -1 through the task result pointer before
 * releasing the task. Return value 1 is the native command convention here. */
#include "dialogue.h"
#include "gba/io_reg.h"
#include "kmp.h"
#include "ncd.h"
#include "sprite_engine.h"
extern void FinishTask(void *);
extern void *CreateTask(void *,void *,u32,s32 *,u32);
extern u8 gMainTaskManager;
extern void sub_08011A60(void *task);
extern void sub_0807BC7C(struct NcdSprite *sprite, s32 container, s32 group,
                         s32 animation, s32 frame);
extern void ScriptAddPendingTasks(u32 count);
extern void sub_08006ADC(u32,u32);
#include "runtime_misc.h"
#include "rom_section.h"

extern const char gMessageWindowMapResourceName[];
extern const char gDialogueCursorResourceName[];

enum
{
    MESSAGE_WINDOW_KMP_SLOT = 3,
    MESSAGE_WINDOW_CHAR_BLOCK = 3
};

/**
 * Configure the ornate message-window background in KMP viewport slot 3.
 *
 * @param plane KMP plane rendered by the viewport.
 * @param loadGraphics Whether to load the MWA palette and tiles into VRAM.
 */
AT("00011718")
void DialogueLoadWindowGraphics(s32 plane, bool32 loadGraphics)
{
    KmpLoadResource(gMessageWindowMapResourceName,
                    BG_CHAR_ADDR(MESSAGE_WINDOW_CHAR_BLOCK),
                    MESSAGE_WINDOW_KMP_SLOT, plane, 0, 0,
                    loadGraphics ? KMP_LOAD_ALL : 0);
}

/**
 * Create the task that displays and updates the dialogue prompt cursor.
 *
 * @param result Optional task completion word.
 * @return The newly created task.
 */
AT("00011A08")
void *DialogueCreatePromptTask(s32 *result)
{
    u8 *task;
    struct NcdSprite *cursor;
    s32 group;

    task = CreateTask(&gMainTaskManager, sub_08011A60, 1, result, 56);
    cursor = (struct NcdSprite *)(task + 32);
    NcdInitSprite(cursor, 0);
    group = SpriteResourceFindGroup(0, gDialogueCursorResourceName);
    sub_0807BC7C(cursor, 0, group, 0, 0);
    ScriptAddPendingTasks(1);
    return task;
}
/** Report dialogue completion and release its task. */
AT("00011774")
void DialogueFinishTask(void *task)
{
 s32 *result = *(s32 **)((u8 *)task + 24);
 if (result) *result = -1;
 FinishTask(task);
}
AT("00011774") const u8 DialogueFinishTaskTail[2]={0,0};
/** Create the task that reports dialogue completion. */
AT("0001174C")
void *DialogueCreateFinishTask(s32 *result)
{
 void *task = CreateTask(&gMainTaskManager,DialogueFinishTask,0,result,4);
 if (!task) return 0;
 return task;
}
/** Start the dialogue prompt cursor and yield to the script VM. */
AT("00011CC4")
s32 DialogueCommandPrompt(void)
{
 DialogueCreatePromptTask(0);
 return 1;
}
AT("00011CC4") const u8 DialogueCommandPromptTail[2]={0,0};
/** Schedule dialogue completion and yield to the script VM. */
AT("00011CD4")
s32 DialogueCommandFinish(void)
{
 DialogueCreateFinishTask(0);
 return 1;
}
AT("00011CD4") const u8 DialogueCommandFinishTail[2]={0,0};
/** Start dialogue rows and enable their two display windows. */
AT("00011C68")
s32 DialogueCommandShow(s32 count, const char **rows)
{
 DialogueStart(0,count,rows,0);
 sub_08006ADC(2,1);
 sub_08006ADC(3,1);
 GameStateGetRecord1190(2)[1] |= 4;
 GameStateGetRecord1190(3)[1] |= 4;
 GameStateGetRecord1190(2)[0] |= 2;
 GameStateGetRecord1190(3)[0] |= 2;
 return 1;
}
