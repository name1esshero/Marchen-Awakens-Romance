/* Script VM resource registration and typed lookup adapters. */
#include "gba/types.h"
#include "script_bytecode.h"

#include "rom_section.h"

extern const struct ScriptResourceEntry gScriptResourceEntries[];
extern const u8 gScriptResourceDefaultValue[];
extern s32 ScriptResourceRemove(s32 type, const char *name);
extern u16 *ScriptResourceFind(s32 type, const char *name);
extern s32 ScriptResourceSet(s32 type, const char *name, const void *data,
                             s32 size);
struct ScriptResourceSlot {
    u32 referenceCount;
    void *allocation;
};

/* 24-entry {name, handler} table of core VM/engine commands (dummy, Pad,
 * Wait, GetBool/SetBool, GetVar/SetVar/AddVar, CrtFade family, Se/Bgm
 * playback) verified against ROM data; not yet reconstructed as a matching
 * C array, so it is aliased rather than re-typed. */
#define gScriptEngineFunctions \
    gScriptResourceEntries
/* Same {name, handler} layout as gScriptNativeCommands, reused here under
 * ScriptResourceEntry's generic type -- the THUMB-bit-set handler addresses
 * verified against decompiled.json in game_tables.c apply here too. */
extern const struct ScriptResourceEntry gScriptNativeCommands[];
/* Zero-filled 16-byte fallback record (the trailing "%d" belongs to
 * unrelated, adjacent ROM data) returned when a resource value slot holds
 * no override. */
#define sScriptResourceDefaultValue ((void *)gScriptResourceDefaultValue)

struct ScriptResourceSlot *ScriptResourceSlotFirst(s32 index);
struct ScriptResourceSlot *ScriptResourceSlotSecond(s32 index);
extern void HeapFree(void *heap, void *allocation);
extern s32 ScriptResourceReset(s32 index);
extern s32 ScriptResourceResetArray(s32 index);
extern u32 *ScriptResourceSelectValueSlot(u32 *record, s32 selector);
extern s32 sub_0807F094(s32 index);
extern s32 ScriptResourceNameFirst(const char *key);
extern s32 ScriptResourceNameSecond(const char *key);
extern void *sub_080050A8(const char *name, s32 slot, s32 *status);
extern void sub_0807E76C(void *resource, const char *name, s32 slot,
                         s32 status);
extern s32 sub_0807EBE0(s32 heap, void *table, s32 type, const char *name);
extern s32 sub_0807EB5C(s32 heap, void *table, s32 type, const char *name,
                        const void *data, s32 size);

/** Register a {name, value} table as type-33 resources, replacing any
 * existing entry of the same name.
 * @param entry Name-terminated table (a null name ends the table).
 * @return Always 0. */
AT("0007EE10") s32 ScriptResourceRegisterTable(
    const struct ScriptResourceEntry *entry)
{
    while (entry->name) {
        ScriptResourceRemove(33, entry->name);
        ScriptResourceSet(33, entry->name, &entry->value,
                          sizeof(entry->value));
        entry++;
    }
    return 0;
}
AT("0007EE10") const u8 ScriptResourceRegisterTableTail[2] = {0};

/** Register a {name, value} table as type-33 resources on a specific heap
 * and table, as ScriptResourceRegisterTable() does on the active VM.
 * @param heap Target heap.
 * @param table Target resource table.
 * @param entry Name-terminated table (a null name ends the table).
 * @return Always 0. */
AT("0007EE3C") s32 ScriptResourceRegisterTableToHeap(
    s32 heap, void *table, const struct ScriptResourceEntry *entry)
{
    while (entry->name) {
        sub_0807EBE0(heap, table, 33, entry->name);
        sub_0807EB5C(heap, table, 33, entry->name, &entry->value,
                     sizeof(entry->value));
        entry++;
    }
    return 0;
}
AT("0007EE3C") const u8 ScriptResourceRegisterTableToHeapTail[2] = {0};

/** Install the built-in VM, game, and native-command resource tables. */
AT("0007EE7C") s32 ScriptResourceRegisterBuiltins(s32 heap, void *table)
{
    if (ScriptResourceRegisterTableToHeap(
            heap, table, gScriptBuiltinFunctions))
        return -1;
    if (ScriptResourceRegisterTableToHeap(
            heap, table, gScriptEngineFunctions))
        return -1;
    if (ScriptResourceRegisterTableToHeap(
            heap, table, gScriptNativeCommands))
        return -1;
    return 0;
}
AT("0007EE7C") const u8 ScriptResourceRegisterBuiltinsTail[2] = {0};

