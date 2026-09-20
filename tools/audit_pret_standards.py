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
ALL_SOURCE_PATHS = tuple(sorted((ROOT / "src").rglob("*.c")))
HEADER_PATHS = tuple(sorted((ROOT / "include").rglob("*.h")))
CODE_PATHS = SOURCE_PATHS + HEADER_PATHS
POINTER_AUDIT_PATHS = ALL_SOURCE_PATHS + HEADER_PATHS

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
POINTER_DECLARATION = re.compile(
    r"\b(?:const\s+|volatile\s+|struct\s+\w+\s+|union\s+\w+\s+)*"
    r"(?:void|u8|s8|u16|s16|u32|s32|char|\w+)\s*\*+\s*(\w+)"
)
POINTER_ARRAY_DECLARATION = re.compile(
    r"\b(?:extern\s+)?(?:const\s+|volatile\s+)*"
    r"(?:void|u8|s8|u16|s16|u32|s32|char|\w+)\s+(\w+)\s*\["
)
SCALAR_DECLARATION = re.compile(
    r"\b(?:const\s+|volatile\s+)*(?:bool8|u8|s8|u16|s16|u32|s32|uintptr_t)"
    r"\s+(\w+)(?!\s*\()"
)
POINTER_INTEGER_CAST = re.compile(
    r"\(\s*(?:u32|s32|uintptr_t)\s*\)\s*(?:\(\s*)?"
    r"(?P<address>&\s*)?(?P<name>[A-Za-z_]\w*)"
    r"(?![A-Za-z0-9_]|\s*(?:->|\.|\[))"
)
THUMB_POINTER_ENCODING = re.compile(
    r"\(\s*void\s*\*\s*\)\s*\(\s*\(\s*u32\s*\)\s*"
    r"(?P<name>[A-Za-z_]\w*)\s*\+\s*1\s*\)"
)
ADDRESS_UNION = re.compile(r"\bunion\s+(\w+)\s*\{(?P<body>.*?)\};", re.DOTALL)
ADDRESS_INTEGER_MEMBER = re.compile(
    r"\b(?:u32|s32|uintptr_t)\s+(?:\w*(?:address|addr)\w*|raw)\s*;",
    re.IGNORECASE,
)
POINTER_MEMBER = re.compile(r"\b\w+(?:\s+\w+)?\s*\*+\s*\w+\s*;")
SERIALIZED_POINTER_FIELD = re.compile(
    r"\.\s*(?:wave|toneWave)\s*=\s*\(\s*u32\s*\)\s*(?P<name>[A-Za-z_]\w*)"
)
POINTER_DISPOSITION = re.compile(
    r"PRET_PTR_INT_OK:\s*(.*?)(?=\s*(?:\*/|//|\\?$))"
)
POINTER_DISPOSITION_FIELDS = re.compile(
    r"^operation=.+;\s*evidence=.+;\s*typed=.+$"
)


def error_fingerprint(item):
    """Return the stable identity used by CI for one hard error.

    Line numbers are deliberately excluded so an unrelated edit above a
    finding does not turn known debt into a false new error.  Duplicate
    findings remain significant because callers compare these as multisets.
    """
    return {
        "kind": item["kind"],
        "file": item["file"],
        "detail": item["detail"],
    }


def fingerprint_key(item):
    return (item["kind"], item["file"], item["detail"])


def compare_error_baseline(findings, baseline):
    """Return known, new, and resolved hard-error counts and new findings."""
    current_errors = [item for item in findings if item["severity"] == "error"]
    current_counts = Counter(fingerprint_key(item) for item in current_errors)
    baseline_counts = Counter(fingerprint_key(item) for item in baseline)
    new_counts = current_counts - baseline_counts
    resolved_counts = baseline_counts - current_counts
    new_findings = []
    remaining_baseline = baseline_counts.copy()
    for item in current_errors:
        key = fingerprint_key(item)
        if remaining_baseline[key]:
            remaining_baseline[key] -= 1
        else:
            new_findings.append(item)
    return {
        "known": sum((current_counts & baseline_counts).values()),
        "new": sum(new_counts.values()),
        "resolved": sum(resolved_counts.values()),
        "new_findings": new_findings,
    }


