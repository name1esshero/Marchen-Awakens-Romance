/* ArmDescriptionPopup, 0x080537D8. Already documented in src/menu_text.c as
 * "the ARM deck/status renderer at 080537D8". Called from ten sites (two per
 * caller, across five different regions: code_0580C0.s:1410/1458,
 * code_0580C0.s:6926/6972, code_0600C0.s:7753/7803, code_0680C0.s:2969/3010,
 * code_0680C0.s:6038/6082). This is the confirmed root cause of the
 * "some items still show Japanese text" bug report: it strcpy()s the
 * description straight out of gArmDefinitions[id].description, bypassing
 * ItemGetDescription() / EnglishItemGetDescription() entirely (id == 0 also
 * bypasses translation, via a raw 13-byte block copy of the hardcoded
 * Japanese literal at 0x080887F4, "何もありません。" -- "There is nothing.").
 *
 * The icon labels (category from ->field64, element from ->element) are
 * NOT part of the bug: they already read through the pointer tables at
 * 0x081B07CC/0x081B07E4, which src/menu_text.c already swaps to English text
 * in place via gMenuTextConstants's #ifdef ENGLISH branch.
 *
 * Confirmed by direct disassembly of the real ROM bytes (not just the
 * project's split.py output, which hex-dumps this whole function as opaque
 * .4byte data because its jump-table-heavy neighbor, the 27-way dispatcher
 * sub_0805593C immediately before it in the same section, defeated
 * split.py's recursive-descent boundary detection):
 *   - KmpLoadResource("AD_BG02.KMP", (void*)0x06008000, 2, 0, 1, 0, 3) loads
 *     the popup window graphic.
 *   - KmpRenderViewport(&gKmpViewports[2], x<<16, y<<16) positions it
 *     (every one of the ten callers passes the same constants, x=0, y=-112).
 *   - If id != 0: ItemGetDefinition(id), build icon text (two strcpy calls
 *     into the same 34-byte buffer -- the second overwrites the first,
 *     confirmed by direct trace, not yet understood why both are computed),
 *     then strcpy the raw description, then build an opaque ~28-byte
 *     task-creation parameter block and call sub_080533F0(&params,
 *     (void*)0x081B08E0) -- sub_080533F0 remains completely unresolved;
 *     nothing here needed to know its internals beyond "two pointer
 *     arguments, presumably a task/popup constructor given the shape of the
 *     block it takes (buffer pointers, position, flags)".
 *   - If id == 0: ItemGetDefinition(0), then a fixed 17-byte raw copy of
 *     the literal Japanese string at 0x080887F4 (via ldmia/stmia, not
 *     strcpy -- confirmed by direct trace) instead of the icon+description
 *     buffer. This branch's params block differs from the id != 0 one in
 *     three confirmed fields, not just the text source: field08 is 1 (not
 *     2), field12 is 2 (not 0), and the task callback is 0x081B08E8 (not
 *     0x081B08E0).
 *
 * NOT byte-matched. Logic, call targets, struct field offsets, and pool
 * constants are all confirmed correct against the real ROM bytes (verified
 * with agbcc_probe.py --bytes), but the ROM sign-extends the narrowed
 * id/x/y parameters (`asrs`) at function entry while every C shape tried
 * here compiles to zero-extension (`lsrs`) for the same two register
 * uses (compare-id-to-zero, shift-x/y-left-by-16) -- both are genuinely
 * insensitive to extension method, so agbcc's choice is not wrong, just
 * different from whatever the original source did to force sign
 * significance this early. Five source shapes were tried and rejected:
 * explicit (s16) cast on s32 parameters, declaring the parameters as s16
 * directly, splitting the narrow-before-call from the shift-after-call
 * across the KmpLoadResource boundary (matching the ROM's own two-phase
 * instruction order exactly), multiplying by 65536 instead of shifting
 * left, and cross-checking real caller argument setups for a signature
 * hint (all ten callers pass x=0, y=-112 as literal constants, which
 * doesn't change the callee-side codegen question). This is a source-shape
 * question, not a struct/logic uncertainty -- the fields below are
 * confirmed, not guessed.
 */
#include "gba/types.h"

