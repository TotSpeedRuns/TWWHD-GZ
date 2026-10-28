/* Appended to the exact retained TU. This resolver only reads private HL/HI/
 * HM metadata. Every table probe, list node and scope step spends one shared
 * visit; depth and index bounds apply independently. It never follows a
 * query, allocation, heap, block, vtable or callback into native memory.
 *
 * Existing observer APIs and diagnostics are untouched. These new functions
 * do not flag corrupt metadata or increment the old index counters. Their
 * only non-stack stores are the existing lock protocol and successful output.
 */
#include "object_binding_try.h"
#define OBT_FN static __attribute__((noinline, section(".text.gz_ob_try")))
#define OBT_API __attribute__((noinline, section(".text.gz_ob_try")))
typedef struct { u32 left, exhausted; } ObTryWork;
/* Alternate C identifiers for the real retained exported ABI prevent GCC
 * from cloning/inlining the old protocol into this new section. Emitted
 * REL24 targets must remain the exact two existing lock symbols. */
extern u32 obt_real_try_lock(void) __asm__("_gz_hl_meta_try_lock");
extern void obt_real_unlock(void) __asm__("_gz_hl_meta_unlock");

OBT_FN u32 obt_take(ObTryWork *work) {
    if (!work->left) { work->exhausted = 1; return 0; }
    --work->left;
    return 1;
}

OBT_FN u32 obt_buffer_span(u32 p) {
    return !(p & 3) && p >= 0x10000000u && p <= 0xbffe0000u - 64;
}

OBT_FN u32 obt_header(const ObBinding *b) {
    return b->magic == 0x4f424431u && b->bytes == 64 && b->version == 1 &&
        span(b->query, b->query_bytes) && b->generation && b->heap_generation &&
        b->missing == HL_MISSING_ALL && !b->session && !b->reserved;
}

OBT_FN u32 obt_equal(const ObBinding *a, const ObBinding *b) {
    u32 i;
    for (i = 0; i < 16; ++i)
        if (((const u32 *)a)[i] != ((const u32 *)b)[i]) return 0;
    return 1;
}

OBT_FN u32 obt_healthy(void) {
    return _gz_hl_control.magic == HL_MAGIC && _gz_hl_control.bytes == 64 &&
        _gz_hl_control.version == 1 && _gz_hl_control.busy == 1 &&
        _gz_hl_control.active_heaps <= HL_HEAPS &&
        _gz_hl_control.active_allocations <= HL_ALLOCS && hr_healthy();
}

/* hr_private(...,0) checks every fixed private exclusion once. The remaining
 * overlap checks are exactly the heap-dependent part of ob_private(). The
 * high-DATA guard makes _gz_abi_metadata_span return without a stack read.
 */
OBT_FN u32 obt_private(u32 p, ObTryWork *work) {
    u32 i;
    HlHeap *h;
    if (!hr_private(p, 64, 0)) return 0;
    for (i = 0; i < HL_HEAPS; ++i) {
        if (!obt_take(work)) return 0;
        h = _gz_hl_heaps + i;
        if (h->heap && (h->state == HL_LIVE || h->state == HL_MUTATING || h->state == HL_ENDING) &&
            (!hr_apart(p, 64, h->start, h->bytes) ||
             !hr_apart(p, 64, h->heap, header_bytes(h->kind)))) return 0;
    }
    return 1;
}

OBT_FN HlHeap *obt_heap(u32 p, ObTryWork *work) {
    u32 i, j, start = ((p >> 2) * 0x9e3779b1u) >> 20;
    HlHeap *h;
    for (i = 0; i < HL_HEAPS; ++i) {
        if (!obt_take(work)) return 0;
        j = (start + i) & (HL_HEAPS - 1);
        h = _gz_hl_heaps + j;
        if (h->heap == p) return h;
        if (!h->heap) return 0;
    }
    return 0;
}

OBT_FN HlStoredAllocation *obt_generation(u32 p, u32 generation, ObTryWork *work) {
    u32 i, j, start = ((p >> 2) * 0x9e3779b1u) >> 14;
    HlStoredAllocation *a;
    for (i = 0; i < HL_ALLOCS; ++i) {
        if (!obt_take(work)) return 0;
        j = (start + i) & (HL_ALLOCS - 1);
        a = _gz_hl_allocations + j;
        if (!a->pointer) return 0;
        if (a->pointer == p && a->generation == generation) return a;
    }
    return 0;
}

