/* Same-TU current allocation reader under the genuine GHS/HG/worker hold.
 * Pool slots are retained as opaque bytes, including inactive/child slots.
 * No actor-profile mapping, RKC impersonation or diagnostic answer is used.
 * This increment captures data only; native replay/lifetime adoption follows.
 */
#include "particle_domain_cut.h"
#include "particle_lifetime_layout.h"

extern volatile u32 _gz_jpc_native_control_slot __asm__("_native_game_literal_10474b60");
extern volatile u32 _gz_jpc_native_manager_slot __asm__("_native_game_literal_1047b2d4");
extern volatile u32 _gz_jpc_native_seed __asm__("_native_game_literal_104b5748");

static struct {
    u32 entered, request, frame, raw, bytes, count, control, manager, seed, raw_total;
    u32 particle_count[JEL_MANAGERS];
    JelState ledger;
    JelPool pools[JEL_MANAGERS];
    JelAuxPool aux_pools[JEL_MANAGERS];
    JpcRegion region[JPC_REGIONS];
    ObBinding query, whole, seed_binding;
} jpc __attribute__((section(".bss.gz_jpc")));

/* A real source-owned DATA receiver outside the protected JPC work object.
 * JP's unchanged private predicate requires DATA even for OB metadata, so a
 * MEM1 stack local cannot receive a generic binding. This exact receiver is
 * used only under jpc_scope and is never enrolled as native particle state.
 */
static ObBinding jpc_receiver __attribute__((section(".bss.gz_jpc_metadata"), aligned(4)));

typedef char JpcLedger10880[sizeof(JelState) == JPC_LEDGER_BYTES ? 1 : -1];
typedef char JpcPool256[sizeof(jpc.pools) == JPC_POOL_BYTES ? 1 : -1];
typedef char JpcAuxPool256[sizeof(jpc.aux_pools) == JPC_AUX_POOL_BYTES ? 1 : -1];
typedef char JpcAuxReceipt32[sizeof(JelAuxPool) == 32 ? 1 : -1];
typedef char JpcMetadata64[sizeof(jpc_receiver) == 64 ? 1 : -1];

GHC_FN u32 jpc_private_apart(u32 p, u32 n) {
    return ghc_apart(p, n, (u32)&jpc, sizeof(jpc));
}

GHC_FN u32 jpc_scope(void) {
    return jpc.entered == 1 && _gz_ghs_scope(jpc.request, jpc.frame, 2) &&
        ghc_native_scope(jpc.request, jpc.frame, 2) &&
        _gz_jwa_ghs_workers_zero(jpc.request) == 0 &&
        ghc_equal(&jpc.ledger, &_gz_jel_state, sizeof(jpc.ledger)) &&
        ghc_equal(jpc.pools, _gz_jel_pools, sizeof(jpc.pools)) &&
        ghc_equal(jpc.aux_pools, _gz_jel_aux_pools, sizeof(jpc.aux_pools));
}

GHC_FN u32 jpc_readable(u32 p, u32 n) {
    return jpc_scope() && span(p, n) &&
        ghc_known_apart(jpc.request, jpc.frame, p, n) && jpc_private_apart(p, n) &&
        ghc_apart(p, n, (u32)&jpc_receiver, sizeof(jpc_receiver));
}

GHC_FN u32 jpc_bind(u32 p, u32 n, ObBinding *binding) {
    u32 readable, captured, validated, result = JPC_LIFETIME;
    /* Generic metadata APIs deliberately reject this module's protected
     * JPC work object. Use the exact ordinary DATA receiver, validate that
     * current binding, then retain it in our own scoped work object.
     * No exception is added to OIS/OB private-range or archive-write guards.
     */
    if (binding != &jpc.query && binding != &jpc.whole && binding != &jpc.seed_binding)
        return JPC_SCOPE;
    _gz_ghc_report[24] = p; _gz_ghc_report[25] = n;
    _gz_ghc_report[27] = 0xffffffffu; _gz_ghc_report[28] = 0xffffffffu;
    readable = jpc_readable(p, n); _gz_ghc_report[26] = readable;
    if (!readable) return JPC_LIFETIME;
    zero(&jpc_receiver, sizeof(jpc_receiver));
    captured = _gz_ob_capture(p, n, &jpc_receiver); _gz_ghc_report[27] = captured;
    if (captured || jpc_receiver.query != p || jpc_receiver.query_bytes != n) goto done;
    validated = _gz_ob_validate_before(&jpc_receiver, jpc.frame); _gz_ghc_report[28] = validated;
    if (validated || !jpc_scope()) goto done;
    ghc_copy(binding, &jpc_receiver, sizeof(jpc_receiver));
    if (!ghc_equal(binding, &jpc_receiver, sizeof(jpc_receiver)) || !jpc_scope()) goto done;
    result = JPC_OK;
done:
    zero(&jpc_receiver, sizeof(jpc_receiver));
    return result;
}

