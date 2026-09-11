/* 08011790: allocate the message drawing task and copy caller-owned rows.
 * Mode 1 reserves row zero; callers must supply at most two rows. The
 * original strcpy has no length check. English callers validate before entry. */
#include "dialogue.h"
extern void *CreateTask(void *,void *,u32,s32 *,u32);
extern void sub_08011870(void *);
extern void sub_0807E420(u32);
extern char *strcpy(char *,const char *);
extern u32 strlen(const char *);
#ifdef ENGLISH
#define DialogueStart DialogueStartOriginal
#define DIALOGUE_SECTION ".english.original"
#else
#define DIALOGUE_SECTION ".rom.00011790"
#endif
__attribute__((section(DIALOGUE_SECTION)))
void *DialogueStart(s32 mode, s32 count, const char **rows, s32 *result)
{
 u8 *task = CreateTask((void *)0x030032C4, sub_08011870, 0, result, 508);
 struct DialogueState *state;
 s32 i;
 if (!task) return 0;
 state = (struct DialogueState *)(task + 32);
 *(s8 *)(task + 0x214) = mode;
 task[0x20C] = 15;
 task[0x20D] = 4;
 *(u16 *)(task + 0x210) = 2;
 switch (*(s8 *)(task + 0x214))
 {
 case 1:
  for (i = 0; i < count; i++) strcpy((char *)state->rows[i + 1], rows[i]);
  break;
 case 0:
  for (i = 0; i < count; i++) strcpy((char *)state->rows[i], rows[i]);
  break;
 }
 for (i = 0; i < count; i++)
  state->lengths[i] = strlen((char *)state->rows[i]);
 sub_0807E420(1);
 return task;
}
