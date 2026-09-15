/* sub_08004DA8, called from RuntimeReadSignedByte() (src/runtime_leaf.c) and
 * RuntimeGetLinkPlayerIfActive() (src/runtime_core.c) as `(s8)sub_08004DA8()`.
 *
 * The disassembly splits this into two labels, sub_08004DA8 and sub_08004DBC,
 * but sub_08004DBC has no callers anywhere in the tree (asm or C) and no
 * `bx lr` of its own before it: it is a false split from an earlier
 * auto-disassembly pass landing on an internal branch target, not a second
 * function. The real, single function spans 0x08004DA8..0x08004DFE (86
 * bytes) with one entry point.
 *
 * Both `bl` instructions inside it target 0x08004CC0, which is
 * SioGetPlayerId() (src/runtime_accessors.c) -- the disassembler's raw
 * `.2byte 0xF7FF` / `.byte 0x7E,0xFF` (and `0x72,0xFF`) pairs are BL encodings
 * it failed to resolve to a name, not unknown data. Manually decoding the
 * standard ARMv4T BL immediate (sign-extend the combined 22-bit halfword
 * offsets, add to `instruction_address + 4`) gives 0x08004CC0 in both cases,
 * confirmed against the linked ELF: `bne`/`beq` targets and the two literal
 * pool words (0x03004014 and, implicitly, the field stride) all line up with
 * this reading and nothing else in the ROM sits at that address.
 *
 * Not yet byte-matching. Two separate problems, not one:
 *
 *   1. Register allocation: the ROM keeps the raw address constant
 *      0x03004014 live in one register across all three field reads (not the
 *      dereferenced pointer -- it reloads `*(u8 **)0x03004014` fresh each of
 *      the three times) and the byte offset 0x130 live in another, reused
 *      throughout. Plain C locals for both land on the wrong pair of
 *      registers no matter which declaration order is tried (confirmed with
 *      the single-.o iteration technique from docs/PRET_STANDARDS.md �8a);
 *      only a `register T x asm("rN")` pin reproduces the ROM's choice, and
 *      per the user's direction this file does not add one.
 *   2. Even with that pin recovered, the very first statement still mismatches
 *      as a two-instruction reorder: the ROM computes the address constant,
 *      then immediately dereferences it, and only after that computes the
 *      0x130 offset (`ldr r5,[pc]; ldr r0,[r5]; movs r4,#152; lsls r4,r4,#1`).
 *      Ordinary C for the equivalent expression makes agbcc schedule both
 *      local-variable initializers (the ALU work for 0x130, and the pc-
 *      relative load) ahead of that first dereference. This is the
 *      "ldr/lsls Scheduling Mismatch" in docs/PRET_STANDARDS.md �5a: a real
 *      agbcc instruction-scheduling preference, not something reorderable
 *      from C.
 *
 * The logic below is confirmed correct against every branch, literal, and
 * call target in the disassembly; only its generated instruction order and
 * register choice remain unmatched.
 *
 * Also checked against old_agbcc (the separate compiler snapshot
 * src/sound_m4a.c and src/sound_cgb_update.c build with, since this game's
 * developer is known to have mixed compiler snapshots across subsystems):
 * identical mismatch. This is a cross-snapshot agbcc allocator behavior, not
 * a wrong-compiler issue for this specific function.
 */

#include "gba/types.h"

extern u32 SioGetPlayerId(void);

s32 RuntimeGetLinkActivityState(void)
{
    u8 **base = (u8 **)0x03004014;
    u32 offset = 0x130;

    if ((*(u32 *)(*base + offset) & 0x180) == 0 && SioGetPlayerId() == 0)
        return 0;

    if ((*(u32 *)(*base + offset) & 0xE) == 0)
        return 1;

    if (*(u32 *)(*base + offset) & (1 << (u8)SioGetPlayerId()))
        return 2;

    return 1;
}
