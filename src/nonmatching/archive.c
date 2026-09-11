/* Asset archive loading.
 *
 * Decompiled from 0x08027EAE. This is the routine every asset in the game
 * passes through, and it explains the structure of the whole cartridge:
 * nothing is addressed by ROM pointer. A caller names an archive and an
 * index; the loader reads that archive's own 20-byte entry table, adds the
 * entry's relative offset to the archive base, and unpacks the payload by the
 * method the entry names.
 *
 * Not yet byte-matching: the logic is right, but the original compiler's
 * register allocation differs. See src/nonmatching/misc.c for why.
 */

#include "gba/types.h"
#include "gba/bios.h"
#include "archive.h"

/* The game's heap handle lives at a fixed spot in IWRAM. */
#define gHeapHandle (*(void **)0x03003FB4)

extern void *HeapAlloc(void *heap, u32 size);            /* sub_0807A2EC */
extern void CpuCopy(void *dest, const void *src, u32 n); /* sub_08001EB4 */
extern void RLUnCompWram(const void *src, void *dest);
extern void LZ77UnCompWram(const void *src, void *dest);

/* Allocate room for one asset and unpack it.
 *
 * On success the buffer is stored in the loader state. On failure the error
 * bit is set in the caller's status word and nothing is unpacked.
 */
void LoadArchiveEntry(struct ArchiveLoader *loader)
{
    struct ArchiveEntry *entry;
    const u8 *src;
    void *dest;

    dest = HeapAlloc(gHeapHandle, loader->size);
    loader->buffer = dest;

    if (dest == NULL)
    {
        *loader->status |= ARCHIVE_ERR_NO_MEMORY;
        return;
    }

    entry = &loader->entries[loader->index];
    src = loader->base + entry->offset;

    switch (entry->type)
    {
    case ARCHIVE_RAW:
        CpuCopy(dest, src, loader->size);
        break;
    case ARCHIVE_RLE:
        RLUnCompWram(src, dest);
        break;
    case ARCHIVE_LZ77:
        LZ77UnCompWram(src, dest);
        break;
    default:
        /* the original falls through without unpacking */
        break;
    }
}

extern void HeapFree(void *heap, void *ptr);    /* sub_0807A42C */
extern void FinishTask(struct ArchiveTask *t);  /* sub_0807A7AC */

/* One step of the asset-loading task, from 0x08027DE0.
 *
 * Loading runs across frames, so a caller can cancel it and an allocation
 * failure has to unwind cleanly. This is the head of that step: if either
 * abort bit is set, release whatever has been allocated, report failure to
 * the caller, and end the task. Otherwise the step falls through into the
 * state machine that eventually reaches LoadArchiveEntry above.
 */
void ArchiveTaskStep(struct ArchiveTask *task)
{
    struct ArchiveLoader *loader = &task->loader;

    if (*loader->status & ARCHIVE_ABORT_MASK)
    {
        if (loader->buffer != NULL)
        {
            HeapFree(gHeapHandle, loader->buffer);
            loader->buffer = NULL;
        }
        if (loader->scratch != NULL)
        {
            HeapFree(gHeapHandle, loader->scratch);
            loader->scratch = NULL;
        }
        if (task->result != NULL)
            *task->result = -1;

        FinishTask(task);
        return;
    }

    /* ... state machine continues; not yet decompiled ... */
}
