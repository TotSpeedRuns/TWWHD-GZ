#ifndef GZ_BRIDGE_FORMAT3_H
#define GZ_BRIDGE_FORMAT3_H

/* BRG3 contains packed CPU values for the three known bridge shapes. A valid
 * component certifies its complete saved CPU cohort, never native ownership,
 * a graphics-resource lifetime, or permission to restore it. Callers own the
 * readable input span. Serialized keys/hashes are never followed as pointers.
 * Legacy/BRG2 compatibility belongs to the outer archive wrapper.
 */
typedef unsigned int Brg3U32;
typedef unsigned char Brg3U8;

enum {
    BRG3_MAGIC = 0x42524733u,
    BRG3_VERSION = 3,
    BRG3_BANK_BYTES = 98308,
    BRG3_HEADER_BYTES = 64,
    BRG3_ROW_BYTES = 64,
    BRG3_MAX_ROWS = 8,
    BRG3_WORLD_BYTES = 24656,
    BRG3_WORLD_CAPACITY = 512,
    BRG3_COMMON_POD_BYTES = 15540,
    BRG3_PLANK_POD_BYTES = 146,
    BRG3_SCALAR_PREFIX_BYTES = 51,
    BRG3_SCALAR_PLANK_BYTES = 93
};

/* Volatile byte loads preserve the unaligned packed-POD contract on PPC. */
static Brg3U32 brg3_byte(Brg3U32 p, Brg3U32 offset)
{
    return *(const volatile Brg3U8 *)(p + offset);
}

static Brg3U32 brg3_half(Brg3U32 p, Brg3U32 offset)
{
    return (brg3_byte(p, offset) << 8) | brg3_byte(p, offset + 1);
}

static Brg3U32 brg3_word(Brg3U32 p, Brg3U32 offset)
{
    return (brg3_byte(p, offset) << 24) |
        (brg3_byte(p, offset + 1) << 16) |
        (brg3_byte(p, offset + 2) << 8) | brg3_byte(p, offset + 3);
}

/* Numeric bounds only; this predicate is not a readable-allocation grant. */
static Brg3U32 brg3_span(Brg3U32 p, Brg3U32 bytes)
{
    return p >= 0x10000000u && p < 0xbffe0000u && bytes &&
        bytes <= 0xbffe0000u - p;
}

static Brg3U32 brg3_hash(Brg3U32 p, Brg3U32 bytes)
{
    Brg3U32 i, hash = 2166136261u;
    for(i = 0; i < bytes; i++)
        hash = (hash ^ brg3_byte(p, i)) * 16777619u;
    return hash;
}

static Brg3U32 brg3_zero(Brg3U32 p, Brg3U32 first, Brg3U32 end)
{
    Brg3U32 i;
    if(first > end)return 0;
    for(i = first; i < end; i++)if(brg3_byte(p, i))return 0;
    return 1;
}

static Brg3U32 brg3_known(Brg3U32 type, Brg3U32 count)
{
    return (type == 0 && (count == 33 || count == 26)) ||
        (type == 4 && count == 9);
}

static Brg3U32 brg3_sparse_count(Brg3U32 count)
{
    return count > 2 ? (count + 1) / 4 : 0;
}

static Brg3U32 brg3_geometry_bytes(Brg3U32 type, Brg3U32 count)
{
    if(!brg3_known(type, count))return 0;
    return BRG3_COMMON_POD_BYTES + BRG3_PLANK_POD_BYTES * count +
        (type == 0 ? 260 * brg3_sparse_count(count) + 340 : 0) + 1;
}

static inline Brg3U32 brg3_descriptor_count(Brg3U32 type, Brg3U32 count)
{
    if(!brg3_known(type, count))return 0;
    return 7 + 5 * count +
        (type == 0 ? 8 * brg3_sparse_count(count) + 3 : 0) + 1;
}

static Brg3U32 brg3_finite(Brg3U32 p, Brg3U32 bytes)
{
    Brg3U32 i;
    if(bytes & 3)return 0;
    for(i = 0; i < bytes; i += 4)
        if((brg3_word(p, i) & 0x7f800000u) == 0x7f800000u)return 0;
    return 1;
}

static Brg3U32 brg3_indices(Brg3U32 p, Brg3U32 bytes)
{
    Brg3U32 i, value;
    if(bytes & 1)return 0;
    for(i = 0; i < bytes; i += 2) {
        value = brg3_half(p, i);
        if(value != 0xffffu && value >= 404)return 0;
    }
    return 1;
}

/* Packed descriptor order is fixed, with no descriptor records or internal
 * padding on wire: seven BG arrays; each plank's five fields and its sparse
 * ropes; then type0's two global lines and active count; finally every row's
 * one-byte BGW +0x74 move counter (kind17). The only padding in BRG3 occurs
 * between each row's scalar/geometry payloads. This is row recipe2.
 */
