#!/usr/bin/env python3
"""Recover driver-referenced M4A samples and rebuild them from PCM WAV + JSON.

The song table at 0808B1B0 and nine-player table at 0808B144 are loaded by
08078A70. Voice records lead to WaveData headers, not statistical PCM guesses.
Song/bank JSON describes references, not decoded musical sequencing. The
sample WAV rate is the header's fixed-point frequency divided by 1024.
"""
import argparse
import collections
import hashlib
import html
import json
from pathlib import Path
import struct
import wave

ROOT = Path(__file__).resolve().parents[1]
BASE = 0x08000000
TABLE = 0x8B1B0
TABLE_END = 0x8B898  # Next object is the zero-track dummy song header.
OUT = ROOT / 'sound'


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2) + '\n')


def discover(rom):
    def word(off): return struct.unpack_from('<I', rom, off)[0]
    songs = []
    banks = set()
    for off in range(TABLE, TABLE_END, 8):
        address, player, other = struct.unpack_from('<IHH', rom, off)
        pos = address - BASE
        assert 0 <= player < 9 and 0 <= other < 9
        count, blocks, priority, reverb, voices = struct.unpack_from('<BBBBI', rom, pos)
        assert count <= 16 and blocks == 0
        tracks = [word(pos + 8 + i * 4) for i in range(count)]
        assert all(BASE <= p < BASE + len(rom) for p in tracks)
        if count:
            assert BASE <= voices < BASE + len(rom)
            banks.add(voices - BASE)
        songs.append(dict(id=(off-TABLE)//8, header_address=address, player=player,
                          other_player=other, track_count=count, priority=priority,
                          reverb=reverb, voices_address=voices, tracks=tracks))
    pending = list(banks)
    visited = set()
    samples = {}
    tones = []
    while pending:
        bank = pending.pop()
        if bank in visited:
            continue
        visited.add(bank)
        # MIDI programs/keys index up to 128 records. Shared/nested views can
        # overlap; record individual addresses, not invented disjoint banks.
        for index in range(128):
            off = bank + index * 12
            kind, key, length, pan, pointer, attack, decay, sustain, release = struct.unpack_from('<BBBBIBBBB', rom, off)
            if kind not in (0, 1, 2, 3, 4, 8, 64, 128):
                continue
            tones.append(dict(bank_address=bank+BASE, index=index, address=off+BASE,
                              type=kind, key=key, length=length, pan_sweep=pan,
                              pointer=pointer, adsr=[attack,decay,sustain,release]))
            if kind in (64, 128):
                assert BASE <= pointer < BASE + len(rom) - 128 * 12
                pending.append(pointer - BASE)
            elif kind in (0, 8):
                pos = pointer - BASE
                if not 0 <= pos < len(rom)-16:
                    continue  # Empty/invalid slots are not sample claims.
                wave_type, status, frequency, loop_start, count = struct.unpack_from('<HHIII', rom, pos)
                assert wave_type == 0 and status in (0, 0x4000)
                assert 0 < count < 0x100000 and loop_start <= count and pos+16+count <= len(rom)
                e = samples.setdefault(pos, dict(rom_offset=pos, size=count+16+(4-count%4), padding=4-count%4,
                    type=wave_type, status=status, frequency=frequency,
                    loop_start=loop_start, sample_count=count,
                    wav=f'sound/samples/{pos:06X}.wav', references=[]))
                if off+BASE not in e['references']:
                    e['references'].append(off+BASE)
    ordered = sorted(samples.values(), key=lambda e:e['rom_offset'])
    assert all(a['rom_offset']+a['size']<=b['rom_offset'] for a,b in zip(ordered,ordered[1:]))
    return songs, tones, ordered


