#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "gba/types.h"
#include "task_manager.h"

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
    u16 value02;            /* 0x02: passed to the map-transfer helper */
    u16 value04;            /* 0x04: passed to the map-transfer helper */
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

/* Values of ArchiveEntry.type, as dispatched by ArchiveTaskStep. */
#define ARCHIVE_RAW   0     /* copied verbatim */
#define ARCHIVE_RLE   1     /* BIOS run-length, RLUnCompWram */
#define ARCHIVE_LZ77  2     /* BIOS LZ77, LZ77UnCompWram */

/* Offset from the IWRAM base to the renderer used by archive map commits. */
#define ARCHIVE_RENDER_CONTEXT_OFFSET 0x00003FD0

/* The loader's working state. Field names follow the offsets the code uses;
 * the gaps are not yet identified. */
struct ArchiveLoader
{
    const u8 *base;                 /* 0x00: archive base address */
    const struct ArchiveHeader *header; /* 0x04 */
    struct ArchiveEntry *entries;   /* 0x08: this archive's entry table */
    u8 *buffer;                     /* 0x0C: destination, allocated below */
    u8 *scratch;                    /* 0x10: second buffer, freed on abort */
    void *tileDestination;          /* 0x14: copied to at 08027FA8 */
    u32 index;                      /* 0x18: which entry to load */
    u32 size;                       /* 0x1C: tileCount << (8bpp ? 6 : 5) */
    u32 *status;                    /* 0x20: flags word, error bit 0x8000 */
};

/* The task that drives loading across frames. */
struct ArchiveTask
{
    struct EngineTask task;
    struct ArchiveLoader loader;    /* 0x20 */
};

#endif /* ARCHIVE_H */
