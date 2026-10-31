#include "generic_process_reconcile.h"
#include <stdint.h>

static u32 equal(const void *a, const void *b, u32 n)
{
    const volatile unsigned char *x = a, *y = b;
    u32 i;
    for (i = 0; i < n; ++i)
        if (x[i] != y[i])
            return 0;
    return 1;
}

static void copy(void *a, const void *b, u32 n)
{
    volatile unsigned char *x = a;
    const volatile unsigned char *y = b;
    u32 i;
    for (i = 0; i < n; ++i)
        x[i] = y[i];
}

static void zero(void *a, u32 n)
{
    volatile unsigned char *x = a;
    u32 i;
    for (i = 0; i < n; ++i)
        x[i] = 0;
}

static u32 zeroed(const void *a, u32 n)
{
    const volatile unsigned char *x = a;
    u32 i;
    for (i = 0; i < n; ++i)
        if (x[i])
            return 0;
    return 1;
}

static u32 span(u32 p, u32 n)
{
    return !(p & 3u) && p >= 0x10000000u && p < 0xc0000000u &&
           n && n <= 0xc0000000u - p;
}

static u32 contains(u32 p, u32 n, u32 q, u32 m)
{
    return n && m && n <= 0xffffffffu - p && m <= 0xffffffffu - q &&
           q >= p && q - p <= n && m <= n - (q - p);
}

/* Host pointers only: arithmetic proves disjoint private buffers. No pointer
 * stored in a PO/HL row or producer is converted to a host/native pointer. */
static u32 apart(const void *a, u32 n, const void *b, u32 m)
{
    uintptr_t p = (uintptr_t)a, q = (uintptr_t)b;
    return n <= UINTPTR_MAX - p && m <= UINTPTR_MAX - q &&
           (p + n <= q || q + m <= p);
}

u32 grc_row_checksum(const GrcProcess *rows, u32 count)
{
    const volatile unsigned char *p = (const volatile unsigned char *)rows;
    u32 i, n, h = 2166136261u;
    if (count > GRC_LIMIT || (count && !rows))
        return 0; /* Invalid-argument return, not a hash sentinel. */
    n = count * (u32)sizeof(*rows);
    for (i = 0; i < n; ++i)
        h = (h ^ p[i]) * 16777619u;
    return h;
}

static u32 heap_format(const HlHeap *h)
{
    return h->generation && h->state == HL_LIVE &&
           h->kind >= HL_EXP && h->kind <= HL_CUSTOM &&
           span(h->heap, 4) && span(h->start, h->bytes) &&
           !(h->flags & ~HL_REBORN) && !h->end_frame && !h->dtor_site;
}

static u32 domain_key(const HlHeap *a, const HlHeap *b)
{
    return a->heap == b->heap && a->generation == b->generation &&
           a->kind == b->kind && a->start == b->start && a->bytes == b->bytes &&
           a->parent == b->parent && a->parent_generation == b->parent_generation &&
           a->epoch == b->epoch && a->born_frame == b->born_frame &&
           a->ctor_site == b->ctor_site && a->vtable == b->vtable;
}

