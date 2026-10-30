#ifndef GZ_BRIDGE_CAPTURE_FORMAT_H
#define GZ_BRIDGE_CAPTURE_FORMAT_H

/* Pure saved-byte parser. Numeric addresses below are span bounds only;
 * no serialized value is followed or becomes a native capability. */
typedef unsigned int BrgU32;
typedef unsigned short BrgU16;
typedef unsigned char BrgU8;

enum {
    BRG_FORMAT_INVALID = 0,
    BRG_FORMAT_LEGACY_MISSING = 1,
    BRG_FORMAT_CAPTURE_COMPLETE = 2,
    BRG_FORMAT_CAPTURE_PARTIAL = 3,
    BRG_BANK_BYTES = 65540,
    BRG_HEADER_BYTES = 64,
    BRG_ENTRY_BYTES = 32,
    BRG_GEOMETRY_BYTES = 30716,
    BRG_GEOMETRY_ENTRIES = 239,
    BRG_GEOMETRY_POD_BYTES = 22778,
    BRG_CAPTURE_ONLY = 1,
    BRG_NATIVE_CLOSURE_MISSING = 31
};

static BrgU32 brg_f_word(BrgU32 p, BrgU32 offset)
{
    return *(const BrgU32 *)(p + offset);
}

static BrgU32 brg_f_half(BrgU32 p, BrgU32 offset)
{
    return *(const BrgU16 *)(p + offset);
}

static BrgU32 brg_f_byte(BrgU32 p, BrgU32 offset)
{
    return *(const BrgU8 *)(p + offset);
}

static BrgU32 brg_f_span(BrgU32 p, BrgU32 bytes)
{
    return !(p & 3) && p >= 0x10000000u && p < 0xbffe0000u &&
        bytes && bytes <= 0xbffe0000u - p;
}

static BrgU32 brg_f_hash_skip(BrgU32 p, BrgU32 bytes, BrgU32 skip, BrgU32 skipped)
{
    BrgU32 i, hash = 2166136261u;
    for(i = 0; i < bytes; i++) {
        BrgU32 byte = i >= skip && i - skip < skipped ? 0 : brg_f_byte(p, i);
        hash = (hash ^ byte) * 16777619u;
    }
    return hash;
}

static BrgU32 brg_f_hash(BrgU32 p, BrgU32 bytes)
{
    return brg_f_hash_skip(p, bytes, 0, 0);
}

static BrgU32 brg_f_zero(BrgU32 p, BrgU32 first, BrgU32 end)
{
    BrgU32 i;
    for(i = first; i < end; i++)if(brg_f_byte(p, i))return 0;
    return 1;
}

static BrgU32 brg_f_descriptor(BrgU32 p, BrgU32 *index, BrgU32 *cursor,
    BrgU32 kind, BrgU32 segment, BrgU32 line, BrgU32 count, BrgU32 width, BrgU32 stride)
{
    BrgU32 d, aligned, bytes;
    if(*index >= BRG_GEOMETRY_ENTRIES)return 0;
    d = p + 128 + 32 * *index;
    aligned = (*cursor + 3) & ~3u;
    bytes = count * width;
    if(aligned > BRG_GEOMETRY_BYTES || bytes > BRG_GEOMETRY_BYTES - aligned ||
        !brg_f_zero(p, *cursor, aligned) ||
        brg_f_word(d, 0) != kind || brg_f_word(d, 4) != segment ||
        brg_f_word(d, 8) != line || brg_f_word(d, 12) != count ||
        brg_f_word(d, 16) != width || brg_f_word(d, 20) != stride ||
        brg_f_word(d, 24) != aligned || brg_f_word(d, 28) != bytes)return 0;
    *index += 1;
    *cursor = aligned + bytes;
    return 1;
}