static Brg3U32 brg3_geometry(Brg3U32 p, Brg3U32 bytes,
    Brg3U32 type, Brg3U32 count)
{
    static const Brg3U32 counts[7] = {204, 204, 404, 404, 66, 3, 121};
    static const Brg3U32 widths[7] = {12, 12, 16, 2, 6, 24, 24};
    Brg3U32 i, j, cursor = 0, length, active = 0;
    if(!brg3_known(type, count) || bytes != brg3_geometry_bytes(type, count) ||
        !brg3_span(p, bytes))return 0;
    if(type == 0) {
        active = brg3_word(p, bytes - 5);
        if(active > 12)return 0;
    }
    for(i = 0; i < 7; i++) {
        length = counts[i] * widths[i];
        if(i == 3 || i == 4) {
            if(!brg3_indices(p + cursor, length))return 0;
        } else if(!brg3_finite(p + cursor, length))return 0;
        cursor += length;
    }
    for(i = 0; i < count; i++) {
        if(!brg3_finite(p + cursor, 72) ||
            !brg3_finite(p + cursor + 72, 12) ||
            !brg3_finite(p + cursor + 86, 12) ||
            !brg3_finite(p + cursor + 98, 48))return 0;
        /* The two bytes at +84 are a signed native timer, not a float. */
        cursor += BRG3_PLANK_POD_BYTES;
        if(type == 0 && i >= 2 && ((i - 2) & 3) == 0) {
            for(j = 0; j < 4; j++) {
                if(!brg3_finite(p + cursor, 60))return 0;
                cursor += 65; /* 60 position bytes, then five byte widths. */
            }
        }
    }
    if(type == 0) {
        for(i = 0; i < 2; i++) {
            /* Unused positions are opaque; the native active range is +2. */
            if(!brg3_finite(p + cursor, (active + 2) * 12))return 0;
            cursor += 168;
        }
        if(brg3_word(p, cursor) != active)return 0;
        cursor += 4;
    }
    /* Native unsigned-byte move/recalc counter: every value is valid. */
    cursor += 1;
    return cursor == bytes;
}

/* Compact expansion of the exact 3120-byte typed_offsets.h prefix, pinned at
 * SHA256 52e3e739b790b5b8086b1d8a52076dfef3658a8e6cb070043c3558e45c93a952.
 * It was derived from its capture order, not from native struct packing.
 * This mapping supplies offsets only; it supplies no native read authority.
 */
static inline Brg3U32 brg3_scalar_offset(Brg3U32 index)
{
    Brg3U32 plank, local;
    if(index < 4)return 0xb0 + index;
    if(index == 4)return 0x326;
    if(index == 5)return 0x544;
    if(index == 6)return 0x540;
    if(index < 9)return 0x3b4 + index - 7;
    if(index < 17)return 0x548 + index - 9;
    if(index < 49)return 0x554 + index - 17;
    if(index < 51)return 0x57a + index - 49;
    if(index >= 51 + 93 * 33)return 0xffffffffu;
    plank = (index - 51) / 93;
    local = (index - 51) % 93;
    if(local < 6)local += 0x604;
    else if(local < 32)local += 0x60c - 6;
    else local += 0x630 - 32;
    return 0x5a8 + plank * 0x680 + local;
}

static Brg3U32 brg3_scalar(Brg3U32 p, Brg3U32 bytes,
    Brg3U32 type, Brg3U32 count)
{
    static const Brg3U8 prefix_float[5] = {9, 13, 25, 29, 33};
    static const Brg3U8 plank_float[17] = {
        6, 10, 14, 18, 22, 26, 32, 36, 40, 44, 48, 52, 64, 68, 72, 76, 80
    };
    Brg3U32 i, j, phase;
    if(!brg3_known(type, count) || bytes != 51 + 93 * count ||
        !brg3_span(p, bytes) || brg3_byte(p, 5) != count ||
        brg3_byte(p, 6) != type)return 0;
    phase = brg3_half(p, 7);
    if(phase != 3 && phase != 5)return 0;
    for(i = 0; i < 5; i++)
        if(!brg3_finite(p + prefix_float[i], 4))return 0;
    for(i = 0; i < count; i++)for(j = 0; j < 17; j++)
        if(!brg3_finite(p + 51 + 93 * i + plank_float[j], 4))return 0;
    return 1;
}

