#include "owned_heap_packed_storage_policy.h"
typedef char PackedSavedRole1[OIS_CUT_SAVED==1?1:-1];
typedef char PackedCurrentRole2[OIS_CUT_CURRENT==2?1:-1];
/* Real Save/Restore-purpose OIS storage. Native capture is authenticated by
 * the producer; current cut storage never overwrites the saved user slot. */
#include "owned_heap_native.h"
static struct {
    U phase, request, frame, thread, used, result, purpose;
    U role, target, image_version;
} ois_heap;
typedef char OisHeap40[sizeof(ois_heap) == 40 ? 1 : -1];

static U ois_heap_pending(void)
{
    return ois_heap.phase ? 1u : 0u;
}

U _gz_ois_heap_role(U request, U frame)
{
    /* Mirror only. Calling the producer here would recurse through scope. */
    return ois_heap.phase && ois_heap.purpose == GHS_VERSION &&
        ois_heap.request == request && ois_heap.frame == frame &&
        ois_heap.thread == (U)OSGetCurrentThread() ? ois_heap.role : 0;
}

static U ois_heap_scope(U request, U frame, U phase)
{
    if (ois_heap.purpose == GHC_VERSION)
        return _gz_gac_heap_scope(request, frame, phase);
    if (ois_heap.purpose == GHS_VERSION)
        return _gz_ghs_scope(request, frame, phase);
    return 0;
}

static U ois_heap_lock(U request, U frame, U phase)
{
    if (!ois_heap_scope(request, frame, phase) || !header()) return GHC_RETAINED;
    if (!__sync_bool_compare_and_swap(&state.entered, 0, 1)) return GHC_RETAINED;
    state.thread = (U)OSGetCurrentThread();
    if (!state.thread) {
        state.entered = 0;
        return GHC_RETAINED;
    }
    return OIS_OK;
}

U _gz_ois_heap_private_apart(U pointer, U bytes)
{
    return apart(pointer, bytes, (U)&ois_heap, sizeof(ois_heap)) &&
        ois_cut_external(pointer, bytes);
}

static U ois_heap_owner(U request, U frame)
{
    return ois_heap.phase && ois_heap.request == request && ois_heap.frame == frame &&
        ois_heap.thread == (U)OSGetCurrentThread() && header() &&
        region_ok(&state.staging) && state.staging.pointer && !state.tx.phase;
}

static U ois_heap_image_identity(void)
{
    const GhcHeader *header = (const GhcHeader *)state.staging.pointer;
    const GhcSaveProducer *producer;
    if (ois_heap.purpose != GHS_VERSION) return 1;
    if (ois_heap.used < 192 || ois_heap.used > state.staging.capacity ||
        header->bytes != ois_heap.used || header->request != ois_heap.request ||
        header->frame != ois_heap.frame || header->thread != ois_heap.thread)
        return 0;
    producer = (const GhcSaveProducer *)(state.staging.pointer + 128u);
    return ois_cut_staged_policy(producer, ois_heap.request, ois_heap.frame,
        ois_heap.thread, ois_heap.target, ois_heap.role);
}

static U ois_heap_abandon(U result)
{
    U cleanup;
    /* This is entered only under actual phase3 or pre-hold phase1. The
     * original allocator owns staging; the saved archive is never freed. */
    ois_held_invalidate();
    release_region(&state.staging);
    if (ois_heap.role == OIS_CUT_CURRENT) {
        cleanup = ois_cut_abort();
        if (cleanup) {
            ois_heap.phase = 4;
            return GHC_RETAINED;
        }
    }
    zero(&ois_heap, sizeof(ois_heap));
    return done(result);
}

