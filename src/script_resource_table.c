/* Hash-table storage used by the script VM's named resources. */
#include "gba/types.h"
#include "script_bytecode.h"

#include "rom_section.h"
extern s32 __modsi3(s32 dividend, s32 divisor);
extern u32 strlen(const char *text);
extern s32 strcmp(const char *left, const char *right);
extern char *strcpy(char *destination, const char *source);
extern void CpuCopy(void *destination, const void *source, u32 size);
extern void *HeapAlloc(void *heap, u32 size);
extern void *HeapCreate(void *memory, u32 size);
extern void HeapFree(void *heap, void *allocation);
extern void CpuFill(void *destination, u32 size, u32 value);
extern void InitializePointerRecord(void **record, void *value);
extern char gScriptResultResourceName[];
extern char gScriptNameResourceName[];

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

/** Find a class/name pair in an explicitly supplied resource bucket array. */
AT("0007EB18") u8 *ScriptResourceTableFind(
    struct ScriptResourceNode **buckets, s32 type, const char *name)
{
    s32 bucket = ScriptResourceHash(type, name);
    struct ScriptResourceNode *node = buckets[bucket];

    while (node != 0) {
        if (type == (s8)node->typedName[0] &&
            strcmp(name, node->typedName + 1) == 0)
            return node->value;
        node = node->next;
    }
    return 0;
}

/** Insert a class/name pair using an explicitly supplied heap and resource
 * bucket array. */
AT("0007EB5C") s32 ScriptResourceTableSet(
    void *heap, struct ScriptResourceNode **buckets, s32 type,
    const char *name, const void *value, s32 size)
{
    s32 bucket = ScriptResourceHash(type, name);
    struct ScriptResourceNode *node;
    u32 nameLength;

    if (ScriptResourceTableFind(buckets, type, name) != 0)
        return 1;

    nameLength = strlen(name);
    node = HeapAlloc(heap, size + nameLength + 13);
    if (node == 0)
        return -1;

    CpuCopy(node->value, value, size);
    node->typedName = (char *)node + (size + 8);
    node->typedName[0] = type;
    strcpy(node->typedName + 1, name);
    node->next = buckets[bucket];
    buckets[bucket] = node;
    return 0;
}

/** Remove a class/name pair from an explicitly supplied bucket array and
 * release its node to the corresponding heap. */
AT("0007EBE0") s32 ScriptResourceTableRemove(
    void *heap, struct ScriptResourceNode **buckets, s32 type,
    const char *name)
{
    s32 bucket = ScriptResourceHash(type, name);
    struct ScriptResourceNode *node = buckets[bucket];
    struct ScriptResourceNode *previous = 0;

    if (node != 0) {
        do {
            if (type == (s8)node->typedName[0] &&
                strcmp(name, node->typedName + 1) == 0)
                goto found;
            previous = node;
            node = node->next;
        } while (node != 0);
    }
    return 1;

found:
    if (previous == 0)
        buckets[bucket] = node->next;
    else
        previous->next = node->next;
    HeapFree(heap, node);
    return 0;
}
AT("0007EBE0") const u8 ScriptResourceTableRemoveTail[2] = {0, 0};

/** Initialize the script execution state and its named-resource tables.
 * A caller may supply an existing bucket array; otherwise one is allocated
 * from resourceHeap and populated with the built-in resource definitions. */
AT("0007EEC4") s32 ScriptExecutionStateInitialize(
    void *resourceHeap, struct ScriptResourceNode **resourceBuckets,
    void *heapMemory, u32 heapSize)
{
    if ((gScriptBytecodeRoot->context->heap =
         HeapCreate(heapMemory, heapSize)) == 0)
        return -1;

    if (resourceBuckets == 0) {
        gScriptBytecodeRoot->context->resourceHeap = resourceHeap;
        gScriptBytecodeRoot->context->resourceBuckets =
            HeapAlloc(resourceHeap,
                      587 * sizeof(*resourceBuckets));
        if (gScriptBytecodeRoot->context->resourceBuckets == 0)
            return -1;
        ScriptResourceRegisterBuiltins(
            resourceHeap, gScriptBytecodeRoot->context->resourceBuckets);
    } else {
        gScriptBytecodeRoot->context->resourceBuckets = resourceBuckets;
        gScriptBytecodeRoot->context->resourceHeap = resourceHeap;
    }

    gScriptBytecodeRoot->context->vm = 0;
    gScriptBytecodeRoot->context->firstNamedResourceCount = 0;
    gScriptBytecodeRoot->context->secondNamedResourceCount = 0;
    CpuFill(gScriptBytecodeRoot->context->firstResources,
            sizeof(gScriptBytecodeRoot->context->firstResources), 0);
    CpuFill(gScriptBytecodeRoot->context->secondResources,
            sizeof(gScriptBytecodeRoot->context->secondResources), 0);
    gScriptBytecodeRoot->context->stepBudget = 10;
    InitializePointerRecord(
        (void **)&gScriptBytecodeRoot->context->result,
        gScriptResultResourceName);
    InitializePointerRecord(
        (void **)&gScriptBytecodeRoot->context->scriptName,
        gScriptNameResourceName);
    gScriptBytecodeRoot->context->pendingTasks = 0;
    return 0;
}