struct ArmDefinition {
    u16 iconId;
    u16 id;
    u16 field04;
    char password[8];
    u16 reserved0E;
    u8 name[34];
    u8 description[34];
    u16 field54;
    u16 field56;
    s8 field58;
    s8 type;
    s8 field5A;
    u8 reserved5B;
    s16 field5C, field5E, field60;
    s8 field62, field63, field64;
    s8 element;
    u8 field66, field67;
    u32 reserved68;
    u32 field6C;
    u8 field70, field71, field72, field73;
    u32 field74;
    u32 field78;
    u8 field7C;
    u8 reserved7D[3];
};

struct KmpViewport { u8 reserved[0xFC]; };

extern struct KmpViewport gKmpViewports[];
extern const struct ArmDefinition *ItemGetDefinition(s32 id);
extern void KmpLoadResource(const char *name, void *tileDestination, s32 slot,
                            s32 plane, s32 paletteOffset, s32 tileOffset,
                            s32 flags);
extern void KmpRenderViewport(struct KmpViewport *view, s32 xFixed, s32 yFixed);
extern char *strcpy(char *destination, const char *source);
extern void CpuFill(void *destination, u32 size, u32 value);
extern void Clear32ByteBlock(void *destination);
extern void sub_080533F0(void *params, void *callback);

#define ARM_CATEGORY_LABELS ((const char *const *)0x081B07CC)
#define ARM_ELEMENT_LABELS  ((const char *const *)0x081B07E4)
#define ARM_POPUP_CALLBACK       ((void *)0x081B08E0)
#define ARM_POPUP_EMPTY_CALLBACK ((void *)0x081B08E8)
#define ARM_POPUP_EMPTY_TEXT ((const char *)0x080887F4)

struct ArmPopupBuffers {
    char *icon;
    char *description;
};

struct ArmPopupParams {
    struct ArmPopupBuffers *buffers;
    void *destination;
    u16 field08;
    u16 field0A;
    u8 field0C;
    u8 reserved0D;
    u16 field0E;
    u16 field10;
    u16 field12;
    u16 reserved14;
    u8 field16;
    u8 reserved17;
    u16 x;
    u16 y;
};

void ArmDescriptionPopup(s32 id, s32 x, s32 y)
{
    s16 narrowId = (s16)id;
    s16 narrowX = (s16)x;
    s16 narrowY = (s16)y;
    char iconBuffer[34];
    struct ArmPopupBuffers buffers;
    struct ArmPopupParams params;
    const struct ArmDefinition *definition;
    s32 fixedX = (s32)narrowX << 16;
    s32 fixedY = (s32)narrowY << 16;

    buffers.icon = iconBuffer;
    KmpLoadResource("AD_BG02.KMP", (void *)0x06008000, 2, 0, 1, 0, 3);
    KmpRenderViewport(&gKmpViewports[2], fixedX, fixedY);
    if (narrowId != 0) {
        definition = ItemGetDefinition(narrowId);
        CpuFill(iconBuffer, 34, 0);
        strcpy(iconBuffer, ARM_CATEGORY_LABELS[definition->field64]);
        strcpy(iconBuffer, ARM_ELEMENT_LABELS[definition->element]);
        buffers.description = (char *)definition->description;
        Clear32ByteBlock(&params);
        params.buffers = &buffers;
        params.destination = (void *)0x06008000;
        params.field08 = 2;
        params.field0A = 2;
        params.field0C = 1;
        params.field0E = 0x040F;
        params.field10 = 20;
        params.field12 = 0;
        params.field16 = 1;
        params.x = (u16)(fixedX >> 16);
        params.y = (u16)(fixedY >> 16);
        sub_080533F0(&params, ARM_POPUP_CALLBACK);
    } else {
        definition = ItemGetDefinition(0);
        buffers.description = (char *)ARM_POPUP_EMPTY_TEXT;
        Clear32ByteBlock(&params);
        params.buffers = &buffers;
        params.destination = (void *)0x06008000;
        params.field08 = 1;
        params.field0A = 2;
        params.field0C = 1;
        params.field0E = 0x040F;
        params.field10 = 20;
        params.field12 = 2;
        params.field16 = 1;
        params.x = (u16)(fixedX >> 16);
        params.y = (u16)(fixedY >> 16);
        sub_080533F0(&params, ARM_POPUP_EMPTY_CALLBACK);
    }
}