/** Load a named resource into a slot and install it.
 * @param name Resource name to load.
 * @param slot Slot to install the loaded resource into.
 * @return Always 0. */
AT("0007EF94") s32 ScriptResourceLoadAndInstall(const char *name, s32 slot)
{
    s32 status;
    sub_0807E76C(sub_080050A8(name, slot, &status), name, slot, status);
    return 0;
}
AT("0007EF94") const u8 ScriptResourceLoadAndInstallTail[2] = {0};

/** Return a resource slot to its empty state. Shared allocations (values above
 * one) belong to the VM heap and must be released before the slot is reused. */
AT("0007F05C") s32 ScriptResourceReset(s32 index)
{
    struct ScriptResourceSlot *slot = ScriptResourceSlotFirst(index);

    if (slot == 0)
        return -1;
    if (slot->referenceCount > 1)
        HeapFree(gScriptBytecodeRoot->context->heap, slot->allocation);
    slot->referenceCount = 1;
    slot->allocation = 0;
    return 0;
}

/** The second slot class owns an array of allocations. Its live count is read
 * again after each free because the heap callback may update VM state. */
AT("0007F094") s32 ScriptResourceResetArray(s32 index)
{
    struct ScriptResourceSlot *slot;
    u32 count;
    u32 i;
    void **blocks;
    void *heap;

    slot = ScriptResourceSlotSecond(index);
    if (slot == 0)
        return -1;

    count = slot->referenceCount;
    if (count > 1) {
        i = 0;
        blocks = slot->allocation;
        if (i < count) {
            do {
                HeapFree(gScriptBytecodeRoot->context->heap, *blocks++);
                i++;
                count = slot->referenceCount;
            } while (i < count);
        }
    }

    heap = gScriptBytecodeRoot->context->heap;
    HeapFree(heap, slot->allocation);
    slot->referenceCount = 1;
    slot->allocation = 0;
    return 0;
}

/** Resolve a first-class resource name to its slot and reset that slot.
 * @return 0 on success, -1 if the slot lookup fails. */
AT("0007F1D8") s32 ScriptResourceLookupFirst(const char *key)
{
    return ScriptResourceReset(ScriptResourceNameFirst(key));
}
AT("0007F1D8") const u8 ScriptResourceLookupFirstTail[2] = {0};

/** Resolve a second-class resource name to its slot and reset that slot's
 * array of allocations.
 * @return 0 on success, -1 if the slot lookup fails. */
AT("0007F1E8") s32 ScriptResourceLookupSecond(const char *key)
{
    return ScriptResourceResetArray(ScriptResourceNameSecond(key));
}
AT("0007F1E8") const u8 ScriptResourceLookupSecondTail[2] = {0};

/** Return the stable numeric slot assigned to a resource name.  The two name
 * classes have independent 32-entry namespaces in the active script VM. */
AT("0007F294") s32 ScriptResourceNameFirst(const char *key)
{
    u8 *root;
    u8 *state;
    u16 *found;

    found = ScriptResourceFind(35, key);
    if (found != 0)
        return *found;

    root = (u8 *)&gScriptBytecodeRoot;
    state = *(u8 **)(*(u8 **)root + 12);
    if (*(u16 *)(state + 16) > 31)
        return -1;

    ScriptResourceSet(35, key, state + 16, sizeof(u16));
    state = *(u8 **)(*(u8 **)root + 12);
    return (*(u16 *)(state + 16))++;
}
AT("0007F294") const u8 ScriptResourceNameFirstTail[2] = {0};

/** Return the stable numeric slot assigned to a second-class resource name,
 * assigning the next free slot on first use. See ScriptResourceNameFirst()
 * for the first-class equivalent; the two share the same scheme over an
 * independent 32-entry namespace.
 * @return The resource's slot index, or -1 if the namespace is full. */
