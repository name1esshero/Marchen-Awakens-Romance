#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "gba/types.h"

/* How the game stores its assets.
 *
 * Recovered from the loader at 0x08027EAE. This background loader uses indices into its entry table and offsets
 * relative to its resource base. This is not a rule for the whole game:
 * the sound driver uses direct ROM pointers.
 *
 * Entries are 20 bytes. Only the two fields the loader reads are identified;
 * the rest are named by offset until their use is found. */
struct ArchiveEntry
{
    u16 type;               /* 0x00: how the payload is stored, see below */
    u16 unk_02;
    u16 unk_04;
    u16 tileCount;          /* 0x06: count multiplied by 32 or 64 at 08027E76 */
    u32 mapOffset;          /* 0x08: relative map source, used at 08027F30 */
    u32 offset;             /* 0x0C: payload start, relative to archive base */
    u32 paletteOffset;      /* 0x10: relative palette source, used at 08027FC8 */
};

/* Background resource header, NOT the outer NFP filesystem header.
 * 08027C52 stores the resource base here; +0x0C resolves the entry table. */
struct ArchiveHeader
{
    u16 entryCount;
    u16 paletteColorCount;
    u16 flags;             /* bit 1: 8bpp tiles; bit 2: shared palette */
    u16 unk_06;
    u32 paletteOffset;
    u32 entriesOffset;
};

/* Values of ArchiveEntry.type, as dispatched by LoadArchiveEntry. */
#define ARCHIVE_RAW   0     /* copied verbatim */
#define ARCHIVE_RLE   1     /* BIOS run-length, RLUnCompWram */
#define ARCHIVE_LZ77  2     /* BIOS LZ77, LZ77UnCompWram */

/* The loader's working state. Field names follow the offsets the code uses;
 * the gaps are not yet identified. */
struct ArchiveLoader
{
    const u8 *base;                 /* 0x00: archive base address */
    const struct ArchiveHeader *header; /* 0x04 */
    struct ArchiveEntry *entries;   /* 0x08: this archive's entry table */
    void *buffer;                   /* 0x0C: destination, allocated below */
    void *scratch;                  /* 0x10: second buffer, freed on abort */
    void *tileDestination;          /* 0x14: copied to at 08027FA8 */
    u32 index;                      /* 0x18: which entry to load */
    u32 size;                       /* 0x1C: tileCount << (8bpp ? 6 : 5) */
    u32 *status;                    /* 0x20: flags word, error bit 0x8000 */
};

/* Bits in the shared status word the loader watches. */
#define ARCHIVE_ERR_NO_MEMORY 0x8000    /* allocation failed */
#define ARCHIVE_CANCELLED     0x1000    /* caller asked to stop */
#define ARCHIVE_ABORT_MASK    0x9000    /* either of the above */

/* The task that drives loading across frames. The loader state sits inside
 * it at offset 32, which is why the code carries a second pointer to it. */
struct ArchiveTask
{
    u8 filler_00[0x18];
    s32 *result;                    /* 0x18: set to -1 if the load aborts */
    u8 filler_1C[4];
    struct ArchiveLoader loader;    /* 0x20 */
};

#endif /* ARCHIVE_H */
