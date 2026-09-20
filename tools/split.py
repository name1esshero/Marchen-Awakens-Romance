#!/usr/bin/env python3
"""Split the ROM into buildable sources.

Produces, in ROM order:
    asm/rom_header.s         cartridge header
    asm/code/code_*.s        nonempty disassembled code regions
    sound/sample_sections.json
                              PCM spans formerly interleaved with code
    data/rom_data_sections.json
                              map/script/graphics/raw placement manifest
    data/*.bin               nonuniform raw data not decoded yet

Each payload still has a section named after its ROM offset, so consolidating
the generated linker sections do not change placement. Uniform zero/FF regions
do not need opaque binary source files.
"""
import glob
import json
from pathlib import Path
import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import emit as emitter

ROM_BASE = 0x08000000
HEADER_END = 0xC0
CODE_END = 0x1B0000            # first offset past the executable region
ROM_END = 0xFFFF00             # everything after this is 0xFF padding
CODE_CHUNK = 0x8000
DATA_CHUNK = 0x40000
AGB_DEBUG_MONITOR_START = 0xFE0000
AGB_DEBUG_MONITOR_DATA_END = 0xFE3090
AGB_DEBUG_MONITOR_END = 0xFE4000
DEBUG_VECTOR_TABLE_START = 0xFFF000
DEBUG_VECTOR_TABLE_WORDS_START = 0xFFF800
DEBUG_VECTOR_TABLE_END = 0xFFFF00
DEBUG_VECTOR_ADDRESS = 0x09FFC000

# English builds replace these complete ROM sections with localized artwork.
# Keep them as independent objects so ld_english can omit only the Japanese
# source. Every other generated data wrapper can be grouped by asset class.
ENGLISH_REPLACEABLE_OFFSETS = {
    0x5AEAF0,                 # EFFECT.NCD
    0xDA8990, 0xDA9BF0,      # SM_BG12 graphics/map
    0xDD69E0,                 # SYSTEM.NCD
    0xF19AA0, 0xF1A320,      # T_C01 graphics/map
    0xF1EF50, 0xF21420,      # T_TTL06 graphics/map
}


def sect(off):
    return ".rom.%08X" % off


def write_header(rom):
    os.makedirs("asm", exist_ok=True)
    with open("data/nintendo_logo.bin", "wb") as f:
        f.write(rom[0x04:0xA0])
    title = rom[0xA0:0xAC].decode("ascii", "replace").rstrip("\0")
    title_pad = 12 - len(title)
    with open("asm/rom_header.s", "w") as f:
        f.write("""@ GBA cartridge header.
@
@ The first word is the reset branch; bytes 0x04..0x9F are the Nintendo logo,
@ which the BIOS checksums on boot and which therefore has to be reproduced
@ byte for byte. The remaining fields describe the cartridge to the BIOS.

\t.section %s, "ax"
\t.arm
\t.global rom_header
rom_header:
\tb Entry                       @ 0x00 reset vector

\t.incbin "data/nintendo_logo.bin"   @ 0x04 Nintendo logo (BIOS-verified)

\t.ascii "%s"
\t.space %d                          @ 0xA0 game title, NUL padded to 12
\t.ascii "%s"                    @ 0xAC game code
\t.ascii "%s"                      @ 0xB0 maker code
\t.byte 0x%02X                       @ 0xB2 fixed value
\t.byte 0x%02X                       @ 0xB3 main unit code
\t.byte 0x%02X                       @ 0xB4 device type
\t.space 7                           @ 0xB5 reserved
\t.byte 0x%02X                       @ 0xBC software version
\t.byte 0x%02X                       @ 0xBD header checksum
\t.space 2                           @ 0xBE reserved
""" % (sect(0), title, title_pad,
       rom[0xAC:0xB0].decode("ascii", "replace"),
       rom[0xB0:0xB2].decode("ascii", "replace"),
       rom[0xB2], rom[0xB3], rom[0xB4], rom[0xBC], rom[0xBD]))


def load_decompiled():
    """Ranges now provided by C in src/, which the assembly must leave alone."""
    try:
        entries = json.load(open("src/decompiled.json"))
    except OSError:
        return []
    out = []
    for e in entries:
        start = int(e["addr"], 16) - ROM_BASE
        out.append((start, start + e["size"], e["name"]))
    out.sort()
    return out


