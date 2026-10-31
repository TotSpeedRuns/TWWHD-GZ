#include "generic_process_native_compare.h"
#include "generic_process_cut_native.h"
#include "owned_heap_dual_cut.h"
#include "owned_heap_native.h"
#include <stdint.h>

typedef unsigned int U;
typedef unsigned char B;
typedef struct {
    U active, request, frame, seen_request, seen_frame, last_result;
    GhcSaveProducer before;
    OisCutPair pair;
    OisCutWork work;
    U has_pair, has_work;
} GpnNativeControl;
typedef char GpnNativeControl240[sizeof(GpnNativeControl) == 240 ? 1 : -1];

GhcSaveProducer _gz_gpn_completed_issuer
    __attribute__((section(".bss.gz_gpn_issuer")));
static GpnNativeControl gpn_native
    __attribute__((section(".bss.gz_gpn_native")));
volatile U _gz_gpn_native_report[32]
    __attribute__((section(".data.gz_gpn_native"))) = {0x474e5031u, 128, 1};

static void bytes_copy(void *destination, const void *source, U bytes)
{
    U i;
    volatile B *out = destination;
    const volatile B *in = source;
    for (i = 0; i < bytes; ++i) out[i] = in[i];
}

static U bytes_equal(const void *left, const void *right, U bytes)
{
    U i;
    const volatile B *a = left, *b = right;
    for (i = 0; i < bytes; ++i) if (a[i] != b[i]) return 0;
    return 1;
}

static U apart(U a, U bytes_a, U b, U bytes_b)
{
    return bytes_a <= 0xffffffffu - a && bytes_b <= 0xffffffffu - b &&
        (a + bytes_a <= b || b + bytes_b <= a);
}

U _gz_gpn_native_private_apart(U pointer, U bytes)
{
    return apart(pointer, bytes, (U)&gpn_native, sizeof(gpn_native)) &&
        apart(pointer, bytes, (U)&_gz_gpn_completed_issuer, sizeof(_gz_gpn_completed_issuer)) &&
        apart(pointer, bytes, (U)_gz_gpn_native_report, sizeof(_gz_gpn_native_report));
}

/* Private leased-image arithmetic only. No process/layer/resource pointer is
 * followed. The actual OIS owner already fully validated both AS6 images. */
static U cut_rows(const OisLease *lease, U *count)
{
    const GhcHeader *header;
    const GhcProcessDirectory *directory;
    U offset;
    if (!lease->archive || lease->bytes < sizeof(GhcHeader) ||
        lease->bytes > lease->capacity || lease->bytes > 0x40000000u) return 0;
    header = (const GhcHeader *)lease->archive;
    offset = header->reserved[0];
    if (header->valid != 1 || header->version != GHC_PROCESS_VERSION ||
        header->bytes != lease->bytes || offset > lease->bytes ||
        sizeof(*directory) > lease->bytes - offset) return 0;
    directory = (const GhcProcessDirectory *)(lease->archive + offset);
    if (directory->count > PO_CAP || directory->row_bytes != sizeof(GrcProcess) ||
        directory->bytes != sizeof(*directory) + directory->count * sizeof(GrcProcess) ||
        directory->bytes > lease->bytes - offset) return 0;
    *count = directory->count;
    return 1;
}

static U add_aligned(U *bytes, U addition)
{
    U aligned;
    if (*bytes > 0xffffffe0u) return 0;
    aligned = (*bytes + 31u) & ~31u;
    if (aligned > OIS_CUT_WORK_MAX) return 0;
    if (addition > OIS_CUT_WORK_MAX - aligned) return 0;
    *bytes = aligned + addition;
    return 1;
}

static U workspace_bytes(U saved, U current, U *capacity)
{
    U bytes = 0, rows;
    if (saved > PO_CAP || current > PO_CAP) return 0;
    rows = saved + current;
    if (!rows) rows = 1;
    if (!add_aligned(&bytes, sizeof(GpnWork)) ||
        !add_aligned(&bytes, sizeof(GrcPlan)) ||
        !add_aligned(&bytes, sizeof(GpnReport)) ||
        !add_aligned(&bytes, sizeof(GrcCutStamp)) ||
        !add_aligned(&bytes, rows * sizeof(U)) ||
        !add_aligned(&bytes, rows * sizeof(GrcPlanRow)) ||
        !add_aligned(&bytes, rows * sizeof(GpnAction))) return 0;
    *capacity = rows;
    return bytes;
}

