/* Mount-table lifetime. The caller owns NfpState; only its mount array is
 * allocated/freed here. Init does not clear or validate the returned array.
 */
#include "nfp.h"
#include "heap.h"
#include "rom_section.h"
extern void CpuFill(void *, u32, u32);
extern void HeapFree(struct Heap *, void *);

AT("0007A9C0")
void NfpInit(struct NfpState *state, struct Heap *heap, s32 count)
{
    gNfpState = state;
    CpuFill(state, 12, 0);
    gNfpState->mounts = HeapAlloc(heap, count * 24);
    gNfpState->heap = heap;
    gNfpState->mount_count = count;
}

AT("0007A9F0")
void NfpShutdown(void)
{
    HeapFree(gNfpState->heap, gNfpState->mounts);
    CpuFill(gNfpState, 12, 0);
    gNfpState = 0;
}