/* Only a bounded current region in our own work object can supply a saved
 * binding. The generic validator receives the same ordinary DATA receiver
 * as capture; no protected JPC pointer or packet-selected address reaches it.
 */
GHC_FN u32 jpc_region_current(u32 index) {
    const ObBinding *binding;
    u32 current = 0;
    if (index >= JPC_REGIONS || index >= jpc.count || !jpc_scope()) return 0;
    binding = &jpc.region[index].allocation;
    zero(&jpc_receiver, sizeof(jpc_receiver));
    ghc_copy(&jpc_receiver, binding, sizeof(jpc_receiver));
    if (!ghc_equal(&jpc_receiver, binding, sizeof(jpc_receiver)) || !jpc_scope()) goto done;
    if (_gz_ob_validate_before(&jpc_receiver, jpc.frame) ||
        !ghc_equal(&jpc_receiver, binding, sizeof(jpc_receiver)) || !jpc_scope()) goto done;
    current = 1;
done:
    zero(&jpc_receiver, sizeof(jpc_receiver));
    return current;
}

GHC_FN u32 jpc_same_allocation(const ObBinding *a, const ObBinding *b) {
    return a->pointer == b->pointer && a->capacity == b->capacity &&
        a->generation == b->generation && a->heap == b->heap &&
        a->heap_generation == b->heap_generation && a->epoch == b->epoch &&
        a->kind == b->kind && a->block == b->block &&
        a->missing == b->missing && a->session == b->session && !a->reserved && !b->reserved;
}

/* Normalize any freshly queried subrange to its entire current allocation.
 * Repeated references merge; overlapping different allocations refuse.
 * Objects already in the main heap remain explicitly marked as covered.
 */
GHC_FN u32 jpc_enroll(u32 p, u32 n, u32 role, u32 manager) {
    u32 z, i, covered;
    JpcRegion *r;
    z = jpc_bind(p, n, &jpc.query); if (z) return z;
    z = jpc_bind(jpc.query.pointer, jpc.query.capacity, &jpc.whole); if (z) return z;
    if (!jpc_same_allocation(&jpc.query, &jpc.whole)) return JPC_LIFETIME;
    for (i = 0; i < jpc.count; ++i) {
        r = jpc.region + i;
        if (jpc_same_allocation(&r->allocation, &jpc.whole)) {
            r->roles |= role; r->managers |= manager; return JPC_OK;
        }
        if (!ghc_apart(r->allocation.pointer, r->allocation.capacity,
                       jpc.whole.pointer, jpc.whole.capacity)) return JPC_CHANGED;
    }
    if (jpc.count == JPC_REGIONS) return JPC_SPACE;
    covered = ghc_contains(jpc.raw, jpc.bytes, jpc.whole.pointer, jpc.whole.capacity);
    if (!covered) {
        if (!ghc_apart(jpc.raw, jpc.bytes, jpc.whole.pointer, jpc.whole.capacity)) return JPC_CHANGED;
        /* This bounds whole external payload work, independently of the
         * unchanged 4 MiB encoded transport. It grants no native authority
         * and allocates no raw-domain scratch. Deduplicated owners count once. */
        if (jpc.raw_total > JPC_MAX_RAW_BYTES ||
            jpc.whole.capacity > JPC_MAX_RAW_BYTES - jpc.raw_total) return JPC_SPACE;
    }
    r = jpc.region + jpc.count;
    zero(r, sizeof(*r)); r->key = ++jpc.count; r->roles = role; r->managers = manager;
    r->raw_bytes = jpc.whole.capacity; ghc_copy(&r->allocation, &jpc.whole, sizeof(ObBinding));
    if (covered) r->flags = JPC_COVERED_BY_HEAP;
    else jpc.raw_total += r->raw_bytes;
    return JPC_OK;
}

