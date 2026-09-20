import importlib.util
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "audit_shiftability", ROOT / "tools" / "audit_shiftability.py")
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


class ShiftabilityAuditTests(unittest.TestCase):
    def audit(self, source, rom_aliases=None, header_aliases=None):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.c"
            path.write_text(source)
            with patch.object(AUDIT, "load_fixed_rom_aliases",
                              return_value=rom_aliases or {}), \
                 patch.object(AUDIT, "load_fixed_header_aliases",
                              return_value=header_aliases or {}):
                return AUDIT.audit_sources((path,))

    def test_ignores_at_placement_and_comments(self):
        findings = self.audit(
            '/* original address 0x08001234 */\n'
            'AT("00001234") void Function(void) {}\n')
        self.assertEqual(findings, [])

    def test_ignores_hardware_address(self):
        findings = self.audit(
            "void Function(void) { *(volatile u16 *)0x04000208 = 0; }\n")
        self.assertEqual(findings, [])

    def test_flags_raw_rom_and_ram_addresses(self):
        findings = self.audit(
            "void Function(void) {\n"
            "    const void *rom = (void *)0x08123456;\n"
            "    void *ram = (void *)0x03001234;\n"
            "}\n")
        self.assertEqual(
            {(item["kind"], item["region"]) for item in findings},
            {("raw_rom_address", "rom"), ("raw_ram_address", "ram")})

    def test_flags_fixed_rom_linker_alias_use(self):
        findings = self.audit(
            "extern void FixedFunction(void);\n"
            "void Function(void) { FixedFunction(); }\n",
            rom_aliases={"FixedFunction": {
                "value": 0x08001234, "file": "asm/fixed.s", "line": 3}})
        self.assertEqual(findings[0]["kind"], "fixed_rom_linker_alias")

    def test_flags_fixed_header_alias_use(self):
        findings = self.audit(
            "void Function(void) { gRuntime->value = 1; }\n",
            header_aliases={"gRuntime": {
                "value": 0x03001234, "region": "ram",
                "file": "include/runtime.h", "line": 4}})
        self.assertEqual(findings[0]["kind"], "fixed_header_address_alias")

    def test_matching_summary_includes_manifest_metrics(self):
        with patch.object(AUDIT, "manifest_stats", return_value={"ranges": 1}):
            summary = AUDIT.summarize((), (), include_manifest=True)
        self.assertEqual(summary["manifest"], {"ranges": 1})


if __name__ == "__main__":
    unittest.main()