static u32 stamp_format(const GrcCutStamp *s)
{
    const GhcSaveProducer *p = &s->producer;
    const PoHeader *po = &s->processes;
    const HlControl *hl = &s->allocations;
    return s->archive_identity && s->archive_generation &&
           s->image_type == GHCP_IMAGE_VERSION && s->image_version == GHCP_IMAGE_VERSION && s->image_bytes &&
           s->missing == GRC_MISSING_ALL && !s->reserved &&
           !(s->census_flags & ~(GHCP_UNBOUND | GHCP_PO_GAP | GHCP_BUDGET)) &&
           !(s->census_flags & GHCP_PO_GAP) &&
           ghs_provenance_format(p, p->request, p->frame, p->hg, p->lease,
                                 p->original_return, p->thread) &&
           s->scene.epoch == p->epoch && s->scene.hash == p->scene_hash &&
           s->scene.hash == ghcp_scene_hash(&s->scene) && !s->scene.reserved &&
           heap_format(&s->selected_domain) && heap_format(&s->sdk_domain) &&
           s->sdk_domain.kind == HL_EXP && !s->sdk_domain.parent &&
           !s->sdk_domain.parent_generation &&
           contains(s->sdk_domain.start, s->sdk_domain.bytes,
                    s->selected_domain.start, s->selected_domain.bytes) &&
           po->magic == PO_MAGIC && po->bytes == sizeof(*po) && po->version == 1 &&
           po->capacity == PO_CAP && span(po->rows, PO_CAP * (u32)sizeof(PoEntry)) &&
           po->busy == 1 && !po->flags && !po->session && po->active <= PO_CAP &&
           hl->magic == HL_MAGIC && hl->bytes == sizeof(*hl) && hl->version == 1 &&
           hl->busy == 1 && !hl->flags && !hl->session && hl->serial &&
           hl->active_heaps <= HL_HEAPS && hl->active_allocations <= HL_ALLOCS &&
           s->row_count <= GRC_LIMIT && po->active <= s->row_count;
}

static u32 process_format(const GrcCutStamp *s, const GrcProcess *r)
{
    const PoEntry *p = &r->process;
    const HlAllocation *a = &r->allocation;
    const HlHeap *h = &r->owner;
    u32 inside;
    if (r->po_index >= PO_CAP || r->reserved || !ghcp_process_format(p) ||
        r->binding > GRC_BIND_OUTSIDE)
        return 0;
    if (!r->binding)
        return !r->heap_index && zeroed(a, sizeof(*a)) &&
               zeroed(h, sizeof(*h)) && zeroed(&r->scope, sizeof(r->scope));
    if (p->state != PO_LIVE || p->flags || p->end_frame ||
        !r->heap_index || r->heap_index > HL_HEAPS || !heap_format(h) ||
        a->state != HL_LIVE || !a->generation ||
        a->heap != h->heap || a->heap_generation != h->generation ||
        a->kind != h->kind || a->heap_epoch != h->epoch ||
        a->capacity < a->requested ||
        !span(a->pointer, a->capacity) ||
        !contains(a->pointer, a->capacity, p->object, p->allocation_bytes) ||
        a->missing != HL_MISSING_ALL ||
        a->end_frame || a->end_site || a->release_token || a->reserved ||
        !(!(a->flags & ~(HM_BORN_DURING | 256u)) || (a->flags & ~256u) == HM_RESIZED) ||
        r->scope.active_token || r->scope.flags || r->scope.reserved ||
        !contains(h->start, h->bytes, a->pointer, a->capacity))
        return 0;
    inside = contains(s->selected_domain.start, s->selected_domain.bytes,
                      p->object, p->allocation_bytes);
    if (r->binding == GRC_BIND_LIVE)
        return inside && contains(s->sdk_domain.start, s->sdk_domain.bytes,
                                   p->object, p->allocation_bytes);
    return p->object <= 0xffffffffu - p->allocation_bytes &&
           s->selected_domain.start <= 0xffffffffu - s->selected_domain.bytes &&
           (p->object + p->allocation_bytes <= s->selected_domain.start ||
            s->selected_domain.start + s->selected_domain.bytes <= p->object);
}

