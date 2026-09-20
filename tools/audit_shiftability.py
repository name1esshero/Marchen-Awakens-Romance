#!/usr/bin/env python3
"""Audit whether linked C refers to software objects through relocatable symbols."""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter, defaultdict
from pathlib import Path

try:
    from tools.audit_pret_standards import code_lines
except ImportError:
    from audit_pret_standards import code_lines


ROOT = Path(__file__).resolve().parents[1]
MATCHING_SOURCES = tuple(sorted((ROOT / "src").glob("*.c"))) + tuple(
    sorted((ROOT / "src" / "libc").glob("*.c"))
)
ENGLISH_SOURCES = tuple(sorted((ROOT / "src" / "english").glob("*.c")))
NONMATCHING_SOURCES = tuple(sorted((ROOT / "src" / "nonmatching").glob("*.c")))
HEADER_PATHS = tuple(sorted((ROOT / "include").rglob("*.h")))

AT_ADDRESS = re.compile(r'\bAT(?:_ROM)?\("[0-9A-Fa-f]{8}"\)')
ABSOLUTE_WORD = re.compile(r"(?<![A-Za-z0-9_])0x([0-9A-Fa-f]{8})(?![A-Za-z0-9_])")
IDENTIFIER = re.compile(r"\b[A-Za-z_]\w*\b")
ASM_ALIAS = re.compile(
    r"^\s*\.(?:set|thumb_set)\s+(\w+)\s*,\s*(0x[0-9A-Fa-f]+)\s*$",
    re.MULTILINE,
)
ADDRESS_MACRO = re.compile(
    r"^\s*#\s*define\s+(\w+)\b.*?(0x[0-9A-Fa-f]{8})"
)