GHC_FN u32 jpc_manager_index(u32 pointer) {
    u32 i, found = JEL_MANAGERS;
    for (i = 0; i < JEL_MANAGERS; ++i) if (jpc.ledger.manager[i].pointer == pointer) {
        if (found != JEL_MANAGERS) return JEL_MANAGERS;
        found = i;
    }
    return found;
}

GHC_FN const JelPool *jpc_pool(u32 manager) {
    const JelPool *found = 0;
    u32 i;
    for (i = 0; i < JEL_MANAGERS; ++i) if (jpc.pools[i].manager == manager) {
        if (found) return 0;
        found = jpc.pools + i;
    }
    return found;
}

GHC_FN const JelAuxPool *jpc_aux_pool(u32 manager) {
    const JelAuxPool *found = 0;
    u32 i;
    for (i = 0; i < JEL_MANAGERS; ++i) if (jpc.aux_pools[i].manager == manager) {
        if (found) return 0;
        found = jpc.aux_pools + i;
    }
    return found;
}

GHC_FN u32 jpc_live_emitter(const JelRow *r, const JelPool *pool) {
    return r->alive == 1 && r->manager == pool->manager &&
        r->manager_epoch == pool->epoch && r->serial && r->resource <= 65535 &&
        r->group < 16 && r->index < 8 && r->emitter >= pool->base &&
        (r->emitter - pool->base) % 0x3fcu == 0 &&
        (r->emitter - pool->base) / 0x3fcu < pool->capacity;
}

/* Manager bodies are read only after a fresh ordinary current OB query.
 * Keep this outside jpc_scope: jpc_bind itself uses that metadata-only scope,
 * so checking native counts there would recurse or precede OB admission. */
GHC_FN u32 jpc_particle_counts_current(void) {
    u32 i, total = 0;
    if (!jpc_scope()) return 0;
    for (i = 0; i < JEL_MANAGERS; ++i) {
        const JelManager *m = jpc.ledger.manager + i;
        if (!m->pointer) { if (jpc.particle_count[i]) return 0; continue; }
        if (jpc.particle_count[i] > JPC_MAX_PARTICLES - total) return 0;
        total += jpc.particle_count[i];
        if (jpc_bind(m->pointer, 0x110, &jpc.query) ||
            R32(m->pointer, 0x24) != jpc.particle_count[i]) return 0;
    }
    return jpc_scope();
}

