/* Same actual lifetime TU and locks. Only the CURRENT capture qualifies
 * membership; saved row indices and generations remain historical data.
 * No actor-profile list, native Create/Delete, allocator or result stub.
 */
#include "metadata_slot_plan.h"

typedef struct {
    HmpSlot plan;
    HlHeap heap;
    HmScope scope;
    HlCustomMeta custom;
    HiHeap links;
} HmdHeap;

typedef struct { HmpSlot plan; HlStoredAllocation allocation; HiLink link; } HmdAllocation;
typedef struct { HmpSlot plan; PoEntry process; } HmdProcess;

typedef struct {
    u32 magic, bytes, version, valid, request, frame, hg, job;
    u32 saved_identity, current_identity, hl_serial, po_serial;
    u32 saved_heaps, current_heaps, heap_count, heap_offset;
    u32 saved_allocations, current_allocations, allocation_count, allocation_offset;
    u32 saved_processes, current_processes, process_count, process_offset;
    u32 classes_offset, classes_bytes, checksum, missing, reserved[4];
    HlControl hl;
    HiControl hi;
    HmControl hm;
    PoHeader po;
} HmdHeader;

typedef char HmdHeap144[sizeof(HmdHeap) == 144 ? 1 : -1];
typedef char HmdAllocation84[sizeof(HmdAllocation) == 84 ? 1 : -1];
typedef char HmdProcess76[sizeof(HmdProcess) == 76 ? 1 : -1];
typedef char HmdHeader384[sizeof(HmdHeader) == 384 ? 1 : -1];

extern u32 _gz_ois_held_work(u32, u32, const OisHeldView *, u32);

GHC_FN u32 hmd_add(u32 *at, u32 count, u32 stride) {
    if (*at > 0x3fffffe0u || count > (0x3fffffe0u - *at) / stride) return 0;
    *at = (*at + count * stride + 31u) & ~31u;
    return 1;
}

GHC_FN u32 hmd_hash(const HmdHeader *header) {
    const unsigned char *p = (const unsigned char *)header;
    u32 i, hash = 2166136261u;
    for (i = 0; i < header->bytes; ++i) {
        u32 byte = ((i >= 12 && i < 16) || (i >= 104 && i < 108)) ? 0 : p[i];
        hash = (hash ^ byte) * 16777619u;
    }
    return hash;
}

GHC_FN const GhcAllocationDirectory *hmd_allocations(const GhcHeader *image) {
    const GhcProcessDirectory *process = (const GhcProcessDirectory *)((const unsigned char *)image + image->reserved[0]);
    return (const GhcAllocationDirectory *)((const unsigned char *)image + ghc_allocation_offset(image->count, process->count));
}

GHC_FN u32 hmd_union(u32 a,u32 b,u32 limit) {
    return a>limit||b>limit?0:(a>limit-b?limit:a+b);
}
GHC_FN u32 hmd_max_sources(const HmdHeader*h) {
    u32 n=h->saved_heaps>h->saved_allocations?h->saved_heaps:h->saved_allocations;
    return n>h->saved_processes?n:h->saved_processes;
}
GHC_FN u32 hmd_zero_row(const void*row,u32 bytes) {
    const u32*p=row;u32 i;for(i=0;i<bytes/4;i++)if(p[i])return 0;return 1;
}

