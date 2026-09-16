#!/usr/bin/env python3
"""Build English-only marscript-aware script bytes for mar_english.gba.

MUST NEVER be wired into the base mar.gba build: swapping a script's bytes
here is only safe because mar_english.gba already intentionally diverges
from baserom.gba, whereas mar.gba's whole purpose is to match it exactly
(PRET_STANDARDS.md's Golden Rule). Reusing this for the base build -- even
for a script that happens to compile back byte-identical today -- would
make every future marscript edit silently apply to the Japanese ROM too.
That's why this is a separate module from named_scripts.py rather than a
change to its existing build(), which stays exactly as it was and keeps
driving mar.gba unchanged.

For a script with no marscript override, this reproduces named_scripts.py's
existing build() output exactly (same text/nfp/*.txt translations, same
maps/events/*.json integer edits) -- marscript is additive, not a
replacement for those already-working mechanisms.

For a script with an override (scripts/marscript/<NAME>.marscript):
  - if the compiled (and, if the original was stored compressed,
    recompressed) bytes fit within the original archive member's size,
    they are placed there directly, padded with the original member's own
    trailing bytes to keep its size exactly unchanged (matching
    named_scripts.py's own compressed-edit padding convention) -- no ROM
    layout impact, safe to .incbin at the script's existing fixed address.
  - if they don't fit, the *original* bytes are kept at the original
    archive location (so nothing else in the ROM shifts) and the new,
    larger content is instead routed to the English build's ROM expansion
    region. See split_scripts_english.py (generates the asm that places
    both the original-slot and expansion-region content) and
    resource_catalog.py's --english output (re-points that one entry's
    catalog record at the expansion symbol) for the other halves of this
    mechanism.
"""
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lz77
import marscript
import named_scripts
import script_events
from script_assembler import assemble

ROOT = Path(__file__).resolve().parent.parent
MARSCRIPT_DIR = ROOT / 'scripts/marscript'
ENGLISH_SCRIPT_DIR = ROOT / 'build/english/scripts/nfp'
ENGLISH_EXPANSION_DIR = ROOT / 'build/english/scripts_expansion'
EXPANSION_MANIFEST = ROOT / 'build/english/scripts_expansion_manifest.json'


def override_path(name):
    return MARSCRIPT_DIR / (name + '.marscript')


def marscript_identifier(name):
    """The script-name identifier decompile_to_source() would have used for
    this manifest entry -- marscript identifiers can't contain '.' or '-'.
    """
    return name.replace('.', '_').replace('-', '_')


def compiled_bytes(name, source):
    document, parsed_name = marscript.compile_source(source)
    expected = marscript_identifier(name)
    if parsed_name != expected:
        raise ValueError(f'{override_path(name)}: script is named {parsed_name!r}, '
                          f'expected {expected!r} for manifest entry {name!r}')
    return assemble(document)


def plan(manifest):
    """Decide, for every script, what build/english/scripts/nfp/<name>.bin
    should contain, and which scripts (if any) additionally need a spot in
    the expansion region because their override no longer fits.

    Returns (per_script_output, expansions): per_script_output maps every
    manifest name to the exact bytes for its *original* archive slot
    (always exactly len(original) bytes long); expansions maps only the
    names that overflowed to their new, larger compiled bytes.
    """
    per_script_output = {}
    expansions = {}
    for e in manifest:
        name = e['name']
        original = (ROOT / e['path']).read_bytes()
        override = override_path(name)
        if not override.exists():
            # Exactly named_scripts.py's existing, already-working pipeline
            # -- marscript changes nothing for a script nobody has edited
            # through it yet.
            per_script_output[name] = script_events.build(
                named_scripts.rebuild(original, named_scripts.edits(ROOT / e['text'])),
                original, name, ROOT)
            continue
        raw = compiled_bytes(name, override.read_text(encoding='utf-8'))
        packed = lz77.compress(raw) if original[0] == 0x10 else raw
        if len(packed) <= len(original):
            per_script_output[name] = packed + original[len(packed):]
        else:
            per_script_output[name] = original
            expansions[name] = packed
    return per_script_output, expansions


def build():
    manifest = json.loads(named_scripts.MANIFEST.read_text())
    per_script_output, expansions = plan(manifest)

    ENGLISH_SCRIPT_DIR.mkdir(parents=True, exist_ok=True)
    for name, out in per_script_output.items():
        dest = ENGLISH_SCRIPT_DIR / (name + '.bin')
        if not dest.exists() or dest.read_bytes() != out:
            dest.write_bytes(out)

    expansion_names = sorted(expansions)
    ENGLISH_EXPANSION_DIR.mkdir(parents=True, exist_ok=True)
    for name in expansion_names:
        dest = ENGLISH_EXPANSION_DIR / (name + '.bin')
        out = expansions[name]
        if not dest.exists() or dest.read_bytes() != out:
            dest.write_bytes(out)

    # Downstream generators (split_scripts_english.py, resource_catalog.py
    # --english) need to agree on exactly which scripts were routed to the
    # expansion region without each re-deriving it (re-running every
    # override through marscript again).
    EXPANSION_MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    EXPANSION_MANIFEST.write_text(json.dumps(expansion_names, indent=2) + '\n')
    print(f'{len(per_script_output)} scripts placed at their original ROM location, '
          f'{len(expansion_names)} routed to the expansion region')


if __name__ == '__main__':
    build()
