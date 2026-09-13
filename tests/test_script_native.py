"""Exercise matching VM native helpers, including private font codes and OOM."""
import ctypes
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]


class ScriptNativeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory();folder=Path(cls.temp.name)
        source=(ROOT/'src/script_native.c').read_text()
        source='struct ScriptContext; extern struct ScriptContext *hostVm; extern const char hostEmpty[],hostFormat[];\n'+source.replace('(*(struct ScriptContext **)0x0300611C)','hostVm').replace('0x081AC6A0','hostEmpty').replace('0x081AC6A4','hostFormat')
        source=source.replace('__attribute__((section(".rom." x)))','')
        (folder/'native.c').write_text(source)
        (folder/'mock.c').write_text(r'''
#include "script_vm.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
const char hostEmpty[]="",hostFormat[]="%d";
struct ScriptContext context,*hostVm=&context;
struct ScriptExecutionState state;
unsigned char buffer[16];
int failAllocation,allocationSize,wrongHeap,randomCallCount;
u32 randomValues[2],seedValue;
void resetNative(void) {
 context.state=&state;state.heap=(void *)0x1234;
 failAllocation=allocationSize=wrongHeap=randomCallCount=0;
 randomValues[0]=0x1234;randomValues[1]=0x5678;
 memset(buffer,0xCC,sizeof(buffer));
}
void *HeapAlloc(void *heap,u32 size) {
 allocationSize=size;wrongHeap=heap!=state.heap;
 if(failAllocation || size>sizeof(buffer))return 0;
 return buffer;
}
s32 siprintf(char *text,const char *format,...) {
 int count;va_list args;va_start(args,format);count=vsprintf(text,format,args);va_end(args);return count;
}
u32 Random(void) { return randomValues[randomCallCount++]; }
void RandomSeed(u32 seed) { seedValue=seed; }
u32 sub_08080E4C(u32 dividend,u32 divisor) { return dividend%divisor; }
s32 sub_08082640(const char *text) { return strtol(text,0,10); }
s32 ParseDecimalInteger(const char *text) { return sub_08082640(text); }
''')
        library=folder/'native.so'
        subprocess.run(['gcc','-shared','-fPIC','-O2','-fno-builtin',
                        '-I'+str(ROOT/'include'),str(folder/'native.c'),str(folder/'mock.c'),
                        '-o',str(library)],check=True)
        cls.lib=ctypes.CDLL(str(library))
        for name in ('ScriptNativeAbs','ScriptNativeMin','ScriptNativeMax'):
            getattr(cls.lib,name).argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
        cls.lib.ScriptNativeCharacterCode.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_uint32)]
        cls.lib.ScriptNativeCharacterString.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_uint32),ctypes.POINTER(ctypes.c_void_p)]
        cls.lib.ScriptNativeIntegerString.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_void_p)]
        cls.lib.ScriptNativeRandomRange.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_uint32),ctypes.POINTER(ctypes.c_uint32)]
        cls.lib.ScriptNativeSeedRandom.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_uint32),ctypes.POINTER(ctypes.c_uint32)]
        cls.lib.ScriptNativeParseInteger.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_int32)]
        cls.lib.ScriptNativeCompareStrings.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_int32)]
        cls.lib.ScriptNativeStringLength.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_uint32)]

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def setUp(self):self.lib.resetNative()

    def test_abs_and_signed_extrema(self):
        for value in (0,1,-1,0x7FFFFFFF,-0x80000000):
            arg=ctypes.c_int32(value);out=ctypes.c_int32()
            self.assertEqual(self.lib.ScriptNativeAbs(1,ctypes.byref(arg),ctypes.byref(out)),1)
            self.assertEqual(out.value,ctypes.c_int32(abs(value)).value)
        values=(ctypes.c_int32*5)(0,-0x80000000,7,0x7FFFFFFF,-2)
        for name,expected in [('ScriptNativeMin',-0x80000000),('ScriptNativeMax',0x7FFFFFFF)]:
            out=ctypes.c_int32();fn=getattr(self.lib,name)
            self.assertEqual(fn(5,values,ctypes.byref(out)),1);self.assertEqual(out.value,expected)
            self.assertEqual(fn(0,values,ctypes.byref(out)),1);self.assertEqual(out.value,0)

    def test_all_two_byte_inputs_and_null_fallback(self):
        source=(ctypes.c_char_p*1)();out=ctypes.c_uint32()
        for code in range(65536):
            high,low=divmod(code,256);source[0]=bytes((high,low,0))
            expected=code if 0x80<=high<=0x9F or high>=0xE0 else high
            self.assertEqual(self.lib.ScriptNativeCharacterCode(1,source,ctypes.byref(out)),1)
            self.assertEqual(out.value,expected,hex(code))
        source[0]=None
        self.assertEqual(self.lib.ScriptNativeCharacterCode(1,source,ctypes.byref(out)),1)
        self.assertEqual(out.value,0)

    def test_character_strings_preserve_byte_order_and_original_truncation(self):
        for value,expected in [(0,b'\0\0'),(65,b'A\0'),(255,b'\xff\0'),
                               (0xF056,b'\xf0\x56\0'),(0xF040,b'\xf0\x40\0'),
                               (0x12345678,b'\x56\x78\0')]:
            self.lib.resetNative();arg=ctypes.c_uint32(value);out=ctypes.c_void_p()
            self.assertEqual(self.lib.ScriptNativeCharacterString(1,ctypes.byref(arg),ctypes.byref(out)),1)
            self.assertEqual(ctypes.string_at(out,len(expected)),expected)
            self.assertEqual(ctypes.c_int.in_dll(self.lib,'allocationSize').value,4)
            self.assertEqual(ctypes.c_int.in_dll(self.lib,'wrongHeap').value,0)

    def test_integer_bounds_and_allocation_failures(self):
        for value in (-0x80000000,-1,0,0x7FFFFFFF):
            arg=ctypes.c_int32(value);out=ctypes.c_void_p()
            self.assertEqual(self.lib.ScriptNativeIntegerString(1,ctypes.byref(arg),ctypes.byref(out)),1)
            self.assertEqual(ctypes.string_at(out),str(value).encode())
            self.assertEqual(ctypes.c_int.in_dll(self.lib,'allocationSize').value,16)
        ctypes.c_int.in_dll(self.lib,'failAllocation').value=1
        for name,kind in [('ScriptNativeIntegerString',ctypes.c_int32),('ScriptNativeCharacterString',ctypes.c_uint32)]:
            arg=kind(42);out=ctypes.c_void_p(0x5678)
            self.assertEqual(getattr(self.lib,name)(1,ctypes.byref(arg),ctypes.byref(out)),-1)
            self.assertEqual(out.value,0x5678)

    def test_random_and_integer_native_adapters(self):
        bound=ctypes.c_uint32(97);out=ctypes.c_uint32()
        combined=0x1234 | (0x5678 << 15)
        self.assertEqual(self.lib.ScriptNativeRandomRange(1,ctypes.byref(bound),ctypes.byref(out)),1)
        self.assertEqual(out.value,combined%97)
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'randomCallCount').value,2)
        seed=ctypes.c_uint32(0x89ABCDEF)
        self.assertEqual(self.lib.ScriptNativeSeedRandom(1,ctypes.byref(seed),ctypes.byref(out)),1)
        self.assertEqual(ctypes.c_uint32.in_dll(self.lib,'seedValue').value,seed.value)
        text=(ctypes.c_char_p*1)(b'-2048');parsed=ctypes.c_int32()
        self.assertEqual(self.lib.ScriptNativeParseInteger(1,text,ctypes.byref(parsed)),1)
        self.assertEqual(parsed.value,-2048)

    def test_string_comparison_and_length(self):
        out=ctypes.c_int32()
        for left,right,expected in ((b'a',b'b',-1),(b'b',b'a',1),(b'a',b'a',0),(None,b'',0)):
            args=(ctypes.c_char_p*2)(left,right)
            self.assertEqual(self.lib.ScriptNativeCompareStrings(2,args,ctypes.byref(out)),1)
            self.assertEqual(out.value,expected)
        args=(ctypes.c_char_p*1)(b'Ginta');length=ctypes.c_uint32()
        self.assertEqual(self.lib.ScriptNativeStringLength(1,args,ctypes.byref(length)),1)
        self.assertEqual(length.value,5)


if __name__=='__main__':unittest.main()
