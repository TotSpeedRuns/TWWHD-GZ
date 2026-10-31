/* Genuine native-read proposal; no native target writes and no apply path.
 * Selection453/454 is the private two-class experiment, not a serializer map.
 * Full process allocations and complete CPU model-heap spans are copied.
 */
#include "generic_capture.h"
#include "process.h"
#include "lifetime.h"
#include "heap_owner.h"

typedef GacU U;
typedef unsigned char B;
typedef struct {
    U magic, bytes, version, valid, diagnostic_only, missing, frame, epoch;
    U thread, issuer, root_serial, heap_gate, process_count, rows, data, used;
    U checksum, second_pass_equal, targets[2], po_serial, hl_serial, hl_release;
    U jpg_missing, root_binding_hash, reserved[7];
} GacHeader;
typedef struct {
    U heap_manager, actors, creates, deletes, layer, priority, link, stage[2];
    U room, layer_number, loading, event, frame, epoch, reserved[17];
} GacRoots;
typedef struct {
    PoEntry process;
    HlAllocation allocation;
    HlHeap owner, model;
    U actor_offset, actor_bytes, model_offset, model_bytes, actor_heap, checksum;
    U reserved[2];
} GacRow;
typedef char GacHeader128[sizeof(GacHeader) == 128 ? 1 : -1];
typedef char GacRoots128[sizeof(GacRoots) == 128 ? 1 : -1];
typedef char GacRow304[sizeof(GacRow) == 304 ? 1 : -1];

extern volatile U _gz_pbac_frame, _gz_pbac_epoch, _gz_pbac_menu;
extern volatile U _gz_pbac_pending, _gz_pbac_tools, _gz_pbac_world_resume_pending;
extern volatile B _gz_pbac_loading;
extern volatile U _gz_roh_live_actor_head, _gz_roh_live_create_head;
extern volatile U _gz_roh_live_delete_head, _gz_rll_current_layer;
extern volatile U _gz_rll_priority_head, _gz_roh_live_link, _gz_roh_live_stage[2];
extern volatile B _gz_roh_live_room, _gz_roh_live_layer, _gz_roh_live_event;
extern const B _gz_gac_heap_manager_global[], _gz_gac_native_frame_return[];
extern const B _gz_frame_done[];
volatile U _gz_gac_callback_latch, _gz_gac_callback_original_return;
volatile U _gz_gac_callback_service_return;
extern U _gz_hg_platform_thread(void), _gz_roh_user_guard(void);
extern U _gz_pbac_idle_for_capture(void), _gz_osa_pin_id(void);
extern U _gz_boh_has_owner(void), _gz_wgp_idle(void), _gz_wgp_unreleased(void);
extern U _gz_ois_capture_cpu_diagnostic(void), _gz_ois_cpu_stage_match(U, U);
extern U _gz_image_write_allowed(U, U);
extern volatile U _gz_gac_root_report[16], _gz_jpg_report[96];

volatile U _gz_gac_request, _gz_gac_ack;
volatile U _gz_gac_report[32] = { 0x47414331u, 128, 1 };
static U gac_entered, gac_frame, gac_thread, gac_issuer, gac_phase;
static U gac_output, gac_capacity;

static U ram(U p, U n)
{
    return !(p & 3u) && p >= 0x10000000u && p < 0xbffe0000u &&
           n && n <= 0xbffe0000u - p;
}

static void copy(void *d, const volatile void *s, U n)
{
    U i;
    for (i = 0; i < n; ++i)
        ((B *)d)[i] = ((const volatile B *)s)[i];
}

static void zero(void *d, U n)
{
    U i;
    for (i = 0; i < n; ++i)
        ((B *)d)[i] = 0;
}

static U same(const void *a, const volatile void *b, U n)
{
    U i;
    for (i = 0; i < n; ++i)
        if (((const B *)a)[i] != ((const volatile B *)b)[i])
            return 0;
    return 1;
}

static U hash(const volatile void *p, U n)
{
    U i, h = 2166136261u;
    for (i = 0; i < n; ++i)
        h = (h ^ ((const volatile B *)p)[i]) * 16777619u;
    return h;
}

