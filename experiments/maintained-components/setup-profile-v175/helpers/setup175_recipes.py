"""Authored recipe encoders/decoders; original and target bytes are owned inputs."""
import hashlib,struct
from native_rpx import Elf,COMPRESSED
from title_engine import unpack
SOURCE_RPX="f9f461738949a09481dc1a31c01ad27db813c4c6058fdd7d015624a4146bbf0b"
SOURCE_TITLE="06aaa05d996605d379b04e7ab1dfb9592d8d739c7c13e97bcd23049d33f29c5a"

def digest(data):
    return hashlib.sha256(data).hexdigest()

def word(data, offset):
    if offset < 0 or offset + 4 > len(data):
        raise ValueError('Truncated installer recipe')
    return struct.unpack_from('>I', data, offset)[0]

def edits(before, after):
    """Merge tiny unchanged gaps so each encoded edit pays for its header."""
    baseline = before[:len(after)].ljust(len(after), b'\0')
    spans, pos = [], 0
    while pos < len(after):
        if baseline[pos] == after[pos]:
            pos += 1
            continue
        start = pos
        pos += 1
        while pos < len(after):
            if baseline[pos] != after[pos]:
                pos += 1
                continue
            end = pos
            while pos < min(end + 16, len(after)) and baseline[pos] == after[pos]:
                pos += 1
            if pos == len(after) or pos == end + 16:
                pos = end
                break
        spans.append((start, after[start:pos]))
    return spans

def make_rpx_recipe(original, target):
    source, result = Elf(original), Elf(target)
    result.verify_crcs()
    if any(s.header[2] & COMPRESSED for s in result.sections):
        raise ValueError('The published installer emitter requires uncompressed target sections')
    output = bytearray(b'GZRPX001' + bytes.fromhex(digest(original)) +
                       struct.pack('>I', len(result.sections)) + bytes(result.header))
    if len(output) != 96:
        raise ValueError('Unexpected GZRPX001 header')
    edit_count = 0
    for index, section in enumerate(result.sections):
        src = index if index < len(source.sections) else 0xffffffff
        before = source.sections[index].data if src != 0xffffffff else b''
        spans = edits(before, section.data)
        edit_count += len(spans)
        output += struct.pack('>12I', src, len(section.data), *section.header)
        output += struct.pack('>I', len(spans))
        for offset, data in spans:
            output += struct.pack('>2I', offset, len(data)) + data
    return bytes(output), dict(sections=len(result.sections), edits=edit_count)

def apply_rpx_recipe(original, recipe):
    """Independent decoder/physical writer matching published rpx_recipe.cpp."""
    if len(recipe) < 96 or recipe[:8] != b'GZRPX001' or recipe[8:40] != bytes.fromhex(digest(original)):
        raise ValueError('Installer input/hash header mismatch')
    source, count, cursor = Elf(original), word(recipe, 40), 96
    if not 0 < count <= 256:
        raise ValueError('Installer section count out of bounds')
    headers, bodies = [], []
    for _ in range(count):
        src, size = word(recipe, cursor), word(recipe, cursor + 4)
        cursor += 8
        if cursor + 44 > len(recipe) or size > 64 * 1024 * 1024:
            raise ValueError('Installer section truncated or too large')
        header = list(struct.unpack_from('>10I', recipe, cursor))
        cursor += 40
        count_edits = word(recipe, cursor)
        cursor += 4
        if src != 0xffffffff and src >= len(source.sections):
            raise ValueError('Installer source section invalid')
        if count_edits > 2000000:
            raise ValueError('Installer edit count exceeds bounds')
        data = bytearray((source.sections[src].data if src != 0xffffffff else b'')[:size].ljust(size, b'\0'))
        previous = 0
        for _ in range(count_edits):
            offset, length = word(recipe, cursor), word(recipe, cursor + 4)
            cursor += 8
            if offset < previous or offset + length > size or cursor + length > len(recipe):
                raise ValueError('Installer edit range invalid')
            data[offset:offset + length] = recipe[cursor:cursor + length]
            cursor += length
            previous = offset + length
        headers.append(header)
        bodies.append(data)
    if cursor != len(recipe):
        raise ValueError('Installer recipe has trailing bytes')
    output = bytearray((64 + count * 40 + 63) & ~63)
    output[:52] = recipe[44:96]
    struct.pack_into('>I', output, 32, 64)

    def rank(index):
        kind, flags = headers[index][1:3]
        if kind == 0x80000003:
            return 0
        if kind == 0x80000004:
            return 1
        if kind in (0x80000001, 0x80000002):
            return 3
        if flags & 4:
            return 4
        return (2 if flags & 1 else 3) if flags & 2 else 5

    for index in sorted(range(count), key=rank):
        h = headers[index]
        if h[1] in (0, 8):
            h[4] = 0
            continue
        output.extend(bytes((-len(output)) % 64))
        h[4], h[5] = len(output), len(bodies[index])
        output += bodies[index]
    for index, header in enumerate(headers):
        struct.pack_into('>10I', output, 64 + 40 * index, *header)
    output.extend(bytes((-len(output)) % 64))
    Elf(output).verify_crcs()
    return bytes(output)