GHC_FN u32 jpc_plan(u32 request, u32 frame, const GhcHeader *parent) {
    u32 i, j, z, configured = 0, particles = 0, bit, count, seed;
    const JelPool *pool;
    const JelAuxPool *aux;
    if (jpc.entered || !_gz_ghs_scope(request, frame, 2) ||
        _gz_jwa_ghs_workers_zero(request) != 0 ||
        _gz_jel_state.magic != 0x4a454c31u || _gz_jel_state.bytes != sizeof(JelState) ||
        _gz_jel_state.version != 1 || _gz_jel_state.enabled != 1 ||
        _gz_jel_state.busy || _gz_jel_state.flags) return JPC_SCOPE;
    _gz_ghc_report[23] = 1;
    zero(&jpc, sizeof(jpc));
    zero(&jpc_receiver, sizeof(jpc_receiver));
    jpc.request = request; jpc.frame = frame; jpc.raw = parent->domain.start; jpc.bytes = parent->domain.span;
    ghc_copy(&jpc.ledger, &_gz_jel_state, sizeof(jpc.ledger));
    ghc_copy(jpc.pools, _gz_jel_pools, sizeof(jpc.pools));
    ghc_copy(jpc.aux_pools, _gz_jel_aux_pools, sizeof(jpc.aux_pools));
    jpc.entered = 1;
    if (!jpc_scope()) return JPC_CHANGED;
    jpc.control = _gz_jpc_native_control_slot;
    jpc.manager = _gz_jpc_native_manager_slot;
    jpc.seed = _gz_jpc_native_seed;
    for (i = 0; i < JEL_MANAGERS; ++i) {
        const JelManager *m = jpc.ledger.manager + i;
        if (!m->pointer) {
            if (m->epoch || m->capacity) return JPC_FORMAT;
            continue;
        }
        ++configured; bit = 1u << i; pool = jpc_pool(m->pointer); aux = jpc_aux_pool(m->pointer);
        _gz_ghc_report[23] = 100u + i;
        if (!m->epoch || !m->capacity || m->capacity > JEL_ROWS || !pool ||
            pool->phase != 2 || !pool->construction || !pool->serial || pool->reserved ||
            pool->epoch != m->epoch || pool->capacity != m->capacity || !pool->base ||
            jpc_manager_index(m->pointer) != i || !aux || aux->phase != 2 || aux->reserved ||
            aux->construction != pool->construction || pool->serial <= pool->construction ||
            pool->serial >= m->epoch || aux->serial != m->epoch ||
            aux->epoch != m->epoch || aux->count > JPC_MAX_AUX_COUNT ||
            (!aux->base && aux->count)) return JPC_LIFETIME;
        _gz_ghc_report[23] = 200u + i;
        z = jpc_enroll(m->pointer, 0x110, JPC_MANAGER, bit); if (z) return z;
        if (R32(m->pointer, 0x28) != m->capacity) return JPC_CHANGED;
        _gz_ghc_report[23] = 300u + i;
        z = jpc_enroll(pool->base, m->capacity * 0x3fcu, JPC_EMITTERS, bit); if (z) return z;
        /* The third constructor-owned array is opaque. Its receipt, cookie
         * and exact current allocation start must all agree. No free-head
         * field is a substitute for its base, even for an empty array. */
        if (aux->base) {
            u32 auxiliary = aux->base - 8, extent = aux->count * 0x9cu + 8;
            _gz_ghc_report[23] = 350u + i;
            if ((aux->base & 3u) || aux->base < 0x10000008u) return JPC_FORMAT;
            z = jpc_bind(auxiliary, extent, &jpc.query); if (z) return z;
            if (jpc.query.pointer != auxiliary || jpc.query.capacity < extent) return JPC_LIFETIME;
            if (R32(auxiliary, 0) != (aux->count ? aux->count * 0x9cu : 1u) ||
                R32(auxiliary, 4) != ~aux->count) return JPC_FORMAT;
            z = jpc_enroll(auxiliary, extent, JPC_AUX, bit); if (z) return z;
        }
        count = R32(m->pointer, 0x24);
        if (count > JPC_MAX_PARTICLES - particles || R32(m->pointer, 8) > count) return JPC_SPACE;
        jpc.particle_count[i] = count;
        particles += count;
        if (!count) continue;
        seed = R32(m->pointer, 0);
        if (!seed) for (j = 0; j < JEL_ROWS; ++j) {
            const JelRow *r = jpc.ledger.row + j;
            if (!r->alive || r->manager != m->pointer) continue;
            if (!jpc_live_emitter(r, pool)) return JPC_LIFETIME;
            z = jpc_bind(r->emitter, 0x3fc, &jpc.query); if (z) return z;
            seed = R32(r->emitter, 0x1ac);
            if (!seed) seed = R32(r->emitter, 0x1b8);
            if (seed) break;
        }
        if (!seed) return JPC_FORMAT;
        _gz_ghc_report[23] = 400u + i;
        z = jpc_bind(seed, 16, &jpc.seed_binding); if (z) return z;
        if (jpc.seed_binding.capacity < count * 0x128u + 8 ||
            seed < jpc.seed_binding.pointer + 8 ||
            (seed - jpc.seed_binding.pointer - 8) % 0x128u ||
            (seed - jpc.seed_binding.pointer - 8) / 0x128u >= count || R32(seed, 0) != seed)
            return JPC_FORMAT;
        z = jpc_bind(jpc.seed_binding.pointer, 8, &jpc.query); if (z) return z;
        if (!jpc_same_allocation(&jpc.seed_binding, &jpc.query) ||
            R32(jpc.query.pointer, 0) != count * 0x128u || R32(jpc.query.pointer, 4) != ~count)
            return JPC_FORMAT;
        z = jpc_enroll(jpc.seed_binding.pointer, count * 0x128u + 8, JPC_PARTICLES, bit); if (z) return z;
    }
    if (!configured || !jpc.control || jpc_manager_index(jpc.manager) == JEL_MANAGERS) return JPC_LIFETIME;
    _gz_ghc_report[23] = 500u;
    z = jpc_enroll(jpc.control, 0x41a4, JPC_CONTROL, 1u << jpc_manager_index(jpc.manager)); if (z) return z;
    /* Whole particle pools already include children. Deferred pointer arrays
     * have independent allocations, so enroll those allocation payloads too.
     * The captured array words remain audit data, never native read targets.
     */
    for (i = 0; i < JEL_ROWS; ++i) {
        const JelRow *r = jpc.ledger.row + i;
        u32 manager, array, size, capacity, offset;
        if (r->alive > 1) return JPC_FORMAT;
        if (!r->alive) continue;
        _gz_ghc_report[23] = 600u + i;
        manager = jpc_manager_index(r->manager); pool = jpc_pool(r->manager);
        if (manager == JEL_MANAGERS || !pool || !jpc_live_emitter(r, pool)) return JPC_LIFETIME;
        z = jpc_bind(r->emitter, 0x3fc, &jpc.query); if (z) return z;
        if (R32(r->emitter, 0x1dc) != r->manager || (R32(r->emitter, 0x260) >> 16) != r->resource ||
            ((R32(r->emitter, 0x260) >> 8) & 255u) != r->group) return JPC_CHANGED;
        for (j = 0; j < 2; ++j) {
            offset = 0x1c4 + 12 * j;
            size = R32(r->emitter, offset); capacity = R32(r->emitter, offset + 4);
            array = R32(r->emitter, offset + 8);
            if (size > capacity || capacity > JPC_MAX_RAW_BYTES / 4 || (!capacity && array)) return JPC_FORMAT;
            if (capacity) {
                if (!array) return JPC_FORMAT;
                z = jpc_enroll(array, capacity * 4, JPC_DEFERRED, 1u << manager); if (z) return z;
            }
        }
    }
    return jpc_scope() && jpc.control == _gz_jpc_native_control_slot &&
        jpc.manager == _gz_jpc_native_manager_slot && jpc.seed == _gz_jpc_native_seed &&
        jpc_particle_counts_current() ? JPC_OK : JPC_CHANGED;
}

