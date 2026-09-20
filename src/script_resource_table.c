/* Hash-table storage used by the script VM's named resources. */
#include "gba/types.h"
#include "script_bytecode.h"

#include "rom_section.h"
struct ScriptResourceNode {
    struct ScriptResourceNode *next;
    char *typedName;
    u8 value[1];
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
    struct ScriptResourceNode *node =
        gScriptBytecodeRoot->context->resourceBuckets[bucket];

    while (node != 0) {
        if (type == (s8)node->typedName[0] &&
            strcmp(name, node->typedName + 1) == 0)
            return node->value;
        node = node->next;
    }
    return 0;
}
AT("0007E9A8") const u8 ScriptResourceFindTail[2] = {0, 0};

/** Insert a new class/name pair and a copy of its value in the VM resource
 * hash table.
 * @return Zero on success, one if the pair already exists, or negative one
 * if its node cannot be allocated. */
AT("0007E9F4") s32 ScriptResourceSet(s32 type, const char *name,
                                     const void *value, s32 size)
{
    s32 bucket = ScriptResourceHash(type, name);
    struct ScriptBytecodeRoot **root;
    struct ScriptResourceNode *node;
    struct ScriptBytecodeContext *context;
    u32 nameLength;

    if (ScriptResourceFind(type, name) != 0)
        return 1;

    nameLength = strlen(name);
    root = &gScriptBytecodeRoot;
    node = HeapAlloc((*root)->context->resourceHeap,
                     size + nameLength + 13);
    if (node == 0)
        return -1;

    CpuCopy(node->value, value, size);
    node->typedName = (char *)node + (size + 8);
    node->typedName[0] = type;
    strcpy(node->typedName + 1, name);

    context = (*root)->context;
    node->next = context->resourceBuckets[bucket];
    context->resourceBuckets[bucket] = node;
    return 0;
}
AT("0007E9F4") const u8 ScriptResourceSetTail[2] = {0, 0};

/** Remove an existing class/name pair and release its table allocation. */
AT("0007EA8C") s32 ScriptResourceRemove(s32 type, const char *name)
{
    s32 localType = type;
    const char *localName = name;
    s32 bucket =
        ScriptResourceHash(localType, localName);
    struct ScriptResourceNode *node =
        gScriptBytecodeRoot->context->resourceBuckets[bucket];
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
        gScriptBytecodeRoot->context->resourceBuckets[bucket] = node->next;
    else
        previous->next = node->next;
    HeapFree(gScriptBytecodeRoot->context->resourceHeap, node);
    return 0;
}
