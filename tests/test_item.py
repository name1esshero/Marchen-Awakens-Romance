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
            subprocess.run(['cc','-D__attribute__(x)=','-I'+str(ROOT/'include'),
                            str(source),str(ROOT/'src/item.c'),'-o',exe],check=True)
            subprocess.run([exe],check=True)

if __name__=='__main__':unittest.main()
