#ifndef OBJECT_H
#define OBJECT_H

#include "gba/types.h"

/* Display objects, managed by the routines around 0x08028000.
 *
 * Recovered from the initialiser at 0x08028008 and the family of accessors
 * that follow it. The flag setters begin by testing bit 15 of
 * the halfword at +0x28, so that bit gates whether an object is live; these setters
 * leave inactive objects unchanged. The raw field setters do not test it.
 *
 * Two allocations are involved. The caller owns a 64-byte struct, which the
 * initialiser clears, and the manager allocates a separate 72-byte record
 * from the heap and stores its pointer at +0x00.
 *
 * Only the fields the initialiser writes are identified. The rest are left
 * as raw bytes rather than guessed at. */

#define OBJECT_STRUCT_SIZE  64      /* the caller's struct, cleared on init */
#define OBJECT_RECORD_SIZE  72      /* the heap record the manager allocates */

/* Bits in the flags halfword at +0x28. */
#define OBJECT_ACTIVE 0x8000        /* gates the flag setters */

struct Object
{
    void *record;               /* 0x00: the 72-byte heap record */
    u8 filler_04[0x1C];
    u16 unk_20;                 /* 0x20: initialised to 0x100, which is 1.0
                                 *       read as 8.8 fixed point */
    u8 filler_22[2];
    u16 unk_24;                 /* reset by ObjectSetField26 */
    u16 unk_26;
    u16 flags;                  /* 0x28: OBJECT_ACTIVE and others */
    u8 filler_2A[2];
    u16 unk_2C;                 /* 0x2C: cleared by one of the setters */
    u8 filler_2E[6];
    u16 unk_34;
    u16 unk_36;                 /* 0x36: written by the setter at 0x08028388 */
    u8 filler_38[8]; 
};

void *ObjectGetActiveRecordData(struct Object *object);

#endif /* OBJECT_H */
