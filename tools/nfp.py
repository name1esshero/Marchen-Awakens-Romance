"""Named ROM filesystem, recovered from 0807AAD0/0807AB08/0807AC28/0807AC3C.

The registration at 080017xx passes [081C0920, 08F28410) to 0807AB14.
Directory entries are 12-byte names followed by a relative payload offset.
Lengths are bounded by the next payload (and can include alignment padding).
"""
import struct

BASE = 0x1C0920
END = 0xF28410


def entries(rom):
    if rom[BASE:BASE + 6] != b'NFP2.0' or len(rom) < END:
        raise ValueError('Expected MAR NFP2.0 archive is missing')
    count, table, data_start = struct.unpack_from('<III', rom, BASE + 0x34)
    if table < 0x40 or table + count * 16 > data_start or BASE + data_start >= END:
        raise ValueError('Invalid NFP directory bounds')
    out = []
    for i in range(count):
        pos = BASE + table + i * 16
        name = rom[pos:pos + 12].split(b'\0', 1)[0].decode('ascii')
        off = BASE + struct.unpack_from('<I', rom, pos + 12)[0]
        if not BASE + data_start <= off < END:
            raise ValueError('Invalid NFP member offset: ' + name)
        out.append(dict(name=name, rom_offset=off, directory_offset=pos))
    offsets = sorted(set(e['rom_offset'] for e in out)) + [END]
    ends = dict(zip(offsets, offsets[1:]))
    for e in out:
        e['end'] = ends[e['rom_offset']]
        e['size_with_padding'] = e['end'] - e['rom_offset']
    return out


def owners(directory, start, end):
    return [e for e in directory if e['rom_offset'] < end and start < e['end']]
