#!/usr/bin/env python3
"""Audit mechanically enforceable rules from docs/PRET_STANDARDS.md."""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATHS = tuple(sorted((ROOT / "src").glob("*.c")))
HEADER_PATHS = tuple(sorted((ROOT / "include").rglob("*.h")))
CODE_PATHS = SOURCE_PATHS + HEADER_PATHS

PROHIBITED = (
    ("forced_register", re.compile(r"\bregister\b[^;\n]*\basm\s*\(")),
    ("target_register_macro", re.compile(r"\bTARGET_REGISTER\s*\(")),
    ("inline_assembly", re.compile(r"(?<![A-Za-z0-9_])asm(?:\s+volatile)?\s*\(")),
    ("naked_function", re.compile(r"\bnaked\b")),
)
ROM_ADDRESS = re.compile(r"(?<![A-Za-z0-9_])0x0[89][0-9A-Fa-f]{6}(?![A-Za-z0-9_])")
AT_ADDRESS = re.compile(r"\bAT(?:_ROM)?\(\"([0-9A-Fa-f]{8})\"\)")
PLACEHOLDER_NAME = re.compile(r"^sub_08[0-9A-Fa-f]{6}$")
NONMATCHING_REASON = re.compile(
    r"non.?match|not instruction-matched|does not reproduce|remains outside the matching build|"
    r"mismatch|register allocation|instruction order|codegen|compiler|agbcc",
    re.IGNORECASE,
)


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def code_lines(path: Path):
    """Yield (line number, original line, comment/string-stripped code)."""
    in_comment = False
    for number, original in enumerate(path.read_text(errors="replace").splitlines(), 1):
        output = []
        index = 0
        while index < len(original):
            if in_comment:
                end = original.find("*/", index)
                if end < 0:
                    index = len(original)
                else:
                    in_comment = False
                    index = end + 2
                continue
            if original.startswith("//", index):
                break
            if original.startswith("/*", index):
                in_comment = True
                index += 2
                continue
            if original[index] in "\"'":
                quote = original[index]
                output.append(" ")
                index += 1
                while index < len(original):
                    if original[index] == "\\":
                        index += 2
                    elif original[index] == quote:
                        index += 1
                        break
                    else:
                        index += 1
                continue
            output.append(original[index])
            index += 1
        yield number, original, "".join(output)


def issue(kind: str, path: Path, line: int, detail: str, severity: str = "error"):
    return {
        "severity": severity,
        "kind": kind,
        "file": relative(path),
        "line": line,
        "detail": detail.strip()[:240],
    }


def audit_prohibited():
    findings = []
    for path in CODE_PATHS:
        for number, original, code in code_lines(path):
            matched = set()
            for kind, pattern in PROHIBITED:
                if pattern.search(code):
                    matched.add(kind)
            # TARGET_REGISTER's two preprocessor definitions are still useful
            # evidence, even though normal code stripping ignores directives.
            if "TARGET_REGISTER" in original and original.lstrip().startswith("#define"):
                matched.add("target_register_macro")
            for kind in sorted(matched):
                findings.append(issue(kind, path, number, original, "error"))
    return findings


def audit_rom_addresses():
    findings = []
    for path in CODE_PATHS:
        for number, original, code in code_lines(path):
            # AT() is placement metadata. PRET's raw-address rule concerns
            # addresses used by logic and tables, not linker section names.
            code = AT_ADDRESS.sub("", code)
            for match in ROM_ADDRESS.finditer(code):
                findings.append(issue("raw_rom_address", path, number,
                                      f"{match.group(0)} in {original.strip()}"))
    return findings


def audit_headers():
    findings = []
    for path in HEADER_PATHS:
        text = path.read_text(errors="replace")
        guarded = "#pragma once" in text or (
            re.search(r"^\s*#ifndef\s+\w+", text, re.MULTILINE)
            and re.search(r"^\s*#define\s+\w+", text, re.MULTILINE)
            and re.search(r"^\s*#endif\b", text, re.MULTILINE)
        )
        if not guarded:
            findings.append(issue("missing_include_guard", path, 1,
                                  "header has neither an include guard nor #pragma once"))
    return findings