static u32 cut_format(const GrcCutView *v)
{
    const GhcProcessDirectory *d;
    u32 i, live = 0, ending = 0, ended = 0, bound = 0, outside = 0, unbound = 0;
    if (!v || !v->stamp || !v->directory || !stamp_format(v->stamp) ||
        v->rows_bytes != v->stamp->row_count * (u32)sizeof(GrcProcess) ||
        (v->rows_bytes && !v->rows))
        return 0;
    d = v->directory;
    if (d->magic != GHCP_MAGIC || d->bytes != sizeof(*d) + v->rows_bytes || d->version != GHCP_VERSION ||
        d->row_bytes != sizeof(GrcProcess) || d->count != v->stamp->row_count ||
        d->flags != v->stamp->census_flags || d->reserved || d->work_used > GHCP_WORK ||
        d->row_checksum != v->stamp->row_checksum ||
        !equal(&d->po_before, &d->po_after, sizeof(PoHeader)) ||
        !equal(&d->hl_before, &d->hl_after, sizeof(HlControl)) ||
        !equal(&d->po_before, &v->stamp->processes, sizeof(PoHeader)) ||
        !equal(&d->hl_before, &v->stamp->allocations, sizeof(HlControl)) ||
        !equal(&d->scene, &v->stamp->scene, sizeof(GhcSaveScene)) ||
        v->directory_offset > v->stamp->image_bytes ||
        sizeof(*d) > v->stamp->image_bytes - v->directory_offset ||
        d->row_offset != sizeof(*d) ||
        d->bytes > v->stamp->image_bytes - v->directory_offset ||
        (uintptr_t)v->directory > UINTPTR_MAX - sizeof(*d) ||
        (uintptr_t)v->rows != (uintptr_t)v->directory + sizeof(*d))
        return 0;
    for (i = 0; i < v->stamp->row_count; ++i) {
        if (!process_format(v->stamp, v->rows + i) ||
            (i && v->rows[i - 1].po_index >= v->rows[i].po_index))
            return 0;
        live += v->rows[i].process.state == PO_LIVE;
        ending += v->rows[i].process.state == PO_ENDING;
        ended += v->rows[i].process.state == PO_ENDED;
        bound += v->rows[i].binding == GRC_BIND_LIVE;
        outside += v->rows[i].binding == GRC_BIND_OUTSIDE;
        unbound += v->rows[i].binding == GRC_BIND_NONE;
    }
    if (d->live != live || d->ending != ending || d->ended != ended ||
        d->bound != bound || d->outside != outside || d->unbound != unbound ||
        ((d->flags & GHCP_UNBOUND) != 0) != (unbound != 0))
        return 0;
    return grc_row_checksum(v->rows, v->stamp->row_count) == v->stamp->row_checksum;
}

static u32 cmp(u32 a, u32 b)
{
    return a < b ? 0 : a > b ? 2 : 1;
}

u32 grc_validate_cut(const GrcCutView *view)
{
    return cut_format(view) ? GRC_OK : GRC_FORMAT;
}

static u32 compare(const GrcProcess *r, u32 a, u32 b, u32 by_address)
{
    const PoEntry *x = &r[a].process, *y = &r[b].process;
    u32 z;
    if (by_address)
        return cmp(x->object, y->object);
    z = cmp(x->pid, y->pid);
    return z == 1 ? cmp(x->generation, y->generation) : z;
}

static void sift(const GrcProcess *r, u32 *index, u32 root, u32 count, u32 mode)
{
    u32 child, t;
    while (root < count / 2) {
        child = root * 2 + 1;
        if (child + 1 < count && compare(r, index[child], index[child + 1], mode) == 0)
            ++child;
        if (compare(r, index[root], index[child], mode) != 0)
            return;
        t = index[root]; index[root] = index[child]; index[child] = t;
        root = child;
    }
}

static void sort(const GrcProcess *r, u32 *index, u32 count, u32 mode)
{
    u32 i, t;
    for (i = 0; i < count; ++i)
        index[i] = i;
    for (i = count / 2; i; --i)
        sift(r, index, i - 1, count, mode);
    for (i = count; i > 1; --i) {
        t = index[0]; index[0] = index[i - 1]; index[i - 1] = t;
        sift(r, index, 0, i - 1, mode);
    }
}

static u32 same_instance(const GrcProcess *a, const GrcProcess *b)
{
    return a->process.pid == b->process.pid &&
           a->process.generation == b->process.generation;
}

static u32 bound_live(const GrcProcess *p)
{
    return p->process.state == PO_LIVE && p->binding == GRC_BIND_LIVE &&
           !p->process.flags;
}

