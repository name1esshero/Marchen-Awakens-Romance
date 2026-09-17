/* Native-script adapters for inventory/resource and encounter helpers.
 * Arguments occupy 32-bit VM slots even when the underlying engine API uses
 * signed bytes or halfwords. Preserve those explicit narrowing operations.
 */
#include "gba/types.h"
#include "game_tables.h"
#include "game_state.h"

#include "rom_section.h"

extern s32 sub_080570BC(s32 id);
extern s32 GameStateGetEntry2768Total(s32 id);
extern s32 GameStateGetEntry2AE0(s32 id);
extern void sub_08056A8C(s32 id, s32 mode);
extern s32 sub_08056290(void);
extern void sub_080562C8(s32 value);
extern void sub_080563AC(void);
extern s32 sub_08056304(s32 value);
extern void sub_08055F88(s32 value, s32 limit);
extern s32 GameStateGetCurrentEntry3894(void);
extern void sub_080087EC(s32 a, s32 b, s32 value);
extern void sub_080083E0(s32 a, s32 b);
extern void sub_0806EFCC(s32 value);
extern u32 Random(void);
extern s32 __umodsi3(u32 random, u32 count);
extern void sub_0806F120(s32 x, s32 y, s32 a, s32 b);
extern s32 GameStateGetField42BA(void);
extern void CreateEncounterTransitionTask(s32 value);
extern void sub_080577C0(s32 value);
extern s32 GameStateGetEncounterValue(void);
extern void sub_08057800(s32 value);
extern s32 GameStateGetEncounterMode(void);
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
extern void *HeapAlloc(void *heap, u32 size);
extern const char *ItemGetName(s32 id);
extern const char *ConsumableGetName(s32 id);
extern char *strcpy(char *destination, const char *source);
extern s32 sub_08055EC8(s32 id);
extern void BitSet(void *bits, s32 index, s32 value);
#define RUNTIME_STATE ({ \
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})

/** Initialize the eight friend ARM slots and mark every valid ARM definition
 * as owned. The clear before each assignment is behavior present in the ROM,
 * even though the following halfword store immediately replaces it. */
AT("00012B98") s32 ScriptNativeSetFriendArms(u32 count, const s32 *args,
                                              s32 *result)
{
    s32 i = 0;
    u8 **root = &gMapGenerationRoot;
    s32 friendOffset = 0x3880;
    const s32 *input = args;
    const s16 *definitions = gFriendArmOwnershipBits;
    s32 invalidDefinition = FRIEND_ARM_NO_OWNERSHIP_BIT;

    do {
        s32 byteOffset;
        u8 *firstSlot;
        register u8 *slot asm("r0");
        s32 definitionIndex;
        s32 ownershipBit;

        firstSlot = *root;
        byteOffset = i << 1;
        firstSlot += friendOffset;
        firstSlot += byteOffset;
        *(u16 *)firstSlot = 0;

        slot = *root;
        slot += friendOffset;
        slot += byteOffset;
        *(u16 *)slot = *input;

        slot = *root;
        slot += friendOffset;
        slot += byteOffset;
        definitionIndex = (s16)sub_08055EC8(*(s16 *)slot);
        ownershipBit = definitions[definitionIndex];
        if (ownershipBit != invalidDefinition)
            BitSet(*root + 0x26F8, ownershipBit, 1);

        input++;
        i++;
    } while (i <= 7);
    return 1;
}

/** Native script command: look up a resource's id via sub_080570BC().
 * @return Always 1. */
AT("00012D98") s32 ScriptNativeQueryResourceId(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)sub_080570BC((s16)args[0]);
    return 1;
}
AT("00012D98") const u8 ScriptNativeQueryResourceIdTail[2] = {0};

/** Native script command: read a resource total from one of two tables,
 * selected by the map-generation state's +0x38B8 mode byte.
 * @return Always 1. */
AT("00012DB4") s32 ScriptNativeQueryModeResource(u32 count, const s32 *args, s32 *result)
{
    s32 *out = result;
    const s16 *shortArgs = (const s16 *)args;
    register u8 *base asm("r0");
    register u32 offset asm("r2");
    s32 value;

    base = gIwramBase;
    offset = (u32)gMapGenerationRootOffset;
    base += offset;
    base = *(u8 **)base;
    offset = 0x38B8;
    base += offset;
    if (*(u8 *)base == 0)
        value = GameStateGetEntry2768Total(shortArgs[0]);
    else
        value = GameStateGetEntry2AE0(shortArgs[0]);
    *out = (s16)value;
    return 1;
}
AT("00012DB4") const u8 ScriptNativeQueryModeResourceTail[2] = {0};

/** Native script command: forward to sub_08056A8C() with a mode of 1 or 2,
 * selected by the map-generation state's +0x38B8 mode byte.
 * @return Always 1. */
AT("00012DF8") s32 ScriptNativeSetModeResource(u32 count, const s32 *args, s32 *result)
{
    register const s16 *shortArgs asm("r1") = (const s16 *)args;
    u8 *base;
    u32 offset;

    base = gIwramBase;
    offset = (u32)gMapGenerationRootOffset;
    base += offset;
    base = *(u8 **)base;
    offset = 0x38B8;
    base += offset;
    if (*(u8 *)base == 0)
        sub_08056A8C(shortArgs[0], 1);
    else
        sub_08056A8C(shortArgs[0], 2);
    return 1;
}

