#include "persistent_projection.h"
#include "fixed_table.inc"

_Static_assert(sizeof(PpU32) == 4, "PPC word size");
_Static_assert(sizeof(void *) == 4, "PPC pointer size");
_Static_assert(sizeof(PpFixedTable) == 15212, "Fixed table extent");
_Static_assert(PP_ROW_BYTES == 24u + 12u * PP_ROW_TUPLES, "Wire extent");
_Static_assert(PP_OUTPUT_BYTES == PP_SAVE_BYTES + 3u + 4u, "Projection extent");

static PpU32 pp_span(PpU32 a, PpU32 n) {
    return n != 0u && a >= PP_NUMERIC_LOW && a < PP_NUMERIC_HIGH &&
           n <= PP_NUMERIC_HIGH - a;
}

static PpU32 pp_apart(PpU32 a, PpU32 n, PpU32 b, PpU32 m) {
    return a >= b ? a - b >= m : b - a >= n;
}

static PpU32 pp_word(PpU32 a) {
    return *(const volatile PpU32 *)a;
}

static PpU32 pp_tuple(PpU32 source, PpU32 index) {
    return source + 24u + 12u * index;
}

static PpU32 pp_finite(PpU32 bits) {
    return (bits & 0x7F800000u) != 0x7F800000u;
}

static PpU32 pp_source(PpU32 source, PpU32 bytes) {
    PpU32 table = (PpU32)&_gz_pp_fixed;
    if (bytes != PP_ROW_BYTES) return PP_SIZE;
    if ((source & 3u) != 0u || !pp_span(source, PP_ROW_BYTES)) return PP_SOURCE;
    if (!pp_span(table, sizeof(_gz_pp_fixed)) ||
        !pp_apart(source, PP_ROW_BYTES, table, sizeof(_gz_pp_fixed))) return PP_ALIAS;
    return PP_OK;
}

/* Called only after integer extent/alias preflight. All source reads precede
 * any caller output store. Fixed table has no caller-selectable replacement.
 */
static PpU32 pp_shape(PpU32 source) {
    PpU32 i, t, stage, base;
    if (pp_word(source) != PP_ROW_TUPLES || pp_word(source + 16u) > 65535u ||
        pp_word(source + 20u) != 1u) return PP_HEADER;
    for (i = 1u; i <= 3u; ++i)
        if (!pp_finite(pp_word(source + 4u * i))) return PP_HEADER;
    for (i = 0u; i < PP_SAVE_BYTES; ++i) {
        t = pp_tuple(source, i);
        if (pp_word(t) != _gz_pp_fixed.offsets[i] || pp_word(t + 4u) > 255u ||
            pp_word(t + 8u) != 1u) return PP_SAVE;
    }
    for (i = 0u; i < 3u; ++i) {
        t = pp_tuple(source, PP_SAVE_BYTES + i);
        if (pp_word(t) != 0x80000000u + i || pp_word(t + 4u) > 255u ||
            pp_word(t + 8u) != 1u) return PP_EQUIP;
    }
    t = pp_tuple(source, PP_SAVE_BYTES + 3u);
    if (pp_word(t) != 0x40000000u || pp_word(t + 8u) != 4u ||
        !pp_finite(pp_word(t + 4u))) return PP_CLOCK;
    stage = pp_word(pp_tuple(source, PP_CAPTURED_STAGE_INDEX) + 4u);
    if (stage >= PP_BANKS) return PP_STAGE;
    base = 0x380u + 36u * stage;
    for (i = 0u; i < PP_STAGE_BYTES; ++i) {
        t = pp_tuple(source, PP_SAVE_BYTES + 4u + i);
        if (pp_word(t) != base + i || pp_word(t + 4u) > 255u ||
            pp_word(t + 8u) != 1u ||
            pp_word(t + 4u) != pp_word(pp_tuple(source, PP_CAPTURED_MIRROR_INDEX + i) + 4u))
            return PP_COMMIT;
    }
    return PP_OK;
}

PpU32 _gz_pp_validate(PpU32 source, PpU32 bytes) {
    PpU32 z = pp_source(source, bytes);
    return z != PP_OK ? z : pp_shape(source);
}

PpU32 _gz_pp_project(PpU32 source, PpU32 bytes,
                     PpU32 output, PpU32 output_bytes) {
    PpU32 z, i, value, stage, first, table = (PpU32)&_gz_pp_fixed;
    volatile PpU8 *out;
    z = pp_source(source, bytes);
    if (z != PP_OK) return z;
    if (output_bytes != PP_OUTPUT_BYTES || !pp_span(output, PP_OUTPUT_BYTES)) return PP_OUTPUT;
    if (!pp_apart(source, PP_ROW_BYTES, output, PP_OUTPUT_BYTES) ||
        !pp_apart(output, PP_OUTPUT_BYTES, table, sizeof(_gz_pp_fixed))) return PP_ALIAS;
    z = pp_shape(source);
    if (z != PP_OK) return z;
    /* Successful pure shape validation is not native write permission. Only
     * caller-owned synthetic/projection output is touched by this component.
     */
    out = (volatile PpU8 *)output;
    for (i = 0u; i < PP_SAVE_BYTES + 3u; ++i)
        out[i] = (PpU8)pp_word(pp_tuple(source, i) + 4u);
    value = pp_word(pp_tuple(source, PP_SAVE_BYTES + 3u) + 4u);
    out[PP_SAVE_BYTES + 3u] = (PpU8)(value >> 24);
    out[PP_SAVE_BYTES + 4u] = (PpU8)(value >> 16);
    out[PP_SAVE_BYTES + 5u] = (PpU8)(value >> 8);
    out[PP_SAVE_BYTES + 6u] = (PpU8)value;
    stage = pp_word(pp_tuple(source, PP_CAPTURED_STAGE_INDEX) + 4u);
    first = _gz_pp_fixed.bank_output_index[stage];
    for (i = 0u; i < PP_STAGE_BYTES; ++i)
        out[first + i] = (PpU8)pp_word(pp_tuple(source, PP_SAVE_BYTES + 4u + i) + 4u);
    return PP_OK;
}

PpU32 _gz_pp_native_admit(void) {
    return 28u;
}