U _gz_ois_heap_reserve(U request, U frame, U capacity)
{
    U result, purpose, role = OIS_CUT_SAVED, target = 9, image_version = GHC_VERSION;
    result = lock();
    if (result) return result;
    purpose = _gz_gac_heap_scope(request, frame, 1) ? GHC_VERSION :
        _gz_ghs_scope(request, frame, 1) ? GHS_VERSION : 0;
    if (purpose == GHS_VERSION) {
        role = _gz_ghs_role(request, frame);
        target = _gz_ghs_target(request, frame);
        image_version = _gz_ghs_image_version(request, frame);
        if (!ois_cut_reservation_policy(role, target, OIS_SLOTS,
                image_version, GHC_PROCESS_VERSION))
            return done(GHC_CONTEXT);
        --target;
    }
    if (ois_heap.phase || !purpose || state.tx.phase || state.staging.pointer ||
        !slot_ok(target)) return done(OIS_BUSY);
    if (role == OIS_CUT_SAVED && state.slots[target].readers) return done(OIS_BUSY);
    if (role == OIS_CUT_SAVED && state.slots[target].generation == 0xffffffffu)
        return done(OIS_EXHAUSTED);

    ois_heap.phase = 1;
    ois_heap.request = request;
    ois_heap.frame = frame;
    ois_heap.purpose = purpose;
    ois_heap.thread = state.thread;
    ois_heap.role = role;
    ois_heap.target = target;
    ois_heap.image_version = image_version;
    if (role == OIS_CUT_CURRENT) {
        result = ois_cut_begin(request, frame, target);
        if (result) {
            if (result == GHC_RETAINED) {
                ois_heap.phase = 4;
                return GHC_RETAINED;
            }
            zero(&ois_heap, sizeof(ois_heap));
            return done(result);
        }
    }
    if (purpose == GHS_VERSION && image_version == GHC_PROCESS_VERSION) {
        void *heap = MEMGetBaseHeapHandle(1);
        U largest = heap ? MEMGetAllocatableSizeForExpHeapEx(heap, 32) : 0;
        capacity = ghc_packed_staging_budget(capacity, largest, role, OIS_CUT_WORK_MAX);
        if (capacity < 192) return ois_heap_abandon(OIS_SPACE);
    }
    result = allocate_region(&state.staging, capacity);
    if (result) {
        if (retained_conflict) {
            /* The rejected address may belong to somebody else. Original
             * fail-retained policy and saved read ownership both remain. */
            ois_heap.phase = 4;
            return GHC_RETAINED;
        }
        return ois_heap_abandon(result);
    }
    zero((void *)state.staging.pointer, 128);
    _gz_ghc_report[8] = state.staging.pointer;
    _gz_ghc_report[9] = state.staging.capacity;
    return done(OIS_OK);
}

U _gz_ois_heap_stage_match(U request, U frame, U pointer, U bytes)
{
    return ois_heap_owner(request, frame) && ois_heap.phase == 2 &&
        state.entered == 1 && state.writer == 4 &&
        state.thread == ois_heap.thread && ois_heap_scope(request, frame, 2) &&
        contains(state.staging.pointer, state.staging.capacity, pointer, bytes);
}

U _gz_ois_heap_apart(U request, U frame, U pointer, U bytes)
{
    const Region *regions[OIS_SLOTS + 5];
    U i, j;
    if (!_gz_ois_heap_stage_match(request, frame, state.staging.pointer,
            state.staging.capacity) || !span(pointer, bytes) ||
        !_gz_ghc_all_private_apart(pointer, bytes) ||
        !apart(pointer, bytes, (U)&state, sizeof(state))) return 0;
    for (i = 0; i < OIS_SLOTS; ++i) {
        if (!slot_ok(i)) return 0;
        regions[i] = &state.slots[i].region;
    }
    regions[OIS_SLOTS] = &state.staging;
    regions[OIS_SLOTS + 1] = &state.tx.undo;
    regions[OIS_SLOTS + 2] = &state.tx.map;
    regions[OIS_SLOTS + 3] = ois_cut_region();
    regions[OIS_SLOTS + 4] = ois_cut_work_region();
    for (i = 0; i < OIS_SLOTS + 5; ++i) {
        const Region *region = regions[i];
        if (!region_ok(region)) return 0;
        if (!region->pointer) continue;
        if (!span(region->heap, 0x54) || !apart(pointer, bytes, region->heap, 0x54) ||
            region->pointer < 0x10000040u || region->capacity > 0xffffffffu - 64 ||
            !apart(pointer, bytes, region->pointer - 64, region->capacity + 64)) return 0;
        for (j = 0; j < i; ++j) {
            if (regions[j]->pointer && !apart(region->pointer, region->capacity,
                    regions[j]->pointer, regions[j]->capacity)) return 0;
        }
    }
    return 1;
}

U _gz_ois_heap_read(U request, U frame)
{
    U result, written = 0;
    result = ois_heap_lock(request, frame, 2);
    if (result) return result;
    if (!ois_heap_owner(request, frame) || ois_heap.phase != 1 ||
        !ois_heap_scope(request, frame, 2)) return done(GHC_CONTEXT);
    ois_heap.phase = 2;
    state.writer = 4;
    result = _gz_ghc_capture(request, frame, state.staging.pointer,
        state.staging.capacity, &written);
    ois_heap.used = result ? 0 : written;
    ois_heap.result = result;
    ois_heap.phase = 3;
    return done(result);
}

