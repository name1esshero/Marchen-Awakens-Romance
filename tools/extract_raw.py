#!/usr/bin/env python3
"""Report heuristic tile and PCM candidates without creating source assets.

These scans do not establish asset types. Named resources and driver-traced
sound samples are authoritative; candidate smoothness is only a measurement.
"""
import json
from pathlib import Path
import hashlib
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import classify_raw as C

def main():
    rom = Path(sys.argv[1]).read_bytes()
    runs = json.loads(Path(sys.argv[2]).read_text())

    os.makedirs("reports/graphics", exist_ok=True)
    os.makedirs("reports/sound", exist_ok=True)
    sound_candidates = []

    out = []
    n_art = n_snd = 0
    for start, end, kind in runs:
        length = end - start
        if kind == "art":
            n_tiles = length // 32
            width = 32 if n_tiles >= 256 else 16
            d, v = C.art_score(rom, start, min(length, 0x4000))
            out.append({
                "rom_offset": start, "raw_size": length, "kind": "raw_dump",
                "bpp": 4, "width_tiles": width, "compressed": False,
                "status": "unverified_tile_candidate",
                "sha256": hashlib.sha256(rom[start:end]).hexdigest(),
                "confidence": {"distinct": round(d, 2), "vert": round(v, 3),
                               "diversity": round(
                                   C.tile_diversity(rom, start,
                                                    min(length, 0x4000)), 3)},
            })
            n_art += 1
        elif kind == "sound":
            step, spread = C.pcm_score(rom, start, min(length, 0x1000))
            sound_candidates.append(dict(rom_offset=start, size=length,
                status="unverified_pcm_candidate", step=round(step,1), spread=spread))
            n_snd += 1

    with open("reports/sound/pcm-candidates.json", "w") as f:
        json.dump(sound_candidates, f, indent=2)

    with open("reports/graphics/raw-candidates.json", "w") as f:
        json.dump(out, f, indent=1)
    print("reported %d unverified tile candidates; no graphics sources created" % n_art)
    print("reported %d unverified PCM candidates; no sound sources created" % n_snd)


if __name__ == "__main__":
    main()
