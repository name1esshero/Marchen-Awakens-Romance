/* Name-based and index-based convenience entry points layered over the
 * mounted-archive primitives. Invalid archive names return zero. */
#include "nfp.h"

#include "rom_section.h"

/** @return An archive's entry count, looked up by name; 0 if not mounted. */
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

/** @return A pointer to an archive entry's data, looked up by archive name
 * and entry index; NULL if the archive isn't mounted or the index is
 * invalid. */
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

/** @return A named entry's index within an archive looked up by name; 0 if
 * the archive isn't mounted. */
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