static BrgU32 brg_f_geometry(BrgU32 p, BrgU32 bytes)
{
    static const BrgU32 counts[7] = {204, 204, 404, 404, 66, 3, 121};
    static const BrgU32 widths[7] = {12, 12, 16, 2, 6, 24, 24};
    static const BrgU32 strides[7] = {12, 12, 24, 8, 6, 32, 28};
    static const BrgU32 model_widths[5] = {72, 12, 2, 12, 48};
    BrgU32 i, j, index = 0, cursor = 128 + 32 * BRG_GEOMETRY_ENTRIES, active;
    if(bytes != BRG_GEOMETRY_BYTES || !brg_f_span(p, bytes) ||
        brg_f_word(p, 0) != 0x42475832u || brg_f_word(p, 4) != 2 ||
        brg_f_word(p, 8) != bytes || brg_f_word(p, 12) != BRG_GEOMETRY_ENTRIES ||
        brg_f_word(p, 16) != cursor || brg_f_word(p, 20) != BRG_GEOMETRY_POD_BYTES ||
        brg_f_word(p, 24) != 89 || !brg_f_word(p, 28) ||
        brg_f_word(p, 28) >= 0xfffffffeu || brg_f_word(p, 44) > 255 ||
        brg_f_word(p, 48) || brg_f_word(p, 52) != 33 ||
        (brg_f_word(p, 56) != 3 && brg_f_word(p, 56) != 5) || brg_f_word(p, 60) != 33 ||
        brg_f_word(p, 64) != 204 || brg_f_word(p, 68) != 404 ||
        brg_f_word(p, 72) != 66 || brg_f_word(p, 76) != 121 ||
        brg_f_word(p, 80) != 3 || brg_f_word(p, 84) != 1 ||
        brg_f_word(p, 96) != BRG_NATIVE_CLOSURE_MISSING ||
        !brg_f_zero(p, 104, 128) ||
        brg_f_word(p, 100) != brg_f_hash_skip(p, bytes, 100, 4))return 0;
    for(i = 0; i < 7; i++)if(!brg_f_descriptor(p, &index, &cursor,
        i + 1, 0xffffffffu, 0xffffffffu, counts[i], widths[i], strides[i]))return 0;
    for(i = 0; i < 33; i++) {
        for(j = 0; j < 5; j++)if(!brg_f_descriptor(p, &index, &cursor,
            j + 8, i, 0xffffffffu, 1, model_widths[j], model_widths[j]))return 0;
        if(i >= 2 && (i - 2) % 4 == 0)for(j = 0; j < 4; j++) {
            if(!brg_f_descriptor(p, &index, &cursor, 13, i, j, 1, 60, 60) ||
                !brg_f_descriptor(p, &index, &cursor, 14, i, j, 1, 5, 5))return 0;
        }
    }
    for(i = 0; i < 2; i++)if(!brg_f_descriptor(p, &index, &cursor,
        15, 0xffffffffu, i, 1, 168, 168))return 0;
    if(!brg_f_descriptor(p, &index, &cursor, 16, 0xffffffffu, 0xffffffffu, 1, 4, 4) ||
        index != BRG_GEOMETRY_ENTRIES || ((cursor + 3) & ~3u) != bytes ||
        !brg_f_zero(p, cursor, bytes))return 0;
    active = brg_f_word(p, brg_f_word(p, 128 + 32 * 238 + 24));
    if(active > 12)return 0;
    for(i = 0; i < BRG_GEOMETRY_ENTRIES; i++) {
        BrgU32 d = p + 128 + 32 * i;
        BrgU32 kind = brg_f_word(d, 0), offset = brg_f_word(d, 24);
        BrgU32 n = brg_f_word(d, 28);
        if(kind == 4 || kind == 5) {
            for(j = 0; j < n; j += 2) {
                BrgU32 value = brg_f_half(p, offset + j);
                if(value != 0xffffu && value >= 404)return 0;
            }
        } else if(kind != 10 && kind != 14 && kind != 16) {
            if(kind == 15)n = (active + 2) * 12;
            for(j = 0; j < n; j += 4)
                if((brg_f_word(p, offset + j) & 0x7f800000u) == 0x7f800000u)return 0;
        }
    }
    return 1;
}

