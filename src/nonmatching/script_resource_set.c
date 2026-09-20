/* Natural-C candidate for ScriptResourceSet (0x0807E9F4).
 *
 * This candidate is nonmatching: typed bucket indexing emits the same operations but reverses the r0/r1
 * temporaries used for the bucket base and scaled index. Casting the bucket
 * pointer through u32 restores those registers, but that spelling only steers
 * code generation. The exact routine therefore remains in assembly.
 */
#include "gba/types.h"
#include "script_bytecode.h"

struct ScriptResourceNode
{
    struct ScriptResourceNode *next;
    char *typedName;
    u8 value[1];
};

struct ScriptResourceTable
{
    u8 unknown00[8];
    struct ScriptResourceNode **buckets;
};

extern s32 ScriptResourceHash(s32 type, const char *name);
extern u8 *ScriptResourceFind(s32 type, const char *name);
extern u32 strlen(const char *text);
extern char *strcpy(char *destination, const char *source);
extern void CpuCopy(void *destination, const void *source, u32 size);
extern void *HeapAlloc(void *heap, u32 size);

s32 ScriptResourceSetCandidate(s32 type, const char *name,
                               const void *value, s32 size)
{
    s32 bucket = ScriptResourceHash(type, name);
    struct ScriptResourceNode *node;
    struct ScriptResourceTable *table;
    u32 nameLength;

    if (ScriptResourceFind(type, name) != 0)
        return 1;
    nameLength = strlen(name);
    table = (struct ScriptResourceTable *)gScriptBytecodeRoot->context;
    node = HeapAlloc(*(void **)((u8 *)table + 4), size + nameLength + 13);
    if (node == 0)
        return -1;
    CpuCopy(node->value, value, size);
    node->typedName = (char *)node + size + 8;
    node->typedName[0] = type;
    strcpy(node->typedName + 1, name);
    node->next = table->buckets[bucket];
    table->buckets[bucket] = node;
    return 0;
}
