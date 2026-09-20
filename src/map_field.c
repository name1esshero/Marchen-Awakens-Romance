/* FldSet handler 080122F8 passes the field basename and signed pixel
 * coordinates here. 08006A1C stores that basename at engine state +0x12F4.
 * The first load enables palette+tile transfer (flags 3); the second reuses
 * those resources (flags 0). Both planes share one KCG/KCL resource pair;
 * this is not a primary/secondary tileset load. Both viewport tile/palette
 * offsets are zero here. Viewport slots have a 0xFC-byte stride.
 * The original 16-byte filename buffer and unchecked copies are preserved.
 */
#include "kmp.h"
#include "gba/io_reg.h"
#include "game_state.h"
#include "map_events.h"
#include "rom_section.h"
#include "runtime_misc.h"
extern const char gMapArchiveKmpExtension[];
extern char *strcpy(char *,const char *);
extern char *strcat(char *,const char *);
extern char *strupr(char *);
extern void sub_08054350(void *, void *, s32, s32, s32, s32, s32);
#define sText_KmpExtension gMapArchiveKmpExtension

extern void sub_08018590(void *actorState);
extern void sub_08017E70(void *actorState, s32 first, s32 second);
extern void sub_08017BD8(void *actorState, void *actionState);

#define FIELD_ACTOR_ACTION_LOCK_OFFSET 0x27A
#define FIELD_ACTION_ACTIVE_OFFSET 10

/**
 * Run the field actor's state update when neither side blocks the action.
 *
 * @param actorState Field actor runtime state.
 * @param actionState State whose signed active byte gates the update.
 * @return TRUE when the update ran, otherwise FALSE.
 */
AT("00017BA8") s32 FieldActorTryRunStateUpdate(void *actorState,
                                               void *actionState)
{
    u8 *actor = actorState;
    u8 *action = actionState;
    s32 result = FALSE;

    if (*(s8 *)(actor + FIELD_ACTOR_ACTION_LOCK_OFFSET) == 0
     && *(s8 *)(action + FIELD_ACTION_ACTIVE_OFFSET) != 0)
    {
        sub_08017BD8(actorState, actionState);
        result = TRUE;
    }

    return result;
}

/** Update a field actor using the handler selected by game-state mode 1. */
AT("00017E4C") void FieldActorUpdateForMode(void *actorState)
{
    if (GameStateGetField4254() == 1)
        sub_08018590(actorState);
    else
        sub_08017E70(actorState, 3, 4);
}

/** Kmp load field for the active map viewport. */
AT("000032B8") void KmpLoadField(const char *name,s16 x,s16 y)
{
 char resource[16]; /* basename + .KMP + terminator; original has no length check */
 struct KmpViewport *view;
 s32 px=x,py=y;
 GameStateSetString12F4(name);
 strcpy(resource,name);
 strcat(resource,sText_KmpExtension);
 strupr(resource);
 KmpLoadResource(resource,BG_CHAR_ADDR(0),0,0,0,0,3);
 KmpLoadResource(resource,BG_CHAR_ADDR(0),1,1,0,0,0);
 view=gKmpViewports;
 px*=KMP_FIXED_ONE;py*=KMP_FIXED_ONE;
 KmpRenderViewport(view,px,py);
 view++;
 KmpRenderViewport(view,px,py);
}
/** Kmp set clip for the active map viewport. */
AT("00003280") void KmpSetClip(struct KmpViewport *view,u32 x,u32 y,u32 width,u32 height)
{view->clipX=x;view->clipY=y;view->clipWidth=width;view->clipHeight=height;}
AT("00003280") const u8 KmpSetClipTail[2]={0,0};
/** Kmp reset clip for the active map viewport. */
AT("00003294") void KmpResetClip(struct KmpViewport *view)
{view->clipX=0;view->clipY=0;view->clipWidth=view->data->widthTiles;view->clipHeight=view->data->heightTiles;}
AT("00003294") const u8 KmpResetClipTail[2]={0,0};

/** Prepare the two field-display substructures used by the map scene. */
AT("00061E70") void InitializeMapFieldDisplay(void *state)
{
    s32 resource = GameStateGetResourceCounter();
    sub_08054350((u8 *)state + 0xB94, (u8 *)state + 0x1BC4,
                 resource, 6, 194, 8, 0);
}

/** Same field-display setup as InitializeMapFieldDisplay, applied to the
 * smaller substructures embedded in a field-event task (see
 * CreateFieldEventTask below) rather than the
 * main engine state. */
AT("00065E50") void InitializeFieldEventDisplay(void *task)
{
    s32 resource = GameStateGetResourceCounter();
    sub_08054350((u8 *)task + 0x104, (u8 *)task + 0x404,
                 resource, 6, 194, 8, 0);
}

/** Third field-display setup, for the substructures at +0x5A4/+0x7AC of the
 * task that 08054C74 creates. */
AT("000558A4") void InitializeFieldDisplay5A4(void *task)
{
    s32 resource = GameStateGetResourceCounter();
    sub_08054350((u8 *)task + 0x5A4, (u8 *)task + 0x7AC,
                 resource, 6, 194, 8, 0);
}

extern void sub_08061F20(struct EngineTask *task);

/** Create a field-event task with four signed coordinates and an object binding.
 * The original assumes task allocation succeeds. The extra signed value's
 * role remains unknown; it is stored at task offset 0xB2. */
AT("00061EA8")
struct EngineTask *CreateFieldEventTask(s16 first, s16 second, s16 third,
    s16 fourth, s16 value, void *objectData, u32 *completion)
{
    struct EngineTask *task;
    struct FieldEventTaskData *data;
    s32 firstCoordinate, secondCoordinate, thirdCoordinate, fourthCoordinate;
    s32 eventValue;

    firstCoordinate = first;
    secondCoordinate = second;
    thirdCoordinate = third;
    fourthCoordinate = fourth;
    eventValue = value;
    task = CreateTask(&gMainTaskManager, sub_08061F20, 0, completion,
        FIELD_EVENT_TASK_DATA_SIZE);
    data = (struct FieldEventTaskData *)((u8 *)task + ENGINE_TASK_HEADER_SIZE);
    data->firstCoordinate = firstCoordinate;
    data->fourthCoordinate = fourthCoordinate;
    data->objectData = objectData;
    data->secondCoordinate = secondCoordinate;
    data->thirdCoordinate = thirdCoordinate;
    data->value = eventValue;
    return task;
}

struct EngineTask *sub_08061EA8(s16, s16, s16, s16, s16, void *, u32 *)
    __attribute__((alias("CreateFieldEventTask")));