static U image_hash(const volatile void *p, U n)
{
    U i, h = 2166136261u, value;
    for (i = 0; i < n; ++i) {
        value = ((const volatile B *)p)[i];
        if (i >= 12 && i < 16)
            value = i == 15;
        if (i >= 64 && i < 68)
            value = 0;
        h = (h ^ value) * 16777619u;
    }
    return h;
}

/* Pure arithmetic only: not a native mapping/owner/exclusion permit. */
U _gz_gac_plan_span(U at, U bytes, U capacity, U *end)
{
    if (!bytes || (bytes & 3u) || at > capacity || bytes > capacity - at)
        return GAC_CAPACITY;
    *end = at + bytes;
    return 0;
}

static U quiet(void)
{
    return _gz_pbac_menu == 1 && !(_gz_pbac_tools & ~8u) &&
           !_gz_pbac_pending && !_gz_pbac_loading && !_gz_roh_live_event &&
           !_gz_pbac_world_resume_pending && !_gz_roh_user_guard() &&
           !_gz_osa_pin_id() && !_gz_boh_has_owner() &&
           _gz_pbac_idle_for_capture() && _gz_wgp_idle() && !_gz_wgp_unreleased() &&
           !_gz_roh_live_create_head && !_gz_roh_live_delete_head &&
           !_gz_rll_priority_head;
}

static U callback(void)
{
    return _gz_gac_callback_latch == 1 &&
           _gz_gac_callback_original_return == (U)_gz_gac_native_frame_return &&
           _gz_gac_callback_service_return == (U)_gz_frame_done + 8u;
}

U _gz_gac_kernel_identity(U frame)
{
    return gac_entered == 1 && gac_phase == 2 && gac_issuer && callback() &&
           frame && frame == gac_frame && frame == _gz_pbac_frame && gac_thread;
}

U _gz_gac_kernel_scope(U frame)
{
    return _gz_gac_kernel_identity(frame) &&
           gac_thread == _gz_hg_platform_thread() && quiet();
}

U _gz_gac_storage_scope(void)
{
    return gac_entered == 1 && (gac_phase == 1 || gac_phase == 2) &&
           gac_issuer && callback() && gac_frame == _gz_pbac_frame &&
           gac_thread && gac_thread == _gz_hg_platform_thread() && quiet();
}

U _gz_gac_cleanup_safe(void)
{
    return _gz_gac_root_report[14] == 0;
}

static void roots(GacRoots *r)
{
    zero(r, sizeof(*r));
    r->heap_manager = *(const volatile U *)_gz_gac_heap_manager_global;
    r->actors = _gz_roh_live_actor_head;
    r->creates = _gz_roh_live_create_head;
    r->deletes = _gz_roh_live_delete_head;
    r->layer = _gz_rll_current_layer;
    r->priority = _gz_rll_priority_head;
    r->link = _gz_roh_live_link;
    r->stage[0] = _gz_roh_live_stage[0];
    r->stage[1] = _gz_roh_live_stage[1];
    r->room = _gz_roh_live_room;
    r->layer_number = _gz_roh_live_layer;
    r->loading = _gz_pbac_loading;
    r->event = _gz_roh_live_event;
    r->frame = _gz_pbac_frame;
    r->epoch = _gz_pbac_epoch;
}

/* A current native process must also be on the real actor queue. Node reads
 * require current HL allocation ownership first; stale PO rows are not enough.
 */
static U in_queue(U target)
{
    U node = _gz_roh_live_actor_head, count = 0, found = 0;
    while (node) {
        U next, object;
        if (++count > 4096 || !ram(node, 20) || _gz_hl_owner(node, 20) ||
            _gz_hl_observation.valid != 1 || _gz_hl_observation.flags ||
            _gz_hl_observation.allocation.state != HL_LIVE)
            return 0;
        next = *(const volatile U *)(node + 8);
        object = *(const volatile U *)(node + 12);
        if (object == target)
            ++found;
        node = next;
    }
    return found == 1;
}

