/* Script VM resource registration and typed lookup adapters. */
#include "gba/types.h"
#include "script_bytecode.h"

#include "rom_section.h"

extern s32 ScriptResourceRemove(s32 type, const char *name);
extern u16 *ScriptResourceFind(s32 type, const char *name);
extern s32 ScriptResourceSet(s32 type, const char *name, const void *data,
                             s32 size);
struct ScriptResourceSlot {
    u32 referenceCount;
    void *allocation;
};

extern struct ScriptResourceSlot *sub_0807F32C(s32 index);
extern struct ScriptResourceSlot *sub_0807F354(s32 index);
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

/* Install the built-in VM, game, and native-command resource tables. */
AT("0007EE7C") s32 ScriptResourceRegisterBuiltins(s32 heap, void *table)
{
    if (ScriptResourceRegisterTableToHeap(
            heap, table, gScriptBuiltinFunctions))
        return -1;
    if (ScriptResourceRegisterTableToHeap(
            heap, table, (const struct ScriptResourceEntry *)0x081ACB7C))
        return -1;
    if (ScriptResourceRegisterTableToHeap(
            heap, table, (const struct ScriptResourceEntry *)0x081AFEA4))
        return -1;
    return 0;
}
AT("0007EE7C") const u8 ScriptResourceRegisterBuiltinsTail[2] = {0};

AT("0007EF94") s32 ScriptResourceLoadAndInstall(const char *name, s32 slot)
{
    s32 status;
    sub_0807E76C(sub_080050A8(name, slot, &status), name, slot, status);
    return 0;
}
AT("0007EF94") const u8 ScriptResourceLoadAndInstallTail[2] = {0};

/* Return a resource slot to its empty state. Shared allocations (values above
 * one) belong to the VM heap and must be released before the slot is reused. */
AT("0007F05C") s32 ScriptResourceReset(s32 index)
{
    struct ScriptResourceSlot *slot = sub_0807F32C(index);

    if (slot == 0)
        return -1;
    if (slot->referenceCount > 1)
        HeapFree(gScriptBytecodeRoot->context->heap, slot->allocation);
    slot->referenceCount = 1;
    slot->allocation = 0;
    return 0;
}

/* The second slot class owns an array of allocations. Its live count is read
 * again after each free because the heap callback may update VM state. */
AT("0007F094") s32 ScriptResourceResetArray(s32 index)
{
    struct ScriptResourceSlot *slot;
    u32 count;
    u32 i;
    void **blocks;
    void *heap;

    slot = sub_0807F354(index);
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

AT("0007F1D8") s32 ScriptResourceLookupFirst(const char *key)
{
    return ScriptResourceReset(ScriptResourceNameFirst(key));
}
AT("0007F1D8") const u8 ScriptResourceLookupFirstTail[2] = {0};

AT("0007F1E8") s32 ScriptResourceLookupSecond(const char *key)
{
    return ScriptResourceResetArray(ScriptResourceNameSecond(key));
}
AT("0007F1E8") const u8 ScriptResourceLookupSecondTail[2] = {0};

/* Return the stable numeric slot assigned to a resource name.  The two name
 * classes have independent 32-entry namespaces in the active script VM. */
AT("0007F294") s32 ScriptResourceNameFirst(const char *key)
{
    u8 *root;
    u8 *state;
    u16 *found;

    found = ScriptResourceFind(35, key);
    if (found != 0)
        return *found;

    root = (u8 *)0x0300611C;
    state = *(u8 **)(*(u8 **)root + 12);
    if (*(u16 *)(state + 16) > 31)
        return -1;

    ScriptResourceSet(35, key, state + 16, sizeof(u16));
    state = *(u8 **)(*(u8 **)root + 12);
    return (*(u16 *)(state + 16))++;
}
AT("0007F294") const u8 ScriptResourceNameFirstTail[2] = {0};

AT("0007F2E0") s32 ScriptResourceNameSecond(const char *key)
{
    u8 *root;
    u8 *state;
    u16 *found;

    found = ScriptResourceFind(36, key);
    if (found != 0)
        return *found;

    root = (u8 *)0x0300611C;
    state = *(u8 **)(*(u8 **)root + 12);
    if (*(u16 *)(state + 18) > 31)
        return -1;

    ScriptResourceSet(36, key, state + 18, sizeof(u16));
    state = *(u8 **)(*(u8 **)root + 12);
    return (*(u16 *)(state + 18))++;
}
AT("0007F2E0") const u8 ScriptResourceNameSecondTail[2] = {0};

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
    return (void *)0x081AC698;
}

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
