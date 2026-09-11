"""Exercise the actual localization C with generated mappings on the host.
This verifies lookup/fallback/row bounds, not a complete emulator playthrough.
"""
import ctypes
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import build_english
import english_layout

ROOT=build_english.ROOT


class EnglishRuntimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory();folder=Path(cls.temp.name)
        cls.entries,cls.rejected=build_english.collect()
        lines=['#include "english.h"','const struct EnglishRowMapping gEnglishRows[] = {']
        for raw,rows in cls.entries:
            fields=[build_english.literal(row[:-1]) for row in rows]+['0']*(3-len(rows))
            lines.append('{'+build_english.literal(raw)+', {'+', '.join(fields)+'}, '+str(len(rows))+'},')
        lines+=['};','const u32 gEnglishRowCount = '+str(len(cls.entries))+';',
                'int captured_count, captured_mode; const char *captured_rows[3];',
                'void *DialogueStartOriginal(int mode,int count,const char **rows,int *result) {',
                'int i;captured_mode=mode;captured_count=count;',
                'for(i=0;i<count && i<3;i++)captured_rows[i]=rows[i];',
                'if(result)*result=77;return (void *)0x1234;}']
        source=folder/'data.c';source.write_text('\n'.join(lines))
        library=folder/'runtime.so'
        subprocess.run(['gcc','-shared','-fPIC','-O2','-I'+str(ROOT/'include'),
                        str(ROOT/'src/english/dialogue_runtime.c'),str(source),'-o',str(library)],check=True)
        cls.lib=ctypes.CDLL(str(library))
        cls.lib.EnglishTranslateRows.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_char_p)]
        cls.lib.EnglishDialogueStart.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.POINTER(ctypes.c_char_p),ctypes.POINTER(ctypes.c_int)]
        cls.lib.EnglishDialogueStart.restype=ctypes.c_void_p

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def call(self,mode,rows):
        source=(ctypes.c_char_p*len(rows))(*rows)
        output=(ctypes.c_char_p*3)()
        n=self.lib.EnglishTranslateRows(mode,len(rows),source,output)
        return n,list(output)[:n]

    def test_every_generated_mapping_reaches_actual_c_output(self):
        self.assertGreater(len(self.entries),1000)
        for raw,rows in self.entries:
            n,output=self.call(0,[raw])
            self.assertEqual(output,[r[:-1] for r in rows],raw)
            self.assertEqual(n,len(rows))
            for row in rows:
                visible=build_english.PREFIX.sub(b'',row[:-1],count=1)
                self.assertLessEqual(len(visible),42)
                self.assertEqual(len(visible)%2,0)
                self.assertLess(len(row),128)

    def test_capacity_modes_unknown_and_ambiguous_rows(self):
        one=next(raw for raw,rows in self.entries if len(rows)==1)
        two=next(raw for raw,rows in self.entries if len(rows)==2)
        self.assertEqual(self.call(0,[one,one,one])[0],3)
        self.assertEqual(self.call(1,[one,one])[0],2)
        self.assertEqual(self.call(1,[one,one,one])[0],0)
        self.assertEqual(self.call(1,[two,one])[0],0)
        self.assertEqual(self.call(0,[two,two])[0],0)
        self.assertEqual(self.call(2,[one])[0],0)
        self.assertEqual(self.call(0,[b'unmapped test string'])[0],0)
        self.assertEqual(self.call(0,[None])[0],0)

    def test_wrapper_preserves_abi_and_falls_back_whole_message(self):
        raw,expected=next((raw,rows) for raw,rows in self.entries if len(rows)==2)
        source=(ctypes.c_char_p*1)(raw);result=ctypes.c_int(-1)
        ptr=self.lib.EnglishDialogueStart(0,1,source,ctypes.byref(result))
        self.assertEqual(ptr,0x1234);self.assertEqual(result.value,77)
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'captured_count').value,2)
        source=(ctypes.c_char_p*2)(raw,b'unknown second row')
        self.lib.EnglishDialogueStart(1,2,source,ctypes.byref(result))
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'captured_mode').value,1)
        self.assertEqual(ctypes.c_int.in_dll(self.lib,'captured_count').value,2)
        captured=(ctypes.c_char_p*3).in_dll(self.lib,'captured_rows')
        self.assertEqual(list(captured)[:2],[raw,b'unknown second row'])


if __name__=='__main__':unittest.main()
