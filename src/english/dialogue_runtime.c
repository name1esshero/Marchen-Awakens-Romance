/* Optional localization bridge. This file is excluded from the matching ROM.
 * Lookup uses the exact bytes passed to the recovered dialogue constructor,
 * so it works for both compressed scripts in RAM and constant ROM strings.
 * Ambiguous translations retain Japanese. Longer translations use a parent
 * task that keeps the script blocked while the original printer draws pages.
 * No VM offsets, resource names, or script command arguments are rewritten. */
#include "dialogue.h"
#include "english.h"
#include "input.h"

extern void *DialogueStartOriginal(s32, s32, const char **, s32 *);
extern void *CreateTask(void *, void *, u32, s32 *, u32);
extern void FinishTask(void *);
extern void ScriptAddPendingTasks(u32); /* Increment current script's pending count. */
extern void ScriptCompletePendingTasks(u32); /* Decrement current script's pending count. */
extern void CpuFill(void *, u32, u32);

/* All mutable storage belongs to engine tasks, not unmapped ROM-extension
 * BSS. The original task header occupies 32 bytes on the GBA. */
struct EnglishPages
{
    const struct EnglishRowMapping *entries[3];
    s32 *result;
    s32 childResult;
    s32 mode, count, entry, row, phase;
    u8 ink, shadow;
    u16 interval;
};

static void EnglishRememberStyle(struct EnglishPages *pages, const char *row)
{
    const u8 *text = (const u8 *)row;
    u32 argument;
    u8 command;
    /* Generated mappings contain validated leading/trailing ASCII controls
     * and double-byte glyphs. Style persists across source rows. */
    while (*text)
    {
        if (*text >= 0x80) { text += 2; continue; }
        command = *text++;
        if (command == 'C' || command == 'c')
        {
            text = DialogueReadHex4(text, &argument);
            pages->ink = argument >> 8;
            pages->shadow = argument;
        }
        else if (command == 'T' || command == 't')
        {
            text = DialogueReadHex4(text, &argument);
            pages->interval = argument;
        }
    }
}

static void EnglishClearPage(void *task)
{
    struct EnglishPages *pages = *(struct EnglishPages **)((u8 *)task + 32);
    u32 column;
    /* Same surface/pattern as 08011A60, executed by the VRAM task manager.
     * Mode 1 reserves the first text row. Preserve its 11 scanlines, including
     * shadow: tile rows contain 4 bytes each, with 24 tiles across the surface. */
    if (pages->mode == 1)
    {
        for (column = 0; column < 24; column++)
            CpuFill((u8 *)0x0600C020 + 768 + column * 32 + 12,
                    20, 0x11111111);
        CpuFill((void *)(0x0600C020 + 1536), 1536, 0x11111111);
    }
    else CpuFill((void *)0x0600C020, 3072, 0x11111111);
    pages->childResult = -1;
    FinishTask(task);
}

static s32 EnglishRowHasGlyph(const char *row)
{
    const u8 *text = (const u8 *)row;
    /* This printer skips ASCII, including C/T controls. Generated printable
     * letters use double-byte font codes with a high-bit first byte. */
    while (*text) if (*text++ >= 0x80) return 1;
    return 0;
}

static void EnglishSkipEmptyPages(struct EnglishPages *pages)
{
    s32 entry, row, used, visible;
    while (pages->entry < pages->count)
    {
        entry = pages->entry; row = pages->row; used = visible = 0;
        while (entry < pages->count && used < 3 - pages->mode)
        {
            if (EnglishRowHasGlyph(pages->entries[entry]->rows[row])) visible = 1;
            used++; row++;
            if (row == pages->entries[entry]->count) { entry++; row = 0; }
        }
        if (visible) return;
        /* Consume controls in skipped pages before moving the cursor. */
        while (pages->entry < entry || (pages->entry == entry && pages->row < row))
        {
            EnglishRememberStyle(pages, pages->entries[pages->entry]->rows[pages->row++]);
            if (pages->row == pages->entries[pages->entry]->count)
            { pages->entry++; pages->row = 0; }
        }
    }
}

/* Blank padding must not create a final page: the original printer's
 * initial no-glyph branch enters state 0x100 without completing its task. */
static s32 EnglishHasRemainingText(const struct EnglishPages *pages)
{
    s32 entry, row;
    for (entry = pages->entry; entry < pages->count; entry++)
        for (row = entry == pages->entry ? pages->row : 0;
             row < pages->entries[entry]->count; row++)
            if (EnglishRowHasGlyph(pages->entries[entry]->rows[row])) return 1;
    return 0;
}

