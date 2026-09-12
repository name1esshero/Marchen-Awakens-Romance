"""Exercise the recovered bytecode cursor, stack, and core VM commands."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ScriptBytecodeTests(unittest.TestCase):
    def test_reads_stack_control_flow_and_alu(self):
        with tempfile.TemporaryDirectory() as temp:
            source = Path(temp) / "test.c"
            source.write_text(r'''
#include "script_bytecode.h"
#include <assert.h>
#include <string.h>

static struct ScriptBytecodeVm vm;
static struct ScriptBytecodeContext context;
static struct ScriptBytecodeRoot root;
static s32 variables[256];

#undef gScriptBytecodeRoot
#undef gScriptBytecodeVm
#define gScriptBytecodeRoot (&root)
#define gScriptBytecodeVm (root.context->vm)
#define AT(x)
#include "src/script_bytecode.c"

s32 *sub_0807F624(u32 operand) { return &variables[operand]; }

static void reset(u8 *bytes) {
    memset(&vm, 0, sizeof(vm));
    memset(variables, 0, sizeof(variables));
    context.vm = &vm;
    root.context = &context;
    vm.bytecode = bytes;
    vm.stackPointer = 240;
}

int main(void) {
    u8 bytes[256] = {0x12,0x34,0x56,0x78,0x9A};
    reset(bytes);
    assert(ScriptReadU8(1) == 0x34);
    assert(ScriptReadU16(1) == 0x5634);
    assert(ScriptReadU32(0) == 0x78563412);
    assert(ScriptReadNextU8() == 0x12 && vm.programCounter == 1);
    assert(ScriptReadNextU16() == 0x5634 && vm.programCounter == 3);

    ScriptPushU32(0x12345678);
    assert(vm.stackPointer == 236);
    assert(ScriptPopU32() == 0x12345678 && vm.stackPointer == 240);

    reset(bytes); bytes[0]=3; bytes[1]=4;
    variables[3]=7; variables[4]=5;
    assert(ScriptCmdAdd() == 1 && variables[3] == 12);

    reset(bytes); bytes[0]=3; bytes[1]=0x78; bytes[2]=0x56;
    bytes[3]=0x34; bytes[4]=0x12;
    assert(ScriptCmdSetImmediate() == 1 && variables[3] == 0x12345678);

    reset(bytes); bytes[0]=2; bytes[1]=2;
    bytes[2]=1; bytes[6]=20;
    bytes[10]=7; bytes[14]=40;
    variables[2]=7;
    assert(ScriptCmdSwitch() == 1 && vm.programCounter == 40);

    reset(bytes); bytes[0]=0x20; bytes[1]=bytes[2]=bytes[3]=0;
    assert(ScriptCmdCall() == 1 && vm.programCounter == 0x20);
    assert(ScriptCmdReturn() == 1 && vm.programCounter == 4);
    return 0;
}
''')
            exe = Path(temp) / "test"
            subprocess.run([
                "gcc", "-O2", "-I" + str(ROOT / "include"), "-I" + str(ROOT),
                str(source), "-o", str(exe)
            ], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    unittest.main()