def literal_yaz0(raw):
    return b'Yaz0' + struct.pack('>I', len(raw)) + bytes(8) + b''.join(
        b'\xff' + raw[i:i + 8] for i in range(0, len(raw), 8))

def make_title_recipe(original, target):
    raw, before = unpack(target), unpack(original)
    if literal_yaz0(raw) != target:
        raise ValueError('Title target must use the exact installer literal-Yaz0 emitter')
    # Same-offset copy/literal runs keep the encoding simple and bounded. A
    # copy is used only after an actual byte equality check against the source.
    operations, cursor = [], 0

    def add_copy_or_data(start, data):
        kind = 0 if before[start:start + len(data)] == data else (2 if data == bytes(len(data)) else 1)
        operations.append((kind, start, data))

    for start, data in edits(before, raw):
        if start > cursor:
            add_copy_or_data(cursor, raw[cursor:start])
        operations.append((2 if data == bytes(len(data)) else 1, start, data))
        cursor = start + len(data)
    if cursor < len(raw):
        add_copy_or_data(cursor, raw[cursor:])
    if not operations or len(operations) > 10000:
        raise ValueError('Title recipe operation count out of bounds')
    output = bytearray(b'GZTITLE1' + bytes.fromhex(digest(original)) + bytes.fromhex(digest(target)) +
                       struct.pack('>2I', len(raw), len(operations)))
    for kind, offset, data in operations:
        if kind == 0 and before[offset:offset + len(data)] != data:
            raise ValueError('Title copy does not match original bytes')
        output += bytes([kind]) + struct.pack('>2I', offset, len(data))
        if kind == 1:
            output += data
    return bytes(output), dict(operations=len(operations), raw_bytes=len(raw))

def apply_title_recipe(original, recipe):
    if len(recipe) < 80 or recipe[:8] != b'GZTITLE1' or recipe[8:40] != bytes.fromhex(digest(original)):
        raise ValueError('Title recipe input/hash header mismatch')
    source, size, count, cursor, out = unpack(original), word(recipe, 72), word(recipe, 76), 80, bytearray()
    if not 0 < size <= 32 * 1024 * 1024 or not 0 < count <= 10000:
        raise ValueError('Title recipe bounds invalid')
    for _ in range(count):
        if cursor + 9 > len(recipe):
            raise ValueError('Truncated title operation')
        kind, offset, length = recipe[cursor], word(recipe, cursor + 1), word(recipe, cursor + 5)
        cursor += 9
        if length > size - len(out):
            raise ValueError('Title output overflow')
        if kind == 0:
            if offset + length > len(source):
                raise ValueError('Title copy overflow')
            out += source[offset:offset + length]
        elif kind == 1:
            if cursor + length > len(recipe):
                raise ValueError('Title literal overflow')
            out += recipe[cursor:cursor + length]
            cursor += length
        elif kind == 2:
            out += bytes(length)
        else:
            raise ValueError('Unknown title recipe operation')
    if cursor != len(recipe) or len(out) != size:
        raise ValueError('Title recipe size/trailing bytes invalid')
    result = literal_yaz0(out)
    if recipe[40:72] != bytes.fromhex(digest(result)):
        raise ValueError('Title recipe target hash mismatch')
    return result
