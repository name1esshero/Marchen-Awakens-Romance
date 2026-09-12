# Heap construction and byte utilities

Six routines at `0807A210..0807A2F8` now compile from C with agbcc, occupying
232 original bytes including literal pools and alignment.

## Heap layout and construction

`src/heap.c` and `include/heap.h` recover the heap header and initial block:

| Offset | Field | Initial value |
| --- | --- | --- |
| heap +0 | Total size | Requested size rounded up to four bytes |
| heap +4 | Scan-start pointer | First block at heap +8 |
| block +0 | Size and flags | `(rounded_size - 8) | 2` |
| block +4 | Metadata | Zero; other uses remain to be decoded |

The assembly allocation search at `0807A54C` masks off the low two size bits
and advances through blocks by size. Bit 0 causes it to skip occupied blocks;
bit 1 makes traversal wrap to the first block. This is not a linked free list.

`HeapCreate` rounds with unsigned 32-bit arithmetic and rejects a rounded size
of 24 bytes or less. Caller-supplied storage is used directly. A null storage
pointer asks `HeapAlloc(NULL, rounded_size)` to obtain storage from the existing
default heap; failure returns null. It initializes the heap header and first
block only, leaving subsequent payload bytes alone. Callers must supply enough
storage for the rounded size.

`HeapInitDefault` requires nonnull caller-owned memory. It clears the first
16 bytes before attempting construction, even when size validation will fail.
It publishes the new pointer at `03006110` only after successful construction;
failure leaves the previous default pointer unchanged.

`HeapAlloc` forwards the heap and size to the assembly allocator with its third
argument fixed to zero. That mode starts scanning at the first block rather
than the saved scan-start pointer. Search, splitting, and freeing are not yet
converted by this batch.

## Byte utilities

`ByteStringLength` counts bytes before the first zero byte, not decoded text
glyphs. It is a distinct helper from the library `strlen` used elsewhere.
`BufferXor` XORs an explicit number of unsigned bytes; zero bytes produces zero.
`BufferXorSeeded` XORs that result with an eight-bit seed; an empty buffer returns
the seed. Neither checksum treats an embedded zero byte as a terminator.

## Verification

`python3 tests/test_heap_byte_utils.py` builds a freestanding 32-bit Linux test
so heap pointer offsets match the GBA ABI. It requires an x86 Linux host with
32-bit executable support; syscall-restricting sandboxes may block execution.
The actual C is tested for rounding/rejection boundaries, allocation failures,
header placement, payload preservation, default-pointer updates, wrapper
arguments, checksum vectors, and high-bit string bytes. ROM matching is checked
separately with the full build and `tools/audit_provenance.py`.