__attribute__((noinline, noclone)) static BrgU32 brg_f_component(BrgU32 p, BrgU32 bytes)
{
    BrgU32 n, legacy, header, entries, i, captured = 0, next, previous = 0, payload_hash;
    if(bytes < 4 || bytes > BRG_BANK_BYTES || (bytes & 3) || !brg_f_span(p, bytes))return 0;
    n = brg_f_word(p, 0);
    if(n > 8)return 0;
    legacy = 4 + 8192 * n;
    if(bytes == legacy)return BRG_FORMAT_LEGACY_MISSING;
    if(legacy > bytes || BRG_HEADER_BYTES > bytes - legacy)return 0;
    header = p + legacy;
    entries = brg_f_word(header, 20);
    if(brg_f_word(header, 0) != 0x42475232u || brg_f_word(header, 4) != 2 ||
        brg_f_word(header, 8) != BRG_HEADER_BYTES || brg_f_word(header, 12) != bytes - legacy ||
        brg_f_word(header, 16) != n || entries > n ||
        brg_f_word(header, 24) != (1u << n) - 1 || !brg_f_word(header, 32) ||
        !brg_f_word(header, 36) || brg_f_word(header, 44) != brg_f_hash(p, legacy) ||
        brg_f_word(header, 56) != BRG_CAPTURE_ONLY || brg_f_word(header, 60))return 0;
    for(i = 0; i < n; i++) {
        BrgU32 row = p + 4 + 8192 * i, scalar = row + 4;
        BrgU32 count = brg_f_byte(scalar, 5), phase;
        if(brg_f_word(row, 0) || count < 1 || count > 50)return 0;
        phase = (brg_f_byte(scalar, 7) << 8) | brg_f_byte(scalar, 8);
        if((phase != 3 && phase != 5) || !brg_f_zero(row, 4 + 51 + count * 93, 8192))return 0;
    }
    next = legacy + BRG_HEADER_BYTES + BRG_ENTRY_BYTES * entries;
    if(next > bytes || brg_f_word(header, 48) != brg_f_hash(header + 64, entries * 32))return 0;
    payload_hash = brg_f_hash(p + next, bytes - next);
    if(brg_f_word(header, 52) != payload_hash)return 0;
    for(i = 0; i < entries; i++) {
        BrgU32 entry = header + 64 + 32 * i, row = brg_f_word(entry, 0);
        BrgU32 offset = brg_f_word(entry, 12), size = brg_f_word(entry, 16), scalar;
        if(row >= n || (i && row <= previous) || brg_f_word(entry, 4) >= 512 ||
            brg_f_word(entry, 8) != 0x00210001u || offset != next ||
            size > bytes - next || !brg_f_geometry(p + offset, size))return 0;
        scalar = p + 4 + 8192 * row + 4;
        if(brg_f_byte(scalar, 5) != 33 || brg_f_byte(scalar, 6) ||
            brg_f_word(entry, 20) != brg_f_word(p + offset, 28) ||
            brg_f_word(entry, 24) != brg_f_hash(scalar, 7) ||
            brg_f_word(entry, 28) != brg_f_hash(p + offset, size) ||
            brg_f_word(p + offset, 32) != brg_f_word(scalar, 0) ||
            brg_f_word(p + offset, 44) != brg_f_byte(scalar, 4) ||
            brg_f_word(p + offset, 56) != ((brg_f_byte(scalar, 7) << 8) | brg_f_byte(scalar, 8)))return 0;
        captured |= 1u << row;
        previous = row;
        next += size;
    }
    if(next != bytes || captured != brg_f_word(header, 28))return 0;
    return captured == ((1u << n) - 1) ? BRG_FORMAT_CAPTURE_COMPLETE : BRG_FORMAT_CAPTURE_PARTIAL;
}

static BrgU32 brg_f_archive(BrgU32 archive, BrgU32 bytes)
{
    BrgU32 offset, length, format, world, world_bytes, count, n, i, j, found = 0, legacy, header;
    if(bytes < 216 || !brg_f_span(archive, bytes))return 0;
    offset = brg_f_word(archive, 144);
    length = brg_f_word(archive, 148);
    if(offset > bytes || length > bytes - offset)return 0;
    format = brg_f_component(archive + offset, length);
    if(format <= BRG_FORMAT_LEGACY_MISSING)return format;
    world = brg_f_word(archive, 168);
    world_bytes = brg_f_word(archive, 172);
    if(world > bytes || world_bytes > bytes - world || world_bytes != 24656)return 0;
    world += archive;
    if(brg_f_word(world, 0) != 0x56525731u || brg_f_word(world, 4) != world_bytes ||
        brg_f_word(world, 12) != 1 || brg_f_word(world, 16) > 512)return 0;
    count = brg_f_word(world, 16);
    n = brg_f_word(archive + offset, 0);
    legacy = 4 + 8192 * n;
    header = archive + offset + legacy;
    if(brg_f_word(header, 32) != brg_f_word(world, 20) ||
        brg_f_word(header, 36) != brg_f_word(world, 68) ||
        brg_f_word(header, 40) != brg_f_word(world, 72))return 0;
    for(i = 0; i < count; i++)if(brg_f_word(world + 80 + 48 * i, 8) == 89)found++;
    if(found != n)return 0;
    for(i = 0; i < brg_f_word(header, 20); i++) {
        BrgU32 entry = header + 64 + 32 * i, index = brg_f_word(entry, 4);
        BrgU32 row, payload;
        if(index >= count)return 0;
        row = world + 80 + 48 * index;
        payload = archive + offset + brg_f_word(entry, 12);
        if(brg_f_word(row, 8) != 89 || brg_f_word(row, 44) != 2 ||
            brg_f_word(row, 4) != brg_f_word(entry, 20) ||
            brg_f_word(row, 12) != brg_f_word(payload, 32) ||
            brg_f_word(row, 20) != brg_f_word(payload, 36) ||
            brg_f_hash(row + 24, 20) != brg_f_word(payload, 40))return 0;
        for(j = 0; j < i; j++)if(brg_f_word(header + 64 + 32 * j, 4) == index)return 0;
    }
    return format;
}

#endif
