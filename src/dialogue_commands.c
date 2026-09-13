/* Native dialogue commands and completion tasks, recovered byte-for-byte.
 * Show starts the printer and sets display-state flags on windows 2 and 3.
 * Prompt creates the CURSOR/input task at 08011A08; its full state machine is
 * still assembly. Finish reports -1 through the task result pointer before
 * releasing the task. Return value 1 is the native command convention here. */
#include "dialogue.h"
extern void FinishTask(void *);
extern void *CreateTask(void *,void *,u32,s32 *,u32);
extern u8 gMainTaskManager;
extern void sub_08011A08(u32);
extern void sub_08006ADC(u32,u32);
#include "runtime_misc.h"
#define AT(x) __attribute__((section(".rom." x)))
AT("00011774")
void DialogueFinishTask(void *task)
{
 s32 *result = *(s32 **)((u8 *)task + 24);
 if (result) *result = -1;
 FinishTask(task);
}
AT("00011774") const u8 DialogueFinishTaskTail[2]={0,0};
AT("0001174C")
void *DialogueCreateFinishTask(s32 *result)
{
 void *task = CreateTask(&gMainTaskManager,DialogueFinishTask,0,result,4);
 if (!task) return 0;
 return task;
}
AT("00011CC4")
s32 DialogueCommandPrompt(void)
{
 sub_08011A08(0);
 return 1;
}
AT("00011CC4") const u8 DialogueCommandPromptTail[2]={0,0};
AT("00011CD4")
s32 DialogueCommandFinish(void)
{
 DialogueCreateFinishTask(0);
 return 1;
}
AT("00011CD4") const u8 DialogueCommandFinishTail[2]={0,0};
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
