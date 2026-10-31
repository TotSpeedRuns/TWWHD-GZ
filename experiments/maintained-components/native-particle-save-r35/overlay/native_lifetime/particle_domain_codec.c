#include "particle_domain_cut.h"
#include "opaque_chunks.h"
#include <stdint.h>

static JpcU jpc_word(const unsigned char *p, JpcU offset) {
    p += offset;
    return (JpcU)p[0] << 24 | (JpcU)p[1] << 16 | (JpcU)p[2] << 8 | p[3];
}

static JpcU jpc_contains(JpcU a, JpcU n, JpcU b, JpcU m) {
    return n <= 0xffffffffu - a && m <= n && b >= a && b - a <= n - m;
}

static JpcU jpc_crc(const unsigned char *p, JpcU n) {
    JpcU i, hash = 2166136261u;
    for (i = 0; i < n; ++i) hash = (hash ^ p[i]) * 16777619u;
    return hash;
}

static JpcU jpc_has_region(const unsigned char *p, JpcU count, JpcU roles,
                         JpcU manager, JpcU pointer, JpcU bytes) {
    JpcU i;
    for (i = 0; i < count; ++i) {
        const unsigned char *r = p + JPC_TABLE + i * JPC_ROW;
        if ((jpc_word(r, 4) & roles) == roles && (jpc_word(r, 8) & manager) &&
            jpc_contains(jpc_word(r, 52), jpc_word(r, 12), pointer, bytes)) return 1;
    }
    return 0;
}

/* Inspect only PRIVATE packet bytes. Covered allocations live in the outer
 * heap packet, which this codec does not receive; native capture still checks
 * their cookies under its real current binding/hold. External cookies are
 * extracted without allocating or decoding a whole raw allocation. */
static JpcU jpc_aux_cookie(const unsigned char *p, JpcU count, JpcU manager,
                          JpcU base, JpcU elements) {
    JpcU i, start = base - 8, extent = elements * 0x9cu + 8;
    unsigned char cookie[8];
    for (i = 0; i < count; ++i) {
        const unsigned char *r = p + JPC_TABLE + i * JPC_ROW;
        if (!(jpc_word(r, 4) & JPC_AUX) || !(jpc_word(r, 8) & manager) ||
            jpc_word(r, 52) != start ||
            !jpc_contains(start, jpc_word(r, 12), start, extent)) continue;
        if (jpc_word(r, 28) & JPC_COVERED_BY_HEAP) return 1;
        if (oc_read(p + jpc_word(r, 16), jpc_word(r, 20), jpc_word(r, 12),
                    0, cookie, sizeof(cookie)) != OC_OK) return 0;
        return jpc_word(cookie, 0) == (elements ? elements * 0x9cu : 1u) &&
               jpc_word(cookie, 4) == ~elements;
    }
    return 0;
}

/* Count metadata requires particle-region presence even when that region is
 * covered by the outer heap. External cookies are visible in this packet;
 * covered cookies still need the outer context/native capture checks. An
 * actual zero count may omit its array or retain an optional empty cookie. */
static JpcU jpc_particle_region(const unsigned char *p, JpcU count,
                               JpcU manager, JpcU elements) {
    JpcU i, found = 0, extent = elements * 0x128u + 8;
    unsigned char cookie[8];
    for (i = 0; i < count; ++i) {
        const unsigned char *r = p + JPC_TABLE + i * JPC_ROW;
        if (!(jpc_word(r, 4) & JPC_PARTICLES) || !(jpc_word(r, 8) & manager)) continue;
        found = 1;
        if (jpc_word(r, 12) < extent) continue;
        if (jpc_word(r, 28) & JPC_COVERED_BY_HEAP) return 1;
        if (oc_read(p + jpc_word(r, 16), jpc_word(r, 20), jpc_word(r, 12),
                    0, cookie, sizeof(cookie)) != OC_OK) continue;
        if (jpc_word(cookie, 0) == (elements ? elements * 0x128u : 1u) &&
            jpc_word(cookie, 4) == ~elements) return 1;
    }
    return !elements && !found;
}

