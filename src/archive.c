/* Background archive loading.
 *
 * ArchiveTaskStep spans 0x08027DE0..0x08028008. The blocks formerly named
 * LoadArchiveEntry (0x08027EAE) and sub_08027F02 are branches inside this
 * function: neither has a prologue, an independent return, or a caller.
 *
 * The complete task state machine is reconstructed below.  Direct indexed
 * entry accesses preserve the original lifetime boundaries across allocation
 * and transfer calls; no register hints or inline assembly are needed.
 */

#include "archive.h"
#include "gba/types.h"
#include "heap.h"
#include "rom_section.h"
#include "runtime_misc.h"
#include "task_adapters.h"
#include "task_manager.h"

#define ARCHIVE_STATUS_STEP_READY (1 << 0)
#define ARCHIVE_STATUS_FINISHED   (1 << 12)
#define ARCHIVE_STATUS_NO_MEMORY  (1 << 15)
#define ARCHIVE_STATUS_STOP_MASK  \
    (ARCHIVE_STATUS_FINISHED | ARCHIVE_STATUS_NO_MEMORY)

#define ARCHIVE_FLAG_MAP_64X64      (1 << 0)
#define ARCHIVE_FLAG_TILES_8BPP     (1 << 1)
#define ARCHIVE_FLAG_SHARED_PALETTE (1 << 2)

#define ARCHIVE_STATE_DECODE 1
#define ARCHIVE_STATE_COMMIT 5

extern u8 gIwramBase[];

extern void CpuCopy(void *destination, const void *source, u32 size);
extern void RLUnCompWram(const void *source, void *destination);
extern void LZ77UnCompWram(const void *source, void *destination);

/* These helpers apply a decoded 64x64 or 32x32 tile map to the render
 * context. Their remaining parameter names stay conservative until their
 * own bodies have been reconstructed. */
extern void sub_0800224C(void *context, void *scratch, u32 width, u32 height,
                         const void *map, u32 value02, u32 value04);
extern void sub_080022CC(void *context, void *scratch, u32 width, u32 height,
                         const void *map, u32 value02, u32 value04);

/** Advance the asynchronous background archive loader by one task stage. */
AT("00027DE0") void ArchiveTaskStep(struct ArchiveTask *task)
{
    struct ArchiveLoader *loader;
    u32 stage;

    loader = &task->loader;

    if (*loader->status & ARCHIVE_STATUS_STOP_MASK)
    {
        if (loader->buffer != NULL)
        {
            u8 *iwram = gIwramBase;
            u32 heapOffset = OBJECT_HEAP_ROOT_OFFSET;
            struct Heap *heap = *(struct Heap **)(iwram + heapOffset);

            HeapFree(heap, loader->buffer);
            loader->buffer = NULL;
        }
        if (loader->scratch != NULL)
        {
            u8 *iwram = gIwramBase;
            u32 heapOffset = OBJECT_HEAP_ROOT_OFFSET;
            struct Heap *heap = *(struct Heap **)(iwram + heapOffset);

            HeapFree(heap, loader->scratch);
            loader->scratch = NULL;
        }
        if (task->task.completion != NULL)
            *task->task.completion = -1;

        FinishTask(&task->task);
        return;
    }

    if (!(*loader->status & ARCHIVE_STATUS_STEP_READY))
        return;

    stage = task->task.stage;
    switch (stage)
    {
    case ARCHIVE_STATE_DECODE:
        if (loader->index < loader->header->entryCount)
        {
            if (loader->buffer != NULL)
            {
                u8 *iwram = gIwramBase;
                u32 heapOffset = OBJECT_HEAP_ROOT_OFFSET;
                struct Heap *heap = *(struct Heap **)(iwram + heapOffset);

                HeapFree(heap, loader->buffer);
                loader->buffer = NULL;
            }

            if (loader->header->flags & ARCHIVE_FLAG_TILES_8BPP)
                loader->size = loader->entries[loader->index].tileCount * 64;
            else
                loader->size = loader->entries[loader->index].tileCount * 32;

            {
                u8 *iwram = gIwramBase;
                u32 heapOffset = OBJECT_HEAP_ROOT_OFFSET;
                struct Heap *heap = *(struct Heap **)(iwram + heapOffset);

                loader->buffer = HeapAlloc(heap, loader->size);
            }
            if (loader->buffer == NULL)
            {
                *loader->status |= ARCHIVE_STATUS_NO_MEMORY;
                return;
            }

            switch (loader->entries[loader->index].type)
            {
            case ARCHIVE_RAW:
                CpuCopy(loader->buffer,
                        loader->base
                            + loader->entries[loader->index].offset,
                        loader->size);
                break;
            case ARCHIVE_RLE:
                RLUnCompWram(loader->base
                                 + loader->entries[loader->index].offset,
                             loader->buffer);
                break;
            case ARCHIVE_LZ77:
                LZ77UnCompWram(loader->base
                                   + loader->entries[loader->index].offset,
                               loader->buffer);
                break;
            }
        }
        task->task.stage++;
        return;

    default:
        task->task.stage = stage + 1;
        return;

    case ARCHIVE_STATE_COMMIT:
        if (loader->index < loader->header->entryCount)
        {
            if (loader->header->flags & ARCHIVE_FLAG_MAP_64X64)
            {
                u8 *iwram = gIwramBase;
                u32 contextOffset = ARCHIVE_RENDER_CONTEXT_OFFSET;
                void *context = *(void **)(iwram + contextOffset);

                sub_0800224C(context, loader->scratch, 64, 64,
                             loader->base
                                 + loader->entries[loader->index].mapOffset,
                             loader->entries[loader->index].value02,
                             loader->entries[loader->index].value04);
            }
            else
            {
                u8 *iwram = gIwramBase;
                u32 contextOffset = ARCHIVE_RENDER_CONTEXT_OFFSET;
                void *context = *(void **)(iwram + contextOffset);

                sub_080022CC(context, loader->scratch, 32, 32,
                             loader->base
                                 + loader->entries[loader->index].mapOffset,
                             loader->entries[loader->index].value02,
                             loader->entries[loader->index].value04);
            }

            CreateCopyTask(loader->tileDestination, loader->buffer,
                           loader->size);
            if (!(loader->header->flags & ARCHIVE_FLAG_SHARED_PALETTE))
            {
                CpuCopy(RuntimeGetBlock6120(0, 0),
                        loader->base
                            + loader->entries[loader->index].paletteOffset,
                        loader->header->paletteColorCount * sizeof(u16));
            }

            loader->index++;
            task->task.stage = 0;
        }
        else
        {
            *loader->status |= ARCHIVE_STATUS_FINISHED;
        }
        return;
    }
}
