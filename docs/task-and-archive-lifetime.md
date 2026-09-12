# Task and archive lifetime

Six routines now compile with agbcc to their original bytes, including literal
pools and alignment. The linked-provider audit verifies their C object ownership.

| ROM address | Function | Bytes |
| --- | --- | ---: |
| `0807A5FC` | `TaskManagerInit` | 72 |
| `0807A644` | `TaskManagerDestroy` | 68 |
| `0807A868` | `TaskManagerCount` | 4 |
| `0807A98C` | `ListInsertBeforeLinked` | 44 |
| `0807A9C0` | `NfpInit` | 48 |
| `0807A9F0` | `NfpShutdown` | 40 |

## Tasks

`TaskManager` contains its owning heap at offset 0, an array of intrusive lists
at +4, the list count at +8, and task count at +12. Task creation indexes the
lists by priority. Initialization allocates 12 bytes per queue and initializes
each list. It does not check allocation failure.

Destruction walks each queue from its head, saves the next link before freeing
the current node, and finally frees the queue array. It clears the queue pointer
and task count, but retains the heap and queue count. It neither invokes task
callbacks nor frees the caller-owned manager. Creation and dispatch remain
assembly; this batch does not establish the complete scheduler behavior.

`ListInsertBeforeLinked` inserts before an existing node, handling both head and
interior insertion. It updates the count and preserves the tail. Its caller must
supply a valid member as the insertion position and a node not already linked.

## Archives

The formerly unnamed `NfpState` field at +4 is the heap that owns the mount
array. Initialization publishes the caller-owned state, clears its 12 bytes,
then allocates 24 bytes per mount and stores the heap and signed mount count.
The newly allocated table is **not cleared** by this routine. Allocation failure
leaves a null table pointer with the supplied heap and count still recorded.

Shutdown frees the table through that heap, clears the state and then clears the
global state pointer. It does not free the caller's state object. These routines
preserve the original lack of defensive null checks.

## Verification

`python3 tests/test_manager_lifecycle.py` compiles the actual C sources for a
32-bit host ABI. It checks structure sizes, head/interior insertion, task counts,
zero queues, and destruction after the allocator overwrites a freed node's link.
Archive checks cover allocation size, state clearing before allocation, unchanged
mount-table contents, ownership during shutdown, and allocation failure.

`make --jobserver-style=pipe -j4 all english` rebuilds both ROMs. Verify Japanese
identity with `cmp baserom.gba mar.gba`, C providers with
`python3 tools/audit_provenance.py`, and English changes with
`python3 tools/audit_english_build.py`. Host behavior checks supplement the exact
ROM comparison; they are not an emulator playthrough.