GHC_FN u32 hmd_shape(const GhcHeader *saved, const GhcHeader *current, HmdHeader *header) {
    const GhcAllocationDirectory *sa = hmd_allocations(saved), *ca = hmd_allocations(current);
    const GhcProcessDirectory *sp = (const GhcProcessDirectory *)((const unsigned char *)saved + saved->reserved[0]);
    const GhcProcessDirectory *cp = (const GhcProcessDirectory *)((const unsigned char *)current + current->reserved[0]);
    u32 at = sizeof(*header);
    zero(header, sizeof(*header));
    header->magic = 0x484d4431u; header->version = 2; header->missing = 255;
    header->saved_heaps = saved->count; header->current_heaps = current->count;
    header->heap_count = hmd_union(saved->count, current->count, HL_HEAPS);
    header->heap_offset = at;
    if (!hmd_add(&at, header->heap_count, sizeof(HmdHeap))) return 0;
    header->saved_allocations = sa->count; header->current_allocations = ca->count;
    header->allocation_count = hmd_union(sa->count, ca->count, HL_ALLOCS);
    header->allocation_offset = at;
    if (!hmd_add(&at, header->allocation_count, sizeof(HmdAllocation))) return 0;
    header->saved_processes = sp->bound; header->current_processes = cp->bound;
    header->process_count = hmd_union(sp->bound, cp->bound, PO_CAP);
    header->process_offset = at;
    if (!hmd_add(&at, header->process_count, sizeof(HmdProcess))) return 0;
    header->classes_offset = at;
    header->classes_bytes = HL_ALLOCS * 5u + hmd_max_sources(header) * 4u + sp->bound * 4u;
    if (!hmd_add(&at, header->classes_bytes, 1)) return 0;
    header->bytes = at;
    return 1;
}

GHC_FN u32 hmd_before_equal(const HmdHeader *header) {
    const unsigned char *p = (const unsigned char *)header;
    const HmdHeap *heaps = (const HmdHeap *)(p + header->heap_offset);
    const HmdAllocation *allocations = (const HmdAllocation *)(p + header->allocation_offset);
    const HmdProcess *processes = (const HmdProcess *)(p + header->process_offset);
    u32 i, id;
    HlControl expected_hl;
    PoHeader expected_po;
    ghc_copy(&expected_hl, &header->hl, sizeof(expected_hl));
    ghc_copy(&expected_po, &header->po, sizeof(expected_po));
    if (header->reserved[0]) {
        if (header->reserved[0] != 2 || header->saved_heaps > 0xffffffffu - header->hl.serial ||
            header->saved_allocations > 0xffffffffu - header->hl.serial - header->saved_heaps ||
            header->saved_heaps > 0xffffffffu - header->hl.release_serial ||
            header->saved_processes > 0xffffffffu - header->po.serial ||
            header->reserved[1] != header->hl.serial + header->saved_heaps + header->saved_allocations ||
            header->reserved[2] != header->hl.release_serial + header->saved_heaps ||
            header->reserved[3] != header->po.serial + header->saved_processes) return 0;
        expected_hl.serial = header->reserved[1]; expected_hl.release_serial = header->reserved[2];
        expected_po.serial = header->reserved[3];
    }
    if (!ghc_equal(&expected_hl, &_gz_hl_control, sizeof(expected_hl)) ||
        !ghc_equal(&header->hi, &_gz_hi_control, sizeof(header->hi)) ||
        !ghc_equal(&header->hm, &_gz_hm_control, sizeof(header->hm)) ||
        !ghc_equal(&expected_po, &_gz_po_header, sizeof(expected_po))) return 0;
    for (i = 0; i < header->heap_count; ++i) {
        id = heaps[i].plan.index;
        if (!id) { if (!hmd_zero_row(heaps+i, sizeof(HmdHeap))) return 0; continue; }
        if (!id || id > HL_HEAPS ||
            !ghc_equal(&heaps[i].heap, _gz_hl_heaps + id - 1, sizeof(HlHeap)) ||
            !ghc_equal(&heaps[i].scope, _gz_hm_scopes + id - 1, sizeof(HmScope)) ||
            !ghc_equal(&heaps[i].custom, _gz_hl_custom_meta + id - 1, sizeof(HlCustomMeta)) ||
            !ghc_equal(&heaps[i].links, _gz_hi_heaps + id - 1, sizeof(HiHeap))) return 0;
    }
    for (i = 0; i < header->allocation_count; ++i) {
        id = allocations[i].plan.index;
        if (!id) { if (!hmd_zero_row(allocations+i, sizeof(HmdAllocation))) return 0; continue; }
        if (!id || id > HL_ALLOCS ||
            !ghc_equal(&allocations[i].allocation, _gz_hl_allocations + id - 1, sizeof(HlStoredAllocation)) ||
            !ghc_equal(&allocations[i].link, _gz_hi_links + id - 1, sizeof(HiLink))) return 0;
    }
    for (i = 0; i < header->process_count; ++i) {
        id = processes[i].plan.index;
        if (!id) { if (!hmd_zero_row(processes+i, sizeof(HmdProcess))) return 0; continue; }
        if (!id || id > PO_CAP || !ghc_equal(&processes[i].process, _gz_po_rows + id - 1, sizeof(PoEntry))) return 0;
    }
    return 1;
}

