/* Name-based and index-based convenience entry points layered over the
 * mounted-archive primitives. Invalid archive names return zero. */
#include "nfp.h"

#include "rom_section.h"

AT("0007AAF0")
u32 NfpGetEntryCountByName(const char *archive)
{
    s32 handle = NfpFindArchive(archive);

    if (handle >= 0)
        handle = NfpGetArchiveBase(handle)->count;
    else
        handle = 0;
    return handle;
}

AT("0007AC78")
void *NfpOpenByIndex(const char *archive, s32 index)
{
    s32 handle = NfpFindArchive(archive);
    struct NfpEntry *entry;

    if (handle < 0)
        return 0;
    entry = NfpGetEntry(handle, index);
    if (entry == 0)
        return 0;
    return (u8 *)NfpGetArchiveBase(handle) + entry->offset;
}

AT("0007ACA8")
s32 NfpFindEntryByArchiveName(const char *archive, const char *member)
{
    s32 handle = NfpFindArchive(archive);

    if (handle >= 0)
        handle = NfpFindEntryIndex(handle, member);
    else
        handle = 0;
    return handle;
}