GHC_FN u32 jpc_crc(const unsigned char *p, u32 n) {
    u32 i, hash = 2166136261u;
    for (i = 0; i < n; ++i) hash = (hash ^ p[i]) * 16777619u;
    return hash;
}

GHC_FN u32 jpc_capture(u32 request, u32 frame, const GhcHeader *parent,
                      u32 output, u32 capacity, u32 *used) {
    JpcHeader *header = (JpcHeader *)output;
    const GhcSaveProducer *producer = (const GhcSaveProducer *)((u32)parent + 128);
    JpcRegion *rows;
    u32 z, i, at, packed;
    if (!used || !_gz_ois_heap_stage_match(request, frame, output, capacity) ||
        capacity < JPC_TABLE || capacity > JPC_MAX_BYTES) return JPC_SCOPE;
    *used = 0;
    z = jpc_plan(request, frame, parent);
    _gz_ghc_report[29] = jpc.count; _gz_ghc_report[31] = z;
    if (z) return z;
    at = (JPC_TABLE + jpc.count * JPC_ROW + 31u) & ~31u;
    if (at > capacity) return JPC_SPACE;
    zero(header, at);
    header->magic = JPC_MAGIC; header->version = JPC_VERSION; header->missing = 255;
    header->regions = jpc.count; header->table = JPC_TABLE; header->data = at;
    header->request = request; header->frame = frame; header->root = parent->domain.heap;
    header->root_generation = parent->domain.heap_generation;
    header->control = jpc.control; header->control_manager = jpc.manager; header->random_seed = jpc.seed;
    header->raw_start = jpc.raw; header->raw_bytes = jpc.bytes;
    header->raw_total = jpc.raw_total;
    ghc_copy(header->particle_count, jpc.particle_count, sizeof(jpc.particle_count));
    ghc_copy((void *)(output + JPC_PRODUCER), producer, 64);
    ghc_copy((void *)(output + JPC_LEDGER), &jpc.ledger, sizeof(jpc.ledger));
    ghc_copy((void *)(output + JPC_POOLS), jpc.pools, sizeof(jpc.pools));
    ghc_copy((void *)(output + JPC_AUX_POOLS), jpc.aux_pools, sizeof(jpc.aux_pools));
    rows = (JpcRegion *)(output + JPC_TABLE);
    for (i = 0; i < jpc.count; ++i) {
        JpcRegion *r = rows + i;
        ghc_copy(r, jpc.region + i, sizeof(*r));
        if (!jpc_scope() || !jpc_region_current(i)) return JPC_CHANGED;
        if (r->flags & JPC_COVERED_BY_HEAP) continue;
        if (!jpc_readable(r->allocation.pointer, r->raw_bytes) || at >= capacity) return JPC_SCOPE;
        z = oc_encode((const void *)r->allocation.pointer, r->raw_bytes,
                      (void *)(output + at), capacity - at, &packed);
        if (z != OC_OK) return z == OC_SPACE ? JPC_SPACE : JPC_CHANGED;
        r->offset = at; r->packed_bytes = packed;
        r->checksum = jpc_crc((const unsigned char *)(output + at), packed);
        at += packed;
        while (at & 31u) { if (at >= capacity) return JPC_SPACE; *(unsigned char *)(output + at++) = 0; }
        if (oc_matches((const void *)(output + r->offset), packed, r->raw_bytes,
                       (const void *)r->allocation.pointer) != OC_OK || !jpc_scope() ||
            !jpc_region_current(i)) return JPC_CHANGED;
    }
    if (!jpc_particle_counts_current()) return JPC_CHANGED;
    header->bytes = at; header->checksum = _gz_jpc_checksum(header, at);
    __sync_synchronize(); header->valid = 1;
    z = _gz_jpc_validate(header, at, producer, parent->domain.heap,
                        parent->domain.heap_generation, jpc.raw, jpc.bytes);
    if (z) {
        /* The same held PRIVATE packet remains valid until its actual failure
         * reason is recorded. Preserve the ordinary status and cleanup path. */
        _gz_ghc_report[23] = 0x80000000u | _gz_jpc_diagnostic(header, at, producer,
            parent->domain.heap, parent->domain.heap_generation, jpc.raw, jpc.bytes);
        header->valid = 0; return z;
    }
    *used = at; return JPC_OK;
}

