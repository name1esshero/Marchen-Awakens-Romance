/* Native script adapters for the paired field-effect systems at 0800D8EC
 * and 0800FC90.  Script arguments arrive as an array in r1; these commands
 * unpack that array into the ordinary C ABI used by the effect/task layer.
 *
 * A return value of 1 completes the native call immediately.  The start/wait
 * commands return 0x7fff after attaching a pending task, which tells the VM to
 * suspend the script until that task supplies its result.
 */
#include "gba/types.h"
#include "script_sprite.h"
#include "task_constructors.h"

#include "rom_section.h"

extern void sub_0800FDFC(s32, s32, s32, s32, s32, s32);
extern void sub_0800FF64(s32, s32, s32, s32, s32);
extern void StartPendingEffectA(s32, s32);
extern void CreatePendingTaskFD3C(s32);
extern void sub_08010194(s32, s32, s32, s32, s32, s32, s32);
extern void sub_08010410(s32, s32, s32 *, s32);
extern void sub_08010568(s32, s32, s32, s32 *, s32);

extern void sub_0800DA3C(s32, s32, s32, s32, s32, s32, s32);
extern void sub_0800DC60(s32, s32, s32, s32, s32, s32);
extern void StartPendingEffectB(s32, s32);
extern void CreatePendingTaskD98C(s32);
extern void sub_0800DEE4(s32, s32, s32, s32, s32, s32, s32);
extern void sub_0800E268(s32, s32, s32 *, s32);
extern void sub_0800E444(s32, s32, s32, s32 *, s32);

extern void sub_08010A2C(s32, s32);
extern void sub_08010E44(s32, s32, s32, s32, s32, s32, s32);
extern void sub_08011340(s32, s32, s32, s32 *, s32);

extern void sub_0800EB18(s32, s32, s32, s32, s32, s32, s32, s32);
extern void CreateFieldEffectTask(s32, s32, s32, s32, s32, s32, s32);
extern void CreateIndexedPendingTask(s32, s32, s32);
extern void StartPendingFieldEffect(s32, s32);
extern void sub_0800E8E4(s32, s32, s32, s32, s32, s32);
extern void sub_0800EDC0(s32, s32, s32, s32 *, s32);
extern void sub_0800EFB0(s32, s32, s32, s32, s32 *, s32);
extern char *strcpy(char *, const char *);
extern char *strupr(char *);
extern u8 *RuntimeGetActorRecord(s32, s32);
extern void sub_0800F4C8(s32, s32, s32, s32, s32, s32, s32);
extern void sub_0800F758(s32, s32, s32 *, s32);
extern void sub_0800F898(s32, s32, s32, s32 *, s32);
extern void sub_08011554(s32, s32, s32, s32, s32, s32, s32);
extern void sub_08011174(s32, s32, s32 *, s32);

/** Native script command: forward five arguments to sub_0800FDFC() for
 * effect system A, with its sixth argument fixed to 0. @return Always 1. */
AT("00011CE4") s32 ScriptNativeEffectAConfigure6(u32 count, const s32 *args, s32 *result)
{
    sub_0800FDFC(args[0], args[1], args[2], args[3], args[4], 0);
    return 1;
}

/** Native script command: forward five arguments to sub_0800FF64() for
 * effect system A. @return Always 1. */
AT("00011D08") s32 ScriptNativeEffectAConfigure5(u32 count, const s32 *args, s32 *result)
{
    sub_0800FF64(args[0], args[1], args[2], args[3], args[4]);
    return 1;
}

/** Native script command: wait on a specific effect-A slot, or on any
 * pending effect-A task if args[0] is -1.
 * @return Always SCRIPT_WAIT (0x7fff). */
AT("00011D28") s32 ScriptNativeEffectAWait(u32 count, const s32 *args, s32 *result)
{
    if (args[0] != -1)
        StartPendingEffectA(args[0], 0);
    else
        CreatePendingTaskFD3C(0);
    return 0x7fff;
}

