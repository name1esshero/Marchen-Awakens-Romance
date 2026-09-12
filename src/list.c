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
