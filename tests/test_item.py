import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[1]

class ItemTests(unittest.TestCase):
    def test_layout_index_narrowing_and_signed_fields(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            source = root/'test.c'
            source.write_text(r'''
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "item.h"
/* The ROM resolves these through asm/game_table_handlers.s; the host link
 * needs its own storage because it compiles src/item.c in isolation. */
const struct ConsumableText gConsumableNoneText;
const char gConsumableNoneDescription[1];
const struct ArmDefinition gArmDefinitions[ARM_COUNT] = {
    [0] = { .name="Empty", .description="No effect" },
    [1] = { .name="Babbo", .description="A living ARM", .field58=-1,
      .type=-2, .field5A=3, .field5C=-300, .field5E=500,
      .field60=-32768, .field62=-128, .field63=127, .field64=-5, .element=6 }
};
int main(void) {
    assert(sizeof(struct ArmDefinition)==128);
    assert(offsetof(struct ArmDefinition,name)==16);
    assert(offsetof(struct ArmDefinition,description)==50);
    assert(offsetof(struct ArmDefinition,field58)==88);
    assert(ItemGetDefinition(0x10001)==&gArmDefinitions[1]);
    assert(!strcmp(ItemGetName(1),"Babbo"));
    assert(!strcmp(ItemGetDescription(1),"A living ARM"));
    assert(ItemGetField58(1)==-1 && ItemGetField59(1)==-2);
    assert(ItemGetField5A(1)==3 && ItemGetField5C(1)==-300);
    assert(ItemGetField5E(1)==500 && ItemGetField60(1)==-32768);
    assert(ItemGetField62(1)==-128 && ItemGetField63(1)==127);
    assert(ItemGetField64(1)==-5 && ItemGetField65(1)==6);
    assert(ItemGetField64(0x10001)==-5);
    return 0;
}
''')
            exe = str(root/'test')
            subprocess.run(['cc','-ffunction-sections','-fdata-sections',
                            '-D__attribute__(x)=','-I'+str(ROOT/'include'),
                            str(source),str(ROOT/'src/item.c'),
                            '-Wl,--gc-sections','-o',exe],check=True)
            subprocess.run([exe],check=True)

    def test_english_consumable_accessors_translate_table_text(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            source = root/'test.c'
            source.write_text(r'''
#include <assert.h>
#include <string.h>
#include "english.h"
#include "item.h"
const char gTranslatedName[] = "Crisp Fruit";
const char gTranslatedDescription[] = "Restores 50 HP.";
const struct ConsumableText gConsumableNoneText = { .name="サックリの実" };
const char gConsumableNoneDescription[] = "体力を５０回復する";
const struct ArmDefinition gArmDefinitions[ARM_COUNT];
const struct ItemDefinition gItemDefinitions[ITEM_COUNT] = {
    [0] = { .name="サックリの実", .description="体力を５０回復する" },
};
const char *EnglishConsumableGetName(s32 id);
const char *EnglishConsumableGetDescription(s32 id);
const char *EnglishConsumableGetResourceName(s32 id);
const char *EnglishTranslateSingle(const char *source) {
    if (!strcmp(source, "サックリの実")) return gTranslatedName;
    if (!strcmp(source, "体力を５０回復する")) return gTranslatedDescription;
    return source;
}
int main(void) {
    assert(!strcmp(EnglishConsumableGetName(0), "Crisp Fruit"));
    assert(!strcmp(EnglishConsumableGetResourceName(0), "Crisp Fruit"));
    assert(!strcmp(EnglishConsumableGetDescription(0), "Restores 50 HP."));
    return 0;
}
''')
            exe = str(root/'test')
            subprocess.run(['cc','-ffunction-sections','-fdata-sections',
                            '-D__attribute__(x)=','-I'+str(ROOT/'include'),
                            str(source),str(ROOT/'src/item.c'),
                            str(ROOT/'src/english/item_name.c'),'-Wl,--gc-sections',
                            '-o',exe],check=True)
            subprocess.run([exe],check=True)

if __name__=='__main__':unittest.main()