def write_code(rom, layout, force_literal=(), decompiled=()):
    # Four words in the opaque table at 0800A03C resemble ADR/shift pairs.
    # Their apparent targets can fall inside C-owned ranges, so preserve the
    # words as data instead of asking the assembler to resolve false labels.
    force_literal = set(force_literal) | {
        0x0800A03C, 0x0800A040, 0x0800A044, 0x0800A048,
    }
    shutil.rmtree("asm/code", ignore_errors=True)
    sample_manifest = Path("sound/samples/manifest.json")
    samples = json.loads(sample_manifest.read_text()) if sample_manifest.exists() else []
    audio_ranges = [(e['rom_offset'], e['rom_offset']+e['size']) for e in samples]
    # PC-relative instructions in opaque data can point into a range now
    # supplied by C.  Tell the emitter about every source-owned hole so it
    # preserves such halfwords literally instead of creating an assembler
    # reference across independently linked sections.
    source_ranges = [(start, end) for start, end, _name in decompiled
                     if start < CODE_END]
    em = emitter.Emitter(rom, layout, CODE_END, force_literal,
                         audio_ranges + source_ranges)
    replacements = list(decompiled) + [(e['rom_offset'], e['rom_offset']+e['size'], e) for e in samples]
    replacements.sort(key=lambda e:e[0])
    assert all(a[1] <= b[0] for a,b in zip(replacements,replacements[1:])), 'Source ranges overlap'
    # Resolve all labels up front so cross-chunk branches find their target.
    em.prescan(HEADER_END + ROM_BASE, CODE_END + ROM_BASE)
    os.makedirs("asm/code", exist_ok=True)
    files = []
    sound_spans = []
    linemaps = {}
    for start in range(HEADER_END, CODE_END, CODE_CHUNK):
        end = min(start + CODE_CHUNK, CODE_END)
        path = "asm/code/code_%06X.s" % start
        # Skip anything a C file now owns, emitting one section per surviving
        # span so the linker can slot the compiled code into the gap.
        spans = []
        pos = start
        for dstart, dend, name in replacements:
            if dend <= start or dstart >= end:
                continue
            dstart, dend = max(dstart, start), min(dend, end)
            if dstart > pos:
                spans.append((pos, dstart, None))
            spans.append((dstart, dend, name))
            pos = dend
        if pos < end:
            spans.append((pos, end, None))
        if not spans:
            spans = [(start, end, None)]

        has_assembly = False
        with open(path, "w") as f:
            f.write("@ ROM %06X..%06X\n" % (start, end))
            f.write("@ Generated by tools/split.py from the recovered layout.\n")
            f.write("@ Instructions come from recursive descent; bytes the\n")
            f.write("@ analysis could not prove are code are kept as literals\n")
            f.write("@ so this file reassembles to the original image.\n\n")
            lines_so_far = 8
            merged = {}
            for sstart, send, name in spans:
                if isinstance(name, dict):
                    offset = sstart - name['rom_offset']
                    sound_spans.append((sstart, send, name['rom_offset'], offset))
                    f.write("\n@ %06X..%06X is PCM sound data; see "
                            "sound/sample_sections.json\n" % (sstart, send))
                    lines_so_far += 2
                    continue
                if name is not None:
                    f.write("\n@ %06X..%06X is decompiled as %s(); see "
                            "src/decompiled.json\n" % (sstart, send, name))
                    lines_so_far += 2
                    continue
                f.write("\n\t.section %s, \"ax\"\n" % sect(sstart))
                f.write("\t.syntax unified\n")
                has_assembly = True
                lines_so_far += 3
                lm = em.emit_range(sstart + ROM_BASE, send + ROM_BASE, f,
                                   first_line=lines_so_far + 1)
                merged.update(lm)
                lines_so_far += sum(1 for _ in open(path)) - lines_so_far \
                    if False else 0
                f.flush()
                lines_so_far = sum(1 for _ in open(path))
            linemaps[path] = {str(k): "%08X" % v for k, v in merged.items()}
        if has_assembly:
            files.append(path)
        else:
            os.remove(path)
    sound_sections = []
    for start, end, sample_start, offset in sound_spans:
        entry = {
            "group": "sound_samples",
            "start": "%08X" % start,
            "end": "%08X" % end,
            "kind": "pcm",
            "source": "build/sound/samples/%06X.bin" % sample_start,
            "source_offset": offset,
        }
        if offset == 0:
            entry["symbol"] = "gWave_%08X" % (ROM_BASE + sample_start)
        sound_sections.append(entry)
    sound_path = "sound/sample_sections.json"
    with open(sound_path, "w") as f:
        json.dump({
            "format": 1,
            "description": ("Placement of editable PCM sample spans interleaved "
                            "with executable ROM."),
            "sections": sound_sections,
        }, f, indent=2)
        f.write("\n")
    return files, em, linemaps


