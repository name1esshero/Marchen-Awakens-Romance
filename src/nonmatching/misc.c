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

/* The SRAM library candidates formerly kept here now match in src/sram.c. */