def compile_sample(entry, root=ROOT):
    with wave.open(str(root / entry['wav']), 'rb') as source:
        if (source.getnchannels(), source.getsampwidth(), source.getcomptype()) != (1,1,'NONE'):
            raise ValueError(entry['wav'] + ': expected unsigned 8-bit mono PCM WAV')
        if source.getframerate() != entry['frequency'] // 1024:
            raise ValueError(entry['wav'] + ': WAV rate differs from JSON frequency / 1024')
        if source.getnframes() != entry['sample_count']:
            raise ValueError(entry['wav'] + ': sample count changed; ROM span is fixed')
        unsigned = source.readframes(source.getnframes())
    if not 0 <= entry['loop_start'] <= len(unsigned):
        raise ValueError('Loop start outside sample')
    if entry['type'] != 0 or entry['status'] not in (0,0x4000):
        raise ValueError('Unsupported sample type/status')
    signed = bytes(b ^ 128 for b in unsigned)
    result = struct.pack('<HHIII',entry['type'],entry['status'],entry['frequency'],entry['loop_start'],len(signed))+signed
    padding = entry['padding']
    if padding != 4-len(signed)%4:
        raise ValueError('Expected sample guard and 4-byte alignment')
    if entry['status'] == 0x4000 and entry['loop_start'] >= len(signed):
        raise ValueError('Loop start must precede sample end')
    guard = signed[entry['loop_start']] if entry['status'] == 0x4000 else 0
    result += bytes([guard]) + bytes(padding-1)
    if len(result) != entry['size']:
        raise ValueError('Sample does not fit fixed ROM span')
    return result


def extract():
    rom = (ROOT/'baserom.gba').read_bytes()
    songs, tones, samples = discover(rom)
    (OUT/'samples').mkdir(parents=True,exist_ok=True)
    for e in samples:
        path = ROOT/e['wav']
        if path.exists():
            raise FileExistsError(f'{path}: extraction will not overwrite editable sound sources')
        raw = rom[e['rom_offset']+16:e['rom_offset']+16+e['sample_count']]
        with wave.open(str(path),'wb') as target:
            target.setnchannels(1);target.setsampwidth(1)
            target.setframerate(e['frequency']//1024)
            target.writeframes(bytes(b^128 for b in raw))
        assert compile_sample(e)==rom[e['rom_offset']:e['rom_offset']+e['size']]
    write_json(OUT/'samples/manifest.json',samples)
    write_json(OUT/'songs.json',songs)
    write_json(OUT/'voice_references.json',tones)
    pages=['<meta charset="utf-8"><title>MAR sound samples</title><style>body{font:16px system-ui;max-width:1000px;margin:30px auto}li{margin:20px 0}audio{display:block}</style><h1>Driver-referenced sound samples</h1><p>Editable mono PCM WAV files. These are instrument/effect samples, not rendered songs. Song sequencing and PSG voices are not rendered.</p><a href="samples/manifest.json">Sample metadata</a> · <a href="songs.json">Song table</a> · <a href="voice_references.json">Voice references</a><ul>']
    for e in samples:
        name=f'{e["rom_offset"]:06X}.wav'; rate=e['frequency']//1024
        pages.append(f'<li><a href="samples/{name}">{name}</a> — {e["sample_count"]} samples, {rate} Hz, loop start {e["loop_start"]}, status 0x{e["status"]:04X}<audio controls preload="none" src="samples/{name}"></audio></li>')
    (OUT/'index.html').write_text('\n'.join(pages)+ '</ul>')
    print(f'{len(songs)} song slots, {sum(e["track_count"]>0 for e in songs)} nonempty; {len(samples)} exact sample round-trips')


def build():
    for e in json.loads((OUT/'samples/manifest.json').read_text()):
        result=compile_sample(e)
        dest=ROOT/'build/sound/samples'/f'{e["rom_offset"]:06X}.bin'
        dest.parent.mkdir(parents=True,exist_ok=True)
        if not dest.exists() or dest.read_bytes()!=result:
            dest.write_bytes(result)
    print('Built driver-referenced samples from WAV and JSON')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command',choices=['extract','build'])
    args=parser.parse_args()
    (extract if args.command=='extract' else build)()