/** Native script command: start effect system A with two arguments and a
 * fixed 0,0,_,1,0 parameter pattern (see ScriptNativeEffectAStartFull() for
 * the full form). @return Always 1. */
AT("00011D50") s32 ScriptNativeEffectAStart(u32 count, const s32 *args, s32 *result)
{
    sub_08010194(args[0], args[1], 0, 0, args[2], 1, 0);
    return 1;
}
AT("00011D50") const u8 ScriptNativeEffectAStartTail[2] = {0};

/** Native script command: query effect system A, publishing the result
 * through the VM's result slot. @return Always 1. */
AT("00011D78") s32 ScriptNativeEffectAQuery(u32 count, const s32 *args, s32 *result)
{
    sub_08010410(args[0], args[1], result, 0);
    return 1;
}
AT("00011D78") const u8 ScriptNativeEffectAQueryTail[2] = {0};

/** Native script command: start effect system A with all six of its own
 * arguments. See ScriptNativeEffectAStart() for the fixed-pattern form.
 * @return Always 1. */
AT("00011D8C") s32 ScriptNativeEffectAStartFull(u32 count, const s32 *args, s32 *result)
{
    sub_08010194(args[0], args[1], args[2], args[3], args[4], args[5], 0);
    return 1;
}

/** Native script command: send a command to effect system A, publishing
 * the result through the VM's result slot. @return Always 1. */
AT("00011DB4") s32 ScriptNativeEffectACommand(u32 count, const s32 *args, s32 *result)
{
    sub_08010568(args[0], args[1], args[2], result, 0);
    return 1;
}

/** Native script command: start effect system B with all six of its own
 * arguments. See ScriptNativeEffectBStart() for the fixed-pattern form.
 * @return Always 1. */
AT("00011DD4") s32 ScriptNativeEffectBStartFull(u32 count, const s32 *args, s32 *result)
{
    sub_0800DA3C(args[0], args[1], args[2], args[3], args[4], args[5], 0);
    return 1;
}

/** Native script command: forward six arguments to sub_0800DC60() for
 * effect system B. @return Always 1. */
AT("00011DFC") s32 ScriptNativeEffectBConfigure(u32 count, const s32 *args, s32 *result)
{
    sub_0800DC60(args[0], args[1], args[2], args[3], args[4], args[5]);
    return 1;
}

/** Native script command: wait on a specific effect-B slot, or on any
 * pending effect-B task if args[0] is -1.
 * @return Always SCRIPT_WAIT (0x7fff). */
AT("00011E20") s32 ScriptNativeEffectBWait(u32 count, const s32 *args, s32 *result)
{
    if (args[0] != -1)
        StartPendingEffectB(args[0], 0);
    else
        CreatePendingTaskD98C(0);
    return 0x7fff;
}

/** Native script command: start effect system B with two arguments and a
 * fixed 0,0,_,1,0 parameter pattern. @return Always 1. */
AT("00011E48") s32 ScriptNativeEffectBStart(u32 count, const s32 *args, s32 *result)
{
    sub_0800DEE4(args[0], args[1], 0, 0, args[2], 1, 0);
    return 1;
}
AT("00011E48") const u8 ScriptNativeEffectBStartTail[2] = {0};

/** Native script command: query effect system B, publishing the result
 * through the VM's result slot. @return Always 1. */
AT("00011E70") s32 ScriptNativeEffectBQuery(u32 count, const s32 *args, s32 *result)
{
    sub_0800E268(args[0], args[1], result, 0);
    return 1;
}
AT("00011E70") const u8 ScriptNativeEffectBQueryTail[2] = {0};

/** Native script command: start effect system B with all six of its own
 * arguments, sharing sub_0800DEE4() with ScriptNativeEffectBStart().
 * @return Always 1. */
AT("00011E84") s32 ScriptNativeEffectBStartExtended(u32 count, const s32 *args, s32 *result)
{
    sub_0800DEE4(args[0], args[1], args[2], args[3], args[4], args[5], 0);
    return 1;
}