def load_asm_set_symbols():
    symbols = set()
    for path in (ROOT / "asm").rglob("*.s"):
        for match in re.finditer(r"^\s*\.set\s+(\w+)\s*,", path.read_text(errors="replace"),
                                 re.MULTILINE):
            symbols.add(match.group(1))
    return symbols


ASM_SET_SYMBOLS = load_asm_set_symbols()


def relative(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


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


def pointer_disposition(current: str, previous: str):
    """Return a complete pointer/integer exception note, if present.

    Every accepted note names the modeled operation, the ROM/caller/ABI
    evidence, and why a typed pointer does not express that operation.
    """
    disposition = POINTER_DISPOSITION.search(current)
    if disposition is None:
        disposition = POINTER_DISPOSITION.search(previous)
    if disposition is None:
        return None
    reason = disposition.group(1).strip()
    if not POINTER_DISPOSITION_FIELDS.match(reason):
        return None
    return reason


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


def audit_pointer_integer_arithmetic(paths=None):
    """Flag address values recast as integers for human review.

    Typed pointer addition is normal C and is deliberately not reported. A
    cast to an integer can be legitimate for a hardware encoding or bit mask,
    but it can also be a register-allocation steering trick. Regex cannot
    prove which case applies, so these findings are warnings.
    """
    findings = []
    if paths is None:
        paths = POINTER_AUDIT_PATHS
    for path in paths:
        relative_path = relative(path)
        # Assembly is not in POINTER_AUDIT_PATHS. Keep the explicit guard so
        # callers cannot accidentally broaden this audit into source wrappers
        # whose job is to express fixed machine interfaces.
        if relative_path.startswith("asm/") or relative_path == "include/gba/bios.h":
            continue
        lines = list(code_lines(path))
        documented_lines = set()
        name_kinds = {}
        previous_original = ""
        for number, original, code in lines:
            # The latest declaration before a cast usually belongs to the
            # current function and supersedes a same-named field or local in
            # an earlier function. This avoids file-wide name collisions.
            for name in SCALAR_DECLARATION.findall(code):
                name_kinds[name] = "scalar"
            for name in POINTER_DECLARATION.findall(code):
                name_kinds[name] = "pointer"
            for name in POINTER_ARRAY_DECLARATION.findall(code):
                name_kinds[name] = "pointer"
            thumb_names = {match.group("name")
                           for match in THUMB_POINTER_ENCODING.finditer(code)}
            serialized_names = {match.group("name")
                                for match in SERIALIZED_POINTER_FIELD.finditer(code)}
            cast_names = set()
            for match in POINTER_INTEGER_CAST.finditer(code):
                name = match.group("name")
                if name in thumb_names or name in serialized_names or name in ASM_SET_SYMBOLS:
                    continue
                if match.group("address") is None and name_kinds.get(name) != "pointer":
                    continue
                # An address used only with & or % is an alignment/low-bit
                # test, not integer address traversal.
                suffix = code[match.end():].lstrip(" )")
                if suffix.startswith("&") or suffix.startswith("%"):
                    continue
                cast_names.add(name)
            if cast_names:
                disposition = pointer_disposition(original, previous_original)
                severity = "exception" if disposition is not None else "warning"
                if disposition is not None:
                    documented_lines.add(
                        number if POINTER_DISPOSITION.search(original) else number - 1)
                prefix = ("documented deliberate recovery ("
                          + disposition + "): "
                          if disposition is not None else
                          "review pointer-to-integer cast(s) "
                          + ", ".join(sorted(cast_names))
                          + " for source authenticity: ")
                findings.append(issue(
                    "pointer_integer_arithmetic", path, number,
                    prefix + original.strip(), severity))
            previous_original = original

        # Complex integer-to-pointer expressions are deliberately outside the
        # conservative cast regex. Still enumerate every complete evidence
        # note so an accepted recovery can never become invisible in totals.
        original_lines = path.read_text(errors="replace").splitlines()
        for number, original in enumerate(original_lines, 1):
            disposition = POINTER_DISPOSITION.search(original)
            if disposition is None or number in documented_lines:
                continue
            reason = disposition.group(1).strip()
            if not POINTER_DISPOSITION_FIELDS.match(reason):
                continue
            findings.append(issue(
                "pointer_integer_arithmetic", path, number,
                "documented deliberate recovery (" + reason + "): "
                + original.strip(), "exception"))

        code_text = "\n".join(code for _, _, code in lines)
        for match in ADDRESS_UNION.finditer(code_text):
            body = match.group("body")
            if not POINTER_MEMBER.search(body) or not ADDRESS_INTEGER_MEMBER.search(body):
                continue
            line = code_text.count("\n", 0, match.start()) + 1
            detail = original_lines[line - 1].strip() if line <= len(original_lines) else match.group(1)
            nearby = "\n".join(original_lines[max(0, line - 4):line + 1])
            disposition_match = POINTER_DISPOSITION.search(nearby)
            disposition = (disposition_match.group(1).strip()
                           if disposition_match is not None
                           and POINTER_DISPOSITION_FIELDS.match(
                               disposition_match.group(1).strip())
                           else None)
            findings.append(issue(
                "pointer_integer_union", path, line,
                (("documented deliberate recovery (" + disposition
                  + "): ") if disposition else
                 "review pointer/integer address union for source authenticity: ")
                + detail, "exception" if disposition else "warning"))
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


def render_markdown(findings, manifest_count, baseline_status=None):
    counts = Counter(item["severity"] for item in findings)
    kinds = Counter(item["kind"] for item in findings)
    lines = [
        "# PRET standards audit",
        "",
        "Generated by `python3 tools/audit_pret_standards.py` from the mechanical",
        "rules in `docs/PRET_STANDARDS.md`. Semantic names, const correctness,",
        "structure accuracy, and whether a match is a fragile optimizer accident",
        "still require human review.",
        "The pointer/integer rule detects only explicit casts and address unions;",
        "zero findings do not rule out steering through declaration order, local",
        "lifetimes, type widths, control-flow spelling, or other C shapes.",
        "",
        f"Manifest ranges reviewed: **{manifest_count}**",
        (f"Audit totals: **{counts['error']} errors / {counts['warning']} warnings / "
         f"{counts['exception']} documented exceptions**"),
    ]
    if baseline_status is not None:
        lines.append(
            "CI hard-error baseline: "
            f"**{baseline_status['known']} known / {baseline_status['new']} new / "
            f"{baseline_status['resolved']} resolved**")
    lines += [
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
                        help="fail on every hard error, or only new errors when --baseline is used")
    parser.add_argument("--baseline",
                        help="JSON list of known hard-error fingerprints")
    args = parser.parse_args()

    entries = load_manifest()
    findings = (audit_prohibited() + audit_rom_addresses()
                + audit_pointer_integer_arithmetic() + audit_headers()
                + audit_manifest(entries) + audit_nonmatching())
    findings.sort(key=lambda item: (item["severity"], item["kind"],
                                    item["file"], item["line"]))

    baseline_status = None
    if args.baseline:
        baseline = json.loads((ROOT / args.baseline).read_text())
        baseline_status = compare_error_baseline(findings, baseline)

    json_path = ROOT / args.json
    markdown_path = ROOT / args.markdown
    json_path.parent.mkdir(parents=True, exist_ok=True)
    markdown_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps(findings, indent=2) + "\n")
    markdown_path.write_text(render_markdown(findings, len(entries), baseline_status))

    counts = Counter(item["severity"] for item in findings)
    print(f"PRET audit: {counts['error']} errors, {counts['warning']} warnings, "
          f"{counts['exception']} documented exceptions")
    if baseline_status is not None:
        print("PRET hard-error baseline: "
              f"{baseline_status['known']} known, {baseline_status['new']} new, "
              f"{baseline_status['resolved']} resolved")
        for item in baseline_status["new_findings"]:
            print(f"NEW {item['kind']}: {item['file']}:{item['line']}: {item['detail']}")
    if args.strict and ((baseline_status is None and counts["error"])
                        or (baseline_status is not None and baseline_status["new"])):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
