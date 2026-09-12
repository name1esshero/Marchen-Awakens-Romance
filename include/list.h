#ifndef LIST_H
#define LIST_H

/* Intrusive doubly linked list: nodes are embedded in caller-owned objects. */
typedef struct ListNode { struct ListNode *next, *prev; } ListNode;
typedef struct List { ListNode *head,*tail; unsigned count; } List;

void ListInit(List *list);
void ListAppend(List *list, ListNode *node);
/* at must be an existing node; node must not already belong to a list. */
void ListInsertBefore(List *list, ListNode *at, ListNode *node);

/* Lookup returns null for an out-of-range index. Removal requires membership. */
ListNode *ListGet(List *list, unsigned index);
void ListRemove(List *list, ListNode *node);
unsigned ListCount(List *list);
ListNode *ListHead(List *list);
ListNode *ListTail(List *list);

/* Alternate insertion routine used by the original index-reordering code. */
void ListInsertBeforeLinked(List *list, ListNode *at, ListNode *node);

#endif
