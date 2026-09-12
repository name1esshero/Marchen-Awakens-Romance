/* The named ROM filesystem.
 *
 * Named graphics, maps and scripts are reached through this. The cartridge holds one
 * NFP2.0 archive covering 83.7% of the image, 830 members with real names and
 * exact offsets, Audio instead uses separate song and voice tables with direct ROM pointers.
 *
 * Recovered from the accessors at 0x0807AAA0, 0x0807AAD0, 0x0807AAE0 and
 * 0x0807AB08. The header fields they read, +0x34, +0x38 and +0x3C, are
 * exactly the count, directory and data offsets the extraction tooling uses,
 * so the C and the tools now agree on one documented structure.
 *
 * Each accessor whose body is not a multiple of four bytes is followed by a
 * two-byte alignment tail declared in the same section. Left to itself the
 * assembler would pad with a THUMB nop, where the cartridge holds zeros.
 */

#include "gba/types.h"
#include "nfp.h"

/* Base of a mounted archive.
 *
 * The global holds a pointer to the filesystem state, whose first field is
 * the mount array, so there are two indirections before the index. Mount
 * records are 24 bytes, which the original builds as ((i * 2) + i) * 8.
 */
__attribute__((section(".rom.0007AAA0")))
struct NfpHeader *NfpGetArchiveBase(s32 handle)
{
    return gNfpState->mounts[handle].base;
}

/* The directory: one 16-byte record per member, a 12-byte name and the
 * payload's offset relative to the header. */
__attribute__((section(".rom.0007AAD0")))
struct NfpEntry *NfpGetDirectory(s32 handle)
{
    struct NfpHeader *header = NfpGetArchiveBase(handle);

    return (struct NfpEntry *)((u8 *)header + header->table_offset);
}

__attribute__((section(".rom.0007AAD0")))
const u8 NfpGetDirectoryTail[2] = {0, 0};

/* Where the payloads begin, past the header and the directory. */
__attribute__((section(".rom.0007AAE0")))
void *NfpGetData(s32 handle)
{
    struct NfpHeader *header = NfpGetArchiveBase(handle);

    return (u8 *)header + header->data_offset;
}

__attribute__((section(".rom.0007AAE0")))
const u8 NfpGetDataTail[2] = {0, 0};

/* How many members the archive holds. 830 for this cartridge. */
__attribute__((section(".rom.0007AB08")))
u32 NfpGetEntryCount(s32 handle)
{
    return NfpGetArchiveBase(handle)->count;
}

/* Mark a mount slot in use, or free it. Counterpart to NfpMountIsActive. */
__attribute__((section(".rom.0007AA30")))
void NfpSetMountActive(s32 handle, s32 active)
{
    gNfpState->mounts[handle].active = active;
}

/* A mount's name, or NULL if the slot is free. The name sits immediately
 * after the active flag, so the record is one byte of state followed by the
 * string the caller will match against. */
/* Resolve an archive name to its handle, or -1 if it is not mounted.
 *
 * A linear scan, unlike the member lookup: there are only a handful of mount
 * slots and they are not kept sorted, so there is nothing to binary search. */
__attribute__((section(".rom.0007AB7C")))
s32 NfpFindArchive(const char *name)
{
    s32 handle;

    for (handle = 0; handle < gNfpState->mount_count; handle++)
    {
        if (!NfpMountIsActive(handle))
            continue;

        if (strcmp(NfpGetMountName(handle), name) == 0)
            return handle;
    }

    return -1;
}

/* Point a mount slot at its archive header. */
__attribute__((section(".rom.0007AAB8")))
void NfpSetArchiveBase(s32 handle, struct NfpHeader *base)
{
    gNfpState->mounts[handle].base = base;
}

/* First unused mount slot, or -1 when they are all taken. */
__attribute__((section(".rom.0007ABF0")))
s32 NfpFindFreeSlot(void)
{
    s32 handle;

    for (handle = 0; handle < gNfpState->mount_count; handle++)
    {
        if (!NfpMountIsActive(handle))
            return handle;
    }

    return -1;
}

