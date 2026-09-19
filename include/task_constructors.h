#ifndef TASK_CONSTRUCTORS_H
#define TASK_CONSTRUCTORS_H

#include "gba/types.h"

struct EngineTask;
union RuntimeTask69DB4State;

struct EngineTask *CreateMapCoordinateTask(s16 mode, s16 coordinate,
                                            s32 *result);
struct EngineTask *CreatePaletteInterpolationTask(
    s32 frameCount, const void *source, const void *target,
    u32 paletteIndex, u32 paletteBank, s32 *completion);
struct EngineTask *CreatePaletteSequenceTask(
    s32 frameCount, const u16 *firstColors, const u16 *secondColors,
    void *target, s32 colorCount, s32 *completion);
s32 CreateInputWaitTask(s32 inputSlot, s32 keyMask, s32 *result);
void ScheduleVramFillTask(void *destination, u32 size, u32 value);
u8 *CreateRuntimeTask69DB4(union RuntimeTask69DB4State *owner,
                           s32 *completion);
u8 *CreateNamedRuntimeRecordTask(const char *resourceName, s32 *completion);
u8 *CreateFieldEventModeTask(s16 mode, s32 *completion);
u8 *CreateSceneModeTask(s16 mode, s32 *result);
u8 *CreateBattleSpriteEffectTask(s32 *completion, u32 value54, s32 owner,
                                 u32 value5C, s32 x, s32 y, u32 valueA4);
u8 *CreateLargeBattleSpriteEffectTask(s32 *completion, u32 value54, s32 owner,
                                      u32 value5C, s32 x, s32 y);
u8 *CreateEncounterSetupTask(s32 value, s32 *result);
u8 *CreateEncounterResetTask(s32 *result);
u8 *CreateBattleNamedTaskA(s32 owner, s32 slot, s32 unused, s32 *result);
u8 *CreateBattleNamedTaskB(s32 owner, s32 slot, s32 unused, s32 *result);
u8 *CreateObjectMotionTaskA(u8 *object, s32 *result);
u8 *CreateObjectMotionTaskB(u8 *object, s32 *result);

#endif /* TASK_CONSTRUCTORS_H */