def load_manifest():
    return json.loads((ROOT / "src/decompiled.json").read_text())


def audit_manifest(entries):
    findings = []
    for entry in entries:
        name = entry["name"]
        path = ROOT / entry["file"]
        if not path.exists() or not path.suffix == ".c":
            continue
        offset = f'{int(entry["addr"], 16) - 0x08000000:08X}'
        lines = path.read_text(errors="replace").splitlines()
        location = next((i for i, line in enumerate(lines)
                         if f'AT("{offset}")' in line), None)
        if location is None:
            continue
        declaration = " ".join(lines[location:min(len(lines), location + 5)])
        is_function = re.search(rf"\b{re.escape(name)}\s*\(", declaration) is not None
        if not is_function:
            continue
        if PLACEHOLDER_NAME.match(name):
            findings.append(issue("placeholder_function_name", path, location + 1,
                                  f"{entry['addr']} remains named {name}", "warning"))
        elif name and not name[0].isupper():
            findings.append(issue("non_pascal_function_name", path, location + 1,
                                  f"{entry['addr']} is named {name}", "warning"))

        before = "\n".join(lines[max(0, location - 12):location])
        if "/**" not in before or "*/" not in before:
            findings.append(issue("missing_doxygen", path, location + 1,
                                  f"{entry['addr']} {name}", "warning"))
    return findings


def audit_nonmatching():
    findings = []
    directory = ROOT / "src/nonmatching"
    for path in sorted(directory.glob("*.c")):
        text = path.read_text(errors="replace")
        if not NONMATCHING_REASON.search(text):
            findings.append(issue("undocumented_nonmatching", path, 1,
                                  "file does not state why its C fails to match", "warning"))
    return findings


def render_markdown(findings, manifest_count):
    counts = Counter(item["severity"] for item in findings)
    kinds = Counter(item["kind"] for item in findings)
    lines = [
        "# PRET standards audit",
        "",
        "Generated by `python3 tools/audit_pret_standards.py` from the mechanical",
        "rules in `docs/PRET_STANDARDS.md`. Semantic names, const correctness,",
        "structure accuracy, and whether a match is a fragile optimizer accident",
        "still require human review.",
        "",
        f"Manifest ranges reviewed: **{manifest_count}**",
        f"Errors: **{counts['error']}**",
        f"Warnings: **{counts['warning']}**",
        f"Documented low-level exceptions: **{counts['exception']}**",
        "",
        "## Findings by rule",
        "",
        "| Rule | Count |",
        "| --- | ---: |",
    ]
    lines.extend(f"| `{kind}` | {count} |" for kind, count in sorted(kinds.items()))
    lines += ["", "## Details", "",
              "| Severity | Rule | Location | Detail |",
              "| --- | --- | --- | --- |"]
    for item in findings:
        detail = item["detail"].replace("|", "\\|").replace("`", "'")
        lines.append(
            f"| {item['severity']} | `{item['kind']}` | "
            f"`{item['file']}:{item['line']}` | {detail} |"
        )
    lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", default="reports/code/pret-standards.json")
    parser.add_argument("--markdown", default="reports/code/pret-standards.md")
    parser.add_argument("--strict", action="store_true",
                        help="return failure while standards errors remain")
    args = parser.parse_args()

    entries = load_manifest()
    findings = (audit_prohibited() + audit_rom_addresses() + audit_headers()
                + audit_manifest(entries) + audit_nonmatching())
    findings.sort(key=lambda item: (item["severity"], item["kind"],
                                    item["file"], item["line"]))

    json_path = ROOT / args.json
    markdown_path = ROOT / args.markdown
    json_path.parent.mkdir(parents=True, exist_ok=True)
    markdown_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps(findings, indent=2) + "\n")
    markdown_path.write_text(render_markdown(findings, len(entries)))

    counts = Counter(item["severity"] for item in findings)
    print(f"PRET audit: {counts['error']} errors, {counts['warning']} warnings, "
          f"{counts['exception']} documented exceptions")
    if args.strict and counts["error"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