/* Mount an archive under a name, returning its handle.
 *
 * Mounting is idempotent: if the name is already mounted its existing handle
 * comes straight back, so callers can mount on demand without tracking state.
 * Otherwise a free slot is claimed and filled in. The size recorded is simply
 * the span the caller passed, which is how the archive knows its own extent
 * without storing a length.
 *
 * This is what the boot code calls to register the cartridge's one archive.
 */
__attribute__((section(".rom.0007AB14")))
s32 NfpMount(const char *name, void *base, void *end)
{
    s32 handle;

    handle = NfpFindArchive(name);
    if (handle >= 0)
        return handle;

    handle = NfpFindFreeSlot();
    if (handle < 0)
        return -1;

    NfpSetMountActive(handle, -1);
    NfpSetArchiveBase(handle, base);
    NfpSetMountName(handle, name);
    gNfpState->mounts[handle].size = (u8 *)end - (u8 *)base;

    return handle;
}

/* Alignment tail, so the section ends with zeros rather than a THUMB nop. */
__attribute__((section(".rom.0007AB14")))
const u8 NfpMountTail[2] = {0, 0};

void sub_0807AAB8(s32, struct NfpHeader *) __attribute__((alias("NfpSetArchiveBase")));
s32 sub_0807ABF0(void) __attribute__((alias("NfpFindFreeSlot")));
s32 sub_0807AB14(const char *, void *, void *) __attribute__((alias("NfpMount")));

/* Names the remaining assembly still calls these by. */
struct NfpHeader *sub_0807AAA0(s32) __attribute__((alias("NfpGetArchiveBase")));
struct NfpEntry *sub_0807AAD0(s32) __attribute__((alias("NfpGetDirectory")));
void *sub_0807AAE0(s32) __attribute__((alias("NfpGetData")));
u32 sub_0807AB08(s32) __attribute__((alias("NfpGetEntryCount")));
s32 sub_0807AA18(s32) __attribute__((alias("NfpMountIsActive")));
struct NfpEntry *sub_0807AC28(s32, s32) __attribute__((alias("NfpGetEntry")));
void *sub_0807AC3C(const char *, const char *) __attribute__((alias("NfpOpenByName")));

void sub_0807AA30(s32, s32) __attribute__((alias("NfpSetMountActive")));
s32 sub_0807AB7C(const char *) __attribute__((alias("NfpFindArchive")));

/* Still assembly: see src/nonmatching/nfp_lookup.c for why. */
extern s32 NfpFindEntryIndex(s32, const char *) __attribute__((alias("sub_0807ACC4")));

/* Whether a mount slot is in use. The flag is the first byte of the record.
 *
 * Returns int rather than u8 deliberately: ldrb already zero-extends, so a
 * narrower return type makes the compiler re-widen the value at every call
 * site with an "lsls r0, r0, #24" the original does not have. */
__attribute__((section(".rom.0007AA18")))
s32 NfpMountIsActive(s32 handle)
{
    return gNfpState->mounts[handle].active;
}

/* One directory record by index. Records are 16 bytes, hence the shift. */
__attribute__((section(".rom.0007AC28")))
struct NfpEntry *NfpGetEntry(s32 handle, s32 index)
{
    return &NfpGetDirectory(handle)[index];
}

__attribute__((section(".rom.0007AC28")))
const u8 NfpGetEntryTail[2] = {0, 0};

/* Resolve "archive", "member" to the member's bytes.
 *
 * This is the front door of the named filesystem: callers request a member
 * by name. Audio resources follow a separate direct-pointer path. The
 * directory is sorted by name, so the index lookup is a binary search.
 *
 * Returns NULL if the archive is not mounted, the member does not exist, or
 * the directory record cannot be reached.
 */
__attribute__((section(".rom.0007AC3C")))
void *NfpOpenByName(const char *archive, const char *member)
{
    s32 handle;
    s32 index;
    struct NfpEntry *entry;

    handle = NfpFindArchive(archive);
    if (handle < 0)
        return NULL;

    index = NfpFindEntryIndex(handle, member);
    if (index < 0)
        return NULL;

    entry = NfpGetEntry(handle, index);
    if (entry == NULL)
        return NULL;

    return (u8 *)NfpGetArchiveBase(handle) + entry->offset;
}