/** Native script command: read a resource state via sub_08056290().
 * @return Always 1. */
AT("00012E34") s32 ScriptNativeQueryResourceState(u32 count, const s32 *args, s32 *result)
{
    *result = sub_08056290();
    return 1;
}
AT("00012E34") const u8 ScriptNativeQueryResourceStateTail[2] = {0};

/** Native script command: forward to sub_080562C8(). @return Always 1. */
AT("00012E48") s32 ScriptNativeSetResourceState(u32 count, const s32 *args, s32 *result)
{
    sub_080562C8(args[0]);
    return 1;
}
AT("00012E48") const u8 ScriptNativeSetResourceStateTail[2] = {0};

/** Native script command: forward to sub_080563AC(). @return Always 1. */
AT("00012E58") s32 ScriptNativeResetResourceState(u32 count, const s32 *args, s32 *result)
{
    sub_080563AC();
    return 1;
}

/** Native script command: consume a resource if sub_08056304() allows it,
 * then apply its follow-up effects.
 * @return Always 1. */
AT("00012E64") s32 ScriptNativeUseResource(u32 count, const s32 *args, s32 *result)
{
    if ((s16)sub_08056304((s16)args[0]))
    {
        sub_08055F88((s16)args[0], 999);
        sub_080087EC(0, 0, (s16)GameStateGetCurrentEntry3894());
        sub_080083E0(0, 0);
    }
    return 1;
}

/** Native script command: allocate a 34-byte buffer from the map generation
 * owner's heap and copy in an item or consumable's name.
 * @return Always 1. */
AT("00012EA8") s32 ScriptNativeGetResourceName(u32 count, const s32 *args,
                                                char **result)
{
    u8 *iwram = gIwramBase;
    u32 rootOffset = (u32)gMapGenerationRootOffset;
    u8 *state = *(u8 **)(iwram + rootOffset);
    rootOffset -= 172;
    {
    void *owner = *(void **)(state + rootOffset);
    void *heap = *(void **)owner;
    char *name = HeapAlloc(heap, 34);
    if (args[1] == 0)
        strcpy(name, ItemGetName((s16)args[0]));
    else
        strcpy(name, ConsumableGetName((s16)args[0]));
    *result = name;
    return 1;
    }
}
AT("00012EA8") const u8 ScriptNativeGetResourceNameTail[2] = {0};

/** Native script command: forward to sub_0806EFCC(0). @return Always 1. */
AT("00012F04") s32 ScriptNativeResetEncounterState(u32 count, const s32 *args, s32 *result)
{
    sub_0806EFCC(0);
    return 1;
}
AT("00012F04") const u8 ScriptNativeResetEncounterStateTail[2] = {0};

/** Native script command: pick one of the VM's own arguments at random.
 * @return Always 1. */
AT("00012F14") s32 ScriptNativeChooseRandomValue(u32 count, const s32 *args, s32 *result)
{
    s32 index = (s16)__umodsi3(Random(), count);
    *result = args[index];
    return 1;
}

/** Native script command: forward map coordinates to sub_0806F120().
 * @return Always 0x7FFF. */
AT("00012F38") s32 ScriptNativeStartMapCoordinateEvent(u32 count, const s32 *args, s32 *result)
{
    sub_0806F120((s16)args[0], (s16)args[1], 0, 0);
    return 0x7FFF;
}

/** Native script command: read GameStateGetField42BA().
 * @return Always 1. */
AT("00012F54") s32 ScriptNativeGetField42BA(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)GameStateGetField42BA();
    return 1;
}
AT("00012F54") const u8 ScriptNativeGetField42BATail[2] = {0};

/** Native script command: start the encounter-transition task.
 * @return Always 0x7FFF. */
AT("00012F6C") s32 ScriptNativeStartEncounter(u32 count, const s32 *args, s32 *result)
{
    CreateEncounterTransitionTask(0);
    return 0x7FFF;
}

/** Native script command: forward to sub_080577C0(). @return Always 1. */
AT("00012F80") s32 ScriptNativeSetEncounterValue(u32 count, const s32 *args, s32 *result)
{
    sub_080577C0((s16)args[0]);
    return 1;
}

/** Native script command: read GameStateGetEncounterValue().
 * @return Always 1. */
AT("00012F90") s32 ScriptNativeGetEncounterValue(u32 count, const s32 *args, s32 *result)
{
    *result = (s16)GameStateGetEncounterValue();
    return 1;
}
AT("00012F90") const u8 ScriptNativeGetEncounterValueTail[2] = {0};

/** Native script command: forward to sub_08057800(). @return Always 1. */
AT("00012FA8") s32 ScriptNativeSetEncounterMode(u32 count, const s32 *args, s32 *result)
{
    sub_08057800((s8)args[0]);
    return 1;
}

/** Native script command: read GameStateGetEncounterMode().
 * @return Always 1. */
AT("00012FB8") s32 ScriptNativeGetEncounterMode(u32 count, const s32 *args, s32 *result)
{
    *result = (s8)GameStateGetEncounterMode();
    return 1;
}
AT("00012FB8") const u8 ScriptNativeGetEncounterModeTail[2] = {0};
