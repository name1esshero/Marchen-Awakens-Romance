"""Exercise intrusive list links and counts using the actual C implementation."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class ListTests(unittest.TestCase):
    def test_empty_append_and_insert(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = pathlib.Path(tmp) / 'test.c'
            source.write_text(r'''
#include <assert.h>
#include "list.h"
int main(void) {
    List list;
    ListNode a, b, c, d;
    ListInit(&list);
    assert(!list.head && !list.tail && list.count == 0);
    ListAppend(&list, &a);
    assert(list.head == &a && list.tail == &a && list.count == 1);
    assert(!a.next && !a.prev);
    ListAppend(&list, &b);
    assert(a.next == &b && b.prev == &a && !b.next);
    ListInsertBefore(&list, &a, &c);
    assert(list.head == &c && !c.prev && c.next == &a && a.prev == &c);
    ListInsertBefore(&list, &b, &d);
    assert(a.next == &d && d.prev == &a && d.next == &b && b.prev == &d);
    assert(list.tail == &b && list.count == 4);
    assert(ListCount(&list) == 4 && ListHead(&list) == &c && ListTail(&list) == &b);
    assert(ListGet(&list, 0) == &c && ListGet(&list, 2) == &d);
    assert(!ListGet(&list, 4) && !ListGet(&list, ~0u));
    ListRemove(&list, &d); /* Middle. */
    assert(a.next == &b && b.prev == &a && !d.next && !d.prev);
    ListRemove(&list, &c); /* Head. */
    assert(list.head == &a && !a.prev);
    ListRemove(&list, &b); /* Tail. */
    assert(list.tail == &a && !a.next);
    ListRemove(&list, &a); /* Last node. */
    assert(!ListHead(&list) && !ListTail(&list) && !ListCount(&list));
    assert(!ListGet(&list, 0) && !a.next && !a.prev);
    ListAppend(&list, &a);
    ListAppend(&list, &b);
    ListAppend(&list, &c);
    ListAppend(&list, &d);
    ListSwapAdjacentIndices(&list, 1, 2);
    assert(ListGet(&list, 0) == &a && ListGet(&list, 1) == &c);
    assert(ListGet(&list, 2) == &b && ListGet(&list, 3) == &d);
    assert(a.next == &c && c.prev == &a && c.next == &b);
    assert(b.prev == &c && b.next == &d && d.prev == &b);
    assert(list.head == &a && list.tail == &d && list.count == 4);
    ListSwapAdjacentIndices(&list, 2, 2);
    assert(ListGet(&list, 2) == &b && list.count == 4);
    return 0;
}
''')
            exe = str(pathlib.Path(tmp) / 'test')
            subprocess.run(['cc', '-D__attribute__(x)=', '-I'+str(ROOT/'include'), str(source),
                            str(ROOT/'src/list.c'), '-o', exe], check=True)
            subprocess.run([exe], check=True)

if __name__ == '__main__':
    unittest.main()