GHC_FN u32 jpc_verify(u32 request, u32 frame, const GhcHeader *parent,
                     const JpcHeader *header, u32 bytes) {
    const JpcRegion *rows = (const JpcRegion *)((u32)header + JPC_TABLE);
    u32 i;
    if (!jpc_scope() || request != jpc.request || frame != jpc.frame ||
        _gz_jpc_validate(header, bytes, (const GhcSaveProducer *)((u32)parent + 128),
                         parent->domain.heap, parent->domain.heap_generation, jpc.raw, jpc.bytes) ||
        header->regions != jpc.count || header->raw_total != jpc.raw_total ||
        header->control != jpc.control || header->control_manager != jpc.manager ||
        header->random_seed != _gz_jpc_native_seed || jpc.control != _gz_jpc_native_control_slot ||
        jpc.manager != _gz_jpc_native_manager_slot ||
        !ghc_equal((const unsigned char *)header + JPC_LEDGER, &jpc.ledger, sizeof(jpc.ledger)) ||
        !ghc_equal((const unsigned char *)header + JPC_POOLS, jpc.pools, sizeof(jpc.pools)) ||
        !ghc_equal((const unsigned char *)header + JPC_AUX_POOLS, jpc.aux_pools, sizeof(jpc.aux_pools)) ||
        !ghc_equal(header->particle_count, jpc.particle_count, sizeof(jpc.particle_count)) ||
        !jpc_particle_counts_current())
        return JPC_CHANGED;
    for (i = 0; i < jpc.count; ++i) {
        const JpcRegion *r = rows + i;
        if (!ghc_equal(&r->allocation, &jpc.region[i].allocation, sizeof(ObBinding)) ||
            !jpc_region_current(i) || !jpc_scope()) return JPC_CHANGED;
        if (!(r->flags & JPC_COVERED_BY_HEAP) &&
            oc_matches((const void *)((u32)header + r->offset), r->packed_bytes, r->raw_bytes,
                       (const void *)r->allocation.pointer) != OC_OK) return JPC_CHANGED;
    }
    return jpc_scope() && jpc_particle_counts_current() ? JPC_OK : JPC_CHANGED;
}

GHC_FN u32 jpc_clear(u32 request, u32 frame) {
    if (!jpc.entered) { zero(&jpc_receiver, sizeof(jpc_receiver)); return JPC_OK; }
    if (jpc.request != request || jpc.frame != frame || !_gz_ghs_scope(request, frame, 3)) return JPC_SCOPE;
    zero(&jpc_receiver, sizeof(jpc_receiver));
    zero(&jpc, sizeof(jpc)); return JPC_OK;
}