def write_data(rom, gfx_manifest, decompiled=()):
    """Emit the non-code tail of the ROM, keeping graphics as separate files."""
    # Clear stale fragments: the region layout changes as more of the ROM is
    # identified, and a leftover .s from a previous split would be linked in
    # on top of the region that replaced it.
    shutil.rmtree("asm/data", ignore_errors=True)
    for f in glob.glob("data/data_*.bin"):
        os.remove(f)
    monitor_path = "data/agb_debug_monitor.bin"
    if os.path.exists(monitor_path):
        os.remove(monitor_path)
    os.makedirs("data", exist_ok=True)

    # Compressed graphics are linked from generated build/*.lz outputs. Uncompressed
    # art is rebuilt from its PNG, which holds the exact tile bytes, so the
    # picture really is the source. Driver PCM is handled by write_code.
    blocked = []
    for m in gfx_manifest:
        if m["rom_offset"] < CODE_END:
            continue
        blocked.append((m["rom_offset"],
                        m["rom_offset"] + m["compressed_size"],
                        "build/" + m["path"] + ".lz", "gfx"))
    try:
        raw_manifest = json.load(open("assets_raw.json"))
    except OSError:
        raw_manifest = []
    for m in raw_manifest:
        if m['kind'] in ('raw_dump', 'sound_dump'):
            continue  # Diagnostic interpretations are not authoritative graphics.
        start = m["rom_offset"]
        if start < CODE_END:
            continue
        end = start + m["raw_size"]
        if m["kind"] == "raw_art":
            blocked.append((start, end,
                            "build/%s.4bpp" % m["path"], "raw_art"))
        else:
            raise ValueError('Unverified raw asset kind cannot enter the ROM: ' + m['kind'])
    # The named font is authoritative. Older heuristic scans found a false
    # RLE stream inside its bitmap; that overlapping scan hit must not hide
    # the editable font source.
    import nfp
    font = next(e for e in nfp.entries(rom) if e['name'] == 'FONT.NFT')
    authoritative = [(font['rom_offset'], font['end'],
                      'build/graphics/fonts/font.nft', 'font')]
    for e in nfp.entries(rom):
        if e['name'].endswith(('.KCL', '.TCL')):
            authoritative.append((e['rom_offset'], e['end'],
                                  'build/graphics/palettes/' + e['name'] + '.bin', 'palette'))
        elif e['name'].endswith('.SPC'):
            authoritative.append((e['rom_offset'], e['end'],
                                  'build/scripts/nfp/' + e['name'] + '.bin', 'script'))
        elif e['name'].endswith('.NCD'):
            authoritative.append((e['rom_offset'], e['end'],
                                  'build/graphics/ncd/' + e['name'][:-4] + '.ncd', 'sprites'))
        elif e['name'].endswith('.TSC'):
            authoritative.append((e['rom_offset'], e['end'],
                                  'build/graphics/tilemaps/nfp/' + e['name'] + '.bin',
                                  'tilemap'))
        elif e['name'].endswith('.KMP'):
            authoritative.append((e['rom_offset'], e['end'],
                                  'build/maps/nfp/' + e['name'] + '.bin', 'map'))
    blocked = [b for b in blocked if not any(
        b[0] < a[1] and a[0] < b[1] for a in authoritative)]
    blocked.extend(authoritative)

    # C can also own initialized tables in the non-code tail. Treat those
    # ranges as holes in raw incbins, just as write_code does for functions.
    # A C range must never silently carve bytes out of an editable asset.
    # Clip ranges at the code/data boundary. A typed table can legitimately
    # straddle 001B0000 (the native-command registry does), and both generated
    # halves must leave the single C-provided section alone.
    data_c = [(max(start, CODE_END), min(end, ROM_END), None, "c")
              for start, end, _ in decompiled
              if end > CODE_END and start < ROM_END]
    for start, end, _, _ in data_c:
        if any(start < other_end and other_start < end
               for other_start, other_end, _, _ in blocked):
            raise ValueError("C-owned data overlaps an editable asset at %06X" % start)
    blocked.extend(data_c)
    blocked.sort()

    regions = []          # (start, end, kind, path)
    pos = CODE_END
    for start, end, path, kind in blocked:
        if start < pos:
            continue      # overlapping detection, keep the first
        if start > pos:
            regions += raw_regions(rom, pos, start)
        regions.append((start, end, kind, path))
        pos = end
    if pos < ROM_END:
        regions += raw_regions(rom, pos, ROM_END)

    fill_regions = []
    pattern_regions = []
    grouped = {}
    localized_entries = []
    for start, end, kind, path in regions:
        if kind == "c":
            continue
        # The SDK debug monitor is executable ARM/Thumb code followed by a
        # large zero-filled reservation. Keep the program distinctly named
        # and describe the reproducible tail without storing it in the binary.
        if (start == AGB_DEBUG_MONITOR_START
                and end == AGB_DEBUG_MONITOR_END and kind == "raw"):
            assert not any(rom[AGB_DEBUG_MONITOR_DATA_END:end])
            with open(monitor_path, "wb") as f:
                f.write(rom[start:AGB_DEBUG_MONITOR_DATA_END])
            grouped.setdefault("debug_monitor", []).append((
                start, AGB_DEBUG_MONITOR_DATA_END, "debug_monitor",
                monitor_path, "build/" + monitor_path,
            ))
            fill_regions.append((AGB_DEBUG_MONITOR_DATA_END, end, 0))
            continue
        # The final initialized island is a regular cartridge-mirror table,
        # not an opaque asset: 0x800 zero bytes followed by 448 identical
        # pointers. Keep that structure visible in the placement manifest.
        if (start == DEBUG_VECTOR_TABLE_START
                and end == DEBUG_VECTOR_TABLE_END and kind == "raw"):
            zero_size = DEBUG_VECTOR_TABLE_WORDS_START - start
            assert rom[start:DEBUG_VECTOR_TABLE_WORDS_START] == bytes(zero_size)
            encoded_address = DEBUG_VECTOR_ADDRESS.to_bytes(4, "little")
            word_count = (end - DEBUG_VECTOR_TABLE_WORDS_START) // 4
            assert rom[DEBUG_VECTOR_TABLE_WORDS_START:end] == encoded_address * word_count
            pattern_regions.extend([
                (start, DEBUG_VECTOR_TABLE_WORDS_START, "fill", 0),
                (DEBUG_VECTOR_TABLE_WORDS_START, end, "word_fill",
                 DEBUG_VECTOR_ADDRESS),
            ])
            continue
        if kind == "fill":
            fill_regions.append((start, end, rom[start]))
            continue
        # assets are staged into build/ by tools/build_assets.py so the
        # originals under graphics/ and scripts/ are never written to
        incpath = path
        if kind == "raw":
            with open(path, "wb") as f:
                f.write(rom[start:end])
            # the build reads the copy that has the text/ scripts applied
            incpath = "build/" + path
        entry = (start, end, kind, path, incpath)
        if start in ENGLISH_REPLACEABLE_OFFSETS:
            localized_entries.append(entry)
        else:
            grouped.setdefault(kind, []).append(entry)

    group_names = {
        "font": "graphics_assets",
        "gfx": "graphics_assets",
        "raw_art": "graphics_assets",
        "palette": "graphics_assets",
        "sprites": "graphics_assets",
        "tilemap": "graphics_assets",
        "map": "map_assets",
        "script": "script_assets",
        "debug_monitor": "raw_data",
        "raw": "raw_data",
    }
    output_groups = {}
    for kind, entries in grouped.items():
        output_groups.setdefault(group_names[kind], []).extend(entries)
    if localized_entries:
        output_groups["japanese_localized_assets"] = localized_entries

    # Short zero-filled gaps are ordinary C data. Large erased-ROM spans stay
    # as compact generated fill sections: spelling
    # hundreds of KiB of 0xFF initializers in C would be less readable and
    # would make the object files needlessly large.
    fill_regions.append((ROM_END, len(rom), 0xFF))
    zero_fills = [region for region in fill_regions
                  if region[2] == 0 and region[1] - region[0] < 0x100]
    generated_fills = [region for region in fill_regions
                       if region[2] == 0xFF
                       or (region[2] == 0 and region[1] - region[0] >= 0x100)]

    cpath = "src/rom_padding.c"
    if zero_fills:
        with open(cpath, "w") as f:
            f.write("#include \"gba/types.h\"\n")
            f.write("#include \"rom_section.h\"\n\n")
            f.write("/* Zero-filled alignment gaps between independently placed ROM data. */\n")
            for start, end, _ in zero_fills:
                f.write("AT(\"%08X\") const u8 gRomPadding%08X[%d] "
                        "__attribute__((aligned(1))) = {0};\n" %
                        (start, start, end - start))
    elif os.path.exists(cpath):
        os.remove(cpath)

    manifest_sections = []
    for group, entries in sorted(output_groups.items()):
        for start, end, kind, _, incpath in sorted(entries):
            manifest_sections.append({
                "group": group,
                "start": "%08X" % start,
                "end": "%08X" % end,
                "kind": kind,
                "source": incpath,
            })
    for start, end, value in generated_fills:
        manifest_sections.append({
            "group": "padding",
            "start": "%08X" % start,
            "end": "%08X" % end,
            "kind": "fill",
            "fill": value,
        })
    for start, end, kind, value in pattern_regions:
        entry = {
            "group": "raw_data",
            "start": "%08X" % start,
            "end": "%08X" % end,
            "kind": kind,
        }
        entry[kind] = "%08X" % value if kind == "word_fill" else value
        manifest_sections.append(entry)
    manifest_sections.sort(key=lambda entry: int(entry["start"], 16))
    manifest_path = "data/rom_data_sections.json"
    with open(manifest_path, "w") as f:
        json.dump({
            "format": 1,
            "description": ("Placement of generated assets, declarative data "
                            "patterns, and unresolved data in the original ROM."),
            "sections": manifest_sections,
        }, f, indent=2)
        f.write("\n")

    return [manifest_path], regions


