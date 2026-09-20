#!/usr/bin/env python3
"""Build linker input for decoded ROM assets from their placement manifest."""

import argparse
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent


def load_sections(manifest_path, group):
    manifest = json.loads(manifest_path.read_text())
    if manifest.get("format") != 1:
        raise ValueError(f"{manifest_path}: unsupported manifest format")
    sections = [entry for entry in manifest["sections"] if entry["group"] == group]
    if not sections:
        raise ValueError(f"{manifest_path}: unknown or empty group {group!r}")
    return sections


def replace_sources(manifest_path, replacements):
    """Update asset paths after a lossless source-format migration."""
    manifest = json.loads(manifest_path.read_text())
    changed = False
    for entry in manifest["sections"]:
        source = entry.get("source")
        if source is None:
            continue
        for old, new in replacements:
            updated = source.replace(old, new)
            if updated != source:
                entry["source"] = source = updated
                changed = True
    if changed:
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")


def write_assembly(sections, output, english_scripts=False):
    lines = [
        "@ Generated ROM data placement; do not edit.",
        "@ Payloads come from human-editable sources and their build tools.",
    ]
    for entry in sections:
        start = int(entry["start"], 16)
        end = int(entry["end"], 16)
        lines += ["", f"@ ROM {start:06X}..{end:06X} ({entry['kind']})",
                  f'\t.section .rom.{start:08X}, "a"']
        if "fill" in entry:
            lines.append(f"\t.fill {end - start}, 1, 0x{entry['fill']:02X}")
            continue
        if "word_fill" in entry:
            size = end - start
            if size % 4:
                raise ValueError(
                    f"ROM {start:06X}..{end:06X}: word fill is not word-aligned"
                )
            value = entry["word_fill"]
            if isinstance(value, str):
                value = int(value, 16)
            lines.append(f"\t.fill {size // 4}, 4, 0x{value:08X}")
            continue

        source = entry["source"]
        if english_scripts:
            source = source.replace("build/scripts/nfp/", "build/english/scripts/nfp/")
        source_path = ROOT / source
        if not source_path.exists():
            raise FileNotFoundError(f"{source}: generated payload is missing")
        actual_size = source_path.stat().st_size
        expected_size = end - start
        source_offset = entry.get("source_offset", 0)
        if "source_offset" not in entry and actual_size != expected_size:
            raise ValueError(
                f"{source}: expected {expected_size} bytes for ROM "
                f"{start:06X}..{end:06X}, found {actual_size}"
            )
        if source_offset + expected_size > actual_size:
            raise ValueError(
                f"{source}: needs bytes {source_offset}.."
                f"{source_offset + expected_size}, found {actual_size}"
            )
        if source_offset == 0 and actual_size == expected_size:
            incbin = f'\t.incbin "{source}"'
        else:
            incbin = f'\t.incbin "{source}", {source_offset}, {expected_size}'
        if "symbol" in entry:
            lines += [f'\t.global {entry["symbol"]}', f'{entry["symbol"]}:']
        lines.append(incbin)

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", type=Path)
    parser.add_argument("group")
    parser.add_argument("output", type=Path)
    parser.add_argument("--english-scripts", action="store_true")
    args = parser.parse_args()
    write_assembly(load_sections(args.manifest, args.group), args.output,
                   english_scripts=args.english_scripts)


if __name__ == "__main__":
    main()
