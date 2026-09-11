"""Shift-JIS script codec for the ROM's text blocks.

Strings are stored as Shift-JIS with embedded single-byte control codes and a
NUL terminator. Extraction writes UTF-8 so the scripts open in any editor;
control bytes survive as <XX> escapes, and every string is verified to
re-encode to its original bytes before it is exported as text.
"""

# Bytes below 0x20 are engine control codes, never text.
CTRL_MAX = 0x20


def is_lead(b):
    return 0x81 <= b <= 0x9F or 0xE0 <= b <= 0xEF


def is_trail(b):
    return (0x40 <= b <= 0x7E) or (0x80 <= b <= 0xFC)


def decode(data):
    """Bytes -> editable text. Returns None if the bytes are not text."""
    out = []
    i = 0
    n = len(data)
    while i < n:
        b = data[i]
        if b < CTRL_MAX:
            out.append("<%02X>" % b)
            i += 1
        elif is_lead(b) and i + 1 < n and is_trail(data[i + 1]):
            try:
                out.append(data[i:i + 2].decode("shift_jis"))
            except UnicodeDecodeError:
                return None
            i += 2
        elif 0x20 <= b < 0x80:
            ch = chr(b)
            # Only the escape delimiters need escaping. Comments are '//',
            # which never occurs in this ROM's text, so '#' and a single '/'
            # are ordinary characters.
            out.append("<%02X>" % b if ch in "<>" else ch)
            i += 1
        elif 0xA1 <= b <= 0xDF:
            out.append(data[i:i + 1].decode("shift_jis"))    # half-width kana
            i += 1
        else:
            return None
    return "".join(out)


def encode(text):
    """Editable text -> bytes. Inverse of decode()."""
    out = bytearray()
    i = 0
    n = len(text)
    while i < n:
        if text[i] == "<" and i + 3 < n and text[i + 3] == ">":
            out.append(int(text[i + 1:i + 3], 16))
            i += 4
        else:
            out += text[i].encode("shift_jis")
            i += 1
    return bytes(out)


def roundtrips(data):
    """True when this byte string survives decode/encode unchanged."""
    t = decode(data)
    if t is None:
        return False
    try:
        return encode(t) == data
    except UnicodeEncodeError:
        return False


def decode_lossless(data):
    """Decode already-framed text, preserving private codes as byte escapes.

    Unlike decode(), this is not a test of whether arbitrary data is text.
    The engine's reader at 080025BC accepts 80..9F and E0..FF as lead bytes.
    """
    result = []
    pos = 0
    while pos < len(data):
        lead = data[pos]
        size = 2 if (0x80 <= lead <= 0x9F or lead >= 0xE0) and pos + 1 < len(data) else 1
        part = data[pos:pos + size]
        text = decode(part)
        if text is None or encode(text) != part:
            text = ''.join('<%02X>' % b for b in part)
        result.append(text)
        pos += size
    return ''.join(result)