static u32 known_live(const GrcProcess *p)
{
    return p->process.state == PO_LIVE && p->binding != GRC_BIND_NONE &&
           !p->process.flags;
}

static u32 unresolved(const GrcProcess *p)
{
    return p->process.state == PO_ENDING ||
           (p->process.state == PO_LIVE &&
            (p->binding == GRC_BIND_NONE || p->process.flags));
}

static u32 exact_instance(const GrcProcess *a, const GrcProcess *b)
{
    return same_instance(a, b) &&
           equal(&a->process, &b->process, sizeof(a->process)) &&
           equal(&a->allocation, &b->allocation, sizeof(a->allocation)) &&
           equal(&a->owner, &b->owner, sizeof(a->owner)) &&
           equal(&a->scope, &b->scope, sizeof(a->scope));
}

static void emit(GrcPlan *p, GrcPlanRow *rows, u32 s, u32 c, u32 kind)
{
    GrcPlanRow *r = rows + p->count++;
    r->saved_index = s; r->current_index = c;
    r->classification = kind; r->reserved = 0;
    if (kind == GRC_MATCHING) ++p->matching;
    if (kind == GRC_CURRENT_ONLY) ++p->current_only;
    if (kind == GRC_SAVED_ONLY) ++p->saved_only;
    if (kind == GRC_REUSED_ADDRESS) ++p->reused;
    if (kind == GRC_TOMBSTONE) ++p->tombstones;
    if (kind == GRC_OUTSIDE_DOMAIN) ++p->outside;
    if (kind == GRC_NOT_RESOLVED) ++p->unresolved;
}

static void one(GrcPlan *p, GrcPlanRow *rows, const GrcProcess *r, u32 i, u32 current)
{
    u32 k;
    if (r->process.state == PO_ENDED)
        k = GRC_TOMBSTONE;
    else if (r->process.state == PO_LIVE && r->binding == GRC_BIND_OUTSIDE)
        k = GRC_OUTSIDE_DOMAIN;
    else if (bound_live(r))
        k = current ? GRC_CURRENT_ONLY : GRC_SAVED_ONLY;
    else
        k = GRC_NOT_RESOLVED;
    emit(p, rows, current ? GRC_NO_ROW : i, current ? i : GRC_NO_ROW, k);
}

