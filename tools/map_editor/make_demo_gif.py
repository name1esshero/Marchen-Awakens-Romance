#!/usr/bin/env python3
"""Encode record_demo.cjs screenshots; requires Pillow only for documentation."""
import json
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
source = ROOT / 'build/map-editor-demo'
manifest = json.loads((source / 'frames.json').read_text())
frames = [Image.open(source / frame['file']).convert('RGB') for frame in manifest['frames']]
# One shared palette avoids color flicker between screenshots. Geometry and
# screenshot contents stay unchanged; only GIF's required color quantization applies.
samples = []
for frame in frames:
    sample = frame.copy()
    sample.thumbnail((340, 225))
    samples.append(sample)
sheet = Image.new('RGB', (340, 225 * len(samples)))
for i, sample in enumerate(samples):
    sheet.paste(sample, (0, 225 * i))
palette = sheet.quantize(colors=256, method=Image.Quantize.MEDIANCUT)
indexed = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in frames]
out = ROOT / 'docs/media/map-editor.gif'
out.parent.mkdir(parents=True, exist_ok=True)
indexed[0].save(out, save_all=True, append_images=indexed[1:],
                duration=[frame['duration'] for frame in manifest['frames']],
                loop=0, optimize=True, disposal=1)
with Image.open(out) as result:
    assert result.size == frames[0].size
    assert result.is_animated
    duration = 0
    for i in range(result.n_frames):
        result.seek(i)
        duration += result.info['duration']
print(f'{out}: {out.stat().st_size:,} bytes, {result.n_frames} frames, {duration / 1000:.2f}s')
