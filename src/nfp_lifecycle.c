/* Mount-table lifetime. The caller owns NfpState; only its mount array is
 * allocated/freed here. Init does not clear or validate the returned array.
 */
#include "nfp.h"
#include "heap.h"
#include "rom_section.h"
extern void CpuFill(void *, u32, u32);
extern void HeapFree(struct Heap *, void *);

/**
 * @brief Initialize the archive mount table.
 * @param state Filesystem state published as the active NFP state.
 * @param heap Heap used to allocate mount records.
 * @param count Number of mount records to allocate.
 */
AT("0007A9C0")
void NfpInit(struct NfpState *state, struct Heap *heap, s32 count)
{
    gNfpState = state;
    CpuFill(state, 12, 0);
    gNfpState->mounts = HeapAlloc(heap, count * 24);
    gNfpState->heap = heap;
    gNfpState->mount_count = count;
}

/** @brief Free the mount table and clear the active NFP state. */
AT("0007A9F0")
void NfpShutdown(void)
{
    HeapFree(gNfpState->heap, gNfpState->mounts);
    CpuFill(gNfpState, 12, 0);
    gNfpState = 0;
}