/* Same parent/header/custom provenance conditions as hm_scope_live; the old
 * depth32/step64 bound remains in addition to the new shared work budget.
 */
OBT_FN u32 obt_scope(HlHeap *h, u32 depth, u32 *scope_left, ObTryWork *work) {
    HmScope *s;
    HlHeap *parent, *header_owner;
    HlStoredAllocation *m, *header;
    for (; depth < 32; ++depth) {
        if (!*scope_left || !obt_take(work)) return 0;
        --*scope_left;
        if (!h || h->state != HL_LIVE) return 0;
        if (h->kind == HL_CUSTOM) {
            HlCustomMeta *c = _gz_hl_custom_meta + hm_index(h);
            HlStoredAllocation *d;
            HlHeap *owner;
            if (c->heap_generation != h->generation || !c->slots || c->slots > 0x0fffffffu) return 0;
            d = obt_generation(c->anchor_pointer, c->anchor_generation, work);
            if (!d || d->state != HL_LIVE || !within(d->pointer, d->capacity, c->table, c->slots * 16)) return 0;
            owner = obt_heap(d->heap, work);
            if (!owner || owner == h || owner->state != HL_LIVE || owner->generation != d->heap_generation ||
                owner->epoch != d->heap_epoch || !obt_scope(owner, depth + 1, scope_left, work)) return 0;
        }
        if (!h->parent) return 1;
        s = _gz_hm_scopes + hm_index(h);
        parent = obt_heap(h->parent, work);
        if (s->flags || !parent || parent->state != HL_LIVE || parent->generation != h->parent_generation ||
            parent->epoch != s->parent_epoch) return 0;
        m = obt_generation(s->managed_pointer, s->managed_generation, work);
        header = obt_generation(s->header_pointer, s->header_generation, work);
        if (!m || !header || m->state != HL_LIVE || header->state != HL_LIVE ||
            m->generation != s->managed_generation || header->generation != s->header_generation ||
            m->heap != parent->heap || m->heap_generation != parent->generation || m->heap_epoch != parent->epoch ||
            !within(m->pointer, m->capacity, h->start, h->bytes) ||
            !within(header->pointer, header->capacity, h->heap, header_bytes(h->kind))) return 0;
        header_owner = obt_heap(header->heap, work);
        if (!header_owner || header_owner->state != HL_LIVE || header_owner->generation != header->heap_generation ||
            header_owner->epoch != header->heap_epoch) return 0;
        if (header_owner != parent && !obt_scope(header_owner, depth + 1, scope_left, work)) return 0;
        h = parent;
    }
    return 0;
}

OBT_FN u32 obt_descends(HlHeap *child, HlHeap *ancestor, HlStoredAllocation *outer, ObTryWork *work) {
    u32 i;
    HlHeap *h = child, *parent;
    if (!within(outer->pointer, outer->capacity, child->start, child->bytes)) return 0;
    for (i = 0; i < 32; ++i) {
        if (!obt_take(work) || !h->parent || !h->parent_generation) return 0;
        parent = obt_heap(h->parent, work);
        if (!parent || parent->state != HL_LIVE || parent->generation != h->parent_generation) return 0;
        if (parent == ancestor) return 1;
        h = parent;
    }
    return 0;
}

/* One list pass combines the old hi_ready validation and owner traversal.
 * A corrupt tail still refuses before output; every id is checked before it
 * indexes allocations or links. No diagnostics/flags are changed here.
 */