/** Native script command: send a command to effect system B, publishing
 * the result through the VM's result slot. @return Always 1. */
AT("00011EAC") s32 ScriptNativeEffectBCommand(u32 count, const s32 *args, s32 *result)
{
    sub_0800E444(args[0], args[1], args[2], result, 0);
    return 1;
}

/** Native script command: wait on a specific sprite-effect slot, or on any
 * pending sprite task if args[0] is -1.
 * @return Always SCRIPT_WAIT (0x7fff). */
AT("00011F14") s32 ScriptNativeSpriteWait(u32 count, const s32 *args, s32 *result)
{
    if (args[0] != -1)
        CreateSpriteResetTask(args[0], 1, 0);
    else
        sub_08010A2C(1, 0);
    return 0x7fff;
}

/** SprHitRect: set and enable a script sprite's collision bounds.
 * @return Always 1. */
AT("00011F7C") s32 ScriptNativeSpriteSetHitBounds(u32 count,
                                                   const s32 *args,
                                                   s32 *result)
{
    ScriptSpriteSetHitBounds(args[0], args[1], args[2], args[3], args[4]);
    return 1;
}

/** Native script command: forward six arguments to sub_08010E44() for the
 * sprite-effect system, with a seventh argument fixed to 0. This shares
 * its worker with ScriptNativeSpriteSet() in script_sprite.c.
 * @return Always 1. */
AT("00011F9C") s32 ScriptNativeSpriteEffect(u32 count, const s32 *args, s32 *result)
{
    sub_08010E44(args[0], args[1], args[2], args[3], args[4], args[5], 0);
    return 1;
}

/** Native script command: send a command to the sprite-effect system,
 * publishing the result through the VM's result slot. @return Always 1. */
AT("00011FC4") s32 ScriptNativeSpriteCommand(u32 count, const s32 *args, s32 *result)
{
    sub_08011340(args[0], args[1], args[2], result, 0);
    return 1;
}

/** Native script command: start a field effect with two arguments and a
 * fixed 0,0,_,1,0 parameter pattern (see ScriptNativeFieldEffectStart8()
 * for the full eight-argument form). @return Always 1. */
AT("00011FE4") s32 ScriptNativeFieldEffectStart(u32 count, const s32 *args, s32 *result)
{
    s32 first = args[0];
    s32 second = args[1];
    sub_0800EB18(first, second, args[2], 0, 0, args[3], 1, 0);
    return 1;
}

/** Native script command: forward to CreateFieldEffectTask() with all six
 * of its own arguments, plus a fixed 0 completion word. @return Always 1. */
AT("0001200C") s32 ScriptNativeFieldEffectStartFull(u32 count, const s32 *args, s32 *result)
{
    CreateFieldEffectTask(args[0], args[1], args[2], args[3], args[4], args[5], 0);
    return 1;
}

/** Native script command: wait on a specific field-effect slot, or on any
 * pending field effect for the owner if args[1] is -1.
 * @return Always SCRIPT_WAIT (0x7fff). */
AT("00012034") s32 ScriptNativeFieldEffectWait(u32 count, const s32 *args, s32 *result)
{
    if (args[1] != -1)
        CreateIndexedPendingTask(args[0], args[1], 0);
    else
        StartPendingFieldEffect(args[0], 0);
    return 0x7fff;
}

/** Native script command: forward six arguments to sub_0800E8E4() for the
 * field-effect system. @return Always 1. */
AT("00012060") s32 ScriptNativeFieldEffectConfigure(u32 count, const s32 *args, s32 *result)
{
    sub_0800E8E4(args[0], args[1], args[2], args[3], args[4], args[5]);
    return 1;
}

/** Native script command: send a command to the field-effect system,
 * publishing the result through the VM's result slot. @return Always 1. */
AT("00012084") s32 ScriptNativeFieldEffectCommand(u32 count, const s32 *args, s32 *result)
{
    sub_0800EDC0(args[0], args[1], args[2], result, 0);
    return 1;
}