static void EnglishPageTask(void *task)
{
    struct EnglishPages *pages = (struct EnglishPages *)((u8 *)task + 32);
    const char *rows[3];
    s32 entry, row, used, i;
    void *clear, *child;
    struct DialogueState *state;
    if (pages->phase == 1)
    {
        if (pages->childResult != -1) return;
        EnglishSkipEmptyPages(pages);
        if (!EnglishHasRemainingText(pages))
        {
            /* Final-page dismissal remains the script's original command. */
            if (pages->result) *pages->result = -1;
            ScriptCompletePendingTasks(1);
            FinishTask(task);
            return;
        }
        pages->phase = 2;
        return; /* Never reuse the press that fast-forwarded the printer. */
    }
    if (pages->phase == 2)
    {
        if (!KeyInputConsumePressed(1, 0)) return;
        pages->phase = 3;
    }
    if (pages->phase == 3)
    {
        clear = CreateTask((void *)0x030032D4, EnglishClearPage, 0, 0,
                           sizeof(struct EnglishPages *));
        if (!clear) return; /* Retry allocation without dropping any text. */
        pages->childResult = 0;
        *(struct EnglishPages **)((u8 *)clear + 32) = pages;
        pages->phase = 4;
        return;
    }
    if (pages->phase == 4 && pages->childResult != -1) return;
    pages->phase = 0;
    EnglishSkipEmptyPages(pages);
    entry = pages->entry;
    row = pages->row;
    used = 0;
    while (entry < pages->count && used < 3 - pages->mode)
    {
        rows[used++] = pages->entries[entry]->rows[row++];
        if (row == pages->entries[entry]->count) { entry++; row = 0; }
    }
    /* Explicit blank lines can also occupy a whole intermediate page.
     * Advance without creating a child that cannot signal completion. */
    for (i = 0; i < used && !EnglishRowHasGlyph(rows[i]); i++) {}
    if (i == used)
    {
        pages->entry = entry;
        pages->row = row;
        pages->childResult = -1;
        pages->phase = 1;
        return;
    }
    pages->childResult = 0;
    child = DialogueStartOriginal(pages->mode, used, rows, &pages->childResult);
    if (!child)
        return; /* Keep the previous cursor until allocation succeeds. */
    state = (struct DialogueState *)((u8 *)child + 32);
    state->ink = pages->ink;
    state->shadow = pages->shadow;
    state->countdown = pages->interval;
    if (pages->entry || pages->row) state->delay = pages->interval;
    for (i = 0; i < used; i++) EnglishRememberStyle(pages, rows[i]);
    pages->entry = entry;
    pages->row = row;
    pages->phase = 1;
}

static s32 CompareRow(const char *left, const char *right)
{
    u32 i;
    for (i = 0; i < 161; i++)
    {
        u32 a = (u8)left[i];
        u32 b = (u8)right[i];
        if (a != b) return (s32)a - (s32)b;
        if (!a) return 0;
    }
    return 1; /* Original row buffer is only 161 bytes. */
}

static const struct EnglishRowMapping *FindRow(const char *source)
{
    u32 low = 0, high = gEnglishRowCount;
    while (low < high)
    {
        u32 middle = low + ((high - low) >> 1);
        s32 comparison = CompareRow(source, gEnglishRows[middle].japanese);
        if (!comparison) return &gEnglishRows[middle];
        if (comparison < 0) high = middle;
        else low = middle + 1;
    }
    return 0;
}

s32 EnglishTranslateRows(s32 mode, s32 count, const char **source, const char **output)
{
    s32 i, j, used = 0;
    s32 capacity;
    if (mode != 0 && mode != 1) return 0;
    capacity = 3 - mode;
    if (count < 1 || count > capacity || !source || !output) return 0;
    for (i = 0; i < count; i++)
    {
        const struct EnglishRowMapping *entry;
        if (!source[i]) return 0;
        entry = FindRow(source[i]);
        if (!entry || !entry->count || used + entry->count > capacity) return 0;
        for (j = 0; j < entry->count; j++) output[used++] = entry->rows[j];
    }
    return used;
}

__attribute__((section(".english.entry")))
void *EnglishDialogueStart(s32 mode, s32 count, const char **rows, s32 *result)
{
    const char *translated[3];
    const struct EnglishRowMapping *entries[3];
    struct EnglishPages *pages;
    void *task;
    s32 i;
    s32 translatedCount = EnglishTranslateRows(mode, count, rows, translated);
    if (translatedCount)
    {
        for (i = 0; i < translatedCount; i++)
            if (EnglishRowHasGlyph(translated[i]))
                return DialogueStartOriginal(mode, translatedCount, translated, result);
        /* Even an entirely omitted translation must complete asynchronously
         * through our parent task, not the original no-glyph dead end. */
    }
    if ((mode == 0 || mode == 1) && count > 0 && count <= 3 - mode && rows)
    {
        for (i = 0; i < count; i++)
        {
            if (!rows[i]) break;
            entries[i] = FindRow(rows[i]);
            if (!entries[i] || !entries[i]->count) break;
        }
        if (i == count)
        {
            task = CreateTask((void *)0x030032C4, EnglishPageTask, 0, result,
                              sizeof(struct EnglishPages));
            if (task)
            {
                pages = (struct EnglishPages *)((u8 *)task + 32);
                for (i = 0; i < count; i++) pages->entries[i] = entries[i];
                pages->mode = mode;
                pages->count = count;
                pages->result = result;
                pages->ink = 15;
                pages->shadow = 4;
                pages->interval = 2;
                ScriptAddPendingTasks(1);
                return task;
            }
        }
    }
    return DialogueStartOriginal(mode, count, rows, result);
}
