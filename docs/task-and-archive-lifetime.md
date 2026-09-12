# Task and archive lifetime

Fourteen routines now compile with agbcc to their original bytes, including literal
pools and alignment. The linked-provider audit verifies their C object ownership.

| ROM address | Function | Bytes |
| --- | --- | ---: |
| `0807A5FC` | `TaskManagerInit` | 72 |
| `0807A644` | `TaskManagerDestroy` | 68 |
| `0807A688` | `TaskCreateInQueue` | 120 |
| `0807A700` | `TaskCreateBefore` | 124 |
| `0807A77C` | `CreateTask` | 48 |
| `0807A7C4` | `TaskManagerRun` | 164 |
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
callbacks nor frees the caller-owned manager. The public creation wrapper and dispatch loop are also decoded below.

The two lower-level creation routines allocate a 32-byte header plus the requested
payload, clear the entire allocation, then append to a priority queue or insert
before an existing task. The header stores the manager at +8, signed state at
+12, callback at +20, completion pointer at +24 and priority at +28. Bytes
+13..+19 remain unnamed. Both routines set state to 1 and clear the optional
completion word. Allocation failure calls the still-undecoded routine at
`0807A4B8` with `0x00600000`, then returns null without changing the lists or
task count. Queue bounds and insertion membership are caller preconditions.

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

## Creation dispatch and scheduler

`CreateTask` interprets its third argument as an unsigned queue index when it is
less than the manager's queue count. Otherwise it casts the value to a task
pointer and inserts before that task. This is an overloaded index/pointer API,
not a bounds-checked queue API; an invalid large index becomes an invalid pointer.

`TaskManagerRun` visits queues in ascending index order. A task with signed state
-1 is removed without invoking its callback. Other states invoke the callback,
then the scheduler checks again for -1 and removes the task if needed. Removal
decrements the owning manager's count, unlinks the node and frees it through the
owning heap. The scheduler itself does not write the completion word.

The next pointer is read **after** the callback, so a callback that appends a task
to the current queue can cause that new task to run during the same pass. Tests
cover that behavior, tasks already finished on entry, self-finishing callbacks,
surviving tasks, and both creation-wrapper branches. They do not establish safety
for arbitrary callback mutations such as freeing the current task directly.

The compiler's `_call_via_r1` symbol aliases the existing `bx r1` trampoline at
`08080BC4`; it adds no replacement code or ROM bytes.

## Mount helpers

| ROM address | Function | Bytes |
| --- | --- | ---: |
| `0807AA48` | `NfpGetMountName` | 44 |
| `0807AA74` | `NfpSetMountName` | 44 |
| `0807AB70` | `NfpUnmount` | 12 |
| `0807ABBC` | `NfpCountMounted` | 52 |

Name lookup returns null for an inactive slot. Name assignment calls the original
`strcpy` and `strupr` routines, reloading the mount table before uppercasing the
copied name. It does not check slot activity or name length: the caller must
supply a valid handle and a name that fits the 15-byte field including its NUL.

Unmounting calls `NfpSetMountActive(handle, 0)` and leaves the name, archive base
and size intact. Counting visits all slots below the signed mount count and
counts any nonzero active value. Neither operation frees archive data.

The 32-bit lifecycle test now exercises these four C implementations with stubbed
string and active-flag primitives. It checks name lookup/normalization, zero-slot
counts, nonzero active values and metadata retention on unmount. Exact ROM
comparison additionally verifies the real calls and compiled instructions.

## Completion and archive convenience APIs

| ROM address | Function | Bytes |
| --- | --- | ---: |
| `0807A7AC` | `FinishTask` | 24 |
| `0807AAF0` | `NfpGetEntryCountByName` | 24 |
| `0807AC78` | `NfpOpenByIndex` | 48 |
| `0807ACA8` | `NfpFindEntryByArchiveName` | 28 |

`FinishTask` changes state 1 to the scheduler's removal state, -1. Other task
states are unchanged. Actual removal remains deferred until `TaskManagerRun`.

The archive helpers first resolve an archive name to a mounted handle. Entry
count returns zero if the archive is absent. Opening by numeric index returns
the archive base plus the entry's relative offset; it returns null if either
the archive or directory entry cannot be resolved. The name-to-index wrapper
returns zero for a missing archive but preserves the binary-search result,
including -1 for a missing member in an existing archive. This asymmetry is
original behavior and callers must distinguish the two cases if it matters.

`ListSwapAdjacentIndices` at `0807A8E8` adds 100 matching bytes in `src/list.c`.
Equal indices are a no-op. The observed useful operation exchanges adjacent
entries while preserving head, tail and count. The original routine performs
no bounds checks and its low-level insertion requires a nonnull position, so
the API documents valid, adjacent indices as a caller precondition.