OBT_FN u32 obt_owner(u32 p, u32 n, HlStoredAllocation **allocation, HlHeap **heap, ObTryWork *work) {
    u32 i, j, node, previous, scope_left;
    HlStoredAllocation *a, *found = 0;
    HlHeap *h, *owner = 0;
    HiHeap *index;
    HiLink *link;
    *allocation = 0;
    *heap = 0;
    if (!span(p, n) || _gz_hl_control.flags || !hi_header() ||
        _gz_hi_control.nodes != _gz_hl_control.active_allocations) return 3;
    for (i = 0; i < HL_HEAPS; ++i) {
        if (!obt_take(work)) return 3;
        h = _gz_hl_heaps + i;
        index = _gz_hi_heaps + i;
        if (!h->heap) {
            if (index->head || index->count || index->generation || index->reserved) return 3;
            continue;
        }
        if (!within(h->start, h->bytes, p, n)) continue;
        if (index->generation != h->generation || index->reserved || index->count > HL_ALLOCS ||
            (!index->head) != (!index->count)) return 3;
        node = index->head;
        previous = 0;
        for (j = 0; j < index->count; ++j) {
            if (!obt_take(work) || !hi_member(node, i + 1, h->generation)) return 3;
            link = _gz_hi_links + node - 1;
            if (link->previous != previous) return 3;
            a = _gz_hl_allocations + node - 1;
            previous = node;
            node = link->next;
            if (a->state != HL_LIVE || !within(a->pointer, a->capacity, p, n)) continue;
            scope_left = 64;
            if (h->state != HL_LIVE || !a->heap_generation || a->heap_generation != h->generation ||
                a->heap_epoch != h->epoch || !obt_scope(h, 0, &scope_left, work)) return 3;
            if (found) {
                if (h == owner) return 2;
                if (obt_descends(h, owner, found, work)) { found = a; owner = h; }
                else if (!obt_descends(owner, h, a, work)) return 2;
            } else { found = a; owner = h; }
        }
        if (node) return 3;
    }
    if (!found) return 1;
    *allocation = found;
    *heap = owner;
    return 0;
}

OBT_FN void obt_fill(ObBinding *b, u32 p, u32 n, const HlStoredAllocation *a, const HlHeap *h) {
    zero(b, sizeof(*b));
    b->magic = 0x4f424431u; b->bytes = 64; b->version = 1;
    b->query = p; b->query_bytes = n; b->pointer = a->pointer; b->capacity = a->capacity;
    b->generation = a->generation; b->heap = h->heap; b->heap_generation = h->generation;
    b->epoch = h->epoch; b->kind = a->kind; b->block = a->block; b->missing = HL_MISSING_ALL;
}

OBT_API ObU32 _gz_ob_try_capture(u32 p, u32 n, ObBinding *out) {
    HlStoredAllocation *a;
    HlHeap *h;
    ObBinding b;
    ObTryWork work = { OB_TRY_WORK_LIMIT, 0 };
    u32 z = OB_OBSERVER;
    if (!span(p, n) || !obt_buffer_span((u32)out) || !_gz_image_write_allowed((u32)out, 64)) return OB_ARGUMENT;
    if (!obt_real_try_lock()) return OB_BUSY;
    if (!obt_healthy()) goto done;
    if (!obt_private((u32)out, &work)) { z = OB_ARGUMENT; goto done; }
    if (obt_owner(p, n, &a, &h, &work)) { z = OB_ABSENT; goto done; }
    if (!a->generation || a->missing != HL_MISSING_ALL || a->reserved || a->release_token ||
        a->state != HL_LIVE || a->kind != h->kind || a->heap != h->heap || (a->flags & ~OB_PROVENANCE)) goto done;
    if (work.exhausted) goto done;
    obt_fill(&b, p, n, a, h);
    copy(out, &b, sizeof(b));
    z = OB_OK;
done:
    if (work.exhausted) z = OB_BUDGET;
    obt_real_unlock();
    return z;
}

OBT_API ObU32 _gz_ob_try_validate(const ObBinding *in) {
    HlStoredAllocation *a;
    HlHeap *h;
    ObBinding b, now;
    ObTryWork work = { OB_TRY_WORK_LIMIT, 0 };
    u32 z = OB_OBSERVER;
    if (!obt_buffer_span((u32)in) || !_gz_image_write_allowed((u32)in, 64)) return OB_ARGUMENT;
    if (!obt_real_try_lock()) return OB_BUSY;
    if (!obt_healthy()) goto done;
    if (!obt_private((u32)in, &work)) { z = OB_ARGUMENT; goto done; }
    copy(&b, in, sizeof(b));
    if (!obt_header(&b)) { z = OB_ARGUMENT; goto done; }
    if (obt_owner(b.query, b.query_bytes, &a, &h, &work)) { z = OB_CHANGED; goto done; }
    if ((a->flags & ~OB_PROVENANCE) || a->reserved || a->release_token || a->missing != HL_MISSING_ALL ||
        a->kind != h->kind || a->heap != h->heap) { z = OB_CHANGED; goto done; }
    obt_fill(&now, b.query, b.query_bytes, a, h);
    z = obt_equal(&b, &now) ? OB_OK : OB_CHANGED;
done:
    if (work.exhausted) z = OB_BUDGET;
    obt_real_unlock();
    return z;
}
#undef OBT_FN
#undef OBT_API