AT("0007F2E0") s32 ScriptResourceNameSecond(const char *key)
{
    u8 *root;
    u8 *state;
    u16 *found;

    found = ScriptResourceFind(36, key);
    if (found != 0)
        return *found;

    root = (u8 *)&gScriptBytecodeRoot;
    state = *(u8 **)(*(u8 **)root + 12);
    if (*(u16 *)(state + 18) > 31)
        return -1;

    ScriptResourceSet(36, key, state + 18, sizeof(u16));
    state = *(u8 **)(*(u8 **)root + 12);
    return (*(u16 *)(state + 18))++;
}
AT("0007F2E0") const u8 ScriptResourceNameSecondTail[2] = {0};

/** Both resource classes keep their slots inline in the VM context: class one
 * starts at offset 20 with its live count at 16, class two at offset 276 with
 * its count at 18.  Out-of-range slots resolve to a null record rather than
 * trapping, which is what the lazy handles in script_resource_handles.c
 * rely on. */
AT("0007F32C") struct ScriptResourceSlot *ScriptResourceSlotFirst(s32 index)
{
    struct ScriptBytecodeContext *context;
    u32 offset;

    if (index >= 0) {
        context = gScriptBytecodeRoot->context;
        if (index < *(u16 *)((u8 *)context + 16))
            goto found;
    }
    return 0;
found:
    offset = index * 8;
    offset += 20;
    return (struct ScriptResourceSlot *)((u8 *)context + offset);
}
AT("0007F32C") const u8 ScriptResourceSlotFirstTail[2] = {0};

/** @return The second-class resource slot at index, or NULL if index is out
 * of range. See ScriptResourceSlotFirst() for the layout this mirrors. */
AT("0007F354") struct ScriptResourceSlot *ScriptResourceSlotSecond(s32 index)
{
    struct ScriptBytecodeContext *context;
    u32 offset;

    if (index >= 0) {
        context = gScriptBytecodeRoot->context;
        if (index < *(u16 *)((u8 *)context + 18))
            goto found;
    }
    return 0;
found:
    offset = index * 8;
    offset += 276;
    return (struct ScriptResourceSlot *)((u8 *)context + offset);
}
AT("0007F354") const u8 ScriptResourceSlotSecondTail[2] = {0};

extern void *HeapAlloc(void *heap, u32 size);
extern u32 strlen(const char *text);
extern char *strcpy(char *destination, const char *source);

/** Replace a value slot with a heap-owned copy of a string.  The previous
 * allocation, if any, is released first; a failed allocation leaves the slot
 * null but still reports success, matching the original. */
AT("0007F3DC") s32 ScriptResourceSetStringValue(u32 *record, s32 selector,
                                                 const char *value)
{
    u32 *slot;
    char *copy;

    if (record == 0)
        return -1;
    slot = ScriptResourceSelectValueSlot(record, selector);
    if (slot == 0)
        return -1;
    if (*slot != 0)
        HeapFree(gScriptBytecodeRoot->context->heap, (void *)*slot);
    copy = HeapAlloc(gScriptBytecodeRoot->context->heap, strlen(value) + 1);
    *slot = (u32)copy;
    if (copy != 0)
        strcpy(copy, value);
    return 0;
}

/** @return The raw value slot for a record/selector pair, or NULL if record
 * is NULL or the selector doesn't resolve to a slot. */
AT("0007F380") u32 *ScriptResourceGetValue(u32 *record, s32 selector)
{
    u32 *slot;

    if (record == 0)
        return 0;
    slot = ScriptResourceSelectValueSlot(record, selector);
    if (slot == 0)
        return 0;
    return (u32 *)*slot;
}

/** Like ScriptResourceGetValue(), but returns the shared zero-filled default
 * record instead of NULL when the slot holds no value. */
AT("0007F398") void *ScriptResourceGetValueOrDefault(u32 *record,
                                                      s32 selector)
{
    u32 *slot;
    void *value;

    if (record == 0)
        return 0;
    slot = ScriptResourceSelectValueSlot(record, selector);
    if (slot == 0)
        return 0;
    value = (void *)*slot;
    if (value != 0)
        return value;
    return sScriptResourceDefaultValue;
}

/** Write a raw value into a record's selected slot.
 * @return 0 on success, -1 if record is NULL or the selector doesn't
 * resolve to a slot. */
AT("0007F3BC") s32 ScriptResourceSetValue(u32 *record, s32 selector,
                                           u32 value)
{
    u32 *slot;

    if (record == 0)
        return -1;
    slot = ScriptResourceSelectValueSlot(record, selector);
    if (slot == 0)
        return -1;
    *slot = value;
    return 0;
}