static U heap(U address, HlHeap *out)
{
    U i, z = GAC_HEAP;
    if (!_gz_hl_meta_try_lock())
        return GAC_STATE;
    if (_gz_hl_control.flags || _gz_hm_control.flags || _gz_hm_control.active ||
        _gz_hm_control.failed)
        goto done;
    for (i = 0; i < HL_HEAPS; ++i) {
        HlHeap *h = _gz_hl_heaps + i;
        if (h->heap != address)
            continue;
        if (h->state != HL_LIVE || h->kind != HL_SOLID ||
            !h->generation || !_gz_hm_scope_live(h) || !ram(h->start, h->bytes) ||
            !ram(h->heap, 0x9c) || h->direction != 1 || h->heap != h->start)
            goto done;
        copy(out, h, sizeof(*out));
        z = 0;
        break;
    }
done:
    _gz_hl_meta_unlock();
    return z;
}

static U identify(U profile, GacRow *row)
{
    U i, p = 0, a, front, back, begin, end;
    for (i = 0; i < PO_CAP; ++i) {
        const volatile PoEntry *r = _gz_po_rows + i;
        if (r->state == PO_LIVE && r->profile == profile) {
            p = r->object;
            break;
        }
    }
    if (!p || _gz_po_lookup(p) || !_gz_po_observation.valid ||
        _gz_po_observation.flags || _gz_po_observation.entry.state != PO_LIVE ||
        _gz_po_observation.entry.profile != profile || !in_queue(p))
        return GAC_TARGET;
    copy(&row->process, &_gz_po_observation.entry, sizeof(row->process));
    if (row->process.object != p || row->process.allocation_bytes < 0x3ac ||
        !ram(p, row->process.allocation_bytes) ||
        _gz_hl_owner(p, row->process.allocation_bytes) ||
        !_gz_hl_observation.valid || _gz_hl_observation.flags)
        return GAC_ALLOCATION;
    copy(&row->allocation, &_gz_hl_observation.allocation, sizeof(row->allocation));
    copy(&row->owner, &_gz_hl_observation.heap, sizeof(row->owner));
    if (row->allocation.state != HL_LIVE || row->allocation.pointer != p ||
        row->allocation.requested < row->process.allocation_bytes ||
        row->allocation.capacity < row->allocation.requested ||
        !ram(p, row->allocation.capacity) || row->owner.state != HL_LIVE)
        return GAC_ALLOCATION;
    a = *(const volatile U *)(p + 0xf4);
    if (heap(a, &row->model))
        return GAC_HEAP;
    if (*(const volatile U *)(a + 0xc) != row->model.vtable ||
        *(const volatile U *)(a + 0x1c) != row->model.start ||
        *(const volatile U *)(a + 0x20) != row->model.bytes ||
        *(const volatile U *)(a + 0x24) != row->model.parent ||
        *(const volatile U *)(a + 0x30) ||
        *(const volatile U *)(a + 0x50) != 1 ||
        (*(const volatile U *)(a + 0x90) & 2u))
        return GAC_HEAP;
    begin = row->model.start + 0x9c;
    end = row->model.start + row->model.bytes;
    front = *(const volatile U *)(a + 0x94);
    back = *(const volatile U *)(a + 0x98);
    if (begin > front || front > back || back > end)
        return GAC_HEAP;
    if (front > begin && _gz_gh_solid(a, begin, front - begin))
        return GAC_HEAP;
    if (back < end && _gz_gh_solid(a, back, end - back))
        return GAC_HEAP;
    row->actor_heap = a;
    row->actor_bytes = row->allocation.capacity;
    row->model_bytes = row->model.bytes;
    return 0;
}

static U row_same(const GacRow *old)
{
    GacRow now;
    zero(&now, sizeof(now));
    if (identify(old->process.profile, &now))
        return 0;
    return same(&old->process, &now.process, sizeof(now.process)) &&
           same(&old->allocation, &now.allocation, sizeof(now.allocation)) &&
           same(&old->owner, &now.owner, sizeof(now.owner)) &&
           same(&old->model, &now.model, sizeof(now.model)) &&
           old->actor_heap == now.actor_heap;
}

