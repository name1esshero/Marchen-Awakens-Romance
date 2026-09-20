#!/usr/bin/env python3
"""Inventory unexplained numeric literals in C sources and headers.

This is a triage report, not a blanket style rule.  It ignores comments,
strings, preprocessor definitions, 0, and 1, then groups remaining literals by
kind and frequency.  Reviewers can use the ranked output to introduce enums,
flags, hardware constants, and typed structure fields where evidence supports
them without inventing names for ordinary arithmetic.
"""
from collections import Counter, defaultdict
from pathlib import Path
import argparse
import json
import re

ROOT = Path(__file__).resolve().parents[1]
NUMBER = re.compile(r"(?<![A-Za-z0-9_.])(?:0[xX][0-9A-Fa-f]+|[0-9]+)(?:[uUlL]*)")
STRING = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')

# These files are useful readable data definitions, but their initializer
# values drown out addresses, flags, sizes, and IDs in executable code.  They
# remain available through --include-data and are reported separately there.
DATA_SOURCE_FILES = {
    "src/battle_character_data.c",
    "src/definition_tables.c",
    "src/game_tables.c",
    "src/menu_text.c",
    "src/rom_padding.c",
    "src/song_headers.c",
    "src/song_tracks.c",
    "src/sound_tables.c",
}
EXCLUDED_PREFIXES = (
    "src/libc/",       # imported newlib algorithms, audited as a separate body
    "include/constants/",  # the named values this audit is meant to encourage
)


def strip_comments(lines):
    in_block = False
    for line in lines:
        result = []
        index = 0
        while index < len(line):
            if in_block:
                end = line.find("*/", index)
                if end < 0:
                    index = len(line)
                else:
                    in_block = False
                    index = end + 2
            else:
                block = line.find("/*", index)
                single = line.find("//", index)
                stops = [value for value in (block, single) if value >= 0]
                if not stops:
                    result.append(line[index:])
                    break
                stop = min(stops)
                result.append(line[index:stop])
                if stop == single:
                    break
                in_block = True
                index = stop + 2
        yield "".join(result)


def value_of(token):
    return int(re.sub(r"[uUlL]+$", "", token), 0)


def category(value):
    if 0x02000000 <= value <= 0x0203FFFF:
        return "ewram_address"
    if 0x03000000 <= value <= 0x03007FFF:
        return "iwram_address"
    if 0x04000000 <= value <= 0x040003FE:
        return "io_register"
    if 0x05000000 <= value <= 0x07FFFFFF:
        return "video_memory_address"
    if 0x08000000 <= value <= 0x09FFFFFF:
        return "rom_address"
    if value > 1 and value & (value - 1) == 0:
        return "power_of_two_or_flag"
    if value <= 0xFF:
        return "small_value_or_id"
    if value <= 0xFFFF:
        return "mask_or_size"
    return "numeric_constant"


def scan(paths):
    entries = []
    for path in paths:
        raw_lines = path.read_text(errors="replace").splitlines()
        for lineno, (raw, code) in enumerate(
                zip(raw_lines, strip_comments(raw_lines)), 1):
            if code.lstrip().startswith("#"):
                continue
            code = STRING.sub("", code)
            for match in NUMBER.finditer(code):
                token = match.group(0)
                value = value_of(token)
                if value in (0, 1):
                    continue
                entries.append({
                    "file": str(path.relative_to(ROOT)),
                    "line": lineno,
                    "literal": token,
                    "value": value,
                    "category": category(value),
                    "context": raw.strip()[:180],
                })
    return entries


def markdown(entries, excluded_data_files):
    by_category = Counter(entry["category"] for entry in entries)
    by_file = Counter(entry["file"] for entry in entries)
    by_literal = defaultdict(list)
    for entry in entries:
        by_literal[(entry["literal"], entry["value"], entry["category"])].append(entry)

    lines = [
        "# C numeric-literal audit",
        "",
        "This report ranks numeric literals that may benefit from named constants,",
        "enums, flags, or typed fields. It intentionally includes legitimate",
        "arithmetic constants; each result still needs call-site evidence before",
        "renaming.",
        "",
        f"Total review candidates: **{len(entries)}**",
        "",
    ]
    if excluded_data_files:
        lines += [
            "Generated and table-heavy sources are excluded by default so the",
            "ranking reflects executable code. Pass `--include-data` to audit",
            "their initializer values too.",
            "",
            f"Excluded data sources: **{len(excluded_data_files)}**",
            "",
        ]
    lines += [
        "## Categories",
        "",
        "| Category | Occurrences |",
        "| --- | ---: |",
    ]
    lines.extend(f"| `{name}` | {count} |"
                 for name, count in by_category.most_common())
    lines += ["", "## Highest-density files", "",
              "| File | Occurrences |", "| --- | ---: |"]
    lines.extend(f"| `{name}` | {count} |" for name, count in by_file.most_common(40))
    lines += ["", "## Repeated literals", "",
              "| Literal | Category | Occurrences | Example |",
              "| --- | --- | ---: | --- |"]
    ranked = sorted(by_literal.items(), key=lambda item: (-len(item[1]), item[0][1]))
    for (literal, _value, kind), uses in ranked[:100]:
        example = uses[0]
        lines.append(
            f"| `{literal}` | `{kind}` | {len(uses)} | "
            f"`{example['file']}:{example['line']}` |")
    lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", default="reports/code/magic-numbers.json")
    parser.add_argument("--markdown", default="reports/code/magic-numbers.md")
    parser.add_argument("--include-data", action="store_true",
                        help="include table-heavy data initializer sources")
    args = parser.parse_args()
    paths = sorted((ROOT / "src").rglob("*.c")) + sorted((ROOT / "include").rglob("*.h"))
    excluded_data_files = []
    if not args.include_data:
        kept = []
        for path in paths:
            relative = str(path.relative_to(ROOT))
            if (relative in DATA_SOURCE_FILES or
                    relative.startswith(EXCLUDED_PREFIXES)):
                excluded_data_files.append(relative)
            else:
                kept.append(path)
        paths = kept
    entries = scan(paths)
    json_path = ROOT / args.json
    markdown_path = ROOT / args.markdown
    json_path.parent.mkdir(parents=True, exist_ok=True)
    markdown_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps(entries, indent=2) + "\n")
    markdown_path.write_text(markdown(entries, excluded_data_files))
    print(f"audited {len(paths)} functional files; "
          f"{len(entries)} numeric literals need review")


if __name__ == "__main__":
    main()
