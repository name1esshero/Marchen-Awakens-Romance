#!/usr/bin/env python3
"""Split the ROM into buildable sources.

Produces, in ROM order:
    asm/rom_header.s         cartridge header
    asm/code/code_*.s        disassembled code region
    asm/data/data_*.s        incbin wrappers for data and for graphics blobs
    data/*.bin               raw data the build re-inserts unchanged

Each generated fragment lives in a section named after its ROM offset, so the
linker script can order the whole image with a single sorted wildcard.
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
                    f.write('\n\t.section %s, "a"\n' % sect(sstart))
                    f.write('\t.incbin "build/sound/samples/%06X.bin", %d, %d\n' %
                            (name['rom_offset'], offset, send-sstart))
                    lines_so_far += 3
                    continue
                if name is not None:
                    f.write("\n@ %06X..%06X is decompiled as %s(); see "
                            "src/decompiled.json\n" % (sstart, send, name))
                    lines_so_far += 2
                    continue
                f.write("\n\t.section %s, \"ax\"\n" % sect(sstart))
                f.write("\t.syntax unified\n")
                lines_so_far += 3
                lm = em.emit_range(sstart + ROM_BASE, send + ROM_BASE, f,
                                   first_line=lines_so_far + 1)
                merged.update(lm)
                lines_so_far += sum(1 for _ in open(path)) - lines_so_far \
                    if False else 0
                f.flush()
                lines_so_far = sum(1 for _ in open(path))
            linemaps[path] = {str(k): "%08X" % v for k, v in merged.items()}
        files.append(path)
    return files, em, linemaps


def write_data(rom, gfx_manifest, decompiled=()):
    """Emit the non-code tail of the ROM, keeping graphics as separate files."""
    # Clear stale fragments: the region layout changes as more of the ROM is
    # identified, and a leftover .s from a previous split would be linked in
    # on top of the region that replaced it.
    shutil.rmtree("asm/data", ignore_errors=True)
    for f in glob.glob("data/data_*.bin"):
        os.remove(f)
    os.makedirs("asm/data", exist_ok=True)
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
    data_c = [(start, end, None, "c") for start, end, _ in decompiled
              if start >= CODE_END and start < ROM_END]
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
            regions += raw_regions(pos, start)
        regions.append((start, end, kind, path))
        pos = end
    if pos < ROM_END:
        regions += raw_regions(pos, ROM_END)

    files = []
    for start, end, kind, path in regions:
        if kind == "c":
            continue
        # assets are staged into build/ by tools/build_assets.py so the
        # originals under graphics/ and scripts/ are never written to
        incpath = path
        if kind == "raw":
            with open(path, "wb") as f:
                f.write(rom[start:end])
            # the build reads the copy that has the text/ scripts applied
            incpath = "build/" + path
        spath = "asm/data/data_%06X.s" % start
        with open(spath, "w") as f:
            f.write("@ ROM %06X..%06X (%s)\n" % (start, end, kind))
            if kind == "raw":
                f.write("@ Source: %s, with text/ applied by "
                        "tools/build_text.py\n" % path)
            f.write("\t.section %s, \"a\"\n" % sect(start))
            f.write("\t.incbin \"%s\"\n" % incpath)
        files.append(spath)

    # trailing 0xFF pad out to the 16 MB cartridge size
    with open("asm/data/data_%06X.s" % ROM_END, "w") as f:
        f.write("@ ROM %06X..%06X: unused cartridge space\n"
                % (ROM_END, len(rom)))
        f.write("\t.section %s, \"a\"\n" % sect(ROM_END))
        f.write("\t.fill %d, 1, 0xFF\n" % (len(rom) - ROM_END))
    files.append("asm/data/data_%06X.s" % ROM_END)
    return files, regions


def raw_regions(start, end):
    out = []
    pos = start
    while pos < end:
        stop = min(pos + DATA_CHUNK, end)
        out.append((pos, stop, "raw", "data/data_%06X.bin" % pos))
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