JpcU _gz_jpc_checksum(const void *image, JpcU bytes) {
    const unsigned char *p = image;
    JpcU i, hash = 2166136261u;
    if (!p || bytes < JPC_TABLE || bytes > JPC_MAX_BYTES) return 0;
    for (i = 0; i < bytes; ++i) {
        JpcU value = (i >= 12 && i < 16) || (i >= 32 && i < 36) ? 0 : p[i];
        hash = (hash ^ value) * 16777619u;
    }
    return hash;
}

static JpcU jpc_validate_impl(const void *image, JpcU bytes, const GhcSaveProducer *producer,
                    JpcU root, JpcU generation, JpcU raw_start, JpcU raw_bytes,
                    JpcU diagnostic) {
    const unsigned char *p = image, *issued = (const unsigned char *)producer;
    JpcU count, data, at, i, j, raw_total = 0, particles = 0;
    JpcU active = 0, configured = 0, control_found = 0;
    if (!p || !issued || ((uintptr_t)p & 3u) || bytes < JPC_TABLE || bytes > JPC_MAX_BYTES ||
        (bytes & 31u) || !root || !generation || !raw_bytes ||
        raw_start < 0x10000000u || raw_start >= 0xbffe0000u || raw_bytes > 0xbffe0000u - raw_start ||
        jpc_word(p, 0) != JPC_MAGIC || jpc_word(p, 4) != bytes ||
        jpc_word(p, 8) != JPC_VERSION || jpc_word(p, 12) != 1 ||
        jpc_word(p, 16) != 255 || jpc_word(p, 24) != JPC_TABLE ||
        jpc_word(p, 36) != jpc_word(issued, 12) || jpc_word(p, 40) != jpc_word(issued, 16) ||
        jpc_word(p, 44) != root || jpc_word(p, 48) != generation ||
        jpc_word(p, 64) != raw_start || jpc_word(p, 68) != raw_bytes) return diagnostic ? 1u : JPC_FORMAT;
    /* JPC v1 is deliberately rejected by the exact version check above. */
    if (jpc_word(p, 72) > JPC_MAX_RAW_BYTES) return diagnostic ? 2u : JPC_FORMAT;
    for (i = 108; i < JPC_HEADER; ++i) if (p[i]) return diagnostic ? 3u : JPC_FORMAT;
    for (i = 0; i < 64; ++i) if (p[JPC_PRODUCER + i] != issued[i]) return diagnostic ? 4u : JPC_FORMAT;
    count = jpc_word(p, 20); data = jpc_word(p, 28);
    if (!count || count > JPC_REGIONS || data != ((JPC_TABLE + count * JPC_ROW + 31u) & ~31u) ||
        data > bytes) return diagnostic ? 5u : JPC_FORMAT;
    if (jpc_word(p, JPC_LEDGER) != 0x4a454c31u ||
        jpc_word(p, JPC_LEDGER + 4) != JPC_LEDGER_BYTES ||
        jpc_word(p, JPC_LEDGER + 8) != 1 || jpc_word(p, JPC_LEDGER + 12) != 1 ||
        jpc_word(p, JPC_LEDGER + 16) || jpc_word(p, JPC_LEDGER + 20)) return diagnostic ? 6u : JPC_FORMAT;
    for (i = 0; i < 256; ++i) {
        const unsigned char *row = p + JPC_LEDGER + 128 + i * 32;
        JpcU alive = jpc_word(row, 28);
        if (alive > 1) return diagnostic ? 7u : JPC_FORMAT;
        if (alive) {
            if (!jpc_word(row, 0) || !jpc_word(row, 4) || !jpc_word(row, 8) ||
                !jpc_word(row, 12) || jpc_word(row, 16) > 65535 ||
                jpc_word(row, 20) >= 16 || jpc_word(row, 24) >= 8) return diagnostic ? 8u : JPC_FORMAT;
            ++active;
        }
    }
    if (active > 256) return diagnostic ? 9u : JPC_FORMAT;
    at = data;
    for (i = 0; i < count; ++i) {
        const unsigned char *r = p + JPC_TABLE + i * JPC_ROW;
        const unsigned char *b = r + 32;
        JpcU n = jpc_word(r, 12), offset = jpc_word(r, 16), packed = jpc_word(r, 20);
        JpcU pointer = jpc_word(b, 20), flags = jpc_word(r, 28);
        if (jpc_word(r, 0) != i + 1 || !jpc_word(r, 4) || (jpc_word(r, 4) & ~JPC_ALL_ROLES) ||
            !jpc_word(r, 8) || (jpc_word(r, 8) & ~255u) || !n ||
            (flags & ~JPC_COVERED_BY_HEAP) ||
            jpc_word(b, 0) != 0x4f424431u || jpc_word(b, 4) != 64 || jpc_word(b, 8) != 1 ||
            jpc_word(b, 12) != pointer || jpc_word(b, 16) != n || jpc_word(b, 24) != n ||
            pointer < 0x10000000u || pointer >= 0xbffe0000u ||
            n > 0xbffe0000u - pointer || !jpc_word(b, 28) ||
            /* Heap mutation epoch0 is a legitimate current OB value.
             * Actual capture/before-frame validation still compares it exactly. */
            !jpc_word(b, 32) || !jpc_word(b, 36) ||
            jpc_word(b, 44) < 1 || jpc_word(b, 44) > 4 || jpc_word(b, 52) != 31 ||
            jpc_word(b, 56) || jpc_word(b, 60)) return diagnostic ? 10u : JPC_FORMAT;
        for (j = 0; j < i; ++j) {
            const unsigned char *prior = p + JPC_TABLE + j * JPC_ROW;
            JpcU old = jpc_word(prior, 52), old_n = jpc_word(prior, 12);
            if (!((pointer >= old && pointer - old >= old_n) ||
                  (old >= pointer && old - pointer >= n))) return diagnostic ? 11u : JPC_FORMAT;
        }
        if (flags & JPC_COVERED_BY_HEAP) {
            if (offset || packed || jpc_word(r, 24) ||
                !jpc_contains(raw_start, raw_bytes, pointer, n)) return diagnostic ? 12u : JPC_FORMAT;
        } else {
            if (raw_total > JPC_MAX_RAW_BYTES || n > JPC_MAX_RAW_BYTES - raw_total) return diagnostic ? 13u : JPC_FORMAT;
            raw_total += n;
            if (!((pointer >= raw_start && pointer - raw_start >= raw_bytes) ||
                  (raw_start >= pointer && raw_start - pointer >= n)) || offset != at || !packed ||
                packed > bytes - at || oc_validate(p + at, packed, n) != OC_OK ||
                jpc_crc(p + at, packed) != jpc_word(r, 24)) return diagnostic ? 14u : JPC_FORMAT;
            at += packed;
            while (at & 31u) { if (at >= bytes || p[at]) return diagnostic ? 15u : JPC_FORMAT; ++at; }
        }
    }
    if (at != bytes || raw_total != jpc_word(p, 72)) return diagnostic ? 16u : JPC_FORMAT;
    for (i = 0; i < 8; ++i) {
        const unsigned char *m = p + JPC_LEDGER + 32 + i * 12;
        JpcU manager = jpc_word(m, 0), epoch = jpc_word(m, 4), capacity = jpc_word(m, 8);
        JpcU particle_count = jpc_word(p, 76 + i * 4);
        const unsigned char *pool = 0, *aux = 0;
        if (!manager) { if (epoch || capacity || particle_count) return diagnostic ? 17u : JPC_FORMAT; continue; }
        if (!epoch || !capacity || capacity > 256) return diagnostic ? 18u : JPC_FORMAT;
        if (particle_count > JPC_MAX_PARTICLES - particles) return diagnostic ? 19u : JPC_FORMAT;
        particles += particle_count;
        if (!jpc_particle_region(p, count, 1u << i, particle_count)) return diagnostic ? 20u : JPC_FORMAT;
        ++configured;
        for (j = 0; j < i; ++j) if (jpc_word(p, JPC_LEDGER + 32 + j * 12) == manager) return diagnostic ? 21u : JPC_FORMAT;
        for (j = 0; j < 8; ++j) {
            const unsigned char *candidate = p + JPC_POOLS + j * 32;
            if (jpc_word(candidate, 0) == manager) { if (pool) return diagnostic ? 22u : JPC_FORMAT; pool = candidate; }
            candidate = p + JPC_AUX_POOLS + j * 32;
            if (jpc_word(candidate, 0) == manager) { if (aux) return diagnostic ? 23u : JPC_FORMAT; aux = candidate; }
        }
        if (!pool || !jpc_word(pool, 4) || !jpc_word(pool, 8) || !jpc_word(pool, 16) ||
            jpc_word(pool, 12) != capacity || jpc_word(pool, 20) != epoch ||
            jpc_word(pool, 24) != 2 || jpc_word(pool, 28) ||
            !jpc_has_region(p, count, JPC_MANAGER, 1u << i, manager, 0x110) ||
            !jpc_has_region(p, count, JPC_EMITTERS, 1u << i, jpc_word(pool, 8), capacity * 0x3fcu))
            return diagnostic ? 24u : JPC_FORMAT;
        if (!aux || jpc_word(aux, 4) != jpc_word(pool, 4) ||
            jpc_word(pool, 16) <= jpc_word(pool, 4) || jpc_word(pool, 16) >= epoch ||
            jpc_word(aux, 16) != epoch || jpc_word(aux, 20) != epoch ||
            jpc_word(aux, 24) != 2 || jpc_word(aux, 28) ||
            jpc_word(aux, 12) > JPC_MAX_AUX_COUNT ||
            (!jpc_word(aux, 8) && jpc_word(aux, 12))) return diagnostic ? 25u : JPC_FORMAT;
        if (jpc_word(aux, 8)) {
            JpcU base = jpc_word(aux, 8);
            if ((base & 3u) || base < 0x10000008u ||
                !jpc_aux_cookie(p, count, 1u << i, base, jpc_word(aux, 12))) return diagnostic ? 26u : JPC_FORMAT;
        }
        if (manager == jpc_word(p, 56)) {
            ++control_found;
            if (!jpc_word(p, 52) || !jpc_has_region(p, count, JPC_CONTROL, 1u << i,
                                                  jpc_word(p, 52), 0x41a4)) return diagnostic ? 27u : JPC_FORMAT;
        }
    }
    if (!configured || control_found != 1) return diagnostic ? 28u : JPC_FORMAT;
    for (i = 0; i < 8; ++i) {
        const unsigned char *pool = p + JPC_POOLS + i * 32;
        JpcU manager = jpc_word(pool, 0), matches = 0;
        if (!manager) {
            for (j = 0; j < 32; ++j) if (pool[j]) return diagnostic ? 29u : JPC_FORMAT;
            continue;
        }
        for (j = 0; j < 8; ++j) matches += jpc_word(p, JPC_LEDGER + 32 + j * 12) == manager;
        if (matches != 1) return diagnostic ? 30u : JPC_FORMAT;
    }
    for (i = 0; i < 8; ++i) {
        const unsigned char *aux = p + JPC_AUX_POOLS + i * 32;
        JpcU manager = jpc_word(aux, 0), matches = 0;
        if (!manager) {
            for (j = 0; j < 32; ++j) if (aux[j]) return diagnostic ? 31u : JPC_FORMAT;
            continue;
        }
        for (j = 0; j < 8; ++j) matches += jpc_word(p, JPC_LEDGER + 32 + j * 12) == manager;
        if (matches != 1) return diagnostic ? 32u : JPC_FORMAT;
    }
    for (i = 0; i < 256; ++i) {
        const unsigned char *r = p + JPC_LEDGER + 128 + i * 32;
        const unsigned char *pool = 0;
        JpcU emitter = jpc_word(r, 8), manager = jpc_word(r, 0);
        if (!jpc_word(r, 28)) continue;
        for (j = 0; j < 8; ++j) {
            const unsigned char *candidate = p + JPC_POOLS + j * 32;
            if (jpc_word(candidate, 0) == manager) pool = candidate;
        }
        if (!pool || jpc_word(r, 4) != jpc_word(pool, 20) || emitter < jpc_word(pool, 8) ||
            (emitter - jpc_word(pool, 8)) % 0x3fcu ||
            (emitter - jpc_word(pool, 8)) / 0x3fcu >= jpc_word(pool, 12)) return diagnostic ? 33u : JPC_FORMAT;
        for (j = 0; j < i; ++j) {
            const unsigned char *prior = p + JPC_LEDGER + 128 + j * 32;
            if (jpc_word(prior, 28) && jpc_word(prior, 8) == emitter) return diagnostic ? 34u : JPC_FORMAT;
        }
    }
    for (i = 0; i < 64; ++i) {
        const unsigned char *child = p + JPC_LEDGER + 8320 + i * 40;
        JpcU alive = jpc_word(child, 8), edge;
        JpcU epoch = jpc_word(child, 4), serial_limit = jpc_word(p, JPC_LEDGER + 24);
        if (alive > 1) return diagnostic ? 35u : JPC_FORMAT;
        if (!alive) continue;
        if (!jpc_word(child, 0) || (jpc_word(child, 0) & 3u) || !epoch ||
            epoch > serial_limit || jpc_word(child, 12)) return diagnostic ? 36u : JPC_FORMAT;
        for (edge = 0; edge < 2; ++edge) {
            JpcU emitter = jpc_word(child, 16 + edge * 4);
            JpcU serial = jpc_word(child, 24 + edge * 4);
            JpcU manager = jpc_word(child, 32 + edge * 4), found = 0;
            JpcU managers = 0, current_epoch = 0, malformed;
            if (!emitter) { if (serial || manager) return diagnostic ? 37u : JPC_FORMAT; continue; }
            /* A completed real edge event consumes a serial after both its
             * child birth and referenced emitter birth. These are structural
             * bounds, not proof that every plausible old serial was issued. */
            malformed = (emitter & 3u) || !serial || !manager || (manager & 3u) ||
                serial >= serial_limit || epoch >= serial_limit;
            for (j = 0; j < 8; ++j) {
                const unsigned char *m = p + JPC_LEDGER + 32 + j * 12;
                if (jpc_word(m, 0) == manager) { ++managers; current_epoch = jpc_word(m, 4); }
            }
            for (j = 0; j < 256; ++j) {
                const unsigned char *r = p + JPC_LEDGER + 128 + j * 32;
                found += jpc_word(r, 28) == 1 && jpc_word(r, 8) == emitter &&
                         jpc_word(r, 12) == serial && jpc_word(r, 0) == manager &&
                         jpc_word(r, 4) == current_epoch;
            }
            if (malformed || managers != 1 || found > 1) return diagnostic ? 38u : JPC_FORMAT;
            /* found1 is derived CURRENT metadata after the existing current
             * row/pool checks. found0 is retained HISTORICAL data, including
             * removal, reuse, eviction or reconstruction with no witness left.
             * Do not require historical pointers inside today's emitter pool.
             * Exact held-source ledger equality supplies capture provenance;
             * neither classification grants native access or replay authority. */
        }
    }
    if (jpc_word(p, 32) != _gz_jpc_checksum(p, bytes)) return diagnostic ? 39u : JPC_FORMAT;
    return JPC_OK;
}


/* Both public leaves execute these exact checks. Diagnostics select only the
 * returned rejection ordinal; they grant no native access or replay authority. */
JpcU _gz_jpc_validate(const void *image, JpcU bytes, const GhcSaveProducer *producer,
                    JpcU root, JpcU generation, JpcU raw_start, JpcU raw_bytes) {
    return jpc_validate_impl(image, bytes, producer, root, generation, raw_start, raw_bytes, 0);
}

JpcU _gz_jpc_diagnostic(const void *image, JpcU bytes, const GhcSaveProducer *producer,
                    JpcU root, JpcU generation, JpcU raw_start, JpcU raw_bytes) {
    return jpc_validate_impl(image, bytes, producer, root, generation, raw_start, raw_bytes, 1);
}
