#ifndef NFP_H
#define NFP_H

#include "gba/types.h"

/* The named ROM filesystem.
 *
 * The cartridge carries one NFP2.0 archive spanning 83.7% of the image: 830
 * members with real filenames, exact offsets and exact lengths. Named graphics, maps and scripts use this filesystem. Sound data instead
 * uses direct ROM pointers through separate song and voice tables.
 *
 * The header begins with an ASCII signature naming the format and its vendor,
 * followed by padding, then the three fields the accessors below read. */

#define NFP_SIGNATURE "NFP2.0"

struct NfpHeader
{
    char signature[0x34];   /* 0x00: format and vendor string, NUL padded */
    u32 count;              /* 0x34: number of directory entries */
    u32 table_offset;       /* 0x38: directory, relative to the header */
    u32 data_offset;        /* 0x3C: first payload, relative to the header */
};

/* A directory entry: a 12-byte name and the payload's relative offset.
 * Lengths are implied by where the next payload begins, so a member's size
 * includes any alignment padding after it. */
struct NfpEntry
{
    char name[12];          /* 0x00: NUL padded, e.g. "FONT.NFT" */
    u32 offset;             /* 0x0C: payload, relative to the header */
};

/* Archives are mounted into a table of 24-byte records, indexed by handle,
 * so more than one can be open at a time. Only the base pointer is
 * identified; the rest of the record is not yet known. */
struct NfpMount
{
    u8 active;                  /* 0x00: slot in use */
    char name[15];              /* 0x01: the mount's name, NUL terminated */
    struct NfpHeader *base;     /* 0x10 */
    u32 size;                   /* 0x14: end - base, set when mounted */
};

/* The filesystem's own state, reached through a pointer in IWRAM. The
 * accessors load the global, dereference it, and only then index the mount
 * table, so the mount array is the first thing it holds. */
struct NfpState
{
    struct NfpMount *mounts;    /* 0x00 */
    u8 filler_04[4];
    s32 mount_count;            /* 0x08: slots to search when resolving a name */
};

#define gNfpState (*(struct NfpState **)0x03006114)

/* Directory entries are sorted by name, which is why lookup binary searches
 * them rather than scanning. Verified against the shipped directory. */
struct NfpHeader *NfpGetArchiveBase(s32 handle);
s32 NfpMountIsActive(s32 handle);
void NfpSetMountActive(s32 handle, s32 active);
void NfpSetArchiveBase(s32 handle, struct NfpHeader *base);
s32 NfpFindFreeSlot(void);
s32 NfpMount(const char *name, void *base, void *end);
const char *NfpGetMountName(s32 handle);
struct NfpEntry *NfpGetEntry(s32 handle, s32 index);
void *NfpOpenByName(const char *archive, const char *member);

s32 NfpFindArchive(const char *name);

s32 NfpFindEntryIndex(s32 handle, const char *name);

extern int strcmp(const char *a, const char *b);
extern void NfpSetMountName(s32 handle, const char *name);
extern void CpuCopy(void *dest, const void *src, u32 size);

/* Directory names are a fixed 12 bytes and are not NUL terminated when they
 * fill the field, so lookup copies them into a 13-byte buffer and terminates
 * the copy itself. */
#define NFP_NAME_SIZE 12
u32 NfpGetEntryCount(s32 handle);
struct NfpEntry *NfpGetDirectory(s32 handle);
void *NfpGetData(s32 handle);

#endif /* NFP_H */