/** Native script command: start a field effect with all seven of its own
 * arguments, sharing sub_0800EB18() with ScriptNativeFieldEffectStart().
 * @return Always 1. */
AT("000120A4") s32 ScriptNativeFieldEffectStart8(u32 count, const s32 *args, s32 *result)
{
    sub_0800EB18(args[0], args[1], args[2], args[3], args[4], args[5], args[6], 0);
    return 1;
}

/** Native script command: query the field-effect system, publishing the
 * result through the VM's result slot. @return Always 1. */
AT("000120D0") s32 ScriptNativeFieldEffectQuery(u32 count, const s32 *args, s32 *result)
{
    sub_0800EFB0(args[0], args[1], args[2], args[3], result, 0);
    return 1;
}

union EffectArgument { s32 integer; const char *string; };

/** Select one of the two short actor-name fields and copy an upper-case script
 * string into it. The original VM trusts its input to fit this 4-byte local. */
AT("000120F4") s32 ScriptNativeSetActorName(u32 count,
    const union EffectArgument *args, s32 *result)
{
    char name[4];
    u8 *actor;
    name[0] = 0;
    name[1] = 0;
    strcpy(name, args[3].string);
    strupr(name);
    actor = RuntimeGetActorRecord(args[0].integer, args[1].integer);
    switch (args[2].integer) {
    case 1:
        strcpy((char *)actor + 67, name);
        break;
    case 2:
        strcpy((char *)actor + 69, name);
        break;
    }
    return 1;
}
AT("000120F4") const u8 ScriptNativeSetActorNameTail[2] = {0};

/** Native script command: start effect system C with two arguments and a
 * fixed 0,0,_,1,0 parameter pattern. @return Always 1. */
AT("00012140") s32 ScriptNativeEffectCStart(u32 count, const s32 *args, s32 *result)
{
    sub_0800F4C8(args[0], args[1], 0, 0, args[2], 1, 0);
    return 1;
}
AT("00012140") const u8 ScriptNativeEffectCStartTail[2] = {0};

/** Native script command: query effect system C, publishing the result
 * through the VM's result slot. @return Always 1. */
AT("00012168") s32 ScriptNativeEffectCQuery(u32 count, const s32 *args, s32 *result)
{
    sub_0800F758(args[0], args[1], result, 0);
    return 1;
}
AT("00012168") const u8 ScriptNativeEffectCQueryTail[2] = {0};

/** Native script command: start effect system C with all six of its own
 * arguments, sharing sub_0800F4C8() with ScriptNativeEffectCStart().
 * @return Always 1. */
AT("0001217C") s32 ScriptNativeEffectCStartFull(u32 count, const s32 *args, s32 *result)
{
    sub_0800F4C8(args[0], args[1], args[2], args[3], args[4], args[5], 0);
    return 1;
}

/** Native script command: send a command to effect system C, publishing
 * the result through the VM's result slot. @return Always 1. */
AT("000121A4") s32 ScriptNativeEffectCCommand(u32 count, const s32 *args, s32 *result)
{
    sub_0800F898(args[0], args[1], args[2], result, 0);
    return 1;
}

/** Native script command: start the hit-effect system with two arguments
 * and a fixed 0,0,_,1,0 parameter pattern. @return Always 1. */
AT("00012208") s32 ScriptNativeHitEffectStart(u32 count, const s32 *args, s32 *result)
{
    sub_08011554(args[0], args[1], 0, 0, args[2], 1, 0);
    return 1;
}
AT("00012208") const u8 ScriptNativeHitEffectStartTail[2] = {0};

/** Native script command: query the hit-effect system, publishing the
 * result through the VM's result slot. This shares its worker with
 * ScriptNativeSpriteGet() in script_sprite.c. @return Always 1. */
AT("00012230") s32 ScriptNativeHitEffectQuery(u32 count, const s32 *args, s32 *result)
{
    sub_08011174(args[0], args[1], result, 0);
    return 1;
}
AT("00012230") const u8 ScriptNativeHitEffectQueryTail[2] = {0};