u32 grc_plan_diagnostic(const GrcCutView *saved, const GrcCutView *current,
             const GrcCutStamp *expected_current, u32 *work, u32 work_words,
             GrcPlan *plan, GrcPlanRow *rows, u32 row_capacity)
{
    u32 n, m, i, j, a, b, z, need, work_bytes, out_bytes;
    u32 *s, *c;
    if (!saved || !current || !saved->stamp || !current->stamp ||
        !expected_current || !plan || !rows || !work)
        return GRC_ARGUMENT;
    n = saved->stamp->row_count; m = current->stamp->row_count;
    if (n > GRC_LIMIT || m > GRC_LIMIT)
        return GRC_CAPACITY;
    need = n + m;
    if (work_words < need || row_capacity < need)
        return GRC_CAPACITY;
    work_bytes = need * (u32)sizeof(u32);
    out_bytes = need * (u32)sizeof(GrcPlanRow);
    /* Do not invalidate or overwrite an aliased input while refusing it. */
    if (!apart(plan, sizeof(*plan), rows, out_bytes) ||
        !apart(plan, sizeof(*plan), work, work_bytes) ||
        !apart(rows, out_bytes, work, work_bytes))
        return GRC_ARGUMENT;
    {
        const GrcCutView *views[2];
        u32 v;
        views[0] = saved; views[1] = current;
        for (v = 0; v < 2; ++v)
            if (!apart(plan, sizeof(*plan), views[v]->stamp, sizeof(GrcCutStamp)) ||
                !apart(rows, out_bytes, views[v]->stamp, sizeof(GrcCutStamp)) ||
                !apart(work, work_bytes, views[v]->stamp, sizeof(GrcCutStamp)) ||
                !apart(plan, sizeof(*plan), views[v], sizeof(GrcCutView)) ||
                !apart(rows, out_bytes, views[v], sizeof(GrcCutView)) ||
                !apart(work, work_bytes, views[v], sizeof(GrcCutView)) ||
                !apart(plan, sizeof(*plan), views[v]->directory, sizeof(GhcProcessDirectory)) ||
                !apart(rows, out_bytes, views[v]->directory, sizeof(GhcProcessDirectory)) ||
                !apart(work, work_bytes, views[v]->directory, sizeof(GhcProcessDirectory)) ||
                !apart(plan, sizeof(*plan), views[v]->rows, views[v]->rows_bytes) ||
                !apart(rows, out_bytes, views[v]->rows, views[v]->rows_bytes) ||
                !apart(work, work_bytes, views[v]->rows, views[v]->rows_bytes))
                return GRC_ARGUMENT;
    }
    if (!apart(plan, sizeof(*plan), expected_current, sizeof(*expected_current)) ||
        !apart(rows, out_bytes, expected_current, sizeof(*expected_current)) ||
        !apart(work, work_bytes, expected_current, sizeof(*expected_current)))
        return GRC_ARGUMENT;
    zero(plan, sizeof(*plan));
    plan->magic = GRC_MAGIC; plan->bytes = sizeof(*plan); plan->version = GRC_VERSION;
    plan->missing = GRC_MISSING_ALL;
    if (!cut_format(saved) || !cut_format(current))
        return plan->status = GRC_FORMAT;
    if (!equal(current->stamp, expected_current, sizeof(*expected_current)))
        return plan->status = GRC_STALE;
    if (!equal(&saved->stamp->scene, &current->stamp->scene, sizeof(GhcSaveScene)) ||
        current->stamp->producer.frame < saved->stamp->producer.frame ||
        !domain_key(&saved->stamp->selected_domain, &current->stamp->selected_domain) ||
        !domain_key(&saved->stamp->sdk_domain, &current->stamp->sdk_domain) ||
        saved->stamp->processes.rows != current->stamp->processes.rows)
        return plan->status = GRC_DOMAIN_CHANGED;
    s = work; c = work + n;
    sort(saved->rows, s, n, 0); sort(current->rows, c, m, 0);
    for (i = 1; i < n; ++i)
        if (same_instance(saved->rows + s[i - 1], saved->rows + s[i]))
            return plan->status = GRC_AMBIGUOUS;
    for (i = 1; i < m; ++i)
        if (same_instance(current->rows + c[i - 1], current->rows + c[i]))
            return plan->status = GRC_AMBIGUOUS;
    i = 0; j = 0;
    while (i < n && j < m) {
        const GrcProcess *x = saved->rows + s[i], *y = current->rows + c[j];
        z = cmp(x->process.pid, y->process.pid);
        if (z == 1) z = cmp(x->process.generation, y->process.generation);
        if (z == 0) ++i;
        else if (z == 2) ++j;
        else {
            /* Same observed instance cannot silently move or rebind. */
            if (x->process.object != y->process.object ||
                (known_live(x) && known_live(y) && !exact_instance(x, y)))
                return plan->status = GRC_IDENTITY_CONFLICT;
            ++i; ++j;
        }
    }
    sort(saved->rows, s, n, 1); sort(current->rows, c, m, 1);
    for (i = 1; i < n; ++i)
        if (saved->rows[s[i - 1]].process.object == saved->rows[s[i]].process.object)
            return plan->status = GRC_AMBIGUOUS;
    for (i = 1; i < m; ++i)
        if (current->rows[c[i - 1]].process.object == current->rows[c[i]].process.object)
            return plan->status = GRC_AMBIGUOUS;
    copy(&plan->saved, saved->stamp, sizeof(plan->saved));
    copy(&plan->current, current->stamp, sizeof(plan->current));
    copy(&plan->saved_directory, saved->directory, sizeof(plan->saved_directory));
    copy(&plan->current_directory, current->directory, sizeof(plan->current_directory));
    i = 0; j = 0;
    while (i < n || j < m) {
        a = i < n ? s[i] : GRC_NO_ROW;
        b = j < m ? c[j] : GRC_NO_ROW;
        z = a == GRC_NO_ROW ? 2 : b == GRC_NO_ROW ? 0 :
            cmp(saved->rows[a].process.object, current->rows[b].process.object);
        if (z == 0) {
            one(plan, rows, saved->rows + a, a, 0); ++i;
        } else if (z == 2) {
            one(plan, rows, current->rows + b, b, 1); ++j;
        } else {
            const GrcProcess *x = saved->rows + a, *y = current->rows + b;
            if (unresolved(x) || unresolved(y))
                emit(plan, rows, a, b, GRC_NOT_RESOLVED);
            else if (bound_live(x) && bound_live(y))
                emit(plan, rows, a, b,
                     exact_instance(x, y) ? GRC_MATCHING : GRC_REUSED_ADDRESS);
            else {
                one(plan, rows, x, a, 0);
                one(plan, rows, y, b, 1);
            }
            ++i; ++j;
        }
    }
    /* The caller's private immutable input contract is checked again. No
     * scope/hold is minted from these comparisons or the diagnostic flag. */
    if (!equal(current->stamp, expected_current, sizeof(*expected_current)) ||
        !equal(&plan->saved, saved->stamp, sizeof(plan->saved)) ||
        !equal(&plan->saved_directory, saved->directory, sizeof(plan->saved_directory)) ||
        !equal(&plan->current_directory, current->directory, sizeof(plan->current_directory)) ||
        grc_row_checksum(saved->rows, n) != saved->stamp->row_checksum ||
        grc_row_checksum(current->rows, m) != current->stamp->row_checksum)
        return plan->status = GRC_STALE;
    plan->status = plan->unresolved ||
                   ((plan->saved.census_flags | plan->current.census_flags) & GHCP_BUDGET)
                   ? GRC_UNRESOLVED : GRC_OK;
    plan->valid = 1; /* Metadata result only, permanently missing255. */
    return plan->status;
}

