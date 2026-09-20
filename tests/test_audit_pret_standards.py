import importlib.util
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "audit_pret_standards", ROOT / "tools" / "audit_pret_standards.py")
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


class PretPointerIntegerAuditTests(unittest.TestCase):
    def audit(self, source):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.c"
            path.write_text(source)
            return AUDIT.audit_pointer_integer_arithmetic((path,))

    def test_flags_pointer_cast_used_as_integer(self):
        findings = self.audit("void F(u8 *cursor) { u32 address = (u32)cursor + 4; }\n")
        self.assertEqual([item["kind"] for item in findings],
                         ["pointer_integer_arithmetic"])

    def test_allows_typed_pointer_arithmetic(self):
        findings = self.audit(
            "struct Entry { u32 value; };\n"
            "u32 F(struct Entry *entries, u32 index) { return entries[index].value; }\n")
        self.assertEqual(findings, [])

    def test_allows_byte_pointer_arithmetic(self):
        findings = self.audit(
            "u16 F(void *record, u32 offset) { return *(u16 *)((u8 *)record + offset); }\n")
        self.assertEqual(findings, [])

    def test_allows_integer_cast_of_pointed_to_field(self):
        findings = self.audit(
            "struct S { u32 width; };\n"
            "s32 F(struct S *map) { return (s32)map->width; }\n")
        self.assertEqual(findings, [])

    def test_uses_nearest_declaration_for_reused_name(self):
        findings = self.audit(
            "void A(u8 *value) { (void)value; }\n"
            "s32 B(u32 value) { return (s32)value; }\n")
        self.assertEqual(findings, [])

    def test_allows_explicit_thumb_pointer_encoding(self):
        findings = self.audit(
            "void Handler(void);\n"
            "void *p = (void *)((u32)Handler + 1);\n")
        self.assertEqual(findings, [])

    def test_allows_low_bit_test(self):
        findings = self.audit(
            "s32 F(u8 *pointer) { return ((u32)pointer & 3) == 0; }\n")
        self.assertEqual(findings, [])

    def test_allows_documented_linker_offset_recovery(self):
        findings = self.audit(
            "extern u8 gIwramBase[];\n"
            "extern u8 gMapGenerationRootOffset[];\n"
            "void *F(void) { return gIwramBase + (u32)gMapGenerationRootOffset; }\n")
        self.assertEqual(findings, [])

    def test_allows_serialized_sound_wave_pointer(self):
        findings = self.audit(
            "extern u8 *waveData;\n"
            "struct Tone { u32 wave; };\n"
            "struct Tone tone = { .wave = (u32)waveData };\n")
        self.assertEqual(findings, [])

    def test_reports_documented_recovery_as_exception(self):
        findings = self.audit(
            "void F(u8 *pointer) {\n"
            "    // PRET_PTR_INT_OK: operation=serialize address; "
            "evidence=hardware ABI consumes one word; typed=field is a u32 word\n"
            "    u32 address = (u32)pointer;\n"
            "}\n")
        self.assertEqual(findings[0]["severity"], "exception")
        self.assertIn("operation=serialize address",
                      findings[0]["detail"])
        self.assertNotIn("*/", findings[0]["detail"])

    def test_rejects_incomplete_exception_note(self):
        findings = self.audit(
            "void F(u8 *pointer) {\n"
            "    // PRET_PTR_INT_OK: needed for matching\n"
            "    u32 address = (u32)pointer;\n"
            "}\n")
        self.assertEqual(findings[0]["severity"], "warning")

    def test_markdown_counts_and_lists_exceptions(self):
        findings = self.audit(
            "void F(u8 *pointer) {\n"
            "    // PRET_PTR_INT_OK: operation=serialize address; "
            "evidence=hardware ABI consumes one word; typed=field is a u32 word\n"
            "    u32 address = (u32)pointer;\n"
            "}\n")
        report = AUDIT.render_markdown(findings, 1)
        self.assertIn("0 errors / 0 warnings / 1 documented exceptions", report)
        self.assertIn("pointer_integer_arithmetic", report)
        self.assertIn("operation=serialize address", report)

    def test_flags_pointer_integer_address_union(self):
        findings = self.audit(
            "union Address { u8 *pointer; u32 address; };\n")
        self.assertEqual([item["kind"] for item in findings],
                         ["pointer_integer_union"])

    def test_error_baseline_ignores_line_number_changes(self):
        baseline = [{
            "kind": "inline_assembly",
            "file": "src/example.c",
            "detail": "asm(\"\")",
        }]
        findings = [{
            "severity": "error",
            "kind": "inline_assembly",
            "file": "src/example.c",
            "line": 87,
            "detail": "asm(\"\")",
        }]
        status = AUDIT.compare_error_baseline(findings, baseline)
        self.assertEqual((status["known"], status["new"], status["resolved"]),
                         (1, 0, 0))

    def test_error_baseline_counts_duplicate_and_new_errors(self):
        baseline = [{
            "kind": "forced_register",
            "file": "src/example.c",
            "detail": "register u32 value asm(\"r0\");",
        }]
        repeated = dict(baseline[0], severity="error", line=2)
        findings = [repeated, dict(repeated, line=9)]
        status = AUDIT.compare_error_baseline(findings, baseline)
        self.assertEqual((status["known"], status["new"], status["resolved"]),
                         (1, 1, 0))
        self.assertEqual(status["new_findings"][0]["line"], 9)


if __name__ == "__main__":
    unittest.main()