def raw_regions(rom, start, end):
    out = []
    pos = start
    while pos < end:
        stop = min(pos + DATA_CHUNK, end)
        data = rom[pos:stop]
        # Alignment gaps and erased cartridge space carry no authored data.
        # Describe them directly in the generated assembly instead of keeping
        # opaque .bin files whose only content is a repeated padding byte.
        if data and data[0] in (0x00, 0xFF) and data.count(data[0]) == len(data):
            out.append((pos, stop, "fill", None))
        else:
            # Large end-of-ROM chunks can contain a small initialized island
            # surrounded by erased flash.  Peel off long 0xFF edges so the
            # source binary represents the island itself rather than hundreds
            # of kilobytes of reproducible erased bytes.  Zero runs are left
            # alone because an all-zero area can be an initialized table.
            first = 0
            while first < len(data) and data[first] == 0xFF:
                first += 1
            last = len(data)
            while last > first and data[last - 1] == 0xFF:
                last -= 1
            if first >= 0x100:
                out.append((pos, pos + first, "fill", None))
            else:
                first = 0
            if last < len(data) and len(data) - last < 0x100:
                last = len(data)
            if first < last:
                raw_start = pos + first
                raw_stop = pos + last
                out.append((raw_start, raw_stop, "raw",
                            "data/data_%06X.bin" % raw_start))
            if last < len(data):
                out.append((pos + last, stop, "fill", None))
        pos = stop
    return out


def main():
    rom = open(sys.argv[1], "rb").read()
    layout = json.load(open(".analysis/layout.json"))
    gfx = json.load(open("assets.json"))

    os.makedirs("data", exist_ok=True)
    os.makedirs(".analysis", exist_ok=True)
    write_header(rom)
    force = []
    if os.path.exists(".analysis/force_literal.json"):
        force = [int(a, 16) for a in json.load(open(".analysis/force_literal.json"))]
    decompiled = load_decompiled()
    if decompiled:
        print("C-provided ranges: %d" % len(decompiled))
    code_files, em, linemaps = write_code(rom, layout, force, decompiled)
    with open(".analysis/linemap.json", "w") as f:
        json.dump(linemaps, f)
    data_files, regions = write_data(rom, gfx, decompiled)

    print("code files : %d" % len(code_files))
    print("data files : %d" % len(data_files))
    print("labels     : %d" % len(em.labels))

    with open(".analysis/split.json", "w") as f:
        json.dump({"code": code_files, "data": data_files,
                   "code_end": CODE_END, "rom_end": ROM_END,
                   "rom_size": len(rom)}, f, indent=1)


if __name__ == "__main__":
    main()