#include "owned_metadata_hash_journal.inc"

GHC_API u32 _gz_hmp_plan_held(u32 request, u32 frame) {
    const OisHeldView *view = &_gz_ois_held_record;
    const GhcHeader *saved, *current;
    HmdHeader shape, *header;
    u32 pointer, result;
    _gz_ghw_report[25] = HMP_SCOPE;
    if (!_gz_ghw_scope(request, frame, GHW_PREPARE) ||
        !_gz_ois_held_owner(request, frame, view) || _gz_ois_held_validate(request, frame, view)) return HMP_SCOPE;
    saved = (const GhcHeader *)view->saved.archive; current = (const GhcHeader *)view->current.archive;
    if (!hmd_shape(saved, current, &shape)) return HMP_SPACE;
    pointer = _gz_ois_held_work(request, frame, view, shape.bytes);
    if (!pointer) return HMP_SPACE;
    header = (HmdHeader *)pointer; zero(header, shape.bytes); ghc_copy(header, &shape, sizeof(shape));
    header->request = request; header->frame = frame; header->hg = view->hg; header->job = view->job_generation;
    header->saved_identity = view->saved.identity; header->current_identity = view->current.identity;
    if (!ghp_po_lock()) return HMP_CHANGED;
    if (!ghc_hl_lock()) { ghp_po_unlock(); return HMP_CHANGED; }
    result = hmd_build_locked(saved, current, header);
    ghc_hl_unlock(); ghp_po_unlock();
    _gz_ghw_report[25] = result;
    if (result || !_gz_ghw_scope(request, frame, GHW_PREPARE) ||
        !_gz_ois_held_owner(request, frame, view)) return result ? result : HMP_SCOPE;
    header->checksum = hmd_hash(header); __sync_synchronize(); header->valid = 1;
    _gz_ghw_report[26] = header->saved_heaps; _gz_ghw_report[27] = header->saved_allocations;
    _gz_ghw_report[28] = header->saved_processes; _gz_ghw_report[29] = header->bytes;
    return HMP_OK;
}

GHC_API u32 _gz_hmp_verify_held(u32 request, u32 frame) {
    const OisHeldView *view = &_gz_ois_held_record;
    HmdHeader shape, *header;
    u32 pointer, result;
    if (!_gz_ghw_scope(request, frame, GHW_INVERSE) || !_gz_ois_held_owner(request, frame, view)) return HMP_SCOPE;
    if (!hmd_shape((const GhcHeader *)view->saved.archive, (const GhcHeader *)view->current.archive, &shape)) return HMP_SPACE;
    pointer = _gz_ois_held_work(request, frame, view, shape.bytes);
    if (!pointer) return HMP_SPACE;
    header = (HmdHeader *)pointer;
    if (header->magic != shape.magic || header->bytes != shape.bytes || header->version != 2 ||
        header->valid != 1 || header->request != request || header->frame != frame ||
        header->hg != view->hg || header->job != view->job_generation || header->missing != 255 ||
        header->saved_identity != view->saved.identity || header->current_identity != view->current.identity ||
        !ghc_equal(&header->saved_heaps, &shape.saved_heaps, 14 * sizeof(u32)) ||
        header->checksum != hmd_hash(header)) return HMP_FORMAT;
    if (!ghp_po_lock()) return HMP_CHANGED;
    if (!ghc_hl_lock()) { ghp_po_unlock(); return HMP_CHANGED; }
    result = hmd_before_equal(header) ? HMP_OK : HMP_CHANGED;
    ghc_hl_unlock(); ghp_po_unlock();
    return result;
}