U _gz_gac_read_under_root(U frame)
{
    GacHeader *h = (GacHeader *)gac_output;
    GacRoots *r = (GacRoots *)(gac_output + sizeof(*h)), after;
    GacRow *rows = (GacRow *)((B *)r + sizeof(*r));
    PoHeader po;
    U i, at = (sizeof(*h) + sizeof(*r) + GAC_TARGETS * sizeof(*rows) + 31u) & ~31u;
    static const U targets[GAC_TARGETS] = { 453, 454 };
    if (!_gz_gac_kernel_scope(frame) || !_gz_gac_root_read_scope(frame) ||
        !_gz_ois_cpu_stage_match(gac_output, gac_capacity) ||
        !_gz_image_write_allowed(gac_output, gac_capacity) || at > gac_capacity)
        return GAC_CONTEXT;
    zero(h, at);
    roots(r);
    copy(&po, &_gz_po_header, sizeof(po));
    if (po.flags || po.busy || po.capacity != PO_CAP)
        return GAC_PROCESS;
    for (i = 0; i < GAC_TARGETS; ++i) {
        U z = identify(targets[i], rows + i), end;
        if (z)
            return z;
        rows[i].actor_offset = at;
        z = _gz_gac_plan_span(at, rows[i].actor_bytes, gac_capacity, &end);
        if (z)
            return z;
        at = end;
        rows[i].model_offset = at;
        z = _gz_gac_plan_span(at, rows[i].model_bytes, gac_capacity, &end);
        if (z)
            return z;
        at = end;
    }
    /* Native payload reads, never actor-field writes. A successful second
     * comparison is an observation and does not clear unknown writer bits. */
    for (i = 0; i < GAC_TARGETS; ++i) {
        GacRow *row = rows + i;
        copy((void *)(gac_output + row->actor_offset), (const void *)row->allocation.pointer, row->actor_bytes);
        copy((void *)(gac_output + row->model_offset), (const void *)row->model.start, row->model_bytes);
        row->checksum = hash((const void *)(gac_output + row->actor_offset), row->actor_bytes + row->model_bytes);
    }
    for (i = 0; i < GAC_TARGETS; ++i) {
        GacRow *row = rows + i;
        if (!row_same(row) ||
            !same((const void *)(gac_output + row->actor_offset), (const void *)row->allocation.pointer, row->actor_bytes) ||
            !same((const void *)(gac_output + row->model_offset), (const void *)row->model.start, row->model_bytes))
            return GAC_CHANGED;
    }
    roots(&after);
    if (!same(r, &after, sizeof(after)) || !same(&po, &_gz_po_header, sizeof(po)) ||
        !_gz_gac_root_read_scope(frame))
        return GAC_CHANGED;
    h->magic = 0x47434431u;
    h->bytes = at;
    h->version = 1;
    h->diagnostic_only = GAC_DIAGNOSTIC_ONLY;
    h->missing = GAC_MISSING_ALL;
    h->frame = frame;
    h->epoch = _gz_pbac_epoch;
    h->thread = gac_thread;
    h->issuer = gac_issuer;
    h->root_serial = _gz_gac_root_report[4];
    h->heap_gate = _gz_gac_root_report[12];
    h->process_count = GAC_TARGETS;
    h->rows = sizeof(*h) + sizeof(*r);
    h->data = (sizeof(*h) + sizeof(*r) + GAC_TARGETS * sizeof(*rows) + 31u) & ~31u;
    h->used = at - h->data;
    h->second_pass_equal = 1;
    h->targets[0] = targets[0];
    h->targets[1] = targets[1];
    h->po_serial = po.serial;
    h->hl_serial = _gz_hl_control.serial;
    h->hl_release = _gz_hl_control.release_serial;
    h->jpg_missing = _gz_jpg_report[52];
    h->root_binding_hash = _gz_gac_root_report[13];
    /* The complete diagnostic header and payload are hashed, with only its
     * own checksum/valid publication normalized. OIS separately binds the
     * completed image to its real issued allocation identity/generation. */
    h->checksum = image_hash(h, at);
    return 0;
}

