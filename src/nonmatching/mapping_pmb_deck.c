/* ScriptNativePmbDeckMake (0x080129F4, 224 bytes), the "PmbDeckMake" native
 * command in gScriptNativeCommands.
 *
 * The logic below is certain, cross-checked entry-by-entry against the ROM's
 * jump table (the true table base is one word past its own label -- the
 * `ldr r1, =TABLE` at the top loads a pooled constant that itself holds the
 * table's address, not the label's own position; skimming the raw .s file
 * without accounting for this misreads the table by one slot, the same trap
 * documented for DeckMake and ShuffleDeckCopy in docs/decompilation-notes.md):
 * args[0] selects a deck/shuffle mode, using the same case set {1,3,4,6,7,8,9,22}
 * as DeckMake and ShuffleDeckCopy. For those cases, sub_08055F4C(mode) is
 * called for a struct pointer; its 20-entry s16 array at offset +14..+52 is
 * zeroed, then args[1..20] are validated one at a time via sub_080569B0() (a
 * value passes if its (s16)-truncated result is <= 98) and only the passing
 * entries are compacted into that same array, each flagging its own bit in
 * the save's deck bitset (GAME_ROOT + 0x26F8, the same field DeckMake writes,
 * via the already-decompiled BitSet at 0x0807A190). sub_08055F4C and
 * sub_080569B0 remain unnamed -- no other call site in the codebase yet
 * references either, so there is nothing to cross-reference a real name from.
 *
 * What blocks a byte-exact match: agbcc's optimizer fuses the args[0] read
 * with the args-pointer advance needed for `src = args + 1` into a single
 * `ldmia r4!, {r0}`, instead of the ROM's separate `ldrsh` (for the read)
 * and a later explicit `adds r5, r4, #4` (for the advance) once the loop
 * actually needs a second pointer. This fusion is a data-dependency-driven
 * scheduling decision, not textual-order-driven -- moving the `src = args+1`
 * statement later in the C source did not stop it, and it reappears through
 * several unrelated-looking rewrites (declaring `src` as its own local,
 * hoisting the zero-fill constant, hoisting the bitset base pointer).
 * Register-hinting the mode value (the same technique that fixed
 * DeckMake's r3/r4 swap) does not touch this fusion either, since it targets
 * a different register. Each targeted fix that resolved one instruction-count
 * mismatch shifted the compiler's own switch-statement jump table (which is
 * generated from this function's compiled code layout, not authored by hand)
 * enough to introduce a new one elsewhere, so this was not pursued further as
 * a chain of increasingly specific hints -- see the "PmbDeckMake attempted
 * and left nonmatching" entry in docs/decompilation-notes.md for the full
 * sequence of what was tried.
 */
#include "gba/types.h"

#include "rom_section.h"

#define GAME_ROOT (*(u8 **)0x03003FDC)

extern u8 *sub_08055F4C(s32 mode);
extern s32 sub_080569B0(s32 value);
extern void sub_080083E0(s32 a, s32 b);
extern void BitSet(u8 *bits, u32 index, s32 enabled);

s32 ScriptNativePmbDeckMake(u32 count, const s32 *args, s32 *result)
{
    switch (args[0]) {
    case 1:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
    case 9:
    case 22:
    {
        u8 *dest = sub_08055F4C((s16)args[0]);
        s16 *values = (s16 *)(dest + 14);
        s16 *out = values;
        u8 *bits = GAME_ROOT + 0x26F8;
        s32 i;

        for (i = 0; i < 20; i++)
            values[i] = 0;
        for (i = 0; i < 20; i++) {
            if ((s16)sub_080569B0((s16)args[i + 1]) <= 98) {
                *out = (s16)args[i + 1];
                BitSet(bits, *out, 1);
                out++;
            }
        }
        sub_080083E0(0, 0);
    }
    }
    return 1;
}
