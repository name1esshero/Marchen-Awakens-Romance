/* Earlier readable reconstructions, not yet verified as matching with agbcc.
 * The default build already uses agbcc; these remain excluded pending an
 * instruction comparison. Register allocation is not assumed to be the only
 * possible difference. */

#include "gba/types.h"

/* sub_08085A24: push a node onto the front of one of a table's buckets.
 *
 * Original:
 *     adds r3, r0, #0      @ the original copies both arguments first
 *     adds r2, r1, #0
 *     cmp  r2, #0
 *     beq  .done
 *     ldr  r0, [r2, #4]    @ node->key
 *     ldr  r1, [r3, #76]   @ manager->buckets
 *     lsls r0, r0, #2
 *     adds r0, r0, r1
 *     ldr  r1, [r0, #0]    @ old head
 *     str  r1, [r2, #0]    @ node->next = old head
 *     str  r2, [r0, #0]    @ head = node
 * .done:
 *     bx   lr
 *
 * Ours is the same nine instructions without the two argument copies, which
 * modern GCC sees no reason to emit.
 */
struct ListNode
{
    struct ListNode *next;
    u32 key;
};

struct BucketTable
{
    u8 filler_00[0x4C];
    struct ListNode **buckets;
};

void AddNodeToBucket(struct BucketTable *table, struct ListNode *node)
{
    struct ListNode **bucket;

    if (node == NULL)
        return;

    bucket = &table->buckets[node->key];
    node->next = *bucket;
    *bucket = node;
}

/* Field setters and counter initialization/reset now match with agbcc;
 * see src/object.c and src/step_counter.c. */

/* sub_08079EDC and its two siblings at 0x08079F1C and 0x08079F5C.
 *
 * Save-memory access. SRAM on the GBA is 8-bit only and needs the slowest
 * wait state, so every one of these sets WAITCNT's SRAM field to 3 (8 cycles)
 * and then moves the data a byte at a time. The first two are copy routines; 08079F5C instead compares bytes and
 * returns the first mismatching destination address. The copy routines are
 * duplicated, which is what you would expect if they are copied into RAM to
 * run: sub_08079FA8 computes the byte span of this block in order to relocate
 * it.
 *
 * Original:
 *     adds r5, r0, #0          @ the arguments are copied into r4/r5 first
 *     adds r4, r1, #0
 *     adds r3, r2, #0
 *     ldrh r0, [REG_WAITCNT]
 *     ands r0, #0xFFFC
 *     orrs r0, #3              @ SRAM wait = 8 cycles
 *     strh r0, [REG_WAITCNT]
 *     ...                      @ for (i = size - 1; i != -1; i--) copy a byte
 *
 * Ours keeps the arguments where they arrive; the copy into callee-saved
 * registers is a habit of the original compiler, not something the source
 * asks for.
 */
#define REG_WAITCNT (*(vu16 *)0x04000204)
#define WAITCNT_SRAM_MASK 0xFFFC
#define WAITCNT_SRAM_8    3

void SramCopy(const u8 *src, u8 *dest, u32 size)
{
    s32 i;

    REG_WAITCNT = (REG_WAITCNT & WAITCNT_SRAM_MASK) | WAITCNT_SRAM_8;

    for (i = size - 1; i != -1; i--)
        *dest++ = *src++;
}
