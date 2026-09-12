/* List primitives at 0807A86C. The list owns links, not node storage. */
#include "list.h"

__attribute__((section(".rom.0007A86C")))
void ListInit(List *list)
{
    list->head = 0;
    list->tail = 0;
    list->count = 0;
}

/* Original alignment after the ten-byte function. */
__attribute__((section(".rom.0007A86C")))
const unsigned char ListInitTail[2] = {0, 0};

__attribute__((section(".rom.0007A878")))
void ListAppend(List *list, ListNode *node)
{
    if (!list->head) {
        list->head = node;
        list->tail = node;
        node->prev = 0;
        node->next = 0;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
        node->next = 0;
    }
    list->tail = node;
    list->count++;
}

__attribute__((section(".rom.0007A8A0")))
void ListInsertBefore(List *list, ListNode *at, ListNode *node)
{
    ListNode *prev = at->prev;

    if (prev)
        prev->next = node;
    else
        list->head = node;
    node->prev = prev;
    node->next = at;
    at->prev = node;
    list->count++;
}

/* Traverse from the head only after checking the stored count. */
__attribute__((section(".rom.0007A8C0")))
ListNode *ListGet(List *list, unsigned index)
{
    unsigned i;
    ListNode *node;

    if (!list->count || list->count <= index)
        return 0;
    node = list->head;
    for (i = 0; i < index; i++)
        node = node->next;
    return node;
}

__attribute__((section(".rom.0007A94C")))
void ListRemove(List *list, ListNode *node)
{
    if (node->next) {
        node->next->prev = node->prev;
        if (node->prev)
            node->prev->next = node->next;
        else
            list->head = node->next;
    } else {
        if (node->prev) {
            node->prev->next = 0;
            list->tail = node->prev;
        } else {
            list->head = 0;
            list->tail = 0;
        }
    }
    node->next = 0;
    node->prev = 0;
    list->count--;
}

__attribute__((section(".rom.0007A8E4")))
unsigned ListCount(List *list)
{
    return list->count;
}

__attribute__((section(".rom.0007A9B8")))
ListNode *ListHead(List *list)
{
    return list->head;
}

__attribute__((section(".rom.0007A9BC")))
ListNode *ListTail(List *list)
{
    return list->tail;
}

/* Same insertion contract as ListInsertBefore. Preserve this routine's link
 * update order because the original ROM contains both implementations. */
__attribute__((section(".rom.0007A98C")))
void ListInsertBeforeLinked(List *list, ListNode *at, ListNode *node)
{
    if (at->prev) {
        node->next = at;
        node->prev = at->prev;
        at->prev = node;
        node->prev->next = node;
    } else {
        list->head = node;
        node->next = at;
        node->prev = 0;
        at->prev = node;
    }
    list->count++;
}

__attribute__((section(".rom.0007A98C")))
const unsigned char ListInsertBeforeLinkedTail[2] = {0, 0};

/* Used by adjacent-item reordering. Both indices must exist and second must
 * immediately follow first; the original helper performs no validation. */
__attribute__((section(".rom.0007A8E8")))
void ListSwapAdjacentIndices(List *list, unsigned first, unsigned second)
{
    ListNode *firstNode;
    ListNode *secondNode;

    if (first == second)
        return;
    firstNode = ListGet(list, first);
    secondNode = ListGet(list, second);
    ListRemove(list, firstNode);
    ListInsertBeforeLinked(list, secondNode->next, firstNode);
    secondNode = ListGet(list, second);
    firstNode = ListGet(list, first);
    ListRemove(list, secondNode);
    ListInsertBeforeLinked(list, firstNode->next, secondNode);
}

__attribute__((section(".rom.0007A8E8")))
const unsigned char ListSwapAdjacentIndicesTail[2] = {0, 0};