static void *part(U base, U *offset, U bytes)
{
    U aligned = (*offset + 31u) & ~31u;
    *offset = aligned + bytes;
    return (void *)(base + aligned);
}

static U issuer_matches(const GhcSaveProducer *issuer, const OisCutPair *pair,
    const GrcCutStamp *current)
{
    return issuer->version == 2 && issuer->reserved == GHS_COMMAND_RESTORE &&
        issuer->request == pair->request && issuer->frame == pair->frame &&
        issuer->slot == pair->slot && current->image_type == GHC_PROCESS_VERSION &&
        current->image_version == GHC_PROCESS_VERSION &&
        current->archive_identity == pair->current.identity &&
        current->archive_generation == pair->current.generation &&
        current->image_bytes == pair->current.bytes && current->missing == 255 &&
        bytes_equal(issuer, &current->producer, sizeof(*issuer));
}

static void record_plan(const GpnReport *report)
{
    _gz_gpn_native_report[10] = report->status;
    _gz_gpn_native_report[11] = report->count;
    _gz_gpn_native_report[12] = report->rewind;
    _gz_gpn_native_report[13] = report->current_cleanup;
    _gz_gpn_native_report[14] = report->saved_reinstate;
    _gz_gpn_native_report[15] = report->reused;
    _gz_gpn_native_report[16] = report->audit_only;
    _gz_gpn_native_report[17] = report->requirements;
    _gz_gpn_native_report[18] = report->missing;
    _gz_gpn_native_report[19] = report->native_eligible;
    _gz_gpn_native_report[20] = report->replay_eligible;
}

static U retain_storage(void)
{
    _gz_gpn_native_report[3] = 3;
    _gz_gpn_native_report[21] = GHC_RETAINED;
    _gz_gpn_native_report[24] = 1;
    return GHC_RETAINED;
}