static U ois_heap_compact(void)
{
    Region tight = {0}, old;
    U result, needed = (ois_heap.used + 31u) & ~31u;
    if (!needed || needed > state.staging.capacity) return OIS_CORRUPT;
    if (needed == state.staging.capacity) return OIS_OK;
    result = allocate_region(&tight, needed);
    if (result) return retained_conflict ? GHC_RETAINED : result;
    copy((void *)tight.pointer, (const void *)state.staging.pointer, ois_heap.used);
    result = _gz_ghc_validate((const void *)tight.pointer, ois_heap.used, 0);
    if (result) { release_region(&tight); return result; }
    copy(&old, &state.staging, sizeof(old));
    copy(&state.staging, &tight, sizeof(tight));
    zero(&tight, sizeof(tight));
    release_region(&old);
    return OIS_OK;
}

U _gz_ois_heap_finish(U request, U frame, U success)
{
    U result;
    Slot old, *target;
    if (!ois_heap.phase) return state.staging.pointer ? GHC_RETAINED : OIS_OK;
    if (!ois_heap_scope(request, frame, 3)) return GHC_RETAINED;
    result = ois_heap_lock(request, frame, 3);
    if (result) return result;
    if (!ois_heap_owner(request, frame) || ois_heap.phase == 2 || ois_heap.phase == 4)
        return GHC_RETAINED;
    target = state.slots + ois_heap.target;
    if (!slot_ok(ois_heap.target)) return GHC_RETAINED;
    if (ois_heap.role == OIS_CUT_SAVED &&
        (target->readers || target->generation == 0xffffffffu)) return GHC_RETAINED;
    if (success && !ois_heap.result && ois_heap.used) {
        result = ois_heap_image_identity() &&
            ((const GhcHeader *)state.staging.pointer)->version == ois_heap.image_version ?
            _gz_ghc_validate((const void *)state.staging.pointer, ois_heap.used, 0) : GHC_FORMAT;
        if (result) {
            success = 0;
            ois_heap.result = result;
        }
    } else success = 0;
    if (!success) return ois_heap_abandon(ois_heap.result);

    if (ois_heap.purpose == GHS_VERSION && ois_heap.image_version == GHC_PROCESS_VERSION &&
        ois_heap.role == OIS_CUT_SAVED) {
        result = ois_heap_compact();
        if (result == GHC_RETAINED) { ois_heap.phase = 4; return result; }
        if (result) return ois_heap_abandon(result);
    }
    ois_held_invalidate();
    ((GhcHeader *)state.staging.pointer)->valid = 1;
    __sync_synchronize();
    result = _gz_ghc_validate((const void *)state.staging.pointer, ois_heap.used, 1);
    if (result) return ois_heap_abandon(result);
    if (ois_heap.role == OIS_CUT_CURRENT) {
        result = ois_cut_publish(request, frame, ois_heap.used);
        if (result == GHC_RETAINED) {
            ois_heap.phase = 4;
            return result;
        }
        if (result) return ois_heap_abandon(result);
        _gz_ghc_report[23] = ois_cut.current.pointer;
        _gz_ghc_report[24] = ois_cut.current.capacity;
        _gz_ghc_report[25] = ois_cut.used;
        _gz_ghc_report[26] = ois_cut.current.identity;
        _gz_ghc_report[27] = ois_cut.generation;
        _gz_ghc_report[28] = GHC_PROCESS_VERSION;
    } else {
        copy(&old, target, sizeof(old));
        copy(&target->region, &state.staging, sizeof(target->region));
        zero(&state.staging, sizeof(state.staging));
        target->used = ois_heap.used;
        target->type = ois_heap.purpose == GHS_VERSION ? GHC_PROCESS_VERSION : GHC_VERSION;
        ++target->generation;
        zero(&target->domain, sizeof(target->domain));
        _gz_ghc_report[23] = target->region.pointer;
        _gz_ghc_report[24] = target->region.capacity;
        _gz_ghc_report[25] = target->used;
        _gz_ghc_report[26] = target->region.identity;
        _gz_ghc_report[27] = target->generation;
        _gz_ghc_report[28] = target->type;
        release_region(&old.region);
    }
    zero(&ois_heap, sizeof(ois_heap));
    result = done(OIS_OK);
    __sync_synchronize();
    _gz_ghc_report[29] = 1;
    return result;
}