def relative(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def software_region(value: int):
    if 0x02000000 <= value < 0x04000000:
        return "ram"
    if 0x08000000 <= value < 0x0A000000:
        return "rom"
    return None


def load_fixed_rom_aliases():
    aliases = {}
    for path in (ROOT / "asm").rglob("*.s"):
        text = path.read_text(errors="replace")
        for match in ASM_ALIAS.finditer(text):
            value = int(match.group(2), 16)
            if software_region(value) == "rom":
                line = text.count("\n", 0, match.start()) + 1
                aliases[match.group(1)] = {
                    "value": value,
                    "file": relative(path),
                    "line": line,
                }
    return aliases


def load_fixed_header_aliases():
    aliases = {}
    for path in HEADER_PATHS:
        for number, original, _ in code_lines(path):
            match = ADDRESS_MACRO.search(original)
            if not match:
                continue
            value = int(match.group(2), 16)
            region = software_region(value)
            if region is None:
                continue
            aliases[match.group(1)] = {
                "value": value,
                "region": region,
                "file": relative(path),
                "line": number,
            }
    return aliases


def finding(kind, path, line, detail, region):
    return {
        "kind": kind,
        "region": region,
        "file": relative(path),
        "line": line,
        "detail": detail,
    }


def audit_sources(paths):
    fixed_rom_aliases = load_fixed_rom_aliases()
    fixed_header_aliases = load_fixed_header_aliases()
    findings = []
    alias_uses = defaultdict(list)
    header_uses = defaultdict(list)

    for path in paths:
        for number, original, code in code_lines(path):
            code = AT_ADDRESS.sub("", code)
            for match in ABSOLUTE_WORD.finditer(code):
                value = int(match.group(1), 16)
                region = software_region(value)
                if region is not None:
                    findings.append(finding(
                        f"raw_{region}_address", path, number,
                        f"0x{value:08X} in {original.strip()}", region))

            stripped = code.lstrip()
            if stripped.startswith("extern "):
                continue
            names = set(IDENTIFIER.findall(code))
            for name in names & fixed_rom_aliases.keys():
                alias_uses[(path, name)].append(number)
            if not stripped.startswith("#define"):
                for name in names & fixed_header_aliases.keys():
                    header_uses[(path, name)].append(number)

    for (path, name), lines in sorted(alias_uses.items(),
                                       key=lambda item: (str(item[0][0]), item[0][1])):
        alias = fixed_rom_aliases[name]
        findings.append(finding(
            "fixed_rom_linker_alias", path, lines[0],
            (f"{name} is fixed at 0x{alias['value']:08X} by "
             f"{alias['file']}:{alias['line']} ({len(lines)} use(s) in this file)"),
            "rom"))

    for (path, name), lines in sorted(header_uses.items(),
                                       key=lambda item: (str(item[0][0]), item[0][1])):
        alias = fixed_header_aliases[name]
        findings.append(finding(
            "fixed_header_address_alias", path, lines[0],
            (f"{name} expands to 0x{alias['value']:08X} at "
             f"{alias['file']}:{alias['line']} ({len(lines)} use(s) in this file)"),
            alias["region"]))

    findings.sort(key=lambda item: (item["region"], item["kind"],
                                    item["file"], item["line"]))
    return findings


def manifest_stats(blocked_files):
    entries = json.loads((ROOT / "src" / "decompiled.json").read_text())
    entries = [entry for entry in entries
               if entry.get("file", "").startswith("src/")
               and entry.get("file", "").endswith(".c")
               and "/nonmatching/" not in entry["file"]]
    total_bytes = sum(int(entry["size"]) for entry in entries)
    clean_entries = [entry for entry in entries if entry["file"] not in blocked_files]
    clean_bytes = sum(int(entry["size"]) for entry in clean_entries)
    return {
        "ranges": len(entries),
        "clean_ranges_lower_bound": len(clean_entries),
        "bytes": total_bytes,
        "clean_bytes_lower_bound": clean_bytes,
    }


def summarize(paths, findings, include_manifest=False):
    files = {relative(path) for path in paths}
    rom_blocked = {item["file"] for item in findings if item["region"] == "rom"}
    all_blocked = {item["file"] for item in findings}
    summary = {
        "source_files": len(files),
        "rom_reference_shiftable_files": len(files - rom_blocked),
        "fully_symbolic_software_reference_files": len(files - all_blocked),
        "rom_blocked_files": len(rom_blocked),
        "all_blocked_files": len(all_blocked),
        "findings": len(findings),
        "findings_by_kind": dict(sorted(Counter(
            item["kind"] for item in findings).items())),
    }
    if include_manifest:
        summary["manifest"] = manifest_stats(all_blocked)
    return summary


def percent(part, whole):
    return 100.0 * part / whole if whole else 100.0


def render_scope(name, summary):
    files = summary["source_files"]
    return (
        f"| {name} | {summary['rom_reference_shiftable_files']} / {files} "
        f"({percent(summary['rom_reference_shiftable_files'], files):.2f}%) | "
        f"{summary['fully_symbolic_software_reference_files']} / {files} "
        f"({percent(summary['fully_symbolic_software_reference_files'], files):.2f}%) |"
    )


def render_markdown(scopes, findings):
    summary = scopes["matching"]
    all_summary = scopes["all"]
    manifest = summary["manifest"]
    lines = [
        "# C shiftability audit",
        "",
        "This report measures whether C refers to software objects through relocatable",
        "linker symbols. `AT()` placement metadata is excluded from the score: exact",
        "placement is an expected property of the byte-matching linker, rather than a",
        "hardcoded reference inside C logic.",
        "Hardware registers and GBA VRAM, palette, OAM, and SRAM addresses are fixed by",
        "the platform and are not blockers. EWRAM/IWRAM literals are listed separately",
        "because they do not block ROM movement but should become named runtime symbols.",
        "",
        "",
        "| Scope | Relocatable ROM references | No raw software addresses |",
        "| --- | ---: | ---: |",
        render_scope("Matching build", scopes["matching"]),
        render_scope("English-only C", scopes["english"]),
        render_scope("Nonmatching candidates", scopes["nonmatching"]),
        render_scope("All tracked C", scopes["all"]),
        "",
        (f"Conservative clean manifest ranges: **{manifest['clean_ranges_lower_bound']} / "
         f"{manifest['ranges']} ({percent(manifest['clean_ranges_lower_bound'], manifest['ranges']):.2f}%)**"),
        (f"Conservative clean manifest bytes: **{manifest['clean_bytes_lower_bound']:,} / "
         f"{manifest['bytes']:,} ({percent(manifest['clean_bytes_lower_bound'], manifest['bytes']):.2f}%)**"),
        "The matching build intentionally fixes section order and addresses. A future",
        "expanded or modern link may relax that placement independently of this score.",
        "",
        "The manifest figures are lower bounds: one blocker marks its whole source file",
        "because generated tables and macro-defined functions do not provide reliable",
        "source spans for every individual manifest range.",
        "",
        "## Findings by kind",
        "",
        "| Kind | Count |",
        "| --- | ---: |",
    ]
    lines.extend(f"| `{kind}` | {count} |"
                 for kind, count in all_summary["findings_by_kind"].items())
    lines += ["", "## Details", "",
              "| Region | Kind | Location | Detail |",
              "| --- | --- | --- | --- |"]
    for item in findings:
        detail = item["detail"].replace("|", "\\|").replace("`", "'")
        lines.append(
            f"| {item['region']} | `{item['kind']}` | "
            f"`{item['file']}:{item['line']}` | {detail} |"
        )
    lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", default="reports/code/shiftability.json")
    parser.add_argument("--markdown", default="reports/code/shiftability.md")
    args = parser.parse_args()

    scopes = {}
    scope_paths = {
        "matching": MATCHING_SOURCES,
        "english": ENGLISH_SOURCES,
        "nonmatching": NONMATCHING_SOURCES,
        "all": MATCHING_SOURCES + ENGLISH_SOURCES + NONMATCHING_SOURCES,
    }
    scope_findings = {}
    for name, paths in scope_paths.items():
        scope_findings[name] = audit_sources(paths)
        scopes[name] = summarize(paths, scope_findings[name],
                                 include_manifest=(name == "matching"))
    findings = scope_findings["all"]
    summary = scopes["matching"]
    json_path = ROOT / args.json
    markdown_path = ROOT / args.markdown
    json_path.parent.mkdir(parents=True, exist_ok=True)
    markdown_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps({"scopes": scopes, "findings": findings},
                                    indent=2) + "\n")
    markdown_path.write_text(render_markdown(scopes, findings))

    files = summary["source_files"]
    print(
        "Shiftability audit: "
        f"{summary['rom_reference_shiftable_files']}/{files} "
        f"({percent(summary['rom_reference_shiftable_files'], files):.2f}%) "
        "matching C files have relocatable ROM references; "
        f"{summary['fully_symbolic_software_reference_files']}/{files} "
        f"({percent(summary['fully_symbolic_software_reference_files'], files):.2f}%) "
        "have no raw software addresses"
    )
    all_summary = scopes["all"]
    print(
        "All tracked C: "
        f"{all_summary['rom_reference_shiftable_files']}/"
        f"{all_summary['source_files']} "
        f"({percent(all_summary['rom_reference_shiftable_files'], all_summary['source_files']):.2f}%) "
        "files have relocatable ROM references"
    )
    print("Matching-link placement is fixed by design and is not part of this score")


if __name__ == "__main__":
    main()