U _gz_gpn_current_compare(U request, U frame)
{
    U result = GPN_NATIVE_REFUSED, code, saved_rows, current_rows;
    U bytes, capacity, offset = 0, i, cleanup;
    GpnWork *work;
    GrcPlan *plan;
    GpnReport *report;
    GrcCutStamp *expected;
    U *sort;
    GrcPlanRow *rows;
    GpnAction *actions;
    GpnImage saved, current;
    GpnNativeControl *control = &gpn_native;

    if (!request || !frame) return GHC_RETAINED;
    if (!__sync_bool_compare_and_swap(&control->active, 0, 1)) return GHC_RETAINED;
    if (control->seen_request != request || control->seen_frame != frame) {
        for (i = 3; i < 32; ++i) _gz_gpn_native_report[i] = 0;
        _gz_gpn_native_report[3] = 1;
        _gz_gpn_native_report[4] = request;
        _gz_gpn_native_report[5] = frame;
        _gz_gpn_native_report[18] = 255;
    }
    /* The getter itself checks the real RELEASED callback and exact fixed
     * receiver before/after. It mints no new native hold. If identity has
     * expired, only the exact unleased OIS publication can be dropped. */
    if (_gz_ghs_completed_source(request, frame, &_gz_gpn_completed_issuer)) {
        cleanup = _gz_ois_cut_drop(request, frame);
        _gz_gpn_native_report[23] = cleanup;
        if (cleanup) return retain_storage();
        _gz_gpn_native_report[3] = 2;
        _gz_gpn_native_report[21] = GPN_NATIVE_REFUSED;
        _gz_gpn_native_report[22] = 1;
        control->active = 0;
        return GPN_NATIVE_REFUSED;
    }
    if (control->seen_request == request && control->seen_frame == frame) {
        control->active = 0;
        return control->last_result;
    }
    control->request = request;
    control->frame = frame;
    control->has_pair = control->has_work = 0;
    bytes_copy(&control->before, &_gz_gpn_completed_issuer, sizeof(control->before));
    code = _gz_ois_cut_acquire(request, frame, &control->pair);
    _gz_gpn_native_report[6] = code;
    if (code) {
        /* No descriptor is treated as a lease. Only an actual unleased
         * publication matching this exact request/frame may be dropped. */
        cleanup = _gz_ois_cut_drop(request, frame);
        _gz_gpn_native_report[23] = cleanup;
        if (cleanup) return retain_storage();
        goto completed;
    }
    control->has_pair = 1;
    if (_gz_ois_cut_validate(&control->pair) ||
        !cut_rows(&control->pair.saved, &saved_rows) ||
        !cut_rows(&control->pair.current, &current_rows)) goto release;
    bytes = workspace_bytes(saved_rows, current_rows, &capacity);
    if (!bytes) goto release;
    code = _gz_ois_cut_work_reserve(&control->pair, bytes, &control->work);
    _gz_gpn_native_report[7] = code;
    if (code == GHC_RETAINED) return retain_storage();
    if (code) goto release;
    control->has_work = 1;
    _gz_gpn_native_report[8] = bytes;
    _gz_gpn_native_report[9] = capacity;
    if (!_gz_ois_cut_work_match(&control->pair, &control->work, control->work.pointer, bytes))
        goto release;

    work = part(control->work.pointer, &offset, sizeof(*work));
    plan = part(control->work.pointer, &offset, sizeof(*plan));
    report = part(control->work.pointer, &offset, sizeof(*report));
    expected = part(control->work.pointer, &offset, sizeof(*expected));
    sort = part(control->work.pointer, &offset, capacity * sizeof(*sort));
    rows = part(control->work.pointer, &offset, capacity * sizeof(*rows));
    actions = part(control->work.pointer, &offset, capacity * sizeof(*actions));
    if (offset != bytes || !_gz_ois_cut_work_match(&control->pair, &control->work,
            control->work.pointer, offset)) goto release;

    saved.image = (const void *)control->pair.saved.archive;
    saved.bytes = control->pair.saved.bytes;
    saved.identity = control->pair.saved.identity;
    saved.generation = control->pair.saved.generation;
    current.image = (const void *)control->pair.current.archive;
    current.bytes = control->pair.current.bytes;
    current.identity = control->pair.current.identity;
    current.generation = control->pair.current.generation;
    code = grc_project_as8_private(current.image, current.bytes, current.identity,
        current.generation, &work->current_stamp, &work->current_view);
    if (code || !issuer_matches(&control->before, &control->pair, &work->current_stamp))
        goto release;
    /* Expected metadata is copied only after both real native owners have
     * joined the genuine issuer64 and actual OIS region identities. A host
     * supplied/self-projected stamp alone never reaches the comparison. */
    bytes_copy(expected, &work->current_stamp, sizeof(*expected));
    code = gpn_build(&saved, &current, expected, work, sort, capacity, plan,
        rows, actions, capacity, report);
    if (code != GPN_INCOMPLETE || report->valid != 1 || report->missing != 255 ||
        report->native_eligible || report->replay_eligible) goto release;
    record_plan(report);
    if (_gz_ghs_completed_source(request, frame, &_gz_gpn_completed_issuer) ||
        !bytes_equal(&control->before, &_gz_gpn_completed_issuer, sizeof(control->before)) ||
        !issuer_matches(&_gz_gpn_completed_issuer, &control->pair, expected) ||
        !_gz_ois_cut_work_match(&control->pair, &control->work, control->work.pointer, bytes) ||
        _gz_ois_cut_validate(&control->pair)) goto release;
    result = GPN_NATIVE_INCOMPLETE;

release:
    /* Copy/report work results before this call: release retires the actual
     * SDK workspace and CURRENT region. No workspace pointer is read later. */
    cleanup = _gz_ois_cut_release(&control->pair);
    _gz_gpn_native_report[23] = cleanup;
    if (cleanup) return retain_storage();
    control->has_pair = control->has_work = 0;
completed:
    _gz_gpn_native_report[3] = 2;
    _gz_gpn_native_report[21] = result;
    _gz_gpn_native_report[22] = 1;
    control->seen_request = request;
    control->seen_frame = frame;
    control->last_result = result;
    __sync_synchronize();
    control->active = 0;
    return result;
}
