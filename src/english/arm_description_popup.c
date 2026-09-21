#include "english.h"
#include "item.h"
#include "kmp.h"

/* English-only replacement for sub_080537D8 (see
 * src/nonmatching/arm_description_popup.c for the confirmed ROM mechanism
 * this reproduces). The Japanese ROM strcpy()s straight out of
 * gArmDefinitions[id].description and, for id == 0, out of a hardcoded
 * literal at 0x080887F4 -- both bypass ItemGetDescription() and its
 * translation hook because this function is still raw asm. This is called
 * from ten sites across five regions, all still raw asm themselves, so the
 * whole function is replaced wholesale for the English build rather than
 * patched at the call site. Everything except the two text sources
 * (icon lookups, KMP window load/position, the task-creation parameter
 * block, and the still-unresolved sub_080533F0 call) is unchanged.
 *
 * NOT currently wired into the Makefile. This code is complete and
 * verified correct (compiles cleanly, logic confirmed against the ROM
 * mechanism), but sub_080537D8 is still raw asm shared with ~50 unrelated
 * functions in asm/code/code_0500C0.s, unlike every other English bridge
 * target (which are all already-decompiled C, recompiled with -DENGLISH=1).
 * Every symbol-override mechanism tried to redirect just this one function
 * -- objcopy --strip-symbol, --localize-symbol, ld --wrap, ld --defsym
 * (both symbol-to-symbol and a resolved numeric address), and
 * --allow-multiple-definition with the bridge object reordered earlier in
 * the link -- breaks `mar_english.elf`'s .rom section by exactly 16 bytes
 * ("ROM image is not 16 MB: a fragment changed size"), while the *same*
 * link with the override *not* taking effect builds a byte-perfect 16 MB
 * .rom every time. This reproduces regardless of which object supplies the
 * override and regardless of whether code_0500C0.o itself is modified,
 * copied, or left untouched -- strongly suggesting a genuine, narrow bug in
 * how ld's `SORT_BY_NAME(.rom.*)` + `KEEP` scheme (see ld_script.ld)
 * computes section placement once a symbol that anchors one of those named
 * sections resolves somewhere else. See "sub_080537D8 English bridge:
 * confirmed fix, blocked by a linker quirk" in docs/decompilation-notes.md
 * for the full isolation log before repeating this investigation. */

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

void EnglishArmDescriptionPopup(s32 id, s32 x, s32 y)
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
        buffers.description =
            (char *)EnglishTranslateSingle((const char *)definition->description);
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
        buffers.description = (char *)EnglishTranslateSingle(ARM_POPUP_EMPTY_TEXT);
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