U _gz_gac_validate(const void *image, U bytes)
{
    const GacHeader *h = image;
    const GacRow *rows;
    U i, at;
    if (!ram((U)image, bytes) || bytes < sizeof(*h) || h->magic != 0x47434431u ||
        h->bytes != bytes || h->version != 1 || h->valid != 1 ||
        h->diagnostic_only != 1 || h->missing != GAC_MISSING_ALL ||
        !h->frame || !h->epoch || !h->thread || !h->issuer || !h->root_serial ||
        !h->heap_gate || h->process_count != GAC_TARGETS || h->second_pass_equal != 1 ||
        h->targets[0] != 453 || h->targets[1] != 454 ||
        h->rows != sizeof(*h) + sizeof(GacRoots) ||
        h->data != ((h->rows + GAC_TARGETS * sizeof(GacRow) + 31u) & ~31u) ||
        h->data > bytes || h->used != bytes - h->data ||
        h->checksum != image_hash(image, bytes))
        return GAC_FORMAT;
    for (i = 0; i < 7; ++i)
        if (h->reserved[i])
            return GAC_FORMAT;
    rows = (const GacRow *)((const B *)image + h->rows);
    at = h->data;
    for (i = 0; i < GAC_TARGETS; ++i) {
        U end;
        const GacRow *r = rows + i;
        if (r->process.profile != h->targets[i] || r->actor_offset != at ||
            r->actor_bytes != r->allocation.capacity ||
            r->model_bytes != r->model.bytes || r->model.kind != HL_SOLID ||
            r->reserved[0] || r->reserved[1] ||
            _gz_gac_plan_span(at, r->actor_bytes, bytes, &end))
            return GAC_FORMAT;
        at = end;
        if (r->model_offset != at || _gz_gac_plan_span(at, r->model_bytes, bytes, &end))
            return GAC_FORMAT;
        at = end;
        if (r->checksum != hash((const B *)image + r->actor_offset, r->actor_bytes + r->model_bytes))
            return GAC_FORMAT;
    }
    return at == bytes ? 0 : GAC_FORMAT;
}

U _gz_gac_capture(U destination, U capacity, U *written)
{
    U z;
    if (!_gz_gac_storage_scope() || capacity != GAC_CAPACITY_BYTES ||
        !_gz_ois_cpu_stage_match(destination, capacity) ||
        !_gz_image_write_allowed(destination, capacity) ||
        !_gz_image_write_allowed((U)written, 4))
        return GAC_STORAGE;
    *written = 0;
    gac_output = destination;
    gac_capacity = capacity;
    gac_phase = 2;
    z = _gz_gac_root_access();
    if (!z) {
        /* Publication follows the successful native HG/JPG cleanup. A
         * cleanup failure can never leave valid evidence for OIS to adopt. */
        __asm__ volatile("sync" ::: "memory");
        ((GacHeader *)destination)->valid = 1;
        z = _gz_gac_validate((const void *)destination, ((GacHeader *)destination)->bytes);
    }
    if (z)
        ((GacHeader *)destination)->valid = 0;
    if (!z)
        *written = ((GacHeader *)destination)->bytes;
    if (_gz_gac_cleanup_safe()) {
        gac_output = gac_capacity = 0;
        gac_phase = 1;
    }
    return z;
}

void _gz_gac_tick(void)
{
    U z = GAC_CONTEXT;
    if (!_gz_gac_request || _gz_gac_ack || gac_entered)
        return;
    /* One request in a fresh private process. Failure is retained; no host
     * rearm/reset or retry can relabel a different native callback. */
    _gz_gac_ack = 1;
    _gz_gac_report[3] = GAC_CONTEXT;
    if (_gz_gac_request != 1 || !callback() || !quiet() || !_gz_pbac_frame)
        goto done;
    gac_entered = gac_phase = 1;
    gac_frame = _gz_pbac_frame;
    gac_thread = _gz_hg_platform_thread();
    gac_issuer = 1;
    _gz_gac_report[4] = gac_frame;
    _gz_gac_report[5] = gac_thread;
    if (!gac_thread)
        goto done;
    z = _gz_ois_capture_cpu_diagnostic();
done:
    _gz_gac_report[3] = z;
    _gz_gac_report[6] = GAC_MISSING_ALL;
    _gz_gac_report[7] = 1; /* observational evidence only */
    if (_gz_gac_cleanup_safe())
        gac_entered = gac_phase = gac_frame = gac_thread = gac_issuer = 0;
}

#include "owned_heap_capture.inc"

#include "owned_heap_save.inc"
