/* Lazy name-backed handles used by the script VM's two resource classes. */
#include "gba/types.h"
#include "script_bytecode.h"

#include "rom_section.h"

extern s32 ScriptResourceNameFirst(const char *name);
extern s32 ScriptResourceNameSecond(const char *name);
extern void *ScriptResourceSlotFirst(s32 index);
extern void *ScriptResourceSlotSecond(s32 index);
extern s32 ScriptResourceGetValue(s32 record, s32 selector);
extern s32 ScriptResourceGetValueOrDefault(s32 record, s32 selector);
extern s32 ScriptResourceSetValue(s32 record, s32 selector, u32 value);
extern s32 ScriptResourceSetStringValue(s32 *record, s32 selector,
                                        const char *value);

AT("0007F438") s32 ScriptResourceGetFirst(u8 *record, s32 selector)
{
    void *slot;

    if (!record)
        return 0;
    slot = *(void **)(record + 4);
    if (!slot) {
        slot = ScriptResourceSlotFirst(
            ScriptResourceNameFirst(*(const char **)record));
        *(void **)(record + 4) = slot;
        if (!slot)
            return -1;
    }
    return ScriptResourceGetValue(*(s32 **)(record + 4), selector);
}

AT("0007F470") s32 ScriptResourceGetSecond(u8 *record, s32 selector)
{
    void *slot;

    if (!record)
        return 0;
    slot = *(void **)(record + 4);
    if (!slot) {
        slot = ScriptResourceSlotSecond(
            ScriptResourceNameSecond(*(const char **)record));
        *(void **)(record + 4) = slot;
        if (!slot)
            return 0;
    }
    return ScriptResourceGetValueOrDefault(*(s32 **)(record + 4), selector);
}
AT("0007F470") const u8 ScriptResourceGetSecondTail[2] = {0};

AT("0007F4A4") s32 ScriptResourceSetFirst(u8 *record, s32 selector,
                                           u32 value)
{
    void *slot;

    if (!record)
        return 0;
    slot = *(void **)(record + 4);
    if (!slot) {
        slot = ScriptResourceSlotFirst(
            ScriptResourceNameFirst(*(const char **)record));
        *(void **)(record + 4) = slot;
        if (!slot)
            return 0;
    }
    return ScriptResourceSetValue(*(s32 **)(record + 4), selector, value);
}
AT("0007F4A4") const u8 ScriptResourceSetFirstTail[2] = {0};

AT("0007F4DC") s32 ScriptResourceSetSecond(u8 *record, s32 selector,
                                            u32 value)
{
    void *slot;

    if (!record)
        return 0;
    slot = *(void **)(record + 4);
    if (!slot) {
        slot = ScriptResourceSlotSecond(
            ScriptResourceNameSecond(*(const char **)record));
        *(void **)(record + 4) = slot;
        if (!slot)
            return -1;
    }
    return ScriptResourceSetStringValue(*(s32 **)(record + 4), selector,
                                       (const char *)value);
}

/* Offset of the VM context's own first-class resource handle. */
#define SCRIPT_CONTEXT_RECORD_224 548

/* Store a value in the VM context's built-in resource handle.  Nothing in the
 * already-disassembled ROM branches here, so the original disassembly left
 * these bytes as data; the argument convention comes from the tail call. */
AT("0007F1B8") void ScriptResourceSetRecord224(u32 value)
{
    u8 *context = (u8 *)gScriptBytecodeRoot->context;

    ScriptResourceSetFirst(context + SCRIPT_CONTEXT_RECORD_224, 0, value);
}