/* These checks classify already-owned metadata; they acquire no native hold.
 * Real OIS/purpose/issuer authentication remains the caller's responsibility.
 * Keep the public default strict; legacy images need the named diagnostic API.
 */
static u32 restore_roles(const GrcCutStamp *saved, const GrcCutStamp *current)
{
    const GhcSaveProducer *s = &saved->producer, *c = &current->producer;
    return s->version == 2 && c->version == 2 &&
           s->reserved == GHS_COMMAND_SAVE && c->reserved == GHS_COMMAND_RESTORE &&
           s->slot == c->slot && c->request > s->request &&
           s->save_return != c->save_return &&
           s->original_return == c->original_return &&
           s->service_return == c->service_return && s->thread == c->thread &&
           (saved->archive_identity != current->archive_identity ||
            saved->archive_generation != current->archive_generation);
}

u32 grc_plan(const GrcCutView *saved, const GrcCutView *current,
             const GrcCutStamp *expected_current, u32 *work, u32 work_words,
             GrcPlan *plan, GrcPlanRow *rows, u32 row_capacity)
{
    u32 z;
    if (!saved || !current || !saved->stamp || !current->stamp)
        return GRC_ARGUMENT;
    if (!restore_roles(saved->stamp, current->stamp))
        return GRC_CAPTURE_ROLE;
    z = grc_plan_diagnostic(saved, current, expected_current, work, work_words,
                            plan, rows, row_capacity);
    if ((z == GRC_OK || z == GRC_UNRESOLVED) &&
        !restore_roles(&plan->saved, &plan->current)) {
        plan->valid = 0;
        plan->status = GRC_CAPTURE_ROLE;
        return GRC_CAPTURE_ROLE;
    }
    return z;
}