/* Exact complete component; a legacy/missing/partial row never passes here. */
static Brg3U32 brg3_component(Brg3U32 p, Brg3U32 bytes)
{
    Brg3U32 count, i, j, row, prior = 0, cursor, type, planks;
    Brg3U32 scalar, scalar_bytes, geometry, geometry_bytes, aligned, pid;
    if((p & 3) || (bytes & 3) || bytes < BRG3_HEADER_BYTES ||
        bytes > BRG3_BANK_BYTES || !brg3_span(p, bytes))return 0;
    count = brg3_word(p, 16);
    if(count > BRG3_MAX_ROWS || brg3_word(p, 0) != BRG3_MAGIC ||
        brg3_word(p, 4) != BRG3_VERSION || brg3_word(p, 8) != 64 ||
        brg3_word(p, 12) != bytes ||
        brg3_word(p, 32) != (1u << count) - 1 ||
        brg3_word(p, 36) != brg3_word(p, 32) ||
        brg3_word(p, 40) != 64 || brg3_word(p, 44) != 64 + 64 * count ||
        !brg3_zero(p, 52, 64))return 0;
    cursor = 64 + 64 * count;
    if(cursor > bytes || brg3_word(p, 48) != brg3_hash(p + 64, bytes - 64))return 0;
    for(i = 0; i < count; i++) {
        row = p + 64 + 64 * i;
        type = brg3_word(row, 8);
        planks = brg3_word(row, 12);
        pid = brg3_word(row, 4);
        if(!brg3_known(type, planks) || !pid || pid == 0xffffffffu ||
            brg3_word(row, 0) >= BRG3_WORLD_CAPACITY ||
            (i && brg3_word(row, 0) <= prior) ||
            brg3_word(row, 48) != (type == 0 ? 4u : 5u) ||
            brg3_word(row, 52) != 9 ||
            brg3_word(row, 56) != (type == 0 ? 126u : 0u) ||
            brg3_word(row, 60) != 2)return 0;
        for(j = 0; j < i; j++)
            if(brg3_word(p + 64 + 64 * j, 4) == pid)return 0;
        scalar = brg3_word(row, 16);
        scalar_bytes = brg3_word(row, 20);
        if(scalar != cursor || scalar_bytes != 51 + 93 * planks ||
            scalar_bytes > bytes - cursor ||
            !brg3_scalar(p + scalar, scalar_bytes, type, planks) ||
            brg3_word(row, 32) != brg3_hash(p + scalar, scalar_bytes))return 0;
        cursor += scalar_bytes;
        aligned = (cursor + 3) & ~3u;
        if(aligned > bytes || !brg3_zero(p, cursor, aligned))return 0;
        cursor = aligned;
        geometry = brg3_word(row, 24);
        geometry_bytes = brg3_word(row, 28);
        if(geometry != cursor || geometry_bytes != brg3_geometry_bytes(type, planks) ||
            geometry_bytes > bytes - cursor ||
            !brg3_geometry(p + geometry, geometry_bytes, type, planks) ||
            brg3_word(row, 36) != brg3_hash(p + geometry, geometry_bytes))return 0;
        cursor += geometry_bytes;
        aligned = (cursor + 3) & ~3u;
        if(aligned > bytes || !brg3_zero(p, cursor, aligned))return 0;
        cursor = aligned;
        prior = brg3_word(row, 0);
    }
    return cursor == bytes;
}

/* Pure directory/row join. The outer archive validator still validates the
 * complete VrWorld checksum and native identity semantics. Here its stored
 * checksum, frame and sequence must join this complete bridge component.
 */
static Brg3U32 brg3_archive(Brg3U32 p, Brg3U32 bytes)
{
    Brg3U32 components, end, i, offset, length, bank, world, world_count;
    Brg3U32 bridge_count, found = 0, index, row, actor;
    if((p & 3) || (bytes & 3) || bytes < 216 || bytes > 0x200000u ||
        !brg3_span(p, bytes) || brg3_word(p, 0) != 0x475a5331u ||
        brg3_word(p, 4) != bytes)return 0;
    components = brg3_word(p, 8);
    if(components < 25 || components > 30 || brg3_word(p, 12) != components)return 0;
    end = 16 + 8 * components;
    if(end > bytes)return 0;
    for(i = 0; i < components; i++) {
        offset = brg3_word(p, 16 + 8 * i);
        length = brg3_word(p, 20 + 8 * i);
        if(offset != end || (length & 3) || length > bytes - end)return 0;
        end += length;
    }
    if(end != bytes || brg3_word(p, 172) != BRG3_WORLD_BYTES)return 0;
    bank = p + brg3_word(p, 144);
    if(!brg3_component(bank, brg3_word(p, 148)))return 0;
    world = p + brg3_word(p, 168);
    if(brg3_word(world, 0) != 0x56525731u ||
        brg3_word(world, 4) != BRG3_WORLD_BYTES || brg3_word(world, 8) != 1 ||
        brg3_word(world, 12) != 1 || brg3_word(world, 16) > BRG3_WORLD_CAPACITY ||
        brg3_word(world, 76) ||
        brg3_word(bank, 20) != brg3_word(world, 20) ||
        brg3_word(bank, 24) != brg3_word(world, 68) ||
        brg3_word(bank, 28) != brg3_word(world, 72))return 0;
    world_count = brg3_word(world, 16);
    bridge_count = brg3_word(bank, 16);
    for(i = 0; i < world_count; i++)
        if(brg3_word(world + 80 + 48 * i, 8) == 89)found++;
    if(found != bridge_count)return 0;
    for(i = 0; i < bridge_count; i++) {
        row = bank + 64 + 64 * i;
        index = brg3_word(row, 0);
        if(index >= world_count)return 0;
        actor = world + 80 + 48 * index;
        if(brg3_word(actor, 8) != 89 || brg3_word(actor, 44) != 2 ||
            brg3_word(actor, 4) != brg3_word(row, 4) ||
            brg3_word(actor, 12) !=
                brg3_word(bank + brg3_word(row, 16), 0))return 0;
    }
    return 1;
}

#endif
