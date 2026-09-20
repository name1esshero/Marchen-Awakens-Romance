/* Hash-table storage used by the script VM's named resources. */
#include "gba/types.h"
#include "script_bytecode.h"

#include "rom_section.h"
struct ScriptResourceNode {
    struct ScriptResourceNode *next;
    char *typedName;
    u8 value[1];
};

struct ScriptResourceTable {
    u8 unknown00[8];
    struct ScriptResourceNode **buckets;
};

extern s32 __modsi3(s32 dividend, s32 divisor);
extern u32 strlen(const char *text);
extern s32 strcmp(const char *left, const char *right);
extern char *strcpy(char *destination, const char *source);
extern void CpuCopy(void *destination, const void *source, u32 size);
extern void *HeapAlloc(void *heap, u32 size);
extern void HeapFree(void *heap, void *allocation);

/** Fold the resource class and name into one of the VM's 587 buckets. */
AT("0007E97C") s32 ScriptResourceHash(s32 type, const char *name)
{
    s32 hash = type;

    while (*(const s8 *)name != 0) {
        hash = __modsi3((hash << 8) + *(const u8 *)name, 587);
        name++;
    }
    return hash;
}

AT("0007E9A8") u8 *ScriptResourceFind(s32 type, const char *name)
{
    s32 bucket = ScriptResourceHash(type, name);
    struct ScriptResourceTable *table =
        (struct ScriptResourceTable *)gScriptBytecodeRoot->context;
    struct ScriptResourceNode *node = table->buckets[bucket];

    while (node != 0) {
        if (type == (s8)node->typedName[0] &&
            strcmp(name, node->typedName + 1) == 0)
            return node->value;
        node = node->next;
    }
    return 0;
}
AT("0007E9A8") const u8 ScriptResourceFindTail[2] = {0, 0};

/** Remove an existing class/name pair and release its table allocation. */
AT("0007EA8C") s32 ScriptResourceRemove(s32 type, const char *name)
{
    s32 localType = type;
    const char *localName = name;
    s32 bucket =
        ScriptResourceHash(localType, localName);
    struct ScriptResourceNode *node =
        ((struct ScriptResourceTable *)gScriptBytecodeRoot->context)
            ->buckets[bucket];
    struct ScriptResourceNode *previous = 0;

    if (node != 0) {
        do {
            if (localType == (s8)node->typedName[0] &&
                strcmp(localName, node->typedName + 1) == 0)
                goto found;
            previous = node;
            node = node->next;
        } while (node != 0);
    }
    return 1;

found:
    if (previous == 0)
        ((struct ScriptResourceTable *)
            gScriptBytecodeRoot->context)
            ->buckets[bucket] = node->next;
    else
        previous->next = node->next;
    HeapFree(*(void **)((u8 *)gScriptBytecodeRoot->context + 4), node);
    return 0;
}
